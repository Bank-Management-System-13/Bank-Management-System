#ifndef BANK_MODELS_H
#define BANK_MODELS_H
#include <string>
#include <vector>

struct Account {
    std::string accountNumber;
    std::string name;
    std::string pin;
    bool locked = false;          // Login lockout after repeated wrong PINs
    int failedAttempts = 0;
    double balance = 0.0;
    bool frozen = false;          // Admin freeze: blocks customer operations
    bool closed = false;          // Closed accounts cannot transact or log in
    bool deleted = false;         // Soft delete: retained for audit/history
};

struct Transaction {
    std::string accountNumber;
    std::string type;
    double amount = 0.0;
    double balanceAfter = 0.0;
    std::string timestamp;
};

#endif
