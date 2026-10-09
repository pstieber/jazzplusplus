# AGENTS.md

## Project

Jazz++ is a cross-platform MIDI sequencer written in C++ using wxWidgets. Imported from SourceForge. Version 5.0.0.

## Build Systems

Two build systems are available:

**Autotools (primary):**
```bash
./bootstrap
mkdir build && cd build
/path/to/configure --prefix=/install/path --enable-alsa --enable-sequencer2
make
make install
```

**CMake (alternative):**
```bash
mkdir build && cd build
cmake .. -DENABLE_ALSA=ON
make
make install
```

## Dependencies

- **wxWidgets** >= 2.9.4 (required, with `--enable-std_string --enable-stl --with-gtk`)
- **ALSA** (optional, Linux audio)
- **PortMidi** (bundled in `portmidi/`, built automatically)

## Architecture

- `src/` — Main application (~100 C++ source files). Entry point: `JazzPlusPlusApplication.cpp`
- `src/Dialogs/` — UI dialog classes
- `src/DeprecatedWx/` — Legacy wxWidgets property code
- `src/mswin/` — Windows-specific code (not built on Linux)
- `portmidi/` — Bundled PortMidi library (C)
- `conf/` — Synthesizer config files (`.jzi`, `.cfg`) installed to `share/Jazz++/conf/`
- `song/` — Demo songs and audio files

## Build Options (CMake)

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_ALSA` | `ON` | ALSA audio support (Linux) |
| `ENABLE_SEQUENCER2` | `OFF` | `/dev/sequencer2` support |
| `ENABLE_MPU401` | `OFF` | MPU-401 support |
| `ENABLE_DEBUG` | `OFF` | Debug symbols |
| `ENABLE_OPTIMIZE` | `OFF` | Optimization flags |

## Key Conventions

- C++11 standard
- Conditional compilation: `DEV_ALSA`, `DEV_SEQUENCER2`, `DEV_MPU401` macros
- `JAZZ_DATADIR` macro defines the data directory path (set at compile time)
- No test suite, no CI, no linter/formatter configuration
- wxWidgets must be built with `--enable-std_string --enable-stl` (not the default unicode build)
