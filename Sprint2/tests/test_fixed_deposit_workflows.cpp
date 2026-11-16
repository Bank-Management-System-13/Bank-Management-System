#include "test_framework.h"
#include "fixed_deposit.h"
#include "audit.h"
#include "storage.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

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

void resetFdEnvironment() {
    removePath(ACCOUNT_FILE);
    removePath(ACCOUNT_FILE + ".tmp");
    removePath(TRANSACTION_FILE);
    removePath("audit.log");
    removePath(FIXED_DEPOSIT_FILE);
    removePath(FIXED_DEPOSIT_FILE + ".tmp");
    accounts.clear();
    nextAccountNumber = 100001;
}

struct FdTestScope {
    FdTestScope() { resetFdEnvironment(); }
    ~FdTestScope() { resetFdEnvironment(); }
};

Account& addAccount(const std::string& number, double balance = 10000.0) {
    Account account;
    account.accountNumber = number;
    account.name = "FD Test Customer";
    account.pin = "1234";
    account.balance = balance;
    accounts.push_back(account);
    return accounts.back();
}

FixedDeposit makeFd(const std::string& id, const std::string& accountNumber,
                    const std::string& maturityDate = "2024-04-01",
                    const std::string& status = "ACTIVE") {
    FixedDeposit fd;
    fd.fdId = id;
    fd.accountNumber = accountNumber;
    fd.principal = 5000.0;
    fd.annualInterestRate = 7.25;
    fd.durationMonths = 3;
    fd.startDate = "2024-01-01";
    fd.maturityDate = maturityDate;
    fd.maturityAmount = calculateFdMaturityAmount(fd.principal, fd.annualInterestRate,
                                                  fd.durationMonths);
    fd.status = status;
    return fd;
}

bool addFd(const FixedDeposit& fd) {
    return createFixedDepositRecord(fd);
}

std::string readFile(const std::string& path) {
    std::ifstream input(path);
    std::ostringstream result;
    result << input.rdbuf();
    return result.str();
}
} // namespace

TEST(FixedDepositDateCalculationHandlesLeapYearsAndMonthEnds) {
    CHECK(calculateFdMaturityDate("2023-11-30", 3) == "2024-02-29");
    CHECK(calculateFdMaturityDate("2024-11-30", 3) == "2025-02-28");
    CHECK(calculateFdMaturityDate("2024-01-31", 3) == "2024-04-30");
    CHECK(calculateFdMaturityDate("bad-date", 3).empty());
    CHECK(calculateFdMaturityDate("2024-01-01", 4).empty());
    CHECK(calculateFdMaturityDate("2024-01-01", 0).empty());
}

TEST(FixedDepositSaveReturnsFalseWhenTemporaryPathCannotBeOpened) {
    FdTestScope scope;
    CHECK(std::filesystem::create_directory(FIXED_DEPOSIT_FILE + ".tmp"));
    CHECK(!saveFixedDeposits({}));
}

TEST(FixedDepositIdentifierSkipsMalformedExistingIds) {
    std::vector<FixedDeposit> deposits(3);
    deposits[0].fdId = "FD000004";
    deposits[1].fdId = "FDnot-a-number";
    deposits[2].fdId = "OTHER000099";
    CHECK(nextFixedDepositId(deposits) == "FD000005");
}

TEST(FixedDepositCreationReservesBalanceAndWritesOpeningLedger) {
    FdTestScope scope;
    addAccount("410001", 10000.0);
    CHECK(saveAccounts());

    std::string output;
    {
        ConsoleCapture io("2000\n7.25\n6\n");
        createFixedDeposit(accounts[0]);
        output = io.output();
    }

    CHECK(output.find("Fixed deposit created successfully.") != std::string::npos);
    CHECK(accounts[0].balance == 8000.0);
    const auto deposits = loadFixedDeposits();
    CHECK(deposits.size() == 1);
    CHECK(deposits[0].fdId == "FD000001");
    CHECK(deposits[0].annualInterestRate == 7.25);
    CHECK(deposits[0].durationMonths == 6);
    CHECK(deposits[0].maturityDate == calculateFdMaturityDate(deposits[0].startDate, 6));
    const auto transactions = loadTransactions();
    CHECK(transactions.size() == 1);
    CHECK(transactions[0].type == "FD_OPEN:FD000001");
    CHECK(transactions[0].amount == 2000.0);
    CHECK(readFile("audit.log").find("FD_CREATED") != std::string::npos);
}

TEST(FixedDepositCreationRejectsUnavailableAccountStatuses) {
    FdTestScope scope;
    Account blocked;
    blocked.accountNumber = "410002";
    blocked.balance = 10000.0;

    for (int state = 0; state < 4; ++state) {
        blocked.frozen = state == 0;
        blocked.closed = state == 1;
        blocked.deleted = state == 2;
        blocked.locked = state == 3;
        ConsoleCapture io("");
        createFixedDeposit(blocked);
        CHECK(io.output().find("cannot create an FD") != std::string::npos);
    }
    CHECK(loadFixedDeposits().empty());
}

TEST(FixedDepositCreationRejectsInvalidTenureAndInsufficientBalance) {
    FdTestScope scope;
    addAccount("410003", 10000.0);
    CHECK(saveAccounts());

    std::string invalidOutput;
    {
        ConsoleCapture io("2000\n7.5\n4\n");
        createFixedDeposit(accounts[0]);
        invalidOutput = io.output();
    }
    CHECK(invalidOutput.find("Invalid FD terms.") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
    CHECK(loadFixedDeposits().empty());

    std::string insufficientOutput;
    {
        ConsoleCapture io("12000\n7.5\n3\n");
        createFixedDeposit(accounts[0]);
        insufficientOutput = io.output();
    }
    CHECK(insufficientOutput.find("Insufficient available balance.") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
    CHECK(loadFixedDeposits().empty());
}

TEST(FixedDepositCreationHandlesNonNumericAmount) {
    FdTestScope scope;
    addAccount("410004", 10000.0);
    ConsoleCapture io("not-a-number\n");
    createFixedDeposit(accounts[0]);
    CHECK(io.output().find("Invalid amount.") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
}

TEST(FixedDepositCreationRollsBackWhenAccountSaveFails) {
    FdTestScope scope;
    addAccount("410005", 10000.0);
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
    std::string output;
    {
        ConsoleCapture io("2000\n7\n3\n");
        createFixedDeposit(accounts[0]);
        output = io.output();
    }
    CHECK(output.find("Could not reserve principal") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
    CHECK(loadFixedDeposits().empty());
}

TEST(FixedDepositCreationRollsBackWhenFdFileSaveFails) {
    FdTestScope scope;
    addAccount("410006", 10000.0);
    CHECK(std::filesystem::create_directory(FIXED_DEPOSIT_FILE + ".tmp"));
    std::string output;
    {
        ConsoleCapture io("2000\n7\n3\n");
        createFixedDeposit(accounts[0]);
        output = io.output();
    }
    CHECK(output.find("Could not save FD") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
    CHECK(saveAccounts());
    CHECK(loadFixedDeposits().empty());
}

TEST(FixedDepositCreationRollsBackWhenOpeningLedgerFails) {
    FdTestScope scope;
    addAccount("410007", 10000.0);
    CHECK(std::filesystem::create_directory(TRANSACTION_FILE));
    std::string output;
    {
        ConsoleCapture io("2000\n7\n3\n");
        createFixedDeposit(accounts[0]);
        output = io.output();
    }
    CHECK(output.find("Could not record FD opening") != std::string::npos);
    CHECK(accounts[0].balance == 10000.0);
    CHECK(loadFixedDeposits().empty());
}

TEST(FixedDepositListingOnlyShowsTheRequestedCustomer) {
    FdTestScope scope;
    CHECK(addFd(makeFd("FD000001", "410008")));
    CHECK(addFd(makeFd("FD000002", "410009")));

    std::string output;
    {
        ConsoleCapture io("");
        Account account;
        account.accountNumber = "410008";
        viewFixedDeposits(account);
        output = io.output();
    }
    CHECK(output.find("FD000001") != std::string::npos);
    CHECK(output.find("FD000002") == std::string::npos);

    {
        ConsoleCapture io("");
        Account account;
        account.accountNumber = "410010";
        viewFixedDeposits(account);
        CHECK(io.output().find("No fixed deposits found.") != std::string::npos);
    }
}

TEST(FixedDepositReturnCalculatorDisplaysQuarterlyReturnsAndRejectsBadTerms) {
    std::string output;
    {
        ConsoleCapture io("50000\n7.25\n24\n");
        calculateFixedDepositReturns();
        output = io.output();
    }
    CHECK(output.find("Compounding: Quarterly") != std::string::npos);
    CHECK(output.find("Interest earned: Rs.") != std::string::npos);
    CHECK(output.find("Maturity amount: Rs.") != std::string::npos);

    {
        ConsoleCapture io("50000\n7.25\n7\n");
        calculateFixedDepositReturns();
        CHECK(io.output().find("Invalid FD terms.") != std::string::npos);
    }
    {
        ConsoleCapture io("50000\n7.25\nbad\n");
        calculateFixedDepositReturns();
        CHECK(io.output().find("Invalid input. Enter a whole number.") != std::string::npos);
    }
}

TEST(FixedDepositMaturityProcessingCreditsOnceAndUpdatesStatus) {
    FdTestScope scope;
    addAccount("410011", 500.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000011", "410011")));

    processMaturedFixedDeposits();
    const double expected = 500.0 + calculateFdMaturityAmount(5000.0, 7.25, 3);
    CHECK(accounts[0].balance == expected);
    CHECK(loadFixedDeposits()[0].status == "MATURED");
    CHECK(loadTransactions().size() == 1);
    CHECK(loadTransactions()[0].type == "FD_MATURITY_CREDIT:FD000011");

    processMaturedFixedDeposits();
    CHECK(accounts[0].balance == expected);
    CHECK(loadTransactions().size() == 1);
}

TEST(FixedDepositMaturityProcessingSkipsBlockedMissingAndFutureDeposits) {
    FdTestScope scope;
    addAccount("410012", 100.0);
    addAccount("410013", 100.0).frozen = true;
    addAccount("410014", 100.0).closed = true;
    addAccount("410015", 100.0).deleted = true;
    CHECK(saveAccounts());

    CHECK(addFd(makeFd("FD000012", "410012")));
    CHECK(addFd(makeFd("FD000013", "410013")));
    CHECK(addFd(makeFd("FD000014", "410014")));
    CHECK(addFd(makeFd("FD000015", "410015")));
    CHECK(addFd(makeFd("FD000016", "419999")));
    CHECK(addFd(makeFd("FD000017", "410012", "2099-04-01")));

    processMaturedFixedDeposits();
    const auto deposits = loadFixedDeposits();
    CHECK(accounts[0].balance > 100.0);
    CHECK(accounts[1].balance == 100.0);
    CHECK(accounts[2].balance == 100.0);
    CHECK(accounts[3].balance == 100.0);
    CHECK(deposits[0].status == "MATURED");
    CHECK(deposits[1].status == "ACTIVE");
    CHECK(deposits[2].status == "ACTIVE");
    CHECK(deposits[3].status == "ACTIVE");
    CHECK(deposits[4].status == "ACTIVE");
    CHECK(deposits[5].status == "ACTIVE");
}

TEST(FixedDepositMaturityProcessingRepairsStatusWhenLedgerCreditAlreadyExists) {
    FdTestScope scope;
    addAccount("410018", 900.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000018", "410018")));
    CHECK(appendTransaction({"410018", "FD_MATURITY_CREDIT:FD000018",
                             calculateFdMaturityAmount(5000.0, 7.25, 3),
                             900.0, "2026-01-01 00:00:00"}));

    processMaturedFixedDeposits();
    CHECK(accounts[0].balance == 900.0);
    CHECK(loadFixedDeposits()[0].status == "MATURED");
    CHECK(loadTransactions().size() == 1);
}

TEST(FixedDepositMaturityProcessingRollsBackWhenAccountSaveFails) {
    FdTestScope scope;
    addAccount("410019", 900.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000019", "410019")));
    CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));

    processMaturedFixedDeposits();
    CHECK(accounts[0].balance == 900.0);
    CHECK(loadFixedDeposits()[0].status == "ACTIVE");
    CHECK(loadTransactions().empty());
}

TEST(FixedDepositMaturityProcessingRollsBackWhenLedgerAppendFails) {
    FdTestScope scope;
    addAccount("410020", 900.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000020", "410020")));
    CHECK(std::filesystem::create_directory(TRANSACTION_FILE));

    processMaturedFixedDeposits();
    CHECK(accounts[0].balance == 900.0);
    CHECK(loadFixedDeposits()[0].status == "ACTIVE");
}

TEST(FixedDepositWithdrawalRejectsUnknownPrematureAndClosedDeposits) {
    FdTestScope scope;
    addAccount("410021", 900.0);
    CHECK(addFd(makeFd("FD000021", "410021", "2099-04-01", "ACTIVE")));
    CHECK(addFd(makeFd("FD000022", "410021", "2024-04-01", "CLOSED")));
    CHECK(addFd(makeFd("FD000023", "410021", "2024-04-01", "MATURED")));

    {
        ConsoleCapture io("FD999999\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("FD not found") != std::string::npos);
    }
    {
        ConsoleCapture io("FD000021\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("Premature closure is not enabled") != std::string::npos);
    }
    {
        ConsoleCapture io("FD000022\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("not active") != std::string::npos);
    }
    {
        ConsoleCapture io("FD000023\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("already matured") != std::string::npos);
    }
    CHECK(accounts[0].balance == 900.0);
}

TEST(FixedDepositWithdrawalCreditsMaturedDepositAndDoesNotDoubleCredit) {
    FdTestScope scope;
    addAccount("410024", 1000.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000024", "410024")));

    {
        ConsoleCapture io("FD000024\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("Maturity amount credited") != std::string::npos);
    }
    const double expected = 1000.0 + calculateFdMaturityAmount(5000.0, 7.25, 3);
    CHECK(accounts[0].balance == expected);
    CHECK(loadFixedDeposits()[0].status == "MATURED");
    CHECK(loadTransactions().size() == 1);

    {
        ConsoleCapture io("FD000024\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("already matured") != std::string::npos);
    }
    CHECK(accounts[0].balance == expected);
    CHECK(loadTransactions().size() == 1);
}

TEST(FixedDepositWithdrawalRepairsStatusWithoutDoubleCredit) {
    FdTestScope scope;
    addAccount("410025", 1000.0);
    CHECK(saveAccounts());
    CHECK(addFd(makeFd("FD000025", "410025")));
    CHECK(std::filesystem::create_directory(FIXED_DEPOSIT_FILE + ".tmp"));

    {
        ConsoleCapture io("FD000025\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("FD status update failed") != std::string::npos);
    }
    const double expected = 1000.0 + calculateFdMaturityAmount(5000.0, 7.25, 3);
    CHECK(accounts[0].balance == expected);
    CHECK(loadFixedDeposits()[0].status == "ACTIVE");
    CHECK(loadTransactions().size() == 1);

    removePath(FIXED_DEPOSIT_FILE + ".tmp");
    {
        ConsoleCapture io("FD000025\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(io.output().find("already credited") != std::string::npos);
    }
    CHECK(accounts[0].balance == expected);
    CHECK(loadFixedDeposits()[0].status == "MATURED");
    CHECK(loadTransactions().size() == 1);
}

TEST(FixedDepositWithdrawalRollsBackWhenAccountSaveOrLedgerAppendFails) {
    {
        FdTestScope scope;
        addAccount("410026", 1000.0);
        CHECK(saveAccounts());
        CHECK(addFd(makeFd("FD000026", "410026")));
        CHECK(std::filesystem::create_directory(ACCOUNT_FILE + ".tmp"));
        ConsoleCapture io("FD000026\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(accounts[0].balance == 1000.0);
        CHECK(loadFixedDeposits()[0].status == "ACTIVE");
        CHECK(loadTransactions().empty());
    }
    {
        FdTestScope scope;
        addAccount("410027", 1000.0);
        CHECK(saveAccounts());
        CHECK(addFd(makeFd("FD000027", "410027")));
        CHECK(std::filesystem::create_directory(TRANSACTION_FILE));
        ConsoleCapture io("FD000027\n");
        withdrawFixedDeposit(accounts[0]);
        CHECK(accounts[0].balance == 1000.0);
        CHECK(loadFixedDeposits()[0].status == "ACTIVE");
    }
}

TEST(FixedDepositMenuHandlesInvalidNonNumericAndExitChoices) {
    FdTestScope scope;
    Account account;
    account.accountNumber = "410028";
    {
        ConsoleCapture io("9\n0\n");
        showFixedDepositMenu(account);
        CHECK(io.output().find("Invalid choice.") != std::string::npos);
    }
    {
        ConsoleCapture io("not-a-choice\n");
        showFixedDepositMenu(account);
        CHECK(io.output().find("Invalid input. Enter a whole number.") != std::string::npos);
    }
    {
        ConsoleCapture io("0\n");
        showFixedDepositMenu(account);
        CHECK(io.output().find("FIXED DEPOSIT MENU") != std::string::npos);
    }
}
