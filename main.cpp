
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <ctime>
#include <cctype>
#include <cmath>

using namespace std;

const string ACCOUNT_FILE = "accounts.txt";
const string TRANSACTION_FILE = "transactions.txt";

struct Account {
    int accountNumber;
    string name;
    string pin;
    bool locked;
    int failedAttempts;
    double balance;
};

vector<Account> accounts;

// Function declarations
bool validPIN(const string& pin);
void loadAccounts();
void saveAccounts();
void createAccount();
void customerLogin();
void customerMenu(int index);
void depositMoney(int index);
void withdrawMoney(int index);
void showTransactionHistory(int accountNumber);
void recordTransaction(int accountNumber, string type,
                       double amount, double balance);
string currentDateTime();

// Check for a four-digit PIN
bool validPIN(const string& pin) {
    if (pin.length() != 4) {
        return false;
    }

    for (char ch : pin) {
        if (!isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    return true;
}

// Get current date and time
string currentDateTime() {
    time_t now = time(nullptr);
    tm* timeInfo = localtime(&now);

    if (timeInfo == nullptr) {
        return "Unknown";
    }

    stringstream result;
    result << put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return result.str();
}

// Save all accounts to a text file
void saveAccounts() {
    ofstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "Error: Could not save accounts.\n";
        return;
    }

    for (const Account& a : accounts) {
        file << a.accountNumber << '|'
             << a.name << '|'
             << a.pin << '|'
             << a.locked << '|'
             << a.failedAttempts << '|'
             << fixed << setprecision(2) << a.balance << '\n';
    }

    file.close();
}

// Load saved accounts when the program starts
void loadAccounts() {
    accounts.clear();

    ifstream file(ACCOUNT_FILE);

    if (!file) {
        cout << "No saved accounts found. Starting fresh.\n";
        return;
    }

    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string numberText, name, pin, lockedText;
        string attemptsText, balanceText;

        if (!getline(ss, numberText, '|') ||
            !getline(ss, name, '|') ||
            !getline(ss, pin, '|') ||
            !getline(ss, lockedText, '|') ||
            !getline(ss, attemptsText, '|') ||
            !getline(ss, balanceText)) {
            continue;
        }

        try {
            Account a;
            a.accountNumber = stoi(numberText);
            a.name = name;
            a.pin = pin;
            a.locked = (stoi(lockedText) != 0);
            a.failedAttempts = stoi(attemptsText);
            a.balance = stod(balanceText);

            if (a.accountNumber <= 0 ||
                a.name.empty() ||
                a.name.find('|') != string::npos ||
                !validPIN(a.pin) ||
                a.failedAttempts < 0 ||
                a.failedAttempts > 3 ||
                !isfinite(a.balance) ||
                a.balance < 0) {
                continue;
            }

            bool duplicate = false;

            for (const Account& existing : accounts) {
                if (existing.accountNumber == a.accountNumber) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                accounts.push_back(a);
            }
        } catch (...) {
            // Ignore invalid records in the file.
        }
    }

    file.close();
    cout << accounts.size() << " account(s) loaded.\n";
}

// Add a transaction to the history file
void recordTransaction(int accountNumber, string type,
                       double amount, double balance) {
    ofstream file(TRANSACTION_FILE, ios::app);

    if (!file) {
        cout << "Warning: Could not save transaction history.\n";
        return;
    }

    file << accountNumber << '|'
         << type << '|'
         << fixed << setprecision(2) << amount << '|'
         << balance << '|'
         << currentDateTime() << '\n';

    file.close();
}

// Create a customer account
void createAccount() {
    Account a;

    cout << "\n--- Create Account ---\n";
    cout << "Enter full name: ";
    getline(cin, a.name);

    if (a.name.empty() || a.name.find('|') != string::npos) {
        cout << "Invalid name.\n";
        return;
    }

    int highestNumber = 1000;

    for (const Account& existing : accounts) {
        if (existing.accountNumber > highestNumber) {
            highestNumber = existing.accountNumber;
        }
    }

    a.accountNumber = highestNumber + 1;
    a.locked = false;
    a.failedAttempts = 0;
    a.balance = 0.0;

    while (true) {
        cout << "Create a four-digit PIN: ";
        getline(cin, a.pin);

        if (validPIN(a.pin)) {
            break;
        }

        cout << "PIN must contain exactly four digits.\n";
    }

    accounts.push_back(a);
    saveAccounts();

    recordTransaction(a.accountNumber, "ACCOUNT_CREATED",
                      0.0, a.balance);

    cout << "\nAccount created successfully!\n";
    cout << "Your account number is: "
         << a.accountNumber << '\n';
}

// Display transaction history for one account
void showTransactionHistory(int accountNumber) {
    ifstream file(TRANSACTION_FILE);

    cout << "\n--- Transaction History ---\n";

    if (!file) {
        cout << "No transaction history available.\n";
        return;
    }

    cout << left << setw(20) << "Transaction"
         << setw(12) << "Amount"
         << setw(12) << "Balance"
         << "Date and Time\n";

    cout << string(65, '-') << '\n';

    string line;
    bool found = false;

    while (getline(file, line)) {
        stringstream ss(line);
        string numberText, type, amountText;
        string balanceText, dateTime;

        if (!getline(ss, numberText, '|') ||
            !getline(ss, type, '|') ||
            !getline(ss, amountText, '|') ||
            !getline(ss, balanceText, '|') ||
            !getline(ss, dateTime)) {
            continue;
        }

        try {
            if (stoi(numberText) != accountNumber) {
                continue;
            }

            cout << left << setw(20) << type
                 << setw(12) << amountText
                 << setw(12) << balanceText
                 << dateTime << '\n';

            found = true;
        } catch (...) {
            continue;
        }
    }

    if (!found) {
        cout << "No transactions found.\n";
    }

    file.close();
}

// Deposit money
void depositMoney(int index) {
    double amount;

    cout << "Enter deposit amount: ";

    if (!(cin >> amount)) {
        cout << "Invalid amount.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (!isfinite(amount) || amount <= 0 ||
        amount > 1000000000.0 ||
        accounts[index].balance + amount > 1000000000.0) {
        cout << "Enter a valid positive amount.\n";
        return;
    }

    accounts[index].balance += amount;
    saveAccounts();

    recordTransaction(accounts[index].accountNumber,
                      "DEPOSIT", amount,
                      accounts[index].balance);

    cout << fixed << setprecision(2);
    cout << "Deposit successful. New balance: Rs. "
         << accounts[index].balance << '\n';
}

// Withdraw money
void withdrawMoney(int index) {
    double amount;

    cout << "Enter withdrawal amount: ";

    if (!(cin >> amount)) {
        cout << "Invalid amount.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (!isfinite(amount) || amount <= 0) {
        cout << "Enter a valid positive amount.\n";
        return;
    }

    if (amount > accounts[index].balance) {
        cout << "Insufficient balance.\n";
        return;
    }

    accounts[index].balance -= amount;

    if (accounts[index].balance < 0.005) {
        accounts[index].balance = 0.0;
    }

    saveAccounts();

    recordTransaction(accounts[index].accountNumber,
                      "WITHDRAWAL", amount,
                      accounts[index].balance);

    cout << fixed << setprecision(2);
    cout << "Withdrawal successful. Remaining balance: Rs. "
         << accounts[index].balance << '\n';
}

// Customer menu after login
void customerMenu(int index) {
    int choice;

    while (true) {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "Welcome, " << accounts[index].name << "!\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit Money\n";
        cout << "3. Withdraw Money\n";
        cout << "4. Transaction History\n";
        cout << "5. Logout\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid choice.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                cout << fixed << setprecision(2);
                cout << "Current balance: Rs. "
                     << accounts[index].balance << '\n';
                break;
            case 2:
                depositMoney(index);
                break;
            case 3:
                withdrawMoney(index);
                break;
            case 4:
                showTransactionHistory(
                    accounts[index].accountNumber);
                break;
            case 5:
                cout << "Logged out successfully.\n";
                return;
            default:
                cout << "Invalid choice.\n";
        }
    }
}

// Customer login
void customerLogin() {
    int accountNumber;

    cout << "\nEnter account number: ";

    if (!(cin >> accountNumber)) {
        cout << "Invalid account number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int index = -1;

    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].accountNumber == accountNumber) {
            index = static_cast<int>(i);
            break;
        }
    }

    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (accounts[index].locked) {
        cout << "Account is locked after failed login attempts.\n";
        return;
    }

    string enteredPIN;
    cout << "Enter your PIN: ";
    getline(cin, enteredPIN);

    if (enteredPIN == accounts[index].pin) {
        accounts[index].failedAttempts = 0;
        saveAccounts();

        cout << "Login successful!\n";
        customerMenu(index);
    } else {
        accounts[index].failedAttempts++;

        if (accounts[index].failedAttempts >= 3) {
            accounts[index].locked = true;
            cout << "Account locked after three incorrect PINs.\n";
        } else {
            cout << "Incorrect PIN. Attempts remaining: "
                 << 3 - accounts[index].failedAttempts << '\n';
        }

        saveAccounts();
    }
}

// Main program
int main() {
    loadAccounts();

    int choice;

    while (true) {
        cout << "\n====================================\n";
        cout << "       BANK MANAGEMENT SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Create Account\n";
        cout << "2. Customer Login\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid choice. Enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                customerLogin();
                break;
            case 3:
                cout << "Thank you for using the Bank Management System.\n";
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
    }
}
