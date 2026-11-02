#ifndef BANK_STORAGE_H
#define BANK_STORAGE_H
#include "models.h"
#include <string>
#include <vector>

extern std::vector<Account> accounts;
extern int nextAccountNumber;
extern const std::string ACCOUNT_FILE;
extern const std::string TRANSACTION_FILE;

bool loadAccounts();
bool saveAccounts();
bool appendTransaction(const Transaction& transaction);
std::vector<Transaction> loadTransactions();
int findAccountIndex(const std::string& accountNumber);
#endif
