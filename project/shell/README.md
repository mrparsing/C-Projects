# Shell

A minimal command-line shell-like emulator written in C using the ncurses library. It allows basic command execution and navigation with a user-friendly interactive interface.

---

Features
-	Text-based UI with ncurses
-	Real-time input editing with arrow keys
-	Command history (navigable with ↑ and ↓)
-	Cursor movement within the command (← and →)
-	Basic backspace/delete functionality
-	Execution of system commands via execvp
-	Captures and displays command output (stdout and stderr)
-	Ctrl+Q to quit

---

## How to Compile
```bash
chmod +x build.sh
./build.sh
```
Make sure ncurses is installed on your system (e.g., sudo apt install libncurses5-dev on Debian/Ubuntu).

---

## Usage

./shell

You’ll see a >  prompt where you can type Linux shell commands, like:
```bash
> ls -l
> echo Hello World
> pwd
```
Use:
-	↑ / ↓ to browse command history
-	← / → to move the cursor within the current command
-	Backspace to delete characters
-	Enter to execute
-	Ctrl+Q to exit

---

Notes
-	Each command is executed in a child process via fork and execvp.
-	Output is captured using a pipe and printed line-by-line using ncurses.
-	Commands are stored and replayed in a dynamic array for history navigation.

---

Limitations
-	Scrolling is not supported: If output exceeds the terminal height, it will be clipped.
-	Not all shell features are supported: Advanced features like pipes (|), I/O redirection (>, <), wildcards (*), etc., are not parsed or handled explicitly.
-	No command auto-completion
-	No support for multi-line commands or signals like Ctrl+C

This is a simplified educational prototype of a terminal—not a full shell.