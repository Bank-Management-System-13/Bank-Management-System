#include "admin.h"
#include "storage.h"
#include "security.h"
#include "audit.h"
#include <iostream>
#include <iomanip>
#include <limits>

void searchAndViewAccounts() {
    std::string number;
    std::cout << "Enter account number to search (or ALL): "; std::cin >> number;
    bool found = false;
    for (const auto& a : accounts) {
        if (number != "ALL" && a.accountNumber != number) continue;
        if (a.deleted) continue;
        found = true;
        std::cout << "\nAccount: " << a.accountNumber << "\nName: " << a.name
                  << "\nBalance: Rs. " << std::fixed << std::setprecision(2) << a.balance
                  << "\nLogin locked: " << (a.locked ? "Yes" : "No")
                  << "\nFrozen: " << (a.frozen ? "Yes" : "No")
                  << "\nClosed: " << (a.closed ? "Yes" : "No") << '\n';
    }
    if (!found) std::cout << "No matching active record found.\n";
    writeAudit("ADMIN", "ACCOUNT_SEARCH", number, found ? "Record(s) viewed" : "No match");
}

void manageAccountStatus() {
    std::string number;
    std::cout << "Enter account number: "; std::cin >> number;
    int idx = findAccountIndex(number);
    if (idx < 0) { std::cout << "Account not found.\n"; return; }
    Account& a = accounts[static_cast<std::size_t>(idx)];
    int action;
    std::cout << "1. Freeze\n2. Unfreeze\n3. Close\n4. Reopen\n5. Soft delete\n6. Unlock login\nChoice: ";
    if (!(std::cin >> action)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "Invalid input.\n"; return; }
    Account old = a;
    std::string label;
    switch (action) {
        case 1: a.frozen = true; label = "ACCOUNT_FROZEN"; break;
        case 2: a.frozen = false; label = "ACCOUNT_UNFROZEN"; break;
        case 3: a.closed = true; label = "ACCOUNT_CLOSED"; break;
        case 4: a.closed = false; label = "ACCOUNT_REOPENED"; break;
        case 5: a.deleted = true; label = "ACCOUNT_SOFT_DELETED"; break;
        case 6: a.locked = false; a.failedAttempts = 0; label = "LOGIN_UNLOCKED"; break;
        default: std::cout << "Invalid action.\n"; return;
    }
    if (!saveAccounts()) { a = old; std::cout << "Could not save status change.\n"; return; }
    writeAudit("ADMIN", label, number, "Administrative account status change");
    std::cout << "Account status updated.\n";
}

void adminMenu() {
    if (!verifyAdminPIN()) { std::cout << "Admin authentication failed.\n"; writeAudit("UNKNOWN", "ADMIN_LOGIN_FAILED", "-", "Incorrect admin PIN"); return; }
    writeAudit("ADMIN", "ADMIN_LOGIN_SUCCESS", "-", "Admin authenticated");
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n--- ADMIN MENU ---\n1. Search/view accounts\n2. Freeze/close/delete/unlock account\n0. Logout\n";
        if (!readMenuChoice("Choice: ", choice)) return;
        if (choice == 1) searchAndViewAccounts();
        else if (choice == 2) manageAccountStatus();
        else if (choice != 0) std::cout << "Invalid choice.\n";
    }
    writeAudit("ADMIN", "ADMIN_LOGOUT", "-", "Admin logged out");
}
