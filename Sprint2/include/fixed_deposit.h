#ifndef BANK_FIXED_DEPOSIT_H
#define BANK_FIXED_DEPOSIT_H
#include "models.h"
#include <string>
#include <vector>

struct FixedDeposit {
    std::string fdId;
    std::string accountNumber;
    double principal = 0.0;
    double annualInterestRate = 0.0; // percentage per annum
    int durationMonths = 0;
    std::string startDate;
    std::string maturityDate;
    double maturityAmount = 0.0;
    std::string status = "ACTIVE"; // ACTIVE, MATURED, CLOSED
};

extern const std::string FIXED_DEPOSIT_FILE;
bool validFixedDepositTerms(double principal, double annualRate, int durationMonths);
double calculateFdMaturityAmount(double principal, double annualRatePercent, int durationMonths);
double calculateFdInterest(double principal, double annualRatePercent, int durationMonths);
// Returns the calendar maturity date for a valid ISO start date and whole-quarter tenure.
std::string calculateFdMaturityDate(const std::string& startDate, int durationMonths);
std::vector<FixedDeposit> loadFixedDeposits();
bool saveFixedDeposits(const std::vector<FixedDeposit>& deposits);
std::string nextFixedDepositId(const std::vector<FixedDeposit>& deposits);
bool createFixedDepositRecord(const FixedDeposit& deposit);
void showFixedDepositMenu(Account& account);
void createFixedDeposit(Account& account);
void viewFixedDeposits(const Account& account);
void calculateFixedDepositReturns();
void withdrawFixedDeposit(Account& account);
void processMaturedFixedDeposits();
#endif
