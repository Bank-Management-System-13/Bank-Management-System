#include "test_framework.h"
#include "storage.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace {
void clearStorageFiles() {
    std::remove(ACCOUNT_FILE.c_str());
    std::remove((ACCOUNT_FILE + ".tmp").c_str());
    std::remove(TRANSACTION_FILE.c_str());
}
}

TEST(StorageSavesAndReloadsAccountFields) {
    clearStorageFiles();
    accounts.clear();

    Account account;
    account.accountNumber = "100101";
    account.name = "Test Customer";
    account.pin = "1234";
    account.balance = 725.50;
    account.failedAttempts = 2;
    account.locked = true;
    account.frozen = true;

    accounts.push_back(account);
    CHECK(saveAccounts());

    accounts.clear();
    CHECK(loadAccounts());
    CHECK(accounts.size() == 1);
    CHECK(accounts[0].accountNumber == "100101");
    CHECK(accounts[0].name == "Test Customer");
    CHECK(accounts[0].pin == "1234");
    CHECK(accounts[0].balance == 725.50);
    CHECK(accounts[0].failedAttempts == 2);
    CHECK(accounts[0].locked);
    CHECK(accounts[0].frozen);
}

TEST(StorageAppendsAndLoadsTimestampedTransactions) {
    clearStorageFiles();

    Transaction transaction{"100102", "DEPOSIT", 250.0, 450.0,
                             "2026-11-05 10:30:00"};
    CHECK(appendTransaction(transaction));

    const std::vector<Transaction> loaded = loadTransactions();
    CHECK(loaded.size() == 1);
    CHECK(loaded[0].accountNumber == "100102");
    CHECK(loaded[0].type == "DEPOSIT");
    CHECK(loaded[0].amount == 250.0);
    CHECK(loaded[0].balanceAfter == 450.0);
    CHECK(loaded[0].timestamp == "2026-11-05 10:30:00");
}

TEST(StorageReadsLegacyTransactionsWithoutTimestamp) {
    clearStorageFiles();

    {
        std::ofstream output(TRANSACTION_FILE);
        CHECK(output.is_open());
        output << "\"100103\" \"WITHDRAWAL\" 50.00 150.00\n";
    }

    const std::vector<Transaction> loaded = loadTransactions();
    CHECK(loaded.size() == 1);
    CHECK(loaded[0].accountNumber == "100103");
    CHECK(loaded[0].timestamp == "Legacy record (no timestamp)");
}

TEST(StorageDoesNotFindSoftDeletedAccounts) {
    accounts.clear();

    Account active;
    active.accountNumber = "100104";
    active.name = "Active";
    accounts.push_back(active);

    Account deleted;
    deleted.accountNumber = "100105";
    deleted.name = "Deleted";
    deleted.deleted = true;
    accounts.push_back(deleted);

    CHECK(findAccountIndex("100104") == 0);
    CHECK(findAccountIndex("100105") == -1);
    CHECK(findAccountIndex("999999") == -1);
}
