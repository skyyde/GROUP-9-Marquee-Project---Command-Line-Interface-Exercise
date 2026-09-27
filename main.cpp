#include <iostream>
#include <string>
#include <atomic>
#include <vector>
#include <algorithm>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif

#if defined(_WIN32) && defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
    // apparently my(member 3) needs this cause of mingw running win32 threads. refer to marquee functions for explanation
    #include <windows.h>
    #define USE_WIN32_THREADS 1
#else
    // Standard C++
    #include <thread>
    #include <chrono>
    #define USE_WIN32_THREADS 0
#endif

using namespace std;

// Marquee thread control
atomic<bool> isRunning(false);
atomic<int> frameDelay(50);

#ifdef _WIN32
// Legacy console text selection otherwise suspends all animation output.
struct ConsoleInputMode {
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD original = 0;
    bool saved = GetConsoleMode(input, &original) != 0;
    ConsoleInputMode() {
        if (saved) SetConsoleMode(input, (original | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE);
    }
    ~ConsoleInputMode() {
        if (saved) SetConsoleMode(input, original);
    }
};
#endif

#if USE_WIN32_THREADS
    HANDLE marqueeThreadHandle = NULL;
    struct MarqueeData {
        string text;
        int speed;
    } g_marqueeData;
#else
    thread marqueeThread;
#endif



// MEMBER 2
void displayHelp() {
    cout << "help - displays the commands and their descriptions" << endl;
    cout << "start_marquee - animates a welcome banner, or your custom text" << endl;
    cout << "stop_marquee - stops the marquee animation" << endl;
    cout << "set_text <your_string> - accepts text input and saves it for the marquee" << endl;
    cout << "set_speed <milliseconds> - sets the marquee animation refresh speed in milliseconds" << endl;
    cout << "exit - terminates the console" << endl;
}
//thingy
void setText(string input, string &marqueeText) {
    string text = input.substr(9);

     if (text.empty()) {
        cout << "no text provided" << endl;
        return;
     }

     marqueeText = text;
    cout << "text saved for marquee: " << marqueeText << endl;
    if (isRunning) {
        cout << "To display your saved text, type 'stop_marquee', then 'start_marquee'." << endl;
    } else {
        cout << "Type 'start_marquee' to display your saved text." << endl;
    }
    }

void setSpeed(string input, int &marqueeSpeed) {
    string speedStr = input.substr(10);

    if (speedStr.empty()) {
        cout << "no speed provided" << endl;
        return;
    }

    try {
        int speed = stoi(speedStr);
        if (speed <= 0) {
            cout << "speed must be a positive number" << endl;
            return;
        }
        marqueeSpeed = speed;
        frameDelay = speed;
        cout << "speed set to " << marqueeSpeed << " ms" << endl;
    } catch (...) {
        cout << "invalid speed format" << endl;
    }
}

vector<string> bannerRows;
int marqueeWidth = 76;
vector<string> marqueeBox(int offset, int width);

// Reuse a command area below the seven-row box instead of allowing command
// output to scroll into the animation's fixed drawing area.
void prepareCommandArea() {
    if (!isRunning) return;
    cout << flush;
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (!GetConsoleScreenBufferInfo(output, &info)) return;
    const int firstRow = info.srWindow.Top + 8;
    if (firstRow >= info.srWindow.Bottom) return;
    DWORD written;
    for (int row = firstRow; row <= info.srWindow.Bottom; ++row) {
        COORD position = {0, static_cast<SHORT>(row)};
        FillConsoleOutputCharacterA(output, ' ', info.dwSize.X, position, &written);
    }
    COORD position = {info.srWindow.Left, static_cast<SHORT>(firstRow)};
    SetConsoleCursorPosition(output, position);
#else
    cout << "\033[9;1H\033[J" << flush;
#endif
}

// Only call while the animation thread is stopped. A fresh display also
// removes remnants left behind when console scrolling moved earlier frames.
void clearMarqueeDisplay() {
    cout << flush;
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (!GetConsoleScreenBufferInfo(output, &info)) return;
    DWORD written;
    for (int row = info.srWindow.Top; row <= info.srWindow.Bottom; ++row) {
        COORD position = {0, static_cast<SHORT>(row)};
        FillConsoleOutputCharacterA(output, ' ', info.dwSize.X, position, &written);
        FillConsoleOutputAttribute(output, info.wAttributes, info.dwSize.X, position, &written);
    }
    COORD beginning = {info.srWindow.Left, info.srWindow.Top};
    SetConsoleCursorPosition(output, beginning);
#else
    cout << "\033[2J\033[H" << flush;
#endif
}

void prepareBanner(const string& text) {
    bannerRows = vector<string>{text.empty() ? "WELCOME TO CSOPESY" : text};
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        marqueeWidth = max(1, min(76, static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1) - 3));
    }
#else
    cout << "\033[2J\033[H";
#endif
    int contentWidth = 0;
    for (const string& row : bannerRows)
        contentWidth = max(contentWidth, static_cast<int>(row.size()));
    // Print the complete blank box first, leaving the command prompt below it.
    for (const string& row : marqueeBox(-contentWidth, marqueeWidth)) cout << row << endl;
}

// Copy only the part of an unchanged row that overlaps the fixed window.
string marqueeFrame(const string& text, int offset, int width) {
    string frame(width, ' ');
    int source = max(0, -offset);
    int destination = max(0, offset);
    int count = min(static_cast<int>(text.size()) - source, width - destination);
    if (count > 0) frame.replace(destination, count, text, source, count);
    return frame;
}

int nextMarqueeOffset(int offset, int contentWidth, int windowWidth) {
    return offset >= windowWidth ? -contentWidth : offset + 1;
}

vector<string> marqueeBox(int offset, int width) {
    string title = "[ NOW SHOWING ]";
    title.resize(min(width, static_cast<int>(title.size())));
    int leftPadding = (width - static_cast<int>(title.size())) / 2;
    string heading = string(leftPadding, ' ') + title;
    heading.resize(width, ' ');
    const string border = "+" + string(width, '=') + "+";
    const string blank = "|" + string(width, ' ') + "|";
    vector<string> rows = {border, "|" + heading + "|",
        "+" + string(width, '-') + "+", blank};
    for (const string& row : bannerRows)
        rows.push_back("|" + marqueeFrame(row, offset, width) + "|");
    rows.push_back(blank);
    rows.push_back(border);
    return rows;
}

void animateMarquee(int speed) {
    frameDelay = speed;
    int contentWidth = 0;
    for (const string& row : bannerRows)
        contentWidth = max(contentWidth, static_cast<int>(row.size()));
    int offset = -contentWidth;
    const int width = marqueeWidth;
    while (isRunning) {
        const vector<string> frame = marqueeBox(offset, width);
#ifdef _WIN32
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info = {};
        // Redirected output has no console region to animate.
        if (!GetConsoleScreenBufferInfo(output, &info)) return;
        // Draw one rectangle at the visible window's top. Unlike individual
        // row writes, this cannot wrap into the next row after a resize.
        // Follow the viewport when command output scrolls the console.
        int drawWidth = min(width + 2, static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1));
        int drawHeight = min(static_cast<int>(frame.size()),
                             static_cast<int>(info.srWindow.Bottom - info.srWindow.Top - 1));
        if (drawWidth > 0 && drawHeight > 0) {
            vector<CHAR_INFO> cells(static_cast<size_t>(drawWidth) * drawHeight);
            for (int row = 0; row < drawHeight; ++row) {
                const string& line = frame[row];
                for (int column = 0; column < drawWidth; ++column) {
                    CHAR_INFO& cell = cells[static_cast<size_t>(row) * drawWidth + column];
                    cell.Char.AsciiChar = line[column];
                    cell.Attributes = info.wAttributes;
                }
            }
            COORD size = {static_cast<SHORT>(drawWidth), static_cast<SHORT>(drawHeight)};
            COORD source = {0, 0};
            SMALL_RECT area = {info.srWindow.Left, info.srWindow.Top,
                static_cast<SHORT>(info.srWindow.Left + drawWidth - 1),
                static_cast<SHORT>(info.srWindow.Top + drawHeight - 1)};
            WriteConsoleOutputA(output, cells.data(), size, source, &area);
        }
#else
        cout << "\033[s";
        for (size_t row = 0; row < frame.size(); ++row) {
            cout << "\033[" << row + 1 << ";1H" << frame[row];
        }
        cout << "\033[u" << flush;
#endif
        offset = nextMarqueeOffset(offset, contentWidth, width);
        // Short waits keep stop/exit responsive even at a slow animation speed.
        for (int elapsed = 0; isRunning && elapsed < frameDelay.load();) {
            int delay = min(frameDelay.load() - elapsed, 25);
            if (delay <= 0) break;
#if USE_WIN32_THREADS
            Sleep(delay);
#else
            this_thread::sleep_for(chrono::milliseconds(delay));
#endif
            elapsed += delay;
        }
    }
}

// MEMBER 3
#if USE_WIN32_THREADS
// THIS IS SCUFFED. anyways my code wouldnt run without win32 thread compatibility soooo. yeah. 
// well this atleast provides compatibility support. this pretty much does identical job to standard c++
// except it uses win32 threads instead of standard c++ threads. so yeah. refer to else statement for inline comment.
DWORD WINAPI marqueeWorker(LPVOID lpParam) {
    MarqueeData* data = (MarqueeData*)lpParam;
    animateMarquee(data->speed);
    return 0;
}

void startMarquee(const string& marqueeText, int marqueeSpeed) {
    if (isRunning) {
        cout << "Marquee is already running." << endl;
        return;
    }

    clearMarqueeDisplay();
    prepareBanner(marqueeText);
    cout << "Marquee started. Type 'stop_marquee' to stop." << endl;
    isRunning = true;
    g_marqueeData = { marqueeText, marqueeSpeed };
    marqueeThreadHandle = CreateThread(NULL, 0, marqueeWorker, &g_marqueeData, 0, NULL);
}

void stopMarquee() {
    if (!isRunning) {
        cout << "Marquee is not running." << endl;
        return;
    }

    isRunning = false;
    if (marqueeThreadHandle != NULL) {
        WaitForSingleObject(marqueeThreadHandle, INFINITE);
        CloseHandle(marqueeThreadHandle);
        marqueeThreadHandle = NULL;
    }
    clearMarqueeDisplay();
    cout << "Marquee stopped." << endl;
}

#else

void marqueeWorker(int speed) {
    animateMarquee(speed);
}

void startMarquee(const string& marqueeText, int marqueeSpeed) {
    // keeps only one instance running
    if (isRunning) {
        cout << "Marquee is already running." << endl;
        return;
    }

    // start
    clearMarqueeDisplay();
    prepareBanner(marqueeText);
    cout << "Marquee started. Type 'stop_marquee' to stop." << endl;
    isRunning = true;
    marqueeThread = thread(marqueeWorker, marqueeSpeed);

}

void stopMarquee() {
    // check if running instance
    if (!isRunning) {
        cout << "Marquee is not running." << endl;
        return;
    }

    // self explanatory
    isRunning = false;

    // clean up thread
    if (marqueeThread.joinable()) {
        marqueeThread.join();
    }
    clearMarqueeDisplay();
    cout << "Marquee stopped." << endl;
}

#endif

int main() {
#ifdef _WIN32
    ConsoleInputMode consoleInputMode;
#endif
    string command;
    string marqueeText;
    int marqueeSpeed = 50; // milliseconds per frame

    // MEMBER 1
    cout << "Welcome to CSOPESY!" << endl;
    cout << endl;

    cout << "Group developer:" << endl;
    cout << "Mikyla Kirsten Aguirre" << endl;
    cout << "Enrique Mateo Cruz" << endl;
    cout << "Cedric Pallarca" << endl;
    cout << "Julian Nicos Reyes" << endl;
    cout << endl;

    cout << "Version date: 2026-09-21" << endl;
    cout << endl;

    cout << "Type 'help' to see the available commands." << endl;
    cout << endl;

    while (true) {
        cout << "Command> ";
        if (!getline(cin, command)) {
            if (isRunning) stopMarquee();
            break;
        }

        prepareCommandArea();

        if (command == "help") {
            displayHelp();
        }

        else if (command == "start_marquee") {
            startMarquee(marqueeText, marqueeSpeed);
        }

        else if (command == "stop_marquee") {
            stopMarquee();
        }
        else if (command.rfind("set_text ", 0) == 0) {
            setText(command, marqueeText);
        }

        else if (command == "set_text") {
            cout << "no text provided" << endl;
        }

        else if (command.rfind("set_speed ", 0) == 0) {
            setSpeed(command, marqueeSpeed);
        }
        else if (command == "set_speed") {
            cout << "no speed provided" << endl;
        }

        else if (command == "exit") {
            if (isRunning) stopMarquee();
            cout << "Terminating console..." << endl;
            break;
        }

        else {
            cout << "Invalid command." << endl;
        }

        cout << endl;
    }

    return 0;
}
