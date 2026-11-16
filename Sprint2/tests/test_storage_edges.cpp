#include "test_framework.h"
#include "storage.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace {
void removePath(const std::string& path) {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
}
struct StorageEdgeScope {
    StorageEdgeScope() {
        removePath(ACCOUNT_FILE);
        removePath(ACCOUNT_FILE + ".tmp");
        removePath(TRANSACTION_FILE);
        accounts.clear();
    }
    ~StorageEdgeScope() {
        removePath(ACCOUNT_FILE);
        removePath(ACCOUNT_FILE + ".tmp");
        removePath(TRANSACTION_FILE);
        accounts.clear();
    }
};
}

TEST(StorageMissingAccountFileIsTreatedAsFirstRun) {
    StorageEdgeScope scope;
    Account existing;
    existing.accountNumber = "100101";
    accounts.push_back(existing);
    CHECK(loadAccounts());
    CHECK(accounts.size() == 1);
}

TEST(StorageSkipsMalformedAndInvalidAccountRowsAndReadsLegacyRows) {
    StorageEdgeScope scope;
    {
        std::ofstream out(ACCOUNT_FILE);
        out << "not an account row\n";
        out << "\"abc\" \"Bad Number\" \"1234\" 0 0 20.00 0 0 0\n";
        out << "\"100000\" \"Old Number\" \"1234\" 0 0 20.00 0 0 0\n";
        out << "\"100101\" \"Legacy Customer\" \"1234\" 0 0 20.50\n";
        out << "\"100102\" \"Bad PIN\" \"12\" 0 0 20.00 0 0 0\n";
        out << "\"100103\" \"Negative Balance\" \"1234\" 0 0 -1.00 0 0 0\n";
        out << "\"100104\" \"Negative Attempts\" \"1234\" 0 -1 20.00 0 0 0\n";
        out << "\"100105\" \"Status Customer\" \"1234\" 0 0 40.00 1 0 0\n";
    }

    CHECK(loadAccounts());
    CHECK(accounts.size() == 2);
    CHECK(accounts[0].accountNumber == "100101");
    CHECK(!accounts[0].frozen && !accounts[0].closed && !accounts[0].deleted);
    CHECK(accounts[1].accountNumber == "100105");
    CHECK(accounts[1].frozen);
    CHECK(nextAccountNumber == 100106);
}

TEST(StorageSaveAccountsFailsWhenTemporaryPathIsBlocked) {
    StorageEdgeScope scope;
    Account account;
    account.accountNumber = "100111";
    account.name = "Save Failure";
    account.pin = "1234";
    accounts.push_back(account);
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
    CHECK(!saveAccounts());
}

TEST(StorageAppendTransactionFailsWhenHistoryPathIsDirectory) {
    StorageEdgeScope scope;
    CHECK(std::filesystem::create_directory(TRANSACTION_FILE));
    CHECK(!appendTransaction({"100112", "DEPOSIT", 10.0, 10.0, "2026-11-16 10:00:00"}));
}

TEST(StorageTransactionLoaderSkipsMalformedLinesAndBlankRows) {
    StorageEdgeScope scope;
    {
        std::ofstream out(TRANSACTION_FILE);
        out << "\n";
        out << "this is malformed\n";
        out << "\"100113\" \"DEPOSIT\" 15.00 15.00 \"2026-11-16 10:00:00\"\n";
    }
    const auto loaded = loadTransactions();
    CHECK(loaded.size() == 1);
    CHECK(loaded[0].accountNumber == "100113");
    CHECK(loaded[0].amount == 15.0);
}

