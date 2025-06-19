# PM TUI Block DAW

A TUI (text user interface) block-based music workstation for designing, sequencing, and exporting phase-modulated waveforms as MIDI and WAV. Compose music by arranging custom wave blocks, edit parameters interactively, and export your creations.

## Features
- Block/step-based music composition with a modular workflow
- Phase-modulated waveform design using custom mathematical formulas
- Multiple editors: Wave, Merge, Synth, Project
- MIDI and WAV export
- TUI interface (ncurses-based)
- Project save/load, log window, and more

## Build

**Requirements:**
- gcc
- ncurses
- libsndfile
- alsa-lib

**To build:**
```sh
make
```

## Usage

**To start:**
```sh
./pm-tui-block-daw
```
or
```sh
make run
```

## Editors & Workflow

PM TUI Block DAW consists of four integrated editors. Use the following keys to switch:
- `o` - Wave Editor
- `i` - Merge Editor
- `u` - Synth Editor
- `y` - Project Editor

**General Controls:**
- Move cursor: Arrow keys ↑ ↓ ← →
- Input values: Hex keys (0-9, a-f)
- Quit: Press `q` from the Project Editor

### Wave Editor
Design phase-modulated waveforms using the formula: `sin(a1 + b1 * t + c * sin(a2 + b2 * t))`.
- Adjust parameters (a1, b1, c, a2, b2, steps, notes) using hex values (0-F) in the matrix.
- Each row controls a part of the formula; each step is a note (MIDI number, e.g., 60=C4).
- Controls:
  - `p` - Preview/play the current wave
  - `s` - Save wave block as .wblk file
  - `l` - Load wave block from .wblk file
  - `t` - Configure time step (note duration)

### Merge Editor
Arrange and sequence multiple wave blocks for a complete track.
- Each row (ch) is an instrument/voice; columns are steps in the sequence.
- Enter wave block numbers in the grid; 0 = silence.
- Controls:
  - `p` - Preview the entire merged sequence
  - `q` - Return to Wave Editor
  - `s` - Save the merge configuration to merge.txt
  - `l` - Load a merge configuration from merge.txt
  - `x` - Preview current column only (single step)
  - `z` - Resize matrix (change rows/columns in the merge grid)

### Synth Editor
Configure synthesizer parameters for each channel (instrument/voice).
- Set wave type, volume, attack, decay, sustain, release, pan, and name for each channel.
- Controls:
  - `w` - Write parameters to the selected channel

### Project Editor
Save, load, and manage your project files.
- Projects are saved as directories named `prj_[name]` containing all relevant files.
- Controls:
  - `s` - Save project
  - `l` - Load project
  - `q` - Quit application

### Typical Workflow
1. **Design waveforms** in the Wave Editor (`o`).
2. **Arrange blocks** in the Merge Editor (`i`).
3. **Configure synths** in the Synth Editor (`u`).
4. **Save/load projects** in the Project Editor (`y`).
5. **Export** as MIDI or WAV from the Wave or Merge Editor.

## Log Window
A log window is displayed on the right side of the screen, showing status messages and error notifications.

## File Formats

The PM TUI Block DAW uses several file types:

- **Block Files**:
  - **Wave Block Files (*.wblk)**: Binary files that store waveform data created in the Wave Editor
  - **Wave Block Specification Files (*.wblk.wbsp)**: Configuration data for wave blocks
- **Output Files**:
  - **MIDI Files (*.mid)**: Generated MIDI output that can be played by external applications
  - **WAV Files (*.wav)**: Generated audio output in WAV format
- **MIDI Data Files (*.mdat)**: Additional MIDI-related data files for project processing
- **Configuration Files**:
  - **preview.txt**: Contains instrument/synth parameters with format: `[type] [volume] [attack] [decay] [sustain] [release] [pan] [name]`
  - **merge.txt**: Block sequencing configuration with arrangement parameters
  - **supplement.txt**: Contains the BPM value for the project
  - **project.txt**: Lists all files included when a project is saved
- **Project Directories**:  
  Each project is stored in a directory named `prj_<project_name>`, containing all relevant files for that project.

## Example Project

An example project is included in the repository to help you get started.

### How to Load the Example Project

1. **Start the application:**
   ```sh
   ./pm-tui-block-daw
   ```
2. **Open the Project Editor:**  
   Press `y` to switch to the Project Editor window.
3. **Load the Example Project:**  
   Press `l` (lowercase L) to load a project.  
   When prompted, enter the example project name (`example`) and press `Enter`.
   - The project will be loaded, and the working directory will change to the example project folder (`prj_example`).
4. **Explore the Editors:**  
   - See the guide above for how to use and switch the editors.

## Troubleshooting

- If you encounter errors, check the log window for details
- Ensure all required libraries are properly installed
- The application requires a terminal that supports ncurses

## Contributing

Contributions, bug reports, and feature requests are welcome! Please open an issue or pull request on GitHub.

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.
