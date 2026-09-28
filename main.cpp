// LIBRARIES AND PLATFORM SETUP ---------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include <vector>
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#if defined(_WIN32) && defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#define USE_WIN32_THREADS 1
#else
#include <thread>
#include <chrono>
#include <system_error>
#define USE_WIN32_THREADS 0
#endif

#ifndef _WIN32
#include <mutex>
#endif

using namespace std;

// SHARED VARIABLES AND CONSOLE SETUP ---------------------------------------------------

atomic<bool> isRunning(false);
atomic<int> frameDelay(50);

#ifndef _WIN32
mutex consoleMutex;
#endif

#ifdef _WIN32
struct ConsoleInputMode {
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD original = 0;
    bool saved = GetConsoleMode(input, &original) != 0;

    ConsoleInputMode() {
        if (saved)
            SetConsoleMode(input, (original | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE);
    }

    ~ConsoleInputMode() {
        if (saved)
            SetConsoleMode(input, original);
    }
};
#endif

#if USE_WIN32_THREADS
HANDLE marqueeThreadHandle = NULL;
#else
thread marqueeThread;
#endif

// COMMAND FUNCTIONS (MEMBER 2) ---------------------------------------------------

void displayHelp() {
    cout << "help - displays the commands and their descriptions" << endl;
    cout << "start_marquee - animates a welcome banner, or your custom text" << endl;
    cout << "stop_marquee - stops the marquee animation" << endl;
    cout << "set_text <your_string> - accepts text input and saves it for the marquee" << endl;
    cout << "set_speed <milliseconds> - sets the marquee animation refresh speed in milliseconds"
         << endl;
    cout << "exit - terminates the console" << endl;
}

bool setText(const string& input, string& marqueeText) {
    string text = input.substr(9);

    if (text.empty()) {
        cout << "no text provided" << endl;
        return false;
    }

    marqueeText = text;
    cout << "text saved for marquee: " << marqueeText << endl;

    if (!isRunning) {
        cout << "Type 'start_marquee' to display your saved text." << endl;
    }
    return true;
}

bool parseSpeed(const string& value, int& speed) {
    try {
        size_t parsed = 0;
        int candidate = stoi(value, &parsed);

        if (value.find_first_not_of(" \t\r\n\f\v", parsed) != string::npos)
            return false;

        speed = candidate;
        return true;
    } catch (const invalid_argument&) {
        return false;
    } catch (const out_of_range&) {
        return false;
    }
}

void setSpeed(const string& input) {
    string speedStr = input.substr(10);

    if (speedStr.empty()) {
        cout << "no speed provided" << endl;
        return;
    }

    int speed = 0;
    if (!parseSpeed(speedStr, speed)) {
        cout << "invalid speed format" << endl;
    } else if (speed <= 0) {
        cout << "speed must be a positive number" << endl;
    } else {
        frameDelay = speed;
        cout << "speed set to " << speed << " ms" << endl;
    }
}

// CONFIG FILE LOADING ---------------------------------------------------

void loadConfig(string& marqueeText) {
    ifstream config("config.txt");

    if (!config) {
        cout << "config.txt not found or unreadable; using default text and speed." << endl;
        return;
    }

    string line;
    int lineNumber = 0;

    while (getline(config, line)) {
        ++lineNumber;
        if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        const size_t first = line.find_first_not_of(" \t");
        if (first == string::npos || line[first] == '#')
            continue;

        const size_t equals = line.find('=', first);
        if (equals != string::npos) {
            string key = line.substr(first, equals - first);
            const size_t last = key.find_last_not_of(" \t");
            key.erase(last == string::npos ? 0 : last + 1);

            const string value = line.substr(equals + 1);
            if (key == "marqueeText") {
                marqueeText = value;
                continue;
            }

            int speed = 0;
            if (key == "marqueeSpeed" && parseSpeed(value, speed) && speed > 0) {
                frameDelay = speed;
                continue;
            }
        }

        cout << "Ignoring invalid config.txt line " << lineNumber << "." << endl;
    }

    if (config.bad())
        cout << "Error reading config.txt; keeping values loaded so far." << endl;
}

// MARQUEE DISPLAY AND LAYOUT ---------------------------------------------------

vector<string> bannerRows;
int marqueeWidth = 76;

vector<string> marqueeBox(int offset, int width);

#ifdef _WIN32
WORD marqueeRowColor(size_t row, size_t rowCount, WORD original, char symbol = '\0') {
    WORD foreground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    if (row >= 4 && row < rowCount - 2)
        foreground = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    if (row == 0 || row == 2 || row == rowCount - 1) {
        if (symbol == '*')
            foreground = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (symbol == 'o' || symbol == '<' || symbol == '>')
            foreground = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    }
    return static_cast<WORD>((original & 0xFFF0) | foreground);
}
#else
const char* marqueeRowColor(size_t row, size_t rowCount, char symbol = '\0') {
    if (row == 0 || row == 2 || row == rowCount - 1) {
        if (symbol == '*')
            return "\033[1;95m";
        if (symbol == 'o' || symbol == '<' || symbol == '>')
            return "\033[1;96m";
    }
    if (row >= 4 && row < rowCount - 2)
        return "\033[22;94m";
    return "\033[1;93m";
}

void printMarqueeRow(const string& line, size_t row, size_t rowCount) {
    const char* previousColor = nullptr;
    for (size_t column = 0; column < line.size(); ++column) {
        const size_t colorRow = column < 2 || column >= line.size() - 2 ? 0 : row;
        const char* color = marqueeRowColor(colorRow, rowCount, line[column]);
        if (color != previousColor)
            cout << color;
        cout << line[column];
        previousColor = color;
    }
}
#endif

void prepareCommandArea() {
    if (!isRunning)
        return;
    cout << flush;
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (!GetConsoleScreenBufferInfo(output, &info))
        return;
    const int firstRow = info.srWindow.Top + 8;
    if (firstRow >= info.srWindow.Bottom)
        return;
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

void clearMarqueeDisplay() {
    cout << flush;
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (!GetConsoleScreenBufferInfo(output, &info))
        return;
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
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    const bool hasConsole = GetConsoleScreenBufferInfo(output, &info) != 0;
    if (hasConsole) {
        marqueeWidth =
            max(1, min(76, static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1) - 5));
    }
#endif
    int contentWidth = 0;
    for (const string& row : bannerRows)
        contentWidth = max(contentWidth, static_cast<int>(row.size()));

    const vector<string> frame = marqueeBox(-contentWidth, marqueeWidth);
    for (size_t row = 0; row < frame.size(); ++row) {
#ifdef _WIN32
        if (hasConsole) {
            for (size_t column = 0; column < frame[row].size(); ++column) {
                const size_t colorRow = column < 2 || column >= frame[row].size() - 2 ? 0 : row;
                const WORD color =
                    marqueeRowColor(colorRow, frame.size(), info.wAttributes, frame[row][column]);
                SetConsoleTextAttribute(output, color);
                cout << frame[row][column] << flush;
            }
        } else {
            cout << frame[row];
        }
#else
        printMarqueeRow(frame[row], row, frame.size());
#endif
        cout << endl;
    }
#ifdef _WIN32
    if (hasConsole)
        SetConsoleTextAttribute(output, info.wAttributes);
#else
    cout << "\033[22;39m" << flush;
#endif
}

string marqueeFrame(const string& text, int offset, int width) {
    string frame(width, ' ');
    int source = max(0, -offset);
    int destination = max(0, offset);
    int count = min(static_cast<int>(text.size()) - source, width - destination);
    if (count > 0)
        frame.replace(destination, count, text, source, count);
    return frame;
}

int nextMarqueeOffset(int offset, int contentWidth, int windowWidth) {
    return offset >= windowWidth ? -contentWidth : offset + 1;
}

string centerMarqueeLabel(const string& text, int width) {
    string label = text.substr(0, width);
    label.insert(0, (width - label.size()) / 2, ' ');
    label.resize(width, ' ');
    return label;
}

vector<string> marqueeBox(int offset, int width) {
    string trim(width, '=');
    for (int column = 3; column < width; column += 8)
        trim[column] = 'o';
    for (int column = 7; column < width; column += 8)
        trim[column] = '*';
    const string border = "++" + trim + "++";
    string divider(width, '=');
    if (width >= 4) {
        divider[width / 2 - 1] = '<';
        divider[width / 2] = '>';
    }
    const string blank = "||" + string(width, ' ') + "||";

    vector<string> rows = {border, "||" + centerMarqueeLabel("NOW SHOWING", width) + "||",
                           "++" + divider + "++", blank};
    for (const string& row : bannerRows)
        rows.push_back("||" + marqueeFrame(row, offset, width) + "||");
    rows.push_back(blank);
    rows.push_back(border);
    return rows;
}

// ANIMATION LOOP ---------------------------------------------------

void animateMarquee() {
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
        if (!GetConsoleScreenBufferInfo(output, &info))
            return;
        int drawWidth =
            min(width + 4, static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1));
        int drawHeight = min(static_cast<int>(frame.size()),
                             static_cast<int>(info.srWindow.Bottom - info.srWindow.Top - 1));
        if (drawWidth > 0 && drawHeight > 0) {
            vector<CHAR_INFO> cells(static_cast<size_t>(drawWidth) * drawHeight);
            for (int row = 0; row < drawHeight; ++row) {
                const string& line = frame[row];
                const WORD edgeColor = marqueeRowColor(0, frame.size(), info.wAttributes);
                for (int column = 0; column < drawWidth; ++column) {
                    CHAR_INFO& cell = cells[static_cast<size_t>(row) * drawWidth + column];
                    cell.Char.AsciiChar = line[column];
                    cell.Attributes =
                        column < 2 || column >= width + 2
                            ? edgeColor
                            : marqueeRowColor(row, frame.size(), info.wAttributes, line[column]);
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
        {
            lock_guard<mutex> outputLock(consoleMutex);
            cout << "\033[s";
            for (size_t row = 0; row < frame.size(); ++row) {
                cout << "\033[" << row + 1 << ";1H";
                printMarqueeRow(frame[row], row, frame.size());
            }
            cout << "\033[22;39m\033[u" << flush;
        }
#endif
        offset = nextMarqueeOffset(offset, contentWidth, width);
        for (int elapsed = 0; isRunning && elapsed < frameDelay.load();) {
            int delay = min(frameDelay.load() - elapsed, 25);
            if (delay <= 0)
                break;
#if USE_WIN32_THREADS
            Sleep(delay);
#else
            this_thread::sleep_for(chrono::milliseconds(delay));
#endif
            elapsed += delay;
        }
    }
}

// ANIMATION THREAD CONTROL (MEMBER 3) ---------------------------------------------------

void marqueeWorker() {
    try {
        animateMarquee();
    } catch (const exception& error) {
#ifndef _WIN32
        lock_guard<mutex> outputLock(consoleMutex);
#endif
        cerr << "Marquee animation failed: " << error.what() << endl;
    }
}

#if USE_WIN32_THREADS
DWORD WINAPI win32MarqueeWorker(LPVOID) {
    marqueeWorker();
    return 0;
}
#endif

void startMarquee(const string& marqueeText) {
    if (isRunning) {
        cout << "Marquee is already running." << endl;
        return;
    }

    clearMarqueeDisplay();
    prepareBanner(marqueeText);
    isRunning = true;
#if USE_WIN32_THREADS
    marqueeThreadHandle = CreateThread(NULL, 0, win32MarqueeWorker, NULL, 0, NULL);
    if (marqueeThreadHandle == NULL) {
        isRunning = false;
        cout << "Unable to start marquee." << endl;
        return;
    }
#else
    try {
        marqueeThread = thread(marqueeWorker);
    } catch (const system_error&) {
        isRunning = false;
        cout << "Unable to start marquee." << endl;
        return;
    }
#endif
    cout << "Marquee started. Type 'stop_marquee' to stop." << endl;
}

void stopMarquee() {
    if (!isRunning) {
        cout << "Marquee is not running." << endl;
        return;
    }

    isRunning = false;
#if USE_WIN32_THREADS
    if (marqueeThreadHandle != NULL) {
        WaitForSingleObject(marqueeThreadHandle, INFINITE);
        CloseHandle(marqueeThreadHandle);
        marqueeThreadHandle = NULL;
    }
#else
    if (marqueeThread.joinable()) {
        marqueeThread.join();
    }
#endif
    clearMarqueeDisplay();
    cout << "Marquee stopped." << endl;
}

// MAIN PROGRAM AND COMMAND LOOP (MEMBER 1) ---------------------------------------------------

int main() {
#ifdef _WIN32
    ConsoleInputMode consoleInputMode;
#endif
    string command;
    string marqueeText;

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

    try {
        loadConfig(marqueeText);

        while (true) {
            {
#ifndef _WIN32
                lock_guard<mutex> outputLock(consoleMutex);
#endif
                cout << "Command> " << flush;
            }

            if (!getline(cin, command)) {
                if (isRunning)
                    stopMarquee();
                break;
            }

#ifndef _WIN32
            unique_lock<mutex> outputLock(consoleMutex);
#endif
            prepareCommandArea();

            if (command == "help") {
                displayHelp();
            } else if (command == "start_marquee") {
                startMarquee(marqueeText);
            } else if (command == "stop_marquee") {
#ifndef _WIN32
                outputLock.unlock();
#endif
                stopMarquee();
            } else if (command.rfind("set_text ", 0) == 0) {
                if (setText(command, marqueeText) && isRunning) {
#ifndef _WIN32
                    outputLock.unlock();
#endif
                    stopMarquee();
#ifndef _WIN32
                    outputLock.lock();
#endif
                    startMarquee(marqueeText);
                }
            } else if (command == "set_text") {
                cout << "no text provided" << endl;
            } else if (command.rfind("set_speed ", 0) == 0) {
                setSpeed(command);
            } else if (command == "set_speed") {
                cout << "no speed provided" << endl;
            } else if (command == "exit") {
#ifndef _WIN32
                outputLock.unlock();
#endif
                if (isRunning)
                    stopMarquee();
                cout << "Terminating console..." << endl;
                break;
            } else {
                cout << "Invalid command." << endl;
            }

            cout << endl;
        }
    } catch (const exception& error) {
        if (isRunning)
            stopMarquee();
        cerr << "Console error: " << error.what() << endl;
        return 1;
    }

    return 0;
}
