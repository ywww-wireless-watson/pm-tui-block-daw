#include "edit_project.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 8192

void copy_file(const char *src_path, const char *dest_path)
{
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0)
    {
        perror("Error opening source file");
        return;
    }

    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0)
    {
        perror("Error opening destination file");
        close(src_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
        if (write(dest_fd, buffer, bytes_read) != bytes_read)
        {
            perror("Error writing to destination file");
            break;
        }
    }

    if (bytes_read < 0)
    {
        perror("Error reading source file");
    }

    close(src_fd);
    close(dest_fd);
}

void initialize_project_matrix(EditorData *data)
{
    data->matrix = NULL;
    data->result = NULL;
}

void cleanup_project_editor(EditorData *data)
{
    (void)data; // suppress unused parameter warning
    // No need to free anything as the matrix and result are NULL
}

void init_project_editor(EditorData *project_data)
{
    project_data->rows = 0;
    project_data->cols = 0;
    initialize_project_matrix(project_data);

    project_data->current_row = 0;
    project_data->current_col = 0;
}

void handle_project_editor(EditorData *project_data, bool project_rewrite)
{
    handle_input(project_data->matrix, project_data->rows, project_data->cols, &project_data->current_row, &project_data->current_col, &project_data->flag);
    if (project_rewrite)
    {
        project_data->flag = 0;
    }
    switch (project_data->flag)
    {
    case 's':
    {
        char *dirname = input_window("SAVE", "input project name", "");
        if (dirname)
        {
            char prj_dirname[256];
            snprintf(prj_dirname, sizeof(prj_dirname), "prj_%s", dirname);
            free(dirname);

            char exe_path[256];
            ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
            if (len <= 0 || (size_t)len >= sizeof(exe_path))
            {
                log_message("Error reading executable path or path too long.");
                perror("readlink");
                break;
            }
            exe_path[len] = '\0';

            char *last_slash = strrchr(exe_path, '/');
            if (last_slash)
            {
                *last_slash = '\0';
            }

            char prj_fullpath[512];
            snprintf(prj_fullpath, sizeof(prj_fullpath), "%s/%s", exe_path, prj_dirname);

            if (mkdir(prj_fullpath, 0777) == -1 && errno != EEXIST)
            {
                log_message("Error creating directory.");
                perror("mkdir");
                break;
            }

            char *cwd = getcwd(NULL, 0);
            if (!cwd)
            {
                log_message("Error getting current directory.");
                perror("getcwd");
                break;
            }

            char filename[768];
            snprintf(filename, sizeof(filename), "%s/project.txt", prj_fullpath);
            FILE *file = fopen(filename, "w");
            if (!file)
            {
                log_message("Error saving project.");
                perror("fopen");
                free(cwd);
                break;
            }

            DIR *dir = opendir(cwd);
            if (!dir)
            {
                perror("opendir failed");
                fclose(file);
                free(cwd);
                break;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_REG)
                { // Only regular files
                    const char *ext = strrchr(entry->d_name, '.');
                    if (ext && (strcmp(ext, ".wblk") == 0 || strcmp(ext, ".mdat") == 0 || strcmp(ext, ".txt") == 0))
                    {
                        char src_path[512];
                        char dest_path[512];

                        snprintf(src_path, sizeof(src_path), "%s/%s", cwd, entry->d_name);
                        int ret = snprintf(dest_path, sizeof(dest_path), "%s/%s", prj_fullpath, entry->d_name);
                        if (ret < 0 || (size_t)ret >= sizeof(dest_path))
                        {
                            log_message("Destination path too long.");
                            continue;
                        }

                        copy_file(src_path, dest_path);
                        fprintf(file, "Copied: %s\n", entry->d_name);
                    }
                }
            }

            closedir(dir);
            fclose(file);
            free(cwd);

            log_message("Project saved and files copied.");
        }
        else
        {
            log_message("Canceled saving project.");
        }
    }
    break;
    case 'l':
    {
        // Move to the directory corresponding to the entered project name
        char *dirname = input_window("LOAD", "input project name", "");
        if (dirname)
        {
            char prj_dirname[256];
            snprintf(prj_dirname, sizeof(prj_dirname), "prj_%s", dirname);
            free(dirname);

            char exe_path[256];
            ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
            if (len <= 0 || (size_t)len >= sizeof(exe_path))
            {
                log_message("Error reading executable path or path too long.");
                perror("readlink");
                break;
            }
            exe_path[len] = '\0';

            char *last_slash = strrchr(exe_path, '/');
            if (last_slash)
            {
                *last_slash = '\0';
            }

            char prj_fullpath[512];
            snprintf(prj_fullpath, sizeof(prj_fullpath), "%s/%s", exe_path, prj_dirname);

            if (chdir(prj_fullpath) == -1)
            {
                log_message("Error changing directory.");
                perror("chdir");
                break;
            }

            log_message("Project loaded and directory changed.");
        }
        else
        {
            log_message("Canceled loading project.");
        }
    }
    break;
    case 0:
        WINDOW *win = get_main_window();
        WINDOW *log_win = get_log_window();
        wclear(win);
        // Display the current directory name
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            log_message("Error getting current directory.");
            perror("getcwd");
            break;
        }
        mvwprintw(win, 2, 0, "Current Directory: %s", cwd);
        // ./project.txtの内容を表示
        char project_file_path[512];
        snprintf(project_file_path, sizeof(project_file_path), "%s/project.txt", cwd);
        FILE *file = fopen(project_file_path, "r");
        if (file)
        {
            char line[256];
            const int offset = 6;
            int i = offset;
            while (fgets(line, sizeof(line), file))
            {
                line[strcspn(line, "\r\n")] = '\0'; // Remove newline character
                mvwprintw(win, i, 0, "%s", line);
                i++;
            }
            fclose(file);
        }
        else
        {
            log_message("Error loading project file.");
            perror("fopen");
        }
        display_guide();
        display_label(3, 0);
        wrefresh(win);
        display_log_messages();
        wrefresh(log_win);
        break;
    default:
        break;
    }
}
