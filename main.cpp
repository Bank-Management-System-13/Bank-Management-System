
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
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

// Save all accounts to a file.
void saveAccounts() {
    ofstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "Error: Could not save accounts.\n";
        return;
    }

    file << fixed << setprecision(2);

    for (const Account& a : accounts) {
        file << a.accountNumber << '|'
             << a.name << '|'
             << a.pin << '|'
             << a.balance << '|'
             << a.locked << '|'
             << a.failedAttempts << '\n';
    }
}

// Load saved accounts when the application starts.
void loadAccounts() {
    ifstream file(ACCOUNT_FILE);
    if (!file) return;

    accounts.clear();
    string line;
    int highestAccountNumber = 100000;

    while (getline(file, line)) {
        stringstream ss(line);
        Account a;
        string balance, locked, attempts;

        if (!getline(ss, a.accountNumber, '|') ||
            !getline(ss, a.name, '|') ||
            !getline(ss, a.pin, '|') ||
            !getline(ss, balance, '|') ||
            !getline(ss, locked, '|') ||
            !getline(ss, attempts)) {
            continue;
        }

        try {
            a.balance = stod(balance);
            a.locked = (locked == "1");
            a.failedAttempts = stoi(attempts);

            accounts.push_back(a);

            int number = stoi(a.accountNumber);
            if (number > highestAccountNumber)
                highestAccountNumber = number;
        } catch (...) {
            cout << "Warning: Skipping an invalid account record.\n";
        }
    }

    nextAccountNumber = highestAccountNumber + 1;
}

// Return the current date and time.
string currentDateTime() {
    time_t now = time(nullptr);
    tm* local = localtime(&now);

    if (!local) return "Unknown";

    stringstream ss;
    ss << put_time(local, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Append a transaction to the transaction history file.
void recordTransaction(
    const Account& a,
    const string& type,
    double amount
) {
    ofstream file(TRANSACTION_FILE, ios::app);

    if (!file) {
        cout << "Error: Could not save transaction.\n";
        return;
    }

    file << a.accountNumber << '|'
         << type << '|'
         << fixed << setprecision(2) << amount << '|'
         << a.balance << '|'
         << currentDateTime() << '\n';
}

// Display transaction history for one account.
void showTransactionHistory(const string& accountNumber) {
    ifstream file(TRANSACTION_FILE);

    if (!file) {
        cout << "No transaction history available.\n";
        return;
    }

    string line;
    bool found = false;

    cout << "\n--- Transaction History ---\n";
    cout << "Account | Type | Amount | Balance | Date/Time\n";

    while (getline(file, line)) {
        stringstream ss(line);
        string number, type, amount, balance, date;

        if (!getline(ss, number, '|') ||
            !getline(ss, type, '|') ||
            !getline(ss, amount, '|') ||
            !getline(ss, balance, '|') ||
            !getline(ss, date)) {
            continue;
        }

        if (number == accountNumber) {
            cout << number << " | " << type
                 << " | " << amount
                 << " | " << balance
                 << " | " << date << '\n';
            found = true;
        }
    }

    if (!found)
        cout << "No transactions for this account.\n";
}

bool validPIN(const string& pin) {
    if (pin.length() != 4) return false;

    for (char ch : pin) {
        if (ch < '0' || ch > '9') return false;
    }

    return true;
}

// Read a PIN without displaying its digits.
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

    account.pin = readPIN();

    accounts.push_back(account);
    nextAccountNumber++;

    saveAccounts();

    cout << "\nAccount created successfully!\n";
    cout << "Account number: " << account.accountNumber << '\n';
    cout << "Opening balance: Rs. 0.00\n";
    cout << "Please remember your account number.\n";
}

void customerMenu(Account& account) {
    int choice;

    do {
        cout << "\n--- Customer Menu ---\n";
        cout << "1. Check balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. View transaction history\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }

        double amount;

        switch (choice) {
            case 1:
                cout << fixed << setprecision(2);
                cout << "Current balance: Rs. "
                     << account.balance << '\n';
                break;

            case 2:
                cout << "Enter deposit amount: Rs. ";

                if (!(cin >> amount)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount.\n";
                    break;
                }

                if (!isfinite(amount) || amount <= 0) {
                    cout << "Amount must be positive and valid.\n";
                    break;
                }

                account.balance += amount;
                saveAccounts();
                recordTransaction(account, "DEPOSIT", amount);

                cout << "Deposit successful.\n";
                cout << "New balance: Rs. "
                     << fixed << setprecision(2)
                     << account.balance << '\n';
                break;

            case 3:
                cout << "Enter withdrawal amount: Rs. ";

                if (!(cin >> amount)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount.\n";
                    break;
                }

                if (!isfinite(amount) || amount <= 0) {
                    cout << "Amount must be positive and valid.\n";
                    break;
                }

                if (amount > account.balance) {
                    cout << "Insufficient balance.\n";
                    break;
                }

                account.balance -= amount;
                saveAccounts();
                recordTransaction(account, "WITHDRAWAL", amount);

                cout << "Withdrawal successful.\n";
                cout << "New balance: Rs. "
                     << fixed << setprecision(2)
                     << account.balance << '\n';
                break;

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

void customerLogin() {
    string accountNumber;

    cout << "\nEnter account number: ";
    cin >> accountNumber;

    // Clear the newline before reading the PIN.
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
                saveAccounts();

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

int main() {
    loadAccounts();

    cout << "Loaded " << accounts.size()
         << " saved account(s).\n";

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
                saveAccounts();
                cout << "Exiting application.\n";
                break;

            default:
                cout << "Invalid menu choice.\n";
        }

    } while (choice != 0);

    return 0;
}
