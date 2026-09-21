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

// MEMBER 3
void startMarquee() {
    // marquee logic
}

void stopMarquee() {
    // marquee logic
}

int main() {
    string command;

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
