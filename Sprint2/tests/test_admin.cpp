#include "test_framework.h"
#include "admin.h"
#include "storage.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <system_error>

TEST(AdminCanFreezeAnExistingAccount) {
    std::remove(ACCOUNT_FILE.c_str());
    accounts.clear();

    Account account;
    account.accountNumber = "300001";
    account.name = "Admin Test";
    account.pin = "1234";
    accounts.push_back(account);

    std::istringstream input("300001\n1\n");
    std::ostringstream output;
    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(output.rdbuf());

    manageAccountStatus();

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);

    CHECK(accounts.size() == 1);
    CHECK(accounts[0].frozen);
}

TEST(AdminCanSoftDeleteAnAccount) {
    std::remove(ACCOUNT_FILE.c_str());
    accounts.clear();

    Account account;
    account.accountNumber = "300002";
    account.name = "Delete Test";
    account.pin = "1234";
    accounts.push_back(account);

    std::istringstream input("300002\n5\n");
    std::ostringstream output;
    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(output.rdbuf());

    manageAccountStatus();

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);

    CHECK(accounts.size() == 1);
    CHECK(accounts[0].deleted);
    CHECK(findAccountIndex("300002") == -1);
}

// ---------------------------------------------------------------------------
// Additional admin tests: search/view, freeze/unfreeze, close/reopen,
// soft delete, login unlock, error handling and admin authentication.
// Each test starts from a clean state and cleans up after itself.
// ---------------------------------------------------------------------------

namespace {

// Feeds `text` to std::cin and captures std::cout / std::cerr while in scope.
class ConsoleCapture {
public:
    explicit ConsoleCapture(const std::string& text)
        : input_(text),
          oldIn_(std::cin.rdbuf(input_.rdbuf())),
          oldOut_(std::cout.rdbuf(output_.rdbuf())),
          oldErr_(std::cerr.rdbuf(errors_.rdbuf())) {}

    ~ConsoleCapture() {
        std::cin.rdbuf(oldIn_);
        std::cout.rdbuf(oldOut_);
        std::cerr.rdbuf(oldErr_);
    }

    ConsoleCapture(const ConsoleCapture&) = delete;
    ConsoleCapture& operator=(const ConsoleCapture&) = delete;

    std::string output() const { return output_.str(); }

private:
    std::istringstream input_;
    std::ostringstream output_;
    std::ostringstream errors_;
    std::streambuf* oldIn_;
    std::streambuf* oldOut_;
    std::streambuf* oldErr_;
};

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

std::string readTextFile(const std::string& path) {
    std::ifstream in(path);
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

void resetAdminTestState() {
    std::remove("audit.log");
    std::remove(ACCOUNT_FILE.c_str());
    std::error_code ignored;
    std::filesystem::remove_all(ACCOUNT_FILE + ".tmp", ignored);
    accounts.clear();
}

// Cleans the account file, audit log and in-memory accounts before and after a test,
// even when a CHECK throws.
struct AdminTestScope {
    AdminTestScope() { resetAdminTestState(); }
    ~AdminTestScope() { resetAdminTestState(); }
};

void addAccount(const std::string& number, const std::string& name, double balance = 0.0) {
    Account account;
    account.accountNumber = number;
    account.name = name;
    account.pin = "1234";
    account.balance = balance;
    accounts.push_back(account);
}

std::string runSearch(const std::string& input) {
    ConsoleCapture io(input);
    searchAndViewAccounts();
    return io.output();
}

std::string runManageStatus(const std::string& input) {
    ConsoleCapture io(input);
    manageAccountStatus();
    return io.output();
}

}  // namespace

// ----- Search and view -----------------------------------------------------

TEST(Admin_SearchShowsDetailsOfOneAccount) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 1250.5);
    addAccount("300102", "Bala Nair", 80.0);

    std::string out = runSearch("300101\n");

    CHECK(contains(out, "Account: 300101"));
    CHECK(contains(out, "Name: Asha Rao"));
    CHECK(contains(out, "Balance: Rs. 1250.50"));
    CHECK(contains(out, "Login locked: No"));
    CHECK(contains(out, "Frozen: No"));
    CHECK(contains(out, "Closed: No"));
    CHECK(!contains(out, "300102"));
}

TEST(Admin_SearchAllListsEveryActiveAccount) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 10.0);
    addAccount("300102", "Bala Nair", 20.0);

    std::string out = runSearch("ALL\n");

    CHECK(contains(out, "Account: 300101"));
    CHECK(contains(out, "Account: 300102"));
}

TEST(Admin_SearchAllSkipsSoftDeletedAccounts) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");
    addAccount("300102", "Bala Nair");
    addAccount("300103", "Chitra Iyer");
    accounts[1].deleted = true;

    std::string out = runSearch("ALL\n");

    CHECK(contains(out, "Account: 300101"));
    CHECK(contains(out, "Account: 300103"));
    CHECK(!contains(out, "300102"));
}

TEST(Admin_SearchUnknownAccountReportsNoMatch) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");

    std::string out = runSearch("999999\n");

    CHECK(contains(out, "No matching active record found."));
    CHECK(!contains(out, "Account: "));
}

TEST(Admin_SearchHidesSoftDeletedAccount) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");
    accounts[0].deleted = true;

    std::string out = runSearch("300101\n");

    CHECK(contains(out, "No matching active record found."));
}

TEST(Admin_SearchShowsFrozenClosedAndLockedFlags) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");
    accounts[0].frozen = true;
    accounts[0].closed = true;
    accounts[0].locked = true;

    std::string out = runSearch("300101\n");

    CHECK(contains(out, "Login locked: Yes"));
    CHECK(contains(out, "Frozen: Yes"));
    CHECK(contains(out, "Closed: Yes"));
}

TEST(Admin_SearchIsWrittenToAuditLog) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");

    runSearch("300101\n");
    std::string log = readTextFile("audit.log");

    CHECK(contains(log, "ACCOUNT_SEARCH"));
    CHECK(contains(log, "acct=300101"));
    CHECK(contains(log, "Record(s) viewed"));
}

// ----- Freeze and unfreeze -------------------------------------------------

TEST(Admin_UnfreezeClearsFrozenFlag) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 500.0);
    accounts[0].frozen = true;

    std::string out = runManageStatus("300101\n2\n");

    CHECK(contains(out, "Account status updated."));
    CHECK(!accounts[0].frozen);
    CHECK(accounts[0].balance == 500.0);
}

TEST(Admin_FreezingTwiceKeepsAccountFrozen) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");

    runManageStatus("300101\n1\n");
    runManageStatus("300101\n1\n");

    CHECK(accounts[0].frozen);
}

// ----- Close and reopen ----------------------------------------------------

TEST(Admin_CloseAndReopenChangeOnlyClosedFlag) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 750.0);
    accounts[0].frozen = true;

    std::string closeOut = runManageStatus("300101\n3\n");
    CHECK(contains(closeOut, "Account status updated."));
    CHECK(accounts[0].closed);
    CHECK(accounts[0].frozen);
    CHECK(accounts[0].balance == 750.0);

    std::string reopenOut = runManageStatus("300101\n4\n");
    CHECK(contains(reopenOut, "Account status updated."));
    CHECK(!accounts[0].closed);
    CHECK(accounts[0].frozen);
    CHECK(accounts[0].balance == 750.0);
}

// ----- Soft delete ---------------------------------------------------------

TEST(Admin_SoftDeleteKeepsRecordAndBalance) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 500.0);

    runManageStatus("300101\n5\n");

    CHECK(accounts.size() == 1);
    CHECK(accounts[0].deleted);
    CHECK(accounts[0].balance == 500.0);
    CHECK(accounts[0].name == "Asha Rao");
}

TEST(Admin_SoftDeletedAccountCannotBeManagedAgain) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");
    accounts[0].deleted = true;

    std::string out = runManageStatus("300101\n1\n");

    CHECK(contains(out, "Account not found."));
    CHECK(!accounts[0].frozen);
}

TEST(Admin_SoftDeleteIsPersistedAfterReload) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 500.0);

    runManageStatus("300101\n5\n");
    CHECK(loadAccounts());

    CHECK(accounts.size() == 1);
    CHECK(accounts[0].deleted);
    CHECK(accounts[0].balance == 500.0);
    CHECK(findAccountIndex("300101") == -1);
}

// ----- Unlock login --------------------------------------------------------

TEST(Admin_UnlockLoginResetsLockAndFailedAttempts) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 300.0);
    accounts[0].locked = true;
    accounts[0].failedAttempts = 3;

    std::string out = runManageStatus("300101\n6\n");

    CHECK(contains(out, "Account status updated."));
    CHECK(!accounts[0].locked);
    CHECK(accounts[0].failedAttempts == 0);
    CHECK(accounts[0].balance == 300.0);
}

// ----- Persistence and audit -----------------------------------------------

TEST(Admin_StatusChangesArePersistedAfterReload) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao", 100.0);

    runManageStatus("300101\n1\n");
    runManageStatus("300101\n3\n");
    CHECK(loadAccounts());

    CHECK(accounts.size() == 1);
    CHECK(accounts[0].frozen);
    CHECK(accounts[0].closed);
    CHECK(!accounts[0].deleted);
}

TEST(Admin_StatusChangesAreAuditLogged) {
    AdminTestScope scope;
    addAccount("300101", "Asha Rao");

    runManageStatus("300101\n1\n");
    runManageStatus("300101\n2\n");
    runManageStatus("300101\n3\n");
    runManageStatus("300101\n4\n");
    runManageStatus("300101\n6\n");
    runManageStatus("300101\n5\n");
    std::string log = readTextFile("audit.log");

    CHECK(contains(log, "ACCOUNT_FROZEN"));
    CHECK(contains(log, "ACCOUNT_UNFROZEN"));
    CHECK(contains(log, "ACCOUNT_CLOSED"));
    CHECK(contains(log, "ACCOUNT_REOPENED"));
    CHECK(contains(log, "LOGIN_UNLOCKED"));
    CHECK(contains(log, "ACCOUNT_SOFT_DELETED"));
    CHECK(contains(log, "acct=300101"));
    CHECK(contains(log, "actor=ADMIN"));
}

