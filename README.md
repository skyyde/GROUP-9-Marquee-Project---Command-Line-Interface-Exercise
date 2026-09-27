# GROUP-9-Marquee-Project---Command-Line-Interface-Exercise

This CSOPESY exercise is a user-space C++ command-line application. The main
thread accepts commands while a worker animates text. It demonstrates command
interpretation, concurrency, shared state, and thread cleanup on the host OS;
it does not implement an operating system kernel or CPU scheduler.

## Build and run

In Windows PowerShell, compile and run as separate commands. With a MinGW
compiler that supports standard C++ threads:

```powershell
g++ -std=c++11 -Wall -Wextra -Wpedantic -pthread -static main.cpp -o main.exe
.\main.exe
```

MinGW is a Windows GCC toolchain. Some older builds lack standard C++ thread
support; this source automatically selects a Windows `CreateThread` fallback
for those builds. Omit `-pthread` in that case:

```powershell
g++ -std=c++11 -Wall -Wextra -Wpedantic -static main.cpp -o main.exe
.\main.exe
```

With MSVC, run this in a Visual Studio Developer PowerShell or Command Prompt:

```text
cl /nologo /std:c++14 /EHsc /W4 main.cpp /Fe:main.exe
.\main.exe
```

On Linux or another Unix-like system with an ANSI-compatible terminal:

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -pthread main.cpp -o main
./main
```

There is no checked-in `.vscode/tasks.json`; these commands are the build setup.
MSVC builds were tested; the MinGW and native Linux commands have not
been tested in this environment.

## Commands and animation

Commands are case-sensitive. Enter one complete command per line.

| Command | Behavior |
| --- | --- |
| `help` | List commands and descriptions. |
| `start_marquee` | Start the saved text, or the default welcome text; a repeated start reports that the session is already running. |
| `stop_marquee` | Stop and wait for the worker; stopping while idle reports that it is not running. |
| `set_text <string>` | Save text, including spaces; stop and restart to display a change. |
| `set_speed <milliseconds>` | Set a positive integer delay; a running worker observes changes during its wait loop. |
| `exit` | Stop an active session, clean up its worker, and close the application. |

End of input (EOF) also cleans up an active session. This is a small built-in
command interpreter; it does not execute external shell commands.

Type `help` to see the commands. Run `start_marquee` without setting text to
animate the plain text "WELCOME TO CSOPESY" inside the NOW SHOWING box.
The marquee has a double yellow frame (`||` sides and `=` horizontal lines),
a bold yellow NOW SHOWING heading, and blue
scrolling text. Cyan `o` lights and a `<>` divider accent, plus magenta `*`
details, decorate the yellow border. Classic Windows consoles use
color intensity for the bold effect. Colors affect
only the foreground. The box has seven rows and up to 76 interior columns.
On Windows, its width is selected when the marquee starts, and drawing is
clipped to the current console window. The non-Windows ANSI path uses a fixed
76-column interior; use a terminal at least 80 columns wide with enough rows
for the box and command replies.
While it runs, command replies appear in a separate area below the box;
the latest reply replaces the previous one.

Both the banner and custom text enter from off-screen left, move right through
a fixed-width window, fully exit on the right, then repeat. The text stays
unchanged; only its position moves, with off-screen portions clipped.
`set_speed` specifies the requested wait between frames in milliseconds and
also works while running. The default is 50 ms, or roughly 20 frames per second
before drawing and scheduling overhead; it is not an exact refresh-rate guarantee.
The value must be a positive integer in the range supported by `int`;
values such as `500abc` are rejected. Short sleep intervals let stop and exit
interrupt a long frame delay.

Windows frames use console buffer APIs. The non-Windows path uses ANSI escape
sequences and a mutex to keep program-generated frames and replies together.
Terminal echo, very small windows, resizing, and Unicode display widths are
limitations; rehearse in the terminal used for the presentation.

For a normal text marquee, enter these commands one at a time:

```text
stop_marquee
set_text Hello World!
set_speed 200
start_marquee
```

Stop and restart the marquee to apply text changes. Use `exit` to
stop the animation and close the program. Recompile after changing `main.cpp`;
the existing executables do not automatically include source changes.

## Submission

Compile `main.cpp` for the application. Check the animation interactively in
the presentation terminal; redirected Windows output does not animate.

No assignment handout or executable-submission rule was found in this repository.
The tracked `main.exe` is preserved. `.gitignore` excludes new build outputs,
but does not untrack that existing binary. If the assignment does not require
an executable, remove it from version control; otherwise rebuild it for the
final submission using the group's compiler.

For the presentation, use [PPT_TECHNICAL_NOTES.md](PPT_TECHNICAL_NOTES.md).
