#include <iostream>
#include <string>
#include <atomic>

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
    cout << "start_marquee - starts the marquee animation" << endl;
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
        cout << "speed set to " << marqueeSpeed << " ms" << endl;
    } catch (...) {
        cout << "invalid speed format" << endl;
    }
}

// MEMBER 3
#if USE_WIN32_THREADS
// THIS IS SCUFFED. anyways my code wouldnt run without win32 thread compatibility soooo. yeah. 
// well this atleast provides compatibility support. this pretty much does identical job to standard c++
// except it uses win32 threads instead of standard c++ threads. so yeah. refer to else statement for inline comment.
DWORD WINAPI marqueeWorker(LPVOID lpParam) {
    MarqueeData* data = (MarqueeData*)lpParam;
    if (data->text.empty()) return 0;

    string currentText = data->text;
    while (isRunning) {
        currentText = currentText.substr(1) + currentText[0];
        cout << "\rMarquee: " << currentText << "    " << flush;
        Sleep(data->speed);
    }
    cout << "\r" << string(currentText.length() + 20, ' ') << "\r" << flush;
    return 0;
}

void startMarquee(const string& marqueeText, int marqueeSpeed) {
    if (isRunning) {
        cout << "Marquee is already running." << endl;
        return;
    }

    if (marqueeText.empty()) {
        cout << "Please set text first using 'set_text <text>'." << endl;
        return;
    }

    cout << "Marquee started." << endl;
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
    cout << "Marquee stopped." << endl;
}

#else

void marqueeWorker(string text, int speed) {
    if (text.empty()) return;

    string currentText = text;
    
    while (isRunning) {
        // char rotation in string
        currentText = currentText.substr(1) + currentText[0];
        
        // overwrite previous line with new marquee text
        cout << "\rMarquee: " << currentText << "    " << flush;
        
        // speed control
        this_thread::sleep_for(chrono::milliseconds(speed));
    }

    // Clear marquee when stopped
    cout << "\r" << string(currentText.length() + 20, ' ') << "\r" << flush;
}

void startMarquee(const string& marqueeText, int marqueeSpeed) {
    // keeps only one instance running
    if (isRunning) {
        cout << "Marquee is already running." << endl;
        return;
    }

    // in case no text
    if (marqueeText.empty()) {
        cout << "Please set text first using 'set_text <text>'." << endl;
        return;
    }

    // start
    cout << "Marquee started." << endl;
    isRunning = true;
    marqueeThread = thread(marqueeWorker, marqueeText, marqueeSpeed);

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
    cout << "Marquee stopped." << endl;
}

#endif

int main() {
    string command;
    string marqueeText;
    int marqueeSpeed = 1000; // default speed in milliseconds

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

    while (true) {
        cout << "Command> ";
        getline(cin, command);

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
