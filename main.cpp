#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cctype>
#include <cmath>
#include <filesystem>
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


bool readInteger(const string& prompt, int& value) {
    cout << prompt;

    if (cin >> value) {
        return true;
    }

    if (cin.eof()) {
        return false;
    }

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Invalid input. Please enter a whole number.\n";
    return false;
}

bool isValidAccountNumber(const string& accountNumber) {
    if (accountNumber.empty()) {
        return false;
    }

    for (char ch : accountNumber) {
        if (!isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
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

    if (!saveAccounts()) {
        accounts.pop_back();
        cout << "Error: Account creation failed because saving was unsuccessful.\n";
        return;
}

nextAccountNumber++;

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


void deposit(Account& account) {
    double amount;

    cout << "\n========== DEPOSIT MONEY ==========\n";
    cout << "Enter deposit amount: Rs. ";

    if (!(cin >> amount)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a numeric amount.\n";
        return;
    }

    if (!isfinite(amount) || amount <= 0) {
        cout << "Deposit amount must be greater than zero.\n";
        return;
    }

    account.balance += amount;

    if (!saveAccounts()) {
        account.balance -= amount;
        cout << "Error saving account. Deposit cancelled.\n";
        return;
    }

    recordTransaction(account.accountNumber, "DEPOSIT",
                     amount, account.balance);

    cout << fixed << setprecision(2);
    cout << "Deposit successful!\n";
    cout << "Deposited: Rs. " << amount << '\n';
    cout << "Updated balance: Rs. " << account.balance << '\n';
}

void withdrawMoney(Account& account) {
    double amount;

    cout << "\n========== WITHDRAW MONEY ==========\n";
    cout << "Enter withdrawal amount: Rs. ";

    if (!(cin >> amount)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a numeric amount.\n";
        return;
    }

    if (!isfinite(amount) || amount <= 0) {
        cout << "Withdrawal amount must be greater than zero.\n";
        return;
    }

    if (amount > account.balance) {
        cout << "Withdrawal failed: Insufficient balance.\n";
        cout << fixed << setprecision(2);
        cout << "Available balance: Rs. "
             << account.balance << '\n';
        return;
    }

    account.balance -= amount;

    if (!saveAccounts()) {
        account.balance += amount;
        cout << "Error saving account. Withdrawal cancelled.\n";
        return;
    }

    recordTransaction(account.accountNumber, "WITHDRAWAL",
                      amount, account.balance);

    cout << fixed << setprecision(2);
    cout << "Withdrawal successful!\n";
    cout << "Withdrawn: Rs. " << amount << '\n';
    cout << "Updated balance: Rs. " << account.balance << '\n';
}


void customerMenu(size_t accountIndex) {
    int choice;

    do {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "1. Balance enquiry\n";
        cout << "2. Deposit money\n";
        cout << "3. Withdraw money\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            continue;
        }

        if (accountIndex >= accounts.size()) {
            cout << "Account not found.\n";
            return;
        }

        switch (choice) {
            case 1:
                balanceEnquiry(accounts[accountIndex]);
                break;
            case 2:
                deposit(accounts[accountIndex]);
                break;
            case 3:
                withdrawMoney(accounts[accountIndex]);
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

    if (!(cin >> accountNumber)) {
        if (cin.eof()) {
            return;
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Error reading account number.\n";
        return;
    }

    if (!isValidAccountNumber(accountNumber)) {
        cout << "Invalid account number. Use digits only.\n";
        return;
    }

    for (size_t i = 0; i < accounts.size(); ++i) {
        Account& account = accounts[i];

        if (account.accountNumber == accountNumber) {
            string enteredPIN = readPIN();

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
    cout << "Working directory: "
     << filesystem::current_path() << '\n';
    loadAccounts();

    int choice;

    do {
        cout << "\n====================================\n";
        cout << "      BANK MANAGEMENT SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Create customer account\n";
        cout << "2. Customer login\n";
        cout << "3. Deposit money\n";
        cout << "4. Withdraw money\n";
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

            case 3:
                cout << "\nPlease log in to access your account.\n";
                customerLogin();
                // Deposit should happen only after successful login.
                break;

            case 4:
                cout << "\nPlease log in to access your account.\n";
                customerLogin();
                // Withdrawal should happen only after successful login.
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
