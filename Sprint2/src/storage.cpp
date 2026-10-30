#include "storage.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif

std::vector<Account> accounts;
int nextAccountNumber = 100001;
const std::string ACCOUNT_FILE = "accounts.txt";
const std::string TRANSACTION_FILE = "transactions.txt";

bool saveAccounts() {
    const std::string tempFile = ACCOUNT_FILE + ".tmp";
    std::ofstream out(tempFile, std::ios::trunc);
    if (!out) { std::cerr << "Could not write account data.\n"; return false; }
    for (const auto& a : accounts) {
        out << std::quoted(a.accountNumber) << ' ' << std::quoted(a.name) << ' '
            << std::quoted(a.pin) << ' ' << a.locked << ' ' << a.failedAttempts << ' '
            << std::setprecision(17) << a.balance << ' ' << a.frozen << ' '
            << a.closed << ' ' << a.deleted << '\n';
    }
    out.close();
    if (!out) { std::remove(tempFile.c_str()); return false; }
#ifdef _WIN32
    if (!MoveFileExA(tempFile.c_str(), ACCOUNT_FILE.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        std::remove(tempFile.c_str());
        std::cerr << "Could not replace account data file. Error: "
                  << GetLastError() << '\n';
        return false;
    }
#else
    if (std::rename(tempFile.c_str(), ACCOUNT_FILE.c_str()) != 0) {
        std::remove(tempFile.c_str());
        std::cerr << "Could not replace account data file.\n";
        return false;
    }
#endif
    return true;
}

bool loadAccounts() {
    std::ifstream in(ACCOUNT_FILE);
    if (!in) return true; // First run: no account file yet.
    std::vector<Account> loaded;
    std::string line;
    int largest = 100000;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        Account a;
        if (!(row >> std::quoted(a.accountNumber) >> std::quoted(a.name)
                  >> std::quoted(a.pin) >> a.locked >> a.failedAttempts >> a.balance)) {
            std::cerr << "Warning: skipped unreadable account record.\n";
            continue;
        }
        // The three status fields are optional to remain compatible with Sprint 1 files.
        if (!(row >> a.frozen)) { row.clear(); a.frozen = false; }
        if (!(row >> a.closed)) { row.clear(); a.closed = false; }
        if (!(row >> a.deleted)) { row.clear(); a.deleted = false; }
        if (a.accountNumber.empty() || a.name.empty() || a.pin.size() != 4 ||
            a.balance < 0.0 || a.failedAttempts < 0) {
            std::cerr << "Warning: skipped invalid account record.\n"; continue;
        }
        try {
            std::size_t used = 0;
            int n = std::stoi(a.accountNumber, &used);
            if (used != a.accountNumber.size() || n < 100001) continue;
            largest = std::max(largest, n);
        } catch (...) { continue; }
        loaded.push_back(a);
    }
    accounts = std::move(loaded);
    nextAccountNumber = largest + 1;
    return true;
}

bool appendTransaction(const Transaction& t) {
    std::ofstream out(TRANSACTION_FILE, std::ios::app);
    if (!out) { std::cerr << "Could not open transaction history.\n"; return false; }
    out << std::quoted(t.accountNumber) << ' ' << std::quoted(t.type) << ' '
        << std::fixed << std::setprecision(2) << t.amount << ' ' << t.balanceAfter
        << ' ' << std::quoted(t.timestamp) << '\n';
    return static_cast<bool>(out);
}

std::vector<Transaction> loadTransactions() {
    std::vector<Transaction> result;
    std::ifstream in(TRANSACTION_FILE);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        Transaction t;
        if (!(row >> std::quoted(t.accountNumber) >> std::quoted(t.type) >> t.amount >> t.balanceAfter)) continue;
        if (!(row >> std::quoted(t.timestamp))) t.timestamp = "Legacy record (no timestamp)";
        result.push_back(t);
    }
    return result;
}

int findAccountIndex(const std::string& number) {
    for (std::size_t i = 0; i < accounts.size(); ++i)
        if (accounts[i].accountNumber == number && !accounts[i].deleted) return static_cast<int>(i);
    return -1;
}
