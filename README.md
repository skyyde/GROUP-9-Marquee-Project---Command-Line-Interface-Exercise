# GROUP-9-Marquee-Project---Command-Line-Interface-Exercise

USE THIS TO RUN ''g++ -Wall -static main.cpp -o main && main.exe''

In Windows PowerShell, compile and run as separate commands (requires g++):

```powershell
g++ -Wall -static main.cpp -o main.exe
.\main.exe
```

Type `help` to see the commands. Run `start_marquee` without setting text to
animate the plain text "WELCOME TO CSOPESY" inside the NOW SHOWING box.
The box has up to 76 interior columns, adjusted to fit the terminal at startup.
While it runs, command replies appear in a separate area below the box;
the latest reply replaces the previous one.

Both the banner and custom text enter from off-screen left, move right through
a fixed-width window, fully exit on the right, then repeat. The text stays
unchanged; only its position moves, with off-screen portions clipped.
`set_speed` specifies the delay between frames in milliseconds and applies
immediately, including while running. The default is 50 ms per frame.

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
