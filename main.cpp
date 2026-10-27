#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cctype>

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
    double balance = 0.0;
};

vector<Account> accounts;
int nextAccountNumber = 100001;

const string ACCOUNT_FILE = "accounts.txt";
const string TRANSACTION_FILE = "transactions.txt";

// Issue #6: Save all accounts, including their current balances.
bool saveAccounts() {
    ofstream file(ACCOUNT_FILE);
    if (!file) {
        cerr << "Error: Could not save account data.\n";
        return false;
    }

    for (const Account& account : accounts) {
        // std::quoted safely stores names containing spaces.
        file << quoted(account.accountNumber) << ' '
             << quoted(account.name) << ' '
             << quoted(account.pin) << ' '
             << account.locked << ' '
             << account.failedAttempts << ' '
             << setprecision(17) << account.balance << '\n';
    }

    if (!file) {
        cerr << "Error: Failed while writing account data.\n";
        return false;
    }
    return true;
}

// Issue #6: Load saved accounts when the program starts.
bool loadAccounts() {
    ifstream file(ACCOUNT_FILE);
    if (!file) {
        // No file yet is normal on the first run.
        return true;
    }

    vector<Account> loadedAccounts;
    Account account;
    int largestAccountNumber = 100000;

    while (file >> quoted(account.accountNumber)
                >> quoted(account.name)
                >> quoted(account.pin)
                >> account.locked
                >> account.failedAttempts
                >> account.balance) {
        if (account.accountNumber.empty() || account.name.empty() ||
            account.pin.size() != 4 || account.balance < 0.0) {
            cerr << "Warning: Skipping an invalid account record.\n";
            continue;
        }

        try {
            int number = stoi(account.accountNumber);
            if (number > largestAccountNumber) {
                largestAccountNumber = number;
            }
        } catch (...) {
            cerr << "Warning: Skipping an account with an invalid account number.\n";
            continue;
        }

        loadedAccounts.push_back(account);
    }

    if (!file.eof()) {
        cerr << "Warning: Some account data could not be read.\n";
    }

    accounts = loadedAccounts;
    nextAccountNumber = largestAccountNumber + 1;
    return true;
}

// Issue #6: Append a transaction record. Deposit/withdrawal code can call this later.
bool recordTransaction(const string& accountNumber,
                       const string& transactionType,
                       double amount,
                       double balanceAfter) {
    ofstream file(TRANSACTION_FILE, ios::app);
    if (!file) {
        cerr << "Error: Could not open transaction history file.\n";
        return false;
    }

    file << quoted(accountNumber) << ' '
         << quoted(transactionType) << ' '
         << fixed << setprecision(2) << amount << ' '
         << balanceAfter << '\n';

    if (!file) {
        cerr << "Error: Failed while writing transaction history.\n";
        return false;
    }
    return true;
}

string readPIN() {
    while (true) {
        string pin;
        char ch;

        cout << "Enter a 4-digit PIN: " << flush;

        while (true) {
#ifdef _WIN32
            ch = _getch();
#else
            ch = cin.get();
#endif

            if (ch == '\r' || ch == '\n') {
                if (pin.length() == 4) {
                    cout << '\n';
                    return pin;
                }

                cout << "\nPIN must contain exactly 4 digits.\n";
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
    }
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

    bool hasNonWhitespace = false;
    for (char ch : account.name) {
        if (!isspace(static_cast<unsigned char>(ch))) {
            hasNonWhitespace = true;
            break;
        }
    }

    if (!hasNonWhitespace) {
        cout << "Name cannot be empty or spaces only.\n";
        return;
    }

    account.accountNumber = to_string(nextAccountNumber);

    while (true) {
        account.pin = readPIN();
        if (validPIN(account.pin)) break;
        cout << "Invalid PIN. Use exactly 4 digits.\n";
    }

    account.balance = 0.0;
    accounts.push_back(account);
    nextAccountNumber++;

    if (!saveAccounts()) {
        cout << "Warning: Account created in memory, but could not be saved.\n";
    }

    cout << "\nAccount created successfully!\n";
    cout << "Account number: " << account.accountNumber << '\n';
    cout << "Initial balance: Rs. " << fixed << setprecision(2)
         << account.balance << '\n';
    cout << "Please remember your account number.\n";
}

// Issue #5: Show details and current balance for the logged-in account.
void balanceEnquiry(const Account& account) {
    cout << "\n========== BALANCE ENQUIRY ==========\n";
    cout << "Account Number: " << account.accountNumber << '\n';
    cout << "Account Holder: " << account.name << '\n';
    cout << fixed << setprecision(2);
    cout << "Available Balance: Rs. " << account.balance << '\n';
    cout << "=====================================\n";
}

void customerMenu(size_t accountIndex) {
    int choice;

    do {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "1. Balance enquiry\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice) {
            case 1:
                if (accountIndex < accounts.size()) {
                    balanceEnquiry(accounts[accountIndex]);
                }
                break;
            case 0:
                cout << "Logged out successfully.\n";
                break;
            default:
                cout << "Invalid menu choice.\n";
        }
    } while (choice != 0);
}

void customerLogin() {
    string accountNumber;
    string enteredPIN;

    cout << "\nEnter account number: ";
    cin >> accountNumber;

    for (size_t i = 0; i < accounts.size(); ++i) {
        Account& account = accounts[i];
        if (account.accountNumber == accountNumber) {
            enteredPIN = readPIN();

            if (enteredPIN == account.pin) {
                cout << "\nLogin successful!\n";
                cout << "Welcome, " << account.name << "!\n";
                customerMenu(i);
            } else {
                cout << "\nIncorrect PIN.\n";
            }
            return;
        }
    }

    cout << "Account number not found.\n";
}

int main() {
    // Issue #6: Restore saved accounts before showing the main menu.
    loadAccounts();

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
                // Save again on exit as an extra safeguard.
                saveAccounts();
                cout << "Exiting application.\n";
                break;
            default:
                cout << "Invalid menu choice.\n";
        }
    } while (choice != 0);

    return 0;
}
