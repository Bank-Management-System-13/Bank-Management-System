#include <filesystem>
#include "test_framework.h"
#include "reports.h"
#include "storage.h"
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
void prepareReportData() {
    std::remove(TRANSACTION_FILE.c_str());
    appendTransaction({"200001", "DEPOSIT", 500.0, 500.0, "2026-11-01 09:00:00"});
    appendTransaction({"200001", "WITHDRAWAL", 125.0, 375.0, "2026-11-02 10:00:00"});
    appendTransaction({"200002", "DEPOSIT", 900.0, 900.0, "2026-11-03 11:00:00"});
}
}

TEST(ReportsSummaryShowsCorrectAccountAndTotals) {
    prepareReportData();
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());
    showAccountSummary(account);
    std::cout.rdbuf(original);

    const std::string output = captured.str();
    CHECK(output.find("Report Customer") != std::string::npos);
    CHECK(output.find("200001") != std::string::npos);
    CHECK(output.find("375.00") != std::string::npos);
    CHECK(output.find("500.00") != std::string::npos);
    CHECK(output.find("125.00") != std::string::npos);
    CHECK(output.find("Total Transactions:  2") != std::string::npos);
}

TEST(ReportsFullHistoryFiltersOtherAccounts) {
    prepareReportData();
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());
    showFullHistory(account);
    std::cout.rdbuf(original);

    const std::string output = captured.str();
    CHECK(output.find("DEPOSIT") != std::string::npos);
    CHECK(output.find("WITHDRAWAL") != std::string::npos);
    CHECK(output.find("200002") == std::string::npos);
}

TEST(ReportsExportStatementCreatesExpectedFile) {
    prepareReportData();
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::remove("statement_200001.txt");
    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());
    exportStatement(account);
    std::cout.rdbuf(original);

    std::ifstream input("statement_200001.txt");
    CHECK(input.is_open());

    std::ostringstream contents;
    contents << input.rdbuf();
    CHECK(contents.str().find("BANK MANAGEMENT SYSTEM - ACCOUNT STATEMENT") != std::string::npos);
    CHECK(contents.str().find("Report Customer") != std::string::npos);
    CHECK(contents.str().find("DEPOSIT") != std::string::npos);
    CHECK(contents.str().find("WITHDRAWAL") != std::string::npos);
}
TEST(ReportsHandleAccountWithNoTransactions) {
    std::remove(TRANSACTION_FILE.c_str());

    Account account;
    account.accountNumber = "200099";
    account.name = "Empty Report Customer";
    account.balance = 0.0;

    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());

    showMiniStatement(account);
    showFullHistory(account);
    showAccountSummary(account);

    std::cout.rdbuf(original);

    const std::string output = captured.str();

    CHECK(output.find("No transactions found.") != std::string::npos);
    CHECK(output.find("Total Transactions:") != std::string::npos);
}

TEST(ReportsExportHandlesFileCreationFailure) {
    prepareReportData();

    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    const std::string filename = "statement_200001.txt";

    std::remove(filename.c_str());
    CHECK(std::filesystem::create_directory(filename));

    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());

    exportStatement(account);

    std::cout.rdbuf(original);

    const std::string output = captured.str();

    std::filesystem::remove_all(filename);

    CHECK(output.find("Error: Could not create") != std::string::npos);
    CHECK(output.find("Statement exported") == std::string::npos);
}

TEST(ReportsMenuReturnsOnZero) {
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::istringstream input("0\n");
    std::ostringstream captured;

    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());

    showReportsMenu(account);

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);

    CHECK(captured.str().find("REPORTS") != std::string::npos);
    CHECK(captured.str().find("Enter choice:") != std::string::npos);
}

TEST(ReportsMenuRejectsInvalidChoice) {
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::istringstream input("9\n0\n");
    std::ostringstream captured;

    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());

    showReportsMenu(account);

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);

    CHECK(captured.str().find("Invalid menu choice.") != std::string::npos);
}

TEST(ReportsMenuHandlesNonNumericInput) {
    Account account;
    account.accountNumber = "200001";
    account.name = "Report Customer";
    account.balance = 375.0;

    std::istringstream input("abc\n0\n");
    std::ostringstream captured;

    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());

    showReportsMenu(account);

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);

    CHECK(captured.str().find("Invalid input. Enter a number.") != std::string::npos);
}


TEST(ReportsMiniStatementShowsOnlyTheMostRecentFiveRows) {
    std::remove(TRANSACTION_FILE.c_str());
    for (int i = 0; i < 7; ++i) {
        appendTransaction({"200120", "TX_" + std::to_string(i),
                           static_cast<double>(i + 1), static_cast<double>(i + 1),
                           "2026-11-0" + std::to_string(i + 1) + " 09:00:00"});
    }
    Account account;
    account.accountNumber = "200120";
    account.name = "Recent History Customer";
    account.balance = 28.0;

    std::ostringstream captured;
    std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());
    showMiniStatement(account);
    std::cout.rdbuf(oldOutput);

    const std::string output = captured.str();
    CHECK(output.find("TX_0") == std::string::npos);
    CHECK(output.find("TX_1") == std::string::npos);
    CHECK(output.find("TX_2") != std::string::npos);
    CHECK(output.find("TX_6") != std::string::npos);
}

TEST(ReportsSummaryCoversEveryAccountStatusPriority) {
    prepareReportData();
    Account account;
    account.accountNumber = "200001";
    account.name = "Status Customer";
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"ACTIVE", "ACTIVE"},
        {"LOCKED", "LOCKED (too many wrong PINs)"},
        {"FROZEN", "FROZEN"},
        {"CLOSED", "CLOSED"},
        {"DELETED", "DELETED"}
    };
    for (const auto& item : cases) {
        account.locked = item.first == "LOCKED";
        account.frozen = item.first == "FROZEN";
        account.closed = item.first == "CLOSED";
        account.deleted = item.first == "DELETED";
        std::ostringstream captured;
        std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());
        showAccountSummary(account);
        std::cout.rdbuf(oldOutput);
        CHECK(captured.str().find("Account Status:      " + item.second) != std::string::npos);
    }
}

TEST(ReportsTotalsIgnoreNonDepositAndNonWithdrawalEvents) {
    std::remove(TRANSACTION_FILE.c_str());
    appendTransaction({"200121", "DEPOSIT", 100.0, 100.0, "2026-11-01 09:00:00"});
    appendTransaction({"200121", "TRANSFER_IN", 50.0, 150.0, "2026-11-01 10:00:00"});
    appendTransaction({"200121", "INTEREST", 5.0, 155.0, "2026-11-01 11:00:00"});
    appendTransaction({"200121", "WITHDRAWAL", 25.0, 130.0, "2026-11-01 12:00:00"});

    Account account;
    account.accountNumber = "200121";
    account.name = "Totals Customer";
    account.balance = 130.0;

    std::ostringstream captured;
    std::streambuf* oldOutput = std::cout.rdbuf(captured.rdbuf());
    showAccountSummary(account);
    std::cout.rdbuf(oldOutput);

    CHECK(captured.str().find("Deposits:            1 (Rs. 100.00)") != std::string::npos);
    CHECK(captured.str().find("Withdrawals:         1 (Rs. 25.00)") != std::string::npos);
    CHECK(captured.str().find("Total Transactions:  4") != std::string::npos);
}

TEST(ReportsMenuReturnsImmediatelyOnEndOfInput) {
    Account account;
    account.accountNumber = "200122";
    account.name = "EOF Customer";
    std::istringstream input;
    std::ostringstream output;
    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(output.rdbuf());

    showReportsMenu(account);

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);
    CHECK(output.str().find("REPORTS") != std::string::npos);
    CHECK(output.str().find("Invalid input.") == std::string::npos);
}
