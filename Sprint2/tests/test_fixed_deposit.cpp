#include "test_framework.h"
#include "fixed_deposit.h"
#include <cstdio>
#include <fstream>
#include <vector>

namespace {
void clearFdFile() {
    std::remove(FIXED_DEPOSIT_FILE.c_str());
    std::remove((FIXED_DEPOSIT_FILE + ".tmp").c_str());
}
}

TEST(FixedDepositCalculatesQuarterlyCompoundReturns) {
    CHECK(calculateFdMaturityAmount(50000.0, 7.0, 24) == 57444.09);
    CHECK(calculateFdInterest(50000.0, 7.0, 24) == 7444.09);
}
TEST(FixedDepositRejectsInvalidTerms) {
    CHECK(!validFixedDepositTerms(999.0, 7.0, 12));
    CHECK(!validFixedDepositTerms(1000.0, 0.0, 12));
    CHECK(!validFixedDepositTerms(1000.0, 101.0, 12));
    CHECK(!validFixedDepositTerms(1000.0, 7.0, 4));
    CHECK(!validFixedDepositTerms(1000.0, 7.0, 0));
    CHECK(!validFixedDepositTerms(1000.0, 7.0, 123));
}
TEST(FixedDepositAcceptsValidTerms) {
    CHECK(validFixedDepositTerms(1000.0, 7.0, 3));
    CHECK(validFixedDepositTerms(100000.0, 8.5, 60));
}
TEST(FixedDepositGeneratesSequentialIdentifiers) {
    std::vector<FixedDeposit> deposits(2);
    deposits[0].fdId = "FD000009";
    deposits[1].fdId = "FD000014";
    CHECK(nextFixedDepositId(deposits) == "FD000015");
}
TEST(FixedDepositPersistsAndReloadsRecord) {
    clearFdFile();
    FixedDeposit fd;
    fd.fdId = "FD000001";
    fd.accountNumber = "100101";
    fd.principal = 50000.0;
    fd.annualInterestRate = 7.0;
    fd.durationMonths = 24;
    fd.startDate = "2026-01-15";
    fd.maturityDate = "2028-01-15";
    fd.maturityAmount = 57425.92;
    fd.status = "ACTIVE";
    CHECK(createFixedDepositRecord(fd));
    const auto loaded = loadFixedDeposits();
    CHECK(loaded.size() == 1);
    CHECK(loaded[0].fdId == fd.fdId);
    CHECK(loaded[0].accountNumber == fd.accountNumber);
    CHECK(loaded[0].principal == fd.principal);
    CHECK(loaded[0].annualInterestRate == fd.annualInterestRate);
    CHECK(loaded[0].durationMonths == fd.durationMonths);
    CHECK(loaded[0].maturityAmount == fd.maturityAmount);
    clearFdFile();
}
TEST(FixedDepositRejectsDuplicateIdentifiers) {
    clearFdFile();
    FixedDeposit fd;
    fd.fdId = "FD000001";
    fd.accountNumber = "100101";
    fd.principal = 1000.0;
    fd.annualInterestRate = 7.0;
    fd.durationMonths = 3;
    fd.startDate = "2026-01-15";
    fd.maturityDate = "2026-04-15";
    fd.maturityAmount = calculateFdMaturityAmount(1000.0, 7.0, 3);
    CHECK(createFixedDepositRecord(fd));
    CHECK(!createFixedDepositRecord(fd));
    clearFdFile();
}
TEST(FixedDepositSkipsInvalidPersistedRows) {
    clearFdFile();
    {
        std::ofstream out(FIXED_DEPOSIT_FILE);
        out << "\"FD000001\" \"100101\" 1000 7 4 \"2026-01-15\" \"2026-05-15\" 1020.00 \"ACTIVE\"\n";
    }
    CHECK(loadFixedDeposits().empty());
    clearFdFile();
}
