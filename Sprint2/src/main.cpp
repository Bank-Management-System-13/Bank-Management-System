#include <iostream>
#include <limits>

#include "auth.h"
#include "admin.h"
#include "storage.h"

int main() {
    if (!loadAccounts()) {
        std::cerr << "Error loading account data.\n";
        return 1;
    }

    int choice = -1;

    while (choice != 0) {
        std::cout << "\n===== BANK MANAGEMENT SYSTEM - SPRINT 2 =====\n";
        std::cout << "1. Create Account\n";
        std::cout << "2. Customer Login\n";
        std::cout << "3. Admin Login\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid input. Enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                createAccount();
                break;

            case 2: {
                int index = customerLogin();
                if (index >= 0) {
                    customerMenu(index);
                }
                break;
            }

            case 3:
                adminMenu();
                break;

            case 0:
                std::cout << "Thank you for using the Bank Management System.\n";
                break;

            default:
                std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}
