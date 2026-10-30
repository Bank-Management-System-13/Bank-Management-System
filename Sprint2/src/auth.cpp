#include "auth.h"
#include "storage.h"
#include "security.h"
#include "audit.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <ctime>

namespace {
std::string timestampNow() {
    std::time_t now = std::time(nullptr); std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char b[32]; std::strftime(b, sizeof(b), "%Y-%m-%d %H:%M:%S", &tm); return b;
}
void record(const Account& a, const std::string& type, double amount) {
    appendTransaction({a.accountNumber, type, amount, a.balance, timestampNow()});
    writeAudit(a.accountNumber, type, a.accountNumber, "amount=" + std::to_string(amount));
}
}

void createAccount() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    Account a;
    std::cout << "Enter customer name: "; std::getline(std::cin, a.name);
    if (!validCustomerName(a.name)) { std::cout << "Name cannot be blank.\n"; return; }
    a.accountNumber = std::to_string(nextAccountNumber);
    a.pin = readPIN("Set a 4-digit PIN: ");
    if (!validPIN(a.pin)) { std::cout << "PIN must contain exactly four digits. Account not created.\n"; return; }
    accounts.push_back(a);
    if (!saveAccounts()) { accounts.pop_back(); std::cout << "Could not save account.\n"; return; }
    ++nextAccountNumber;
    writeAudit("SYSTEM", "ACCOUNT_CREATED", a.accountNumber, "New customer account");
    std::cout << "Account created. Account number: " << a.accountNumber << "\n";
}

int customerLogin() {
    std::string number;
    std::cout << "Enter account number: ";
    if (!(std::cin >> number)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return -1; }
    if (!validAccountNumber(number)) { std::cout << "Account number must contain digits only.\n"; return -1; }
    int index = findAccountIndex(number);
    if (index < 0) { std::cout << "Account not found.\n"; writeAudit("UNKNOWN", "LOGIN_FAILED", number, "Account not found"); return -1; }
    Account& a = accounts[static_cast<std::size_t>(index)];
    if (a.locked) { std::cout << "Account is locked after three incorrect PIN attempts. Contact admin.\n"; return -1; }
    if (a.frozen || a.closed) { std::cout << (a.frozen ? "Account is frozen.\n" : "Account is closed.\n"); return -1; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string pin = readPIN();
    if (pin != a.pin) {
        ++a.failedAttempts;
        writeAudit(number, "LOGIN_FAILED", number, "Incorrect PIN");

        if (a.failedAttempts >= 3) {
            a.locked = true;
            std::cout << "Three incorrect attempts. Account locked.\n";
            writeAudit(number, "ACCOUNT_LOCKED", number,
                       "Account locked after three incorrect PIN attempts");
        } else {
            std::cout << "Incorrect PIN. Attempts remaining: "
                      << (3 - a.failedAttempts) << "\n";
        }
        saveAccounts();
        return -1;
    }
    a.failedAttempts = 0;
    if (!saveAccounts()) std::cout << "Warning: login succeeded but attempt counter could not be saved.\n";
    writeAudit(number, "LOGIN_SUCCESS", number, "Customer authenticated");
    std::cout << "Login successful. Welcome, " << a.name << "!\n";
    return index;
}

void customerMenu(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= accounts.size()) return;
    int choice = -1;
    while (choice != 0) {
        Account& a = accounts[static_cast<std::size_t>(index)];
        std::cout << "\n--- CUSTOMER MENU ---\n1. Balance enquiry\n2. Deposit\n3. Withdraw\n0. Logout\n";
        if (!readMenuChoice("Choice: ", choice)) return;
        if (choice == 1) {
            std::cout << "Account: " << a.accountNumber << "\nName: " << a.name
                      << "\nBalance: Rs. " << std::fixed << std::setprecision(2) << a.balance << '\n';
            writeAudit(a.accountNumber, "BALANCE_ENQUIRY", a.accountNumber, "Balance viewed");
        } else if (choice == 2 || choice == 3) {
            if (a.frozen || a.closed || a.deleted) { std::cout << "This account cannot transact.\n"; continue; }
            double amount;
            if (!readPositiveAmount(choice == 2 ? "Deposit amount: Rs. " : "Withdrawal amount: Rs. ", amount)) continue;
            double old = a.balance;
            if (choice == 3 && amount > a.balance) {
                std::cout << "Insufficient balance.\n";
                writeAudit(a.accountNumber, "WITHDRAWAL_REJECTED",
                           a.accountNumber,
                           "Insufficient balance. Attempted: Rs. " +
                           std::to_string(amount) +
                           ", Available: Rs. " + std::to_string(a.balance));
                continue;
            }
            a.balance += (choice == 2 ? amount : -amount);
            if (!saveAccounts()) { a.balance = old; std::cout << "Save failed; transaction cancelled.\n"; continue; }
            record(a, choice == 2 ? "DEPOSIT" : "WITHDRAWAL", amount);
            std::cout << "Transaction successful. New balance: Rs. " << std::fixed << std::setprecision(2) << a.balance << '\n';
        } else if (choice != 0) std::cout << "Invalid choice.\n";
    }
    writeAudit(accounts[static_cast<std::size_t>(index)].accountNumber, "LOGOUT", accounts[static_cast<std::size_t>(index)].accountNumber, "Customer logged out");
}
