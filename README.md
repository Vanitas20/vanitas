<div align="center">
  <img src="./assets/vanitas-logo.png" alt="The Vanitas" width="400">
  
  <h3><i>A small “book” for cursed logs</i></h3>
  <h3><i>"—Non! I am a doctor. One who specializes in Terminals. I came… to heal outputs!"</i></h3>

  <p>
    Vanitas is a C++ terminal log analyzer
  </p>

  <p>
    <a href="./LICENSE">
      <img src="https://img.shields.io/badge/LICENSE-MIT-black?style=for-the-badge" alt="License">
    </a>
  </p>

  <p>
    <a href="https://en.cppreference.com/w/cpp/23">
      <img src="https://img.shields.io/badge/C++-23-blue?logo=c%2B%2B&logoColor=white" alt="C++23">
    </a>
    <a href="https://github.com/cpm-cmake/CPM.cmake">
      <img src="https://img.shields.io/badge/CPM-Enabled-green?logo=cmake%2B%2B&logoColor=white" alt="CPM">
    </a>
    <a href="https://cmake.org/">
      <img src="https://img.shields.io/badge/Build-CMake-orange?logo=cmake&logoColor=white" alt="CMake">
    </a>
    <a href="https://github.com/ToruNiina/toml11">
      <img src="https://img.shields.io/badge/toml11-Enabled-purple?style=flat&logo=toml&Color=white" alt="toml11">
    </a>
  </p>
</div>

## Prologue

Some logs arrive as clean reports.
Most come in as case files: noisy, distorted, and full of false leads — color codes, progress lines that overwrite themselves, and stack traces scattered across the scene.

Vanitas was made for investigations like these.
When one real failure is buried under thousands of lines, it collects the evidence, restores the timeline, and turns the chaos into a readable record.

Open the file.
Follow the trail.
Extract the truth — and heal the terminal output.

## The Case File

Vanitas is a terminal log analyzer (CLI now, TUI soon) that:

- Removes terminal “illusions” (ANSI escape codes, control sequences).
- Understands overwritten progress lines (`\r`) so they don’t turn into nonsense.
- Groups multi-line failures (stack traces, nested errors) into one readable **case**.
- Separates the story into: `Errors`, `Warnings`, `Tests`, and useful `Info`.

## Build the book

To use all power the book of Vanitas, you should make a contract with vampire of the Blue Moon.

### Arch linux:
```bash
cmake -B build -G Ninja .
ninja -C build
```

## How to use the book

### Help

Show help and list of commands:
```bash
./build/vanitas help
# or
./build/vanitas --help
```

### Analyze a file

Analyze a log file:
```bash
./build/vanitas file tests/log
```

Tip: choose a profile (name from ~/.vanitas/profiles/*.toml or a direct path):
```bash
./build/vanitas file --profile default tests/log
# or
./build/vanitas file --profile ~/.vanitas/profiles/default.toml tests/log
```

### Analyze stdin (pipe)

Read from stdin until EOF and analyze:
```bash
echo "Hello wordl" | ./build/vanitas pipe
```

Example output:
```bash
INFO:  Hello wordl
```

You can also pipe a real command output:
```bash
ninja -C build 2>&1 | ./build/vanitas pipe
```

### Run a command and analyze its output

Run any command, capture its stdout+stderr, and show only the analysis:
```bash
./build/vanitas run -- ls /no-such-file
```

Run a command with arguments (note the -- separator):
```bash
./build/vanitas run -- sh -c 'echo out; echo err 1>&2'
```

## Configuration & Profiles 

Vanitas can load configuration from ~/.vanitas/config.toml and profiles from ~/.vanitas/profiles/*.toml 

### Profile selection (precedence)

The effective profile is selected in this order (highest → lowest):
1. --profile <name> CLI flag
2. profile = "<name>" in ~/.vanitas/config.toml
3. default
​

Use vanitas profile list to see what profiles are available and where they come from.

### Inline profiles in config.toml

You can define inline profiles inside ~/.vanitas/config.toml under [profiles.<name>...].
Important: nested tables must use dotted headers, otherwise they will not be inside the profile table (TOML table scoping).

#### Example:
```bash
# ~/.vanitas/config.toml
profile = "nvim"

[profiles.nvim.classify]
err = ["decor_provider_error:"]
```

### File profiles in ~/.vanitas/profiles/

A file profile ~/.vanitas/profiles/<name>.toml is a standalone profile, and its keys live at the root of the document (no [profiles.<name>] wrapper).
​
#### Example:
```bash
# ~/.vanitas/profiles/base.toml
extends = "default"

[classify]
tests = ["MY_BASE_TEST"]
```

### Extends (profile inheritance)

Profiles can inherit from another profile using:
```bash
extends = "base"
```

The effective profile is built as: base_profile overlaid by current_profile (current values override base values).
Note: list fields currently use “replace” semantics (if classify.err is set in the child profile, it replaces the base list).

Debugging config/profile
- --dump-config prints the effective configuration after precedence.
- --dump-profile currently validates that the selected profile resolves and compiles successfully (including extends), and exits with non-zero code on errors like cycles.
