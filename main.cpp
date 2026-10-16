
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

struct Account {
    string accountNumber;
    string name;
    string pin;
    bool locked = false;
    int failedAttempts = 0;
};

vector<Account> accounts;
int nextAccountNumber = 100001;


string readPIN() {
    string pin;
    char ch;

    cout << "Enter a 4-digit PIN: ";

    while (true) {
#ifdef _WIN32
        ch = _getch();
#else
        ch = cin.get();
#endif

        if (ch == '\r' || ch == '\n') {
            break;
        }

        if (ch == '\b' || ch == 127) {
            if (!pin.empty()) {
                pin.pop_back();
                cout << "\b \b";
            }
            continue;
        }

        if (ch >= '0' && ch <= '9' && pin.length() < 4) {
            pin += ch;
            cout << '*';
        }
    }

    cout << '\n';
    return pin;
}


bool validPIN(const string& pin) {
    if (pin.length() != 4) return false;

    for (char ch : pin) {
        if (ch < '0' || ch > '9') return false;
    }

    return true;
}

void createAccount() {
    Account account;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\nEnter customer name: ";
    getline(cin, account.name);

    if (account.name.empty()) {
        cout << "Name cannot be empty.\n";
        return;
    }

    account.accountNumber = to_string(nextAccountNumber);

    while (true) {
        account.pin = readPIN();

        if (validPIN(account.pin)) break;

        cout << "Invalid PIN. Use exactly 4 digits.\n";
    }

    accounts.push_back(account);
    nextAccountNumber++;

    cout << "\nAccount created successfully!\n";
    cout << "Account number: " << account.accountNumber << '\n';
    cout << "Please remember your account number.\n";
}


void customerLogin() {
    string accountNumber;
    string enteredPIN;

    cout << "\nEnter account number: ";
    cin >> accountNumber;

    bool accountFound = false;

    for (Account& account : accounts) {
        if (account.accountNumber == accountNumber) {
            accountFound = true;

            enteredPIN = readPIN();

            if (enteredPIN == account.pin) {
                cout << "\nLogin successful!\n";
                cout << "Welcome, " << account.name << "!\n";
            } else {
                cout << "\nIncorrect PIN.\n";
            }

            break;
        }
    }

    if (!accountFound) {
        cout << "Account number not found.\n";
    }
}


int main() {
    int choice;

    do {
        cout << "\n====================================\n";
        cout << "      BANK MANAGEMENT SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Create customer account\n";
        cout << "2. Customer login\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                customerLogin();
                break;
            case 0:
                cout << "Exiting application.\n";
                break;
            default:
                cout << "Invalid menu choice.\n";
        }
    } while (choice != 0);

    return 0;
}
