#include "reports.h"

#include "audit.h"
#include "storage.h"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

using namespace std;

namespace {

struct Totals {
    int depositCount = 0;
    int withdrawalCount = 0;
    double totalDeposited = 0.0;
    double totalWithdrawn = 0.0;
};

const int TABLE_WIDTH = 63;

// All transactions of one account, oldest first.
vector<Transaction> transactionsFor(const string& accountNumber) {
    vector<Transaction> result;
    for (const Transaction& transaction : loadTransactions()) {
        if (transaction.accountNumber == accountNumber) {
            result.push_back(transaction);
        }
    }
    return result;
}

string shownTimestamp(const Transaction& transaction) {
    return transaction.timestamp.empty() ? "N/A" : transaction.timestamp;
}

Totals calculateTotals(const vector<Transaction>& transactions) {
    Totals totals;
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

string accountStatus(const Account& account) {
    if (account.deleted) return "DELETED";
    if (account.closed) return "CLOSED";
    if (account.frozen) return "FROZEN";
    if (account.locked) return "LOCKED (too many wrong PINs)";
    return "ACTIVE";
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
    out << string(TABLE_WIDTH, '-') << '\n';

    for (const Transaction& transaction : transactions) {
        out << left << setw(21) << shownTimestamp(transaction)
            << setw(12) << transaction.type
            << right << setw(14) << transaction.amount
            << setw(16) << transaction.balanceAfter << '\n';
    }
    out << right;
}

// Used for both the screen and the exported file.
void printStatement(ostream& out, const Account& account,
                    const vector<Transaction>& transactions) {
    out << fixed << setprecision(2);
    out << "Account Number: " << account.accountNumber << '\n';
    out << "Account Holder: " << account.name << '\n';
    out << "Generated on:   " << auditTimestamp() << "\n\n";

    printTransactionTable(out, transactions);

    Totals totals = calculateTotals(transactions);
    out << string(TABLE_WIDTH, '-') << '\n';
    out << "Total deposits:    Rs. " << totals.totalDeposited
        << " (" << totals.depositCount << " transactions)\n";
    out << "Total withdrawals: Rs. " << totals.totalWithdrawn
        << " (" << totals.withdrawalCount << " transactions)\n";
    out << "Current balance:   Rs. " << account.balance << '\n';
}

}  // namespace

void showMiniStatement(const Account& account) {
    const size_t MINI_COUNT = 5;

    vector<Transaction> all = transactionsFor(account.accountNumber);
    size_t start = all.size() > MINI_COUNT ? all.size() - MINI_COUNT : 0;
    vector<Transaction> recent(all.begin() + static_cast<ptrdiff_t>(start), all.end());

    cout << "\n========== MINI STATEMENT (LAST " << MINI_COUNT << ") ==========\n";
    cout << "Account Number: " << account.accountNumber << '\n';
    cout << "Account Holder: " << account.name << "\n\n";
    printTransactionTable(cout, recent);
    cout << fixed << setprecision(2);
    cout << string(TABLE_WIDTH, '-') << '\n';
    cout << "Current balance: Rs. " << account.balance << '\n';

    logAudit("MINI_STATEMENT_VIEWED", account.accountNumber,
             "records_shown=" + to_string(recent.size()));
}

void showFullHistory(const Account& account) {
    vector<Transaction> all = transactionsFor(account.accountNumber);

    cout << "\n========== TRANSACTION HISTORY ==========\n";
    printStatement(cout, account, all);

    logAudit("STATEMENT_VIEWED", account.accountNumber,
             "records_shown=" + to_string(all.size()));
}

void showAccountSummary(const Account& account) {
    vector<Transaction> all = transactionsFor(account.accountNumber);
    Totals totals = calculateTotals(all);

    cout << fixed << setprecision(2);
    cout << "\n========== ACCOUNT SUMMARY ==========\n";
    cout << "Account Number:      " << account.accountNumber << '\n';
    cout << "Account Holder:      " << account.name << '\n';
    cout << "Account Status:      " << accountStatus(account) << '\n';
    cout << "Current Balance:     Rs. " << account.balance << '\n';
    cout << "Total Transactions:  " << all.size() << '\n';
    cout << "Deposits:            " << totals.depositCount
         << " (Rs. " << totals.totalDeposited << ")\n";
    cout << "Withdrawals:         " << totals.withdrawalCount
         << " (Rs. " << totals.totalWithdrawn << ")\n";
    if (!all.empty()) {
        cout << "First Transaction:   " << shownTimestamp(all.front()) << '\n';
        cout << "Latest Transaction:  " << shownTimestamp(all.back()) << '\n';
    }
    cout << "=====================================\n";

    logAudit("SUMMARY_VIEWED", account.accountNumber);
}

void exportStatement(const Account& account) {
    vector<Transaction> all = transactionsFor(account.accountNumber);
    string fileName = "statement_" + account.accountNumber + ".txt";

    ofstream file(fileName);
    if (!file) {
        logAudit("STATEMENT_EXPORT_FAILED", account.accountNumber,
                 "reason=cannot_open_file");
        cout << "Error: Could not create " << fileName << ".\n";
        return;
    }

    file << "BANK MANAGEMENT SYSTEM - ACCOUNT STATEMENT\n";
    file << string(TABLE_WIDTH, '=') << '\n';
    printStatement(file, account, all);

    if (!file) {
        logAudit("STATEMENT_EXPORT_FAILED", account.accountNumber,
                 "reason=write_error");
        cout << "Error: Failed while writing " << fileName << ".\n";
        return;
    }

    logAudit("STATEMENT_EXPORTED", account.accountNumber,
             "file=" + fileName + " | records=" + to_string(all.size()));
    cout << "\nStatement exported to " << fileName << '\n';
}

void showReportsMenu(const Account& account) {
    int choice = -1;

    do {
        cout << "\n========== REPORTS ==========\n";
        cout << "1. Mini statement (last 5 transactions)\n";
        cout << "2. Full transaction history\n";
        cout << "3. Account summary report\n";
        cout << "4. Export statement to file\n";
        cout << "0. Back\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            if (cin.eof()) {
                return;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            choice = -1;
            continue;
        }

        switch (choice) {
            case 1: showMiniStatement(account); break;
            case 2: showFullHistory(account);   break;
            case 3: showAccountSummary(account); break;
            case 4: exportStatement(account);   break;
            case 0: break;
            default: cout << "Invalid menu choice.\n";
        }
    } while (choice != 0);
}
