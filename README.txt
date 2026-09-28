CSOPESY Semi-Major Output 1 - Group 9
Command-Line Marquee

GROUP MEMBERS
Mikyla Kirsten Aguirre
Enrique Mateo Cruz
Cedric Pallarca
Julian Nicos Reyes

SOURCE FILE AND ENTRY POINT
Source file: main.cpp
Entry point: int main() in main.cpp.
This C++ program uses a main function rather than an entry class.
All application source code is contained in main.cpp.

BUILD AND RUN ON WINDOWS
Open a terminal in the folder containing main.cpp. Use one of the compiler
options below. Run compilation and execution as separate commands.

Option 1: Microsoft Visual C++ (MSVC)
Open a Visual Studio Developer PowerShell or Developer Command Prompt:

cl /nologo /std:c++14 /EHsc /W4 main.cpp /Fe:main.exe
.\main.exe

Option 2: MinGW g++ with standard C++ thread support
Make sure g++ is installed and available in PATH:

g++ -std=c++11 -Wall -Wextra -Wpedantic -pthread -static main.cpp -o main.exe
.\main.exe

For older MinGW libraries without standard C++ thread support, the source
selects its Windows thread fallback. Compile without -pthread:

g++ -std=c++11 -Wall -Wextra -Wpedantic -static main.cpp -o main.exe
.\main.exe

MSVC builds were tested during development. The MinGW commands have not been
tested on the development machine. Validate the chosen compiler before the quiz.
If a prepared main.exe is supplied, run it with .\main.exe from its folder.

USING THE PROGRAM
Enter one command per line and press Enter. Commands are case-sensitive.

help                     List the commands and their descriptions.
start_marquee            Start the saved text or default welcome message.
stop_marquee             Stop the marquee animation.
set_text <string>        Save a message; spaces are allowed.
set_speed <milliseconds> Set a positive integer delay between animation frames.
exit                     Stop an active marquee and close the application.

Example sequence:
set_text Hello Operating Systems
set_speed 50
start_marquee
stop_marquee
exit

Text changes made while running automatically restart the animation with the
new message. No extra command is needed; the scrolling position resets.
Speed changes apply while running. The default frame delay is 50 milliseconds.
Use an interactive console with enough space for the box and command replies.

CONFIGURATION
Edit config.txt before Run/Debug. Set the IDE's working directory to this project
folder so the program can find it. The file is read once at startup:

marqueeText=CSOPESY
marqueeSpeed=100
pollingRate=250
startRunning=true

marqueeText sets the message. Spaces are preserved; optional surrounding double
quotes are removed, so "  Hello  " keeps the spaces inside the quotes. Empty text
uses the welcome message. A missing closing quote makes the setting invalid.
marqueeSpeed accepts positive integer milliseconds (1 to 2147483647).
pollingRate accepts integer milliseconds from 1 to 1000. It controls how
long keyboard polling waits before checking again when no input is queued.
startRunning accepts true or false: start immediately or wait for start_marquee.

Blank lines and lines starting with # (after optional spaces) are ignored.
Spaces around keys and numeric values are allowed. Do not add inline comments.
Missing settings keep defaults: welcome message, 50 ms refresh, 10 ms polling,
and startRunning=false. Invalid lines warn and keep the previous valid value;
the last valid duplicate wins. Config changes apply on the next launch, not
during a run. set_speed changes refresh speed only.
A missing/unreadable config.txt silently uses defaults.

KEYBOARD POLLING
In a Windows interactive console, the main thread checks queued keyboard events
and sleeps for pollingRate when none remain. Already queued characters are
read together; it does not sleep once per character. At 250 ms, a new key may
wait roughly up to one quarter of a second before appearing, plus scheduling
overhead. Lower intervals check more often. The animation thread has its own
refresh delay and continues moving while the input thread sleeps.

Characters still enter through std::cin and display through std::cout. A small
ConsoleInput stream buffer supplies cin with polled Windows keyboard events.
The program handles Enter, Backspace (including wrapped lines and tabs), key
repeats and pasted commands. Ctrl+C, Ctrl+D and Ctrl+Z request clean shutdown.
It uses basic end-of-line editing; arrow-key navigation/history is not provided.
The original console mode and cin buffer are restored on normal/error cleanup.

Use a real Windows console for polling tests, not an IDE's output-only panel.
Redirected input and the existing non-Windows path use getline instead; they do
not exercise keyboard polling. Text display remains byte-oriented: use ASCII
for predictable results. Config files also accept UTF-8 with an optional BOM.

SUBMISSION AND RECORDING
For a SOURCE folder submission, include main.cpp, config.txt and this README.txt.
Before the timed quiz, prepare and verify the final build and your IDE's
Run/Debug setup and working directory. The recorded test must show editing
config.txt first, then Run/Debug launching the program.
During the test case, use the program's commands without accessing or modifying
source code or recompiling to accommodate a case.
