
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cctype>
#include <cmath>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

// Account structure
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

// Save accounts to persistent storage
bool saveAccounts() {
    ofstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "Error: Could not save accounts.\n";
        return false;
    }

    file << fixed << setprecision(2);

    for (const Account& account : accounts) {
        file << account.accountNumber << '|'
             << account.name << '|'
             << account.pin << '|'
             << account.balance << '|'
             << account.locked << '|'
             << account.failedAttempts << '\n';
    }

    if (!file) {
        cout << "Error writing account data.\n";
        return false;
    }

    return true;
}

// Load previously saved accounts
void loadAccounts() {
    ifstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "No previous account data found. Starting fresh.\n";
        return;
    }

    accounts.clear();

    string line;
    int highestAccountNumber = 100000;

    while (getline(file, line)) {
        stringstream ss(line);
        Account account;

        string balanceText;
        string lockedText;
        string attemptsText;

        if (!getline(ss, account.accountNumber, '|') ||
            !getline(ss, account.name, '|') ||
            !getline(ss, account.pin, '|') ||
            !getline(ss, balanceText, '|') ||
            !getline(ss, lockedText, '|') ||
            !getline(ss, attemptsText)) {
            cout << "Warning: Skipping invalid account record.\n";
            continue;
        }

        try {
            account.balance = stod(balanceText);
            account.locked = (lockedText == "1");
            account.failedAttempts = stoi(attemptsText);

            int number = stoi(account.accountNumber);

            if (!validPIN(account.pin) ||
                number < 100001 ||
                !isfinite(account.balance) ||
                account.balance < 0 ||
                account.failedAttempts < 0 ||
                (lockedText != "0" && lockedText != "1")) {
                cout << "Warning: Skipping invalid account record.\n";
                continue;
            }

            bool duplicate = false;

            for (const Account& existing : accounts) {
                if (existing.accountNumber == account.accountNumber) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
                cout << "Warning: Skipping duplicate account number.\n";
                continue;
            }

            accounts.push_back(account);

            if (number > highestAccountNumber) {
                highestAccountNumber = number;
            }
        } catch (...) {
            cout << "Warning: Skipping invalid account record.\n";
        }
    }

    nextAccountNumber = highestAccountNumber + 1;

    cout << "Loaded " << accounts.size()
         << " saved account(s).\n";
}

// Get current date and time
string currentDateTime() {
    time_t now = time(nullptr);
    tm* local = localtime(&now);

    if (local == nullptr) {
        return "Unknown";
    }

    stringstream ss;
    ss << put_time(local, "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

// Save a transaction in append mode
bool recordTransaction(
    const Account& account,
    const string& type,
    double amount
) {
    ofstream file(TRANSACTION_FILE, ios::app);

    if (!file) {
        cout << "Error: Could not save transaction.\n";
        return false;
    }

    file << account.accountNumber << '|'
         << type << '|'
         << fixed << setprecision(2) << amount << '|'
         << account.balance << '|'
         << currentDateTime() << '\n';

    if (!file) {
        cout << "Error writing transaction data.\n";
        return false;
    }

    return true;
}

// Validate a four-digit PIN
bool validPIN(const string& pin) {
    if (pin.length() != 4) {
        return false;
    }

    for (char ch : pin) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }

    return true;
}

// Read a PIN without displaying its digits
string readPIN() {
    while (true) {
        string pin;
        char ch;

        cout << "Enter a 4-digit PIN: " << flush;

#ifndef _WIN32
        termios oldSettings{};
        termios newSettings{};
        bool terminalChanged = false;

        if (tcgetattr(STDIN_FILENO, &oldSettings) == 0) {
            newSettings = oldSettings;
            newSettings.c_lflag &= ~(ICANON | ECHO);
            newSettings.c_cc[VMIN] = 1;
            newSettings.c_cc[VTIME] = 0;

            if (tcsetattr(STDIN_FILENO, TCSANOW, &newSettings) == 0) {
                terminalChanged = true;
            }
        }
#endif

        while (true) {
#ifdef _WIN32
            ch = _getch();
#else
            if (terminalChanged) {
                if (read(STDIN_FILENO, &ch, 1) != 1) {
                    continue;
                }
            } else {
                if (!cin.get(ch)) {
                    cin.clear();
                    continue;
                }
            }
#endif

            if (ch == '\r' || ch == '\n') {
                if (pin.length() == 4) {
                    cout << '\n';

#ifndef _WIN32
                    if (terminalChanged) {
                        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
                    }
#endif

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

#ifndef _WIN32
        if (terminalChanged) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        }
#endif
    }
}

// Create a new customer account
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

    if (!hasNonWhitespace ||
        account.name.find('|') != string::npos) {
        cout << "Enter a valid name without the | character.\n";
        return;
    }

    account.accountNumber = to_string(nextAccountNumber);
    account.pin = readPIN();

    if (!validPIN(account.pin)) {
        cout << "Invalid PIN.\n";
        return;
    }

    accounts.push_back(account);

    if (!saveAccounts()) {
        accounts.pop_back();
        cout << "Account creation cancelled because saving failed.\n";
        return;
    }

    nextAccountNumber++;

    cout << "\nAccount created successfully!\n";
    cout << "Account number: " << account.accountNumber << '\n';
    cout << "Opening balance: Rs. 0.00\n";
    cout << "Please remember your account number.\n";
}

// Display transaction history for a customer
void showTransactionHistory(const string& accountNumber) {
    ifstream file(TRANSACTION_FILE);

    if (!file) {
        cout << "No transaction history available.\n";
        return;
    }

    string line;
    bool found = false;

    cout << "\n========== TRANSACTION HISTORY ==========\n";
    cout << "Account | Type | Amount | Balance | Date/Time\n";

    while (getline(file, line)) {
        stringstream ss(line);

        string number;
        string type;
        string amount;
        string balance;
        string date;

        if (!getline(ss, number, '|') ||
            !getline(ss, type, '|') ||
            !getline(ss, amount, '|') ||
            !getline(ss, balance, '|') ||
            !getline(ss, date)) {
            continue;
        }

        if (number == accountNumber) {
            cout << number << " | "
                 << type << " | "
                 << amount << " | "
                 << balance << " | "
                 << date << '\n';

            found = true;
        }
    }

    if (!found) {
        cout << "No transactions found for this account.\n";
    }
}

// Customer menu after successful login
void customerMenu(Account& account) {
    int choice;

    do {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "1. Check balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. View transaction history\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            continue;
        }

        double amount;

        switch (choice) {
            case 1:
                cout << fixed << setprecision(2);
                cout << "Current balance: Rs. "
                     << account.balance << '\n';
                break;

            case 2: {
                cout << "Enter deposit amount: Rs. ";

                if (!(cin >> amount)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount.\n";
                    break;
                }

                if (!isfinite(amount) ||
                    amount <= 0 ||
                    amount > 1000000000000.0 ||
                    account.balance > 1000000000000.0 - amount) {
                    cout << "Enter a valid deposit amount.\n";
                    break;
                }

                double oldBalance = account.balance;
                account.balance += amount;

                if (!saveAccounts()) {
                    account.balance = oldBalance;
                    cout << "Deposit cancelled because saving failed.\n";
                    break;
                }

                if (!recordTransaction(account, "DEPOSIT", amount)) {
                    cout << "Warning: Deposit saved, but transaction "
                            "history could not be saved.\n";
                } else {
                    cout << "Deposit successful.\n";
                }

                cout << fixed << setprecision(2)
                     << "Current balance: Rs. "
                     << account.balance << '\n';
                break;
            }

            case 3: {
                cout << "Enter withdrawal amount: Rs. ";

                if (!(cin >> amount)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount.\n";
                    break;
                }

                if (!isfinite(amount) || amount <= 0) {
                    cout << "Enter a valid withdrawal amount.\n";
                    break;
                }

                if (amount > account.balance) {
                    cout << "Insufficient balance.\n";
                    break;
                }

                double oldBalance = account.balance;
                account.balance -= amount;

                if (!saveAccounts()) {
                    account.balance = oldBalance;
                    cout << "Withdrawal cancelled because saving failed.\n";
                    break;
                }

                if (!recordTransaction(account, "WITHDRAWAL", amount)) {
                    cout << "Warning: Withdrawal saved, but transaction "
                            "history could not be saved.\n";
                } else {
                    cout << "Withdrawal successful.\n";
                }

                cout << fixed << setprecision(2)
                     << "Current balance: Rs. "
                     << account.balance << '\n';
                break;
            }

            case 4:
                showTransactionHistory(account.accountNumber);
                break;

            case 0:
                cout << "Logged out successfully.\n";
                break;

            default:
                cout << "Invalid menu choice.\n";
        }

    } while (choice != 0);
}

// Customer login
void customerLogin() {
    string accountNumber;

    cout << "\nEnter account number: ";
    cin >> accountNumber;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (Account& account : accounts) {
        if (account.accountNumber == accountNumber) {
            if (account.locked) {
                cout << "This account is locked.\n";
                return;
            }

            string enteredPIN = readPIN();

            if (enteredPIN == account.pin) {
                account.failedAttempts = 0;

                if (!saveAccounts()) {
                    cout << "Warning: Could not save login status.\n";
                }

                cout << "\nLogin successful!\n";
                cout << "Welcome, " << account.name << "!\n";

                customerMenu(account);
            } else {
                account.failedAttempts++;

                cout << "\nIncorrect PIN.\n";

                if (account.failedAttempts >= 3) {
                    account.locked = true;
                    cout << "Account locked after 3 failed attempts.\n";
                }

                saveAccounts();
            }

            return;
        }
    }

    cout << "Account number not found.\n";
}

// Main program
int main() {
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
                cout << "Exiting application.\n";
                break;

            default:
                cout << "Invalid menu choice.\n";
        }

    } while (choice != 0);

    return 0;
}
