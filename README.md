<div align="center">
  <!--<img src="./assets/vanitas-logo.png" alt="Vanitas" width="350">-->
</div>

# Vanitas

*A small “book” for cursed logs.*  
It doesn’t fight monsters — it heals terminal output.

</div>

## Prologue

Some logs are born clean.  
Most are **cursed**: endless noise, blinking colors, broken progress lines, and errors hiding in the shadows.

Vanitas was created for those moments — when a single real failure is buried under thousands of lines.

## The Case File

Vanitas is a terminal log analyzer (CLI now, TUI soon) that:

- Removes terminal “illusions” (ANSI escape codes, control sequences).
- Understands overwritten progress lines (`\r`) so they don’t turn into nonsense.
- Groups multi-line failures (stack traces, nested errors) into one readable **case**.
- Separates the story into: `Errors`, `Warnings`, `Tests`, and useful `Info`.

## How to use the book

### Analyze a file

```bash
./build/vanitas <logfile>
