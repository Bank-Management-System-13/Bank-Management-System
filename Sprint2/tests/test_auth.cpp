#include "test_framework.h"
#include "auth.h"
#include "storage.h"
#include "audit.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>

namespace {
class ConsoleCapture {
public:
    explicit ConsoleCapture(const std::string& inputText)
        : input_(inputText),
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

void removePath(const std::string& path) {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
}
void resetAuthState() {
    removePath(ACCOUNT_FILE);
    removePath(ACCOUNT_FILE + ".tmp");
    removePath(TRANSACTION_FILE);
    removePath("audit.log");
    removePath("fixed_deposits.txt");
    removePath("fixed_deposits.txt.tmp");
    accounts.clear();
    nextAccountNumber = 100001;
}
struct AuthTestScope {
    AuthTestScope() { resetAuthState(); }
    ~AuthTestScope() { resetAuthState(); }
};
Account& addCustomer(const std::string& number, double balance = 1000.0) {
    Account account;
    account.accountNumber = number;
    account.name = "Auth Test Customer";
    account.pin = "1234";
    account.balance = balance;
    accounts.push_back(account);
    return accounts.back();
}
std::string readFile(const std::string& path) {
    std::ifstream input(path);
    std::ostringstream output;
    output << input.rdbuf();
    return output.str();
}
}

TEST(AuthCreateAccountPersistsAccountAndAuditEvent) {
    AuthTestScope scope;
    std::string output;
    {
        ConsoleCapture io("\nKiran BG\n1234\n");
        createAccount();
        output = io.output();
    }
    CHECK(accounts.size() == 1);
    CHECK(accounts[0].accountNumber == "100001");
    CHECK(accounts[0].name == "Kiran BG");
    CHECK(accounts[0].pin == "1234");
    CHECK(nextAccountNumber == 100002);
    CHECK(output.find("Account created.") != std::string::npos);
    CHECK(loadAccounts());
    CHECK(accounts.size() == 1);
    CHECK(readFile("audit.log").find("ACCOUNT_CREATED") != std::string::npos);
}

TEST(AuthCreateAccountRejectsBlankNameAndMalformedPin) {
    AuthTestScope scope;
    {
        ConsoleCapture io("\n  \t  \n");
        createAccount();
        CHECK(io.output().find("Name cannot be blank.") != std::string::npos);
    }
    CHECK(accounts.empty());
    {
        ConsoleCapture io("\nValid Name\n12a4\n");
        createAccount();
        CHECK(io.output().find("PIN must contain exactly four digits.") != std::string::npos);
    }
    CHECK(accounts.empty());
    CHECK(nextAccountNumber == 100001);
}

TEST(AuthCreateAccountRollsBackWhenAccountStorageFails) {
    AuthTestScope scope;
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
    ConsoleCapture io("\nStorage Failure\n1234\n");
    createAccount();
    CHECK(io.output().find("Could not save account.") != std::string::npos);
    CHECK(accounts.empty());
    CHECK(nextAccountNumber == 100001);
}

TEST(AuthCustomerLoginHandlesMalformedUnknownAndBlockedAccounts) {
    AuthTestScope scope;
    {
        ConsoleCapture io("ABC123\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("digits only") != std::string::npos);
    }
    {
        ConsoleCapture io("999999\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Account not found.") != std::string::npos);
    }

    addCustomer("420001");
    accounts[0].locked = true;
    {
        ConsoleCapture io("420001\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Account is locked") != std::string::npos);
    }
    accounts[0].locked = false;
    accounts[0].frozen = true;
    {
        ConsoleCapture io("420001\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Account is frozen") != std::string::npos);
    }
    accounts[0].frozen = false;
    accounts[0].closed = true;
    {
        ConsoleCapture io("420001\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Account is closed") != std::string::npos);
    }
    CHECK(accounts[0].failedAttempts == 0);
}

TEST(AuthCustomerLoginCountsIncorrectPinsAndLocksAfterThreeAttempts) {
    AuthTestScope scope;
    addCustomer("420002");
    accounts[0].failedAttempts = 1;
    {
        ConsoleCapture io("420002\n9999\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Attempts remaining: 1") != std::string::npos);
    }
    CHECK(accounts[0].failedAttempts == 2);
    CHECK(!accounts[0].locked);
    {
        ConsoleCapture io("420002\n9999\n");
        CHECK(customerLogin() == -1);
        CHECK(io.output().find("Three incorrect attempts") != std::string::npos);
    }
    CHECK(accounts[0].failedAttempts == 3);
    CHECK(accounts[0].locked);
    CHECK(readFile("audit.log").find("ACCOUNT_LOCKED") != std::string::npos);
}

TEST(AuthCustomerLoginResetsFailedAttemptsOnSuccess) {
    AuthTestScope scope;
    addCustomer("420003");
    accounts[0].failedAttempts = 2;
    {
        ConsoleCapture io("420003\n1234\n");
        CHECK(customerLogin() == 0);
        CHECK(io.output().find("Login successful.") != std::string::npos);
    }
    CHECK(accounts[0].failedAttempts == 0);
    CHECK(!accounts[0].locked);
    CHECK(readFile("audit.log").find("LOGIN_SUCCESS") != std::string::npos);
}

TEST(AuthCustomerLoginWarnsWhenSuccessCounterCannotBeSaved) {
    AuthTestScope scope;
    addCustomer("420004");
    accounts[0].failedAttempts = 1;
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
    ConsoleCapture io("420004\n1234\n");
    CHECK(customerLogin() == 0);
    CHECK(io.output().find("login succeeded but attempt counter could not be saved") != std::string::npos);
    CHECK(accounts[0].failedAttempts == 0);
}

TEST(AuthCustomerMenuCoversTransactionsReportsFdMenuAndLogout) {
    AuthTestScope scope;
    addCustomer("420005", 1000.0);
    CHECK(saveAccounts());
    std::string output;
    {
        ConsoleCapture io("1\n2\n250\n3\n2000\n3\n100\n4\n0\n5\n0\n0\n");
        customerMenu(0);
        output = io.output();
    }
    CHECK(output.find("Balance: Rs. 1000.00") != std::string::npos);
    CHECK(output.find("Insufficient balance.") != std::string::npos);
    CHECK(output.find("Fixed Deposit Menu") == std::string::npos); // heading is uppercase in the UI
    CHECK(output.find("FIXED DEPOSIT MENU") != std::string::npos);
    CHECK(accounts[0].balance == 1150.0);
    const auto transactions = loadTransactions();
    CHECK(transactions.size() == 2);
    CHECK(transactions[0].type == "DEPOSIT");
    CHECK(transactions[1].type == "WITHDRAWAL");
    CHECK(readFile("audit.log").find("LOGOUT") != std::string::npos);
}

TEST(AuthCustomerMenuRejectsBlockedTransactionsAndInvalidAmounts) {
    AuthTestScope scope;
    addCustomer("420006", 1000.0);
    accounts[0].frozen = true;
    {
        ConsoleCapture io("2\n100\n3\n100\n0\n");
        customerMenu(0);
        CHECK(io.output().find("This account cannot transact.") != std::string::npos);
    }
    CHECK(accounts[0].balance == 1000.0);
    accounts[0].frozen = false;
    {
        ConsoleCapture io("2\nnot-money\n9\n0\n");
        customerMenu(0);
        CHECK(io.output().find("Invalid amount.") != std::string::npos);
        CHECK(io.output().find("Invalid choice.") != std::string::npos);
    }
    CHECK(accounts[0].balance == 1000.0);
}

TEST(AuthCustomerMenuReturnsForInvalidIndexAndEof) {
    AuthTestScope scope;
    {
        ConsoleCapture io("");
        customerMenu(-1);
        CHECK(io.output().empty());
    }
    addCustomer("420007");
    {
        ConsoleCapture io("");
        customerMenu(0);
        CHECK(io.output().find("CUSTOMER MENU") != std::string::npos);
    }
}

TEST(AuthCustomerMenuRollsBackDepositWhenAccountSaveFails) {
    AuthTestScope scope;
    addCustomer("420008", 1000.0);
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
    ConsoleCapture io("2\n100\n0\n");
    customerMenu(0);
    CHECK(io.output().find("Save failed; transaction cancelled.") != std::string::npos);
    CHECK(accounts[0].balance == 1000.0);
    CHECK(loadTransactions().empty());
}

TEST(AuthCustomerMenuRollsBackDepositWhenTransactionWriteFails) {
    AuthTestScope scope;
    addCustomer("420009", 1000.0);
    CHECK(std::filesystem::create_directory(TRANSACTION_FILE));
    ConsoleCapture io("2\n100\n0\n");
    customerMenu(0);
    CHECK(io.output().find("Transaction history/audit write failed") != std::string::npos);
    CHECK(accounts[0].balance == 1000.0);
}
