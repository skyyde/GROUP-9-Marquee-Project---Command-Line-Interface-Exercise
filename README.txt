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

marqueeText=Hello World
marqueeSpeed=50

marqueeText preserves everything after '='; an empty value uses the welcome
message. marqueeSpeed accepts positive integer milliseconds (1 to 2147483647).
Blank lines and lines starting with # (after optional spaces) are ignored.
Spaces around keys and numeric values are allowed. Do not add inline comments.
Missing settings keep the defaults (welcome message and 50 ms). Invalid lines
produce a warning and keep the previous value; the last valid duplicate wins.
A missing/unreadable config.txt warns and uses defaults. No keyboard polling
setting exists: std::getline waits for input while the worker animates.
Use plain text (ASCII or UTF-8); the Windows renderer is byte-oriented.

SUBMISSION AND RECORDING
For a SOURCE folder submission, include main.cpp, config.txt and this README.txt.
Before the timed quiz, prepare and verify the final build and your IDE's
Run/Debug setup and working directory. The recorded test must show editing
config.txt first, then Run/Debug launching the program.
During the test case, use the program's commands without accessing or modifying
source code or recompiling to accommodate a case.
