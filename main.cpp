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
#include <ctime>
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
const string AUDIT_FILE = "audit.log";

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

// ---------------------------------------------------------------------------
// Issue #9: Transaction / audit logging
// ---------------------------------------------------------------------------

// Current local date and time as "YYYY-MM-DD HH:MM:SS".
string currentTimestamp() {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

// Formats a money value with exactly two decimal places (e.g. 500.00).
string formatMoney(double amount) {
    ostringstream out;
    out << fixed << setprecision(2) << amount;
    return out.str();
}

// Appends one line to audit.log:
//   [timestamp] EVENT | acct=<account number or -> | optional details
// Never pass PINs or other secrets in `details`.
bool logAudit(const string& event,
              const string& accountNumber,
              const string& details = "") {
    ofstream file(AUDIT_FILE, ios::app);
    if (!file) {
        cerr << "Warning: Could not open the audit log.\n";
        return false;
    }

    file << '[' << currentTimestamp() << "] "
         << left << setw(22) << event
         << " | acct=" << (accountNumber.empty() ? "-" : accountNumber);
    if (!details.empty()) {
        file << " | " << details;
    }
    file << '\n';

    if (!file) {
        cerr << "Warning: Failed while writing to the audit log.\n";
        return false;
    }
    return true;
}

// Issue #6 / #9: Append a transaction record (with a timestamp as the last field).
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
         << balanceAfter << ' '
         << quoted(currentTimestamp()) << '\n';

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
        logAudit("ACCOUNT_CREATE_FAILED", account.accountNumber, "reason=save_error");
        cout << "Error: Account creation failed because saving was unsuccessful.\n";
        return;
}

nextAccountNumber++;

    logAudit("ACCOUNT_CREATED", account.accountNumber,
             "initial_balance=" + formatMoney(account.balance));

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
        logAudit("DEPOSIT_FAILED", account.accountNumber,
                 "amount=" + formatMoney(amount) + " | reason=save_error");
        cout << "Error saving account. Deposit cancelled.\n";
        return;
    }

    bool depositLogged = recordTransaction(account.accountNumber, "DEPOSIT",
                     amount, account.balance);

    cout << fixed << setprecision(2);
    logAudit("DEPOSIT_SUCCESS", account.accountNumber,
             "amount=" + formatMoney(amount) +
             " | balance=" + formatMoney(account.balance));
    if (!depositLogged) {
        logAudit("TRANSACTION_LOG_FAILED", account.accountNumber, "type=DEPOSIT");
    }

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
        logAudit("WITHDRAWAL_FAILED", account.accountNumber,
                 "amount=" + formatMoney(amount) +
                 " | reason=insufficient_balance | balance=" + formatMoney(account.balance));
        cout << "Withdrawal failed: Insufficient balance.\n";
        cout << fixed << setprecision(2);
        cout << "Available balance: Rs. "
             << account.balance << '\n';
        return;
    }

    account.balance -= amount;

    if (!saveAccounts()) {
        account.balance += amount;
        logAudit("WITHDRAWAL_FAILED", account.accountNumber,
                 "amount=" + formatMoney(amount) + " | reason=save_error");
        cout << "Error saving account. Withdrawal cancelled.\n";
        return;
    }

    bool withdrawalLogged = recordTransaction(account.accountNumber, "WITHDRAWAL",
                      amount, account.balance);

    cout << fixed << setprecision(2);
    logAudit("WITHDRAWAL_SUCCESS", account.accountNumber,
             "amount=" + formatMoney(amount) +
             " | balance=" + formatMoney(account.balance));
    if (!withdrawalLogged) {
        logAudit("TRANSACTION_LOG_FAILED", account.accountNumber, "type=WITHDRAWAL");
    }

    cout << "Withdrawal successful!\n";
    cout << "Withdrawn: Rs. " << amount << '\n';
    cout << "Updated balance: Rs. " << account.balance << '\n';
}


// ---------------------------------------------------------------------------
// Issue #12: Account and transaction reports
// ---------------------------------------------------------------------------

struct Transaction {
    string accountNumber;
    string type;
    double amount = 0.0;
    double balanceAfter = 0.0;
    string timestamp;
};

struct TransactionTotals {
    int depositCount = 0;
    int withdrawalCount = 0;
    double totalDeposited = 0.0;
    double totalWithdrawn = 0.0;
};

// Reads transactions.txt and returns the records of one account, oldest first.
// Records written before timestamps existed are shown with "N/A".
vector<Transaction> loadTransactionsFor(const string& accountNumber) {
    vector<Transaction> result;
    ifstream file(TRANSACTION_FILE);
    if (!file) {
        return result;  // No transactions yet.
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        istringstream in(line);
        Transaction transaction;
        if (!(in >> quoted(transaction.accountNumber)
                 >> quoted(transaction.type)
                 >> transaction.amount
                 >> transaction.balanceAfter)) {
            continue;  // Skip malformed lines.
        }
        if (!(in >> quoted(transaction.timestamp))) {
            transaction.timestamp = "N/A";
        }

        if (transaction.accountNumber == accountNumber) {
            result.push_back(transaction);
        }
    }
    return result;
}

TransactionTotals calculateTotals(const vector<Transaction>& transactions) {
    TransactionTotals totals;
    for (const Transaction& transaction : transactions) {
        if (transaction.type == "DEPOSIT") {
            ++totals.depositCount;
            totals.totalDeposited += transaction.amount;
        } else if (transaction.type == "WITHDRAWAL") {
            ++totals.withdrawalCount;
            totals.totalWithdrawn += transaction.amount;
        }
    }
    return totals;
}

void printTransactionTable(ostream& out, const vector<Transaction>& transactions) {
    out << fixed << setprecision(2);

    if (transactions.empty()) {
        out << "No transactions found.\n";
        return;
    }

    out << left << setw(21) << "Date & Time"
        << setw(12) << "Type"
        << right << setw(14) << "Amount (Rs.)"
        << setw(16) << "Balance (Rs.)" << '\n';
    out << string(63, '-') << '\n';

    for (const Transaction& transaction : transactions) {
        out << left << setw(21) << transaction.timestamp
            << setw(12) << transaction.type
            << right << setw(14) << transaction.amount
            << setw(16) << transaction.balanceAfter << '\n';
    }
    out << right;
}

// Full statement: account details, every transaction, and totals.
// Works for any output stream, so it is used for both the screen and the export file.
void printStatement(ostream& out, const Account& account,
                    const vector<Transaction>& transactions) {
    out << fixed << setprecision(2);
    out << "Account Number: " << account.accountNumber << '\n';
    out << "Account Holder: " << account.name << '\n';
    out << "Generated on:   " << currentTimestamp() << "\n\n";

    printTransactionTable(out, transactions);

    TransactionTotals totals = calculateTotals(transactions);
    out << string(63, '-') << '\n';
    out << "Total deposits:    Rs. " << totals.totalDeposited
        << " (" << totals.depositCount << " transactions)\n";
    out << "Total withdrawals: Rs. " << totals.totalWithdrawn
        << " (" << totals.withdrawalCount << " transactions)\n";
    out << "Current balance:   Rs. " << account.balance << '\n';
}

// Last 5 transactions of the logged-in account.
void showMiniStatement(const Account& account) {
    const size_t MINI_COUNT = 5;

    vector<Transaction> all = loadTransactionsFor(account.accountNumber);
    size_t start = all.size() > MINI_COUNT ? all.size() - MINI_COUNT : 0;
    vector<Transaction> recent(all.begin() + static_cast<ptrdiff_t>(start), all.end());

    cout << "\n========== MINI STATEMENT (LAST " << MINI_COUNT << ") ==========\n";
    cout << "Account Number: " << account.accountNumber << '\n';
    cout << "Account Holder: " << account.name << "\n\n";
    printTransactionTable(cout, recent);
    cout << fixed << setprecision(2);
    cout << string(63, '-') << '\n';
    cout << "Current balance: Rs. " << account.balance << '\n';

    logAudit("MINI_STATEMENT_VIEWED", account.accountNumber,
             "records_shown=" + to_string(recent.size()));
}

// Complete transaction history of the logged-in account.
void showFullHistory(const Account& account) {
    vector<Transaction> all = loadTransactionsFor(account.accountNumber);

    cout << "\n========== TRANSACTION HISTORY ==========\n";
    printStatement(cout, account, all);

    logAudit("STATEMENT_VIEWED", account.accountNumber,
             "records_shown=" + to_string(all.size()));
}

// One-page account summary report.
void showAccountSummary(const Account& account) {
    vector<Transaction> all = loadTransactionsFor(account.accountNumber);
    TransactionTotals totals = calculateTotals(all);

    cout << fixed << setprecision(2);
    cout << "\n========== ACCOUNT SUMMARY ==========\n";
    cout << "Account Number:      " << account.accountNumber << '\n';
    cout << "Account Holder:      " << account.name << '\n';
    cout << "Account Status:      " << (account.locked ? "LOCKED" : "ACTIVE") << '\n';
    cout << "Current Balance:     Rs. " << account.balance << '\n';
    cout << "Total Transactions:  " << all.size() << '\n';
    cout << "Deposits:            " << totals.depositCount
         << " (Rs. " << totals.totalDeposited << ")\n";
    cout << "Withdrawals:         " << totals.withdrawalCount
         << " (Rs. " << totals.totalWithdrawn << ")\n";
    if (!all.empty()) {
        cout << "First Transaction:   " << all.front().timestamp << '\n';
        cout << "Latest Transaction:  " << all.back().timestamp << '\n';
    }
    cout << "=====================================\n";

    logAudit("SUMMARY_VIEWED", account.accountNumber);
}

// Writes the full statement to statement_<account number>.txt.
void exportStatement(const Account& account) {
    vector<Transaction> all = loadTransactionsFor(account.accountNumber);
    string fileName = "statement_" + account.accountNumber + ".txt";

    ofstream file(fileName);
    if (!file) {
        logAudit("STATEMENT_EXPORT_FAILED", account.accountNumber, "reason=cannot_open_file");
        cout << "Error: Could not create " << fileName << ".\n";
        return;
    }

    file << "BANK MANAGEMENT SYSTEM - ACCOUNT STATEMENT\n";
    file << string(63, '=') << '\n';
    printStatement(file, account, all);

    if (!file) {
        logAudit("STATEMENT_EXPORT_FAILED", account.accountNumber, "reason=write_error");
        cout << "Error: Failed while writing " << fileName << ".\n";
        return;
    }

    logAudit("STATEMENT_EXPORTED", account.accountNumber,
             "file=" + fileName + " | records=" + to_string(all.size()));
    cout << "\nStatement exported to " << fileName << '\n';
}


void customerMenu(size_t accountIndex) {
    int choice;

    do {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "1. Balance enquiry\n";
        cout << "2. Deposit money\n";
        cout << "3. Withdraw money\n";
        cout << "4. Mini statement (last 5 transactions)\n";
        cout << "5. Full transaction history\n";
        cout << "6. Account summary report\n";
        cout << "7. Export statement to file\n";
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
            case 4:
                showMiniStatement(accounts[accountIndex]);
                break;
            case 5:
                showFullHistory(accounts[accountIndex]);
                break;
            case 6:
                showAccountSummary(accounts[accountIndex]);
                break;
            case 7:
                exportStatement(accounts[accountIndex]);
                break;
            case 0:
                logAudit("LOGOUT", accounts[accountIndex].accountNumber);
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
                logAudit("LOGIN_SUCCESS", account.accountNumber);
                cout << "\nLogin successful!\n";
                cout << "Welcome, " << account.name << "!\n";
                customerMenu(i);
            } else {
                logAudit("LOGIN_FAILED", account.accountNumber, "reason=incorrect_pin");
                cout << "\nIncorrect PIN.\n";
            }

            return;
        }
    }

    logAudit("LOGIN_FAILED", accountNumber, "reason=unknown_account");
    cout << "Account number not found.\n";
}



int main() {
    cout << "Working directory: "
     << filesystem::current_path() << '\n';
    loadAccounts();
    logAudit("APP_START", "", "accounts_loaded=" + to_string(accounts.size()));

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
                logAudit("APP_EXIT", "");
                cout << "Exiting application.\n";
                break;

            default:
                cout << "Invalid menu choice.\n";
        }

    } while (choice != 0);

    return 0;
}
