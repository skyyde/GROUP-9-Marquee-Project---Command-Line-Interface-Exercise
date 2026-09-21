#include <iostream>
#include <string>

using namespace std;

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
void startMarquee() {
    // marquee logic
}

void stopMarquee() {
    // marquee logic
}

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
            startMarquee();
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
