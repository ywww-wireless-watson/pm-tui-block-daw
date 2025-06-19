CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lncurses -lm -lasound -lsndfile
TARGET = pm-tui-block-daw

SRCS = \
  src/main.c \
  src/ncurses_init.c \
  src/matrix_input.c \
  src/midi_writer.c \
  src/wav_player.c \
  src/mtw.c \
  src/input_window.c \
  src/edit_music.c \
  src/edit_synth.c \
  src/edit_project.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
