#include "fixed_deposit.h"
#include "audit.h"
#include "security.h"
#include "storage.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

const std::string FIXED_DEPOSIT_FILE = "fixed_deposits.txt";

namespace {
std::string today() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char value[11];
    std::strftime(value, sizeof(value), "%Y-%m-%d", &local);
    return value;
}

std::string addMonths(const std::string& date, int months) {
    int year = 0, month = 0, day = 0;
    char d1 = 0, d2 = 0;
    std::istringstream input(date);
    if (!(input >> year >> d1 >> month >> d2 >> day) || d1 != '-' || d2 != '-') return {};
    month += months;
    year += (month - 1) / 12;
    month = (month - 1) % 12 + 1;
    static const int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = monthDays[month - 1];
    const bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    if (month == 2 && leap) maxDay = 29;
    day = std::min(day, maxDay);
    std::ostringstream out;
    out << std::setfill('0') << std::setw(4) << year << '-'
        << std::setw(2) << month << '-' << std::setw(2) << day;
    return out.str();
}

bool validDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i)
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (month < 1 || month > 12 || day < 1) return false;
    static const int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = monthDays[month - 1];
    const bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    if (month == 2 && leap) maxDay = 29;
    return day <= maxDay;
}

bool readInteger(const std::string& prompt, int& value) {
    std::cout << prompt;
    if (std::cin >> value) return true;
    if (std::cin.eof()) return false;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid input. Enter a whole number.\n";
    return false;
}

bool recordMaturity(const FixedDeposit& fd, const Account& account) {
    const Transaction tx{account.accountNumber, "FD_MATURITY_CREDIT",
                         fd.maturityAmount, account.balance, auditTimestamp()};
    if (!appendTransaction(tx)) return false;
    return writeAudit(account.accountNumber, "FD_MATURITY_CREDIT",
                      account.accountNumber,
                      "fd_id=" + fd.fdId + "; amount=" + std::to_string(fd.maturityAmount));
}

void printFd(const FixedDeposit& fd) {
    std::cout << std::fixed << std::setprecision(2)
              << "FD ID: " << fd.fdId
              << " | Principal: Rs. " << fd.principal
              << " | Annual rate: " << fd.annualInterestRate << "%"
              << " | Tenure: " << fd.durationMonths << " months"
              << " | Maturity: Rs. " << fd.maturityAmount
              << " | Maturity date: " << fd.maturityDate
              << " | Status: " << fd.status << '\n';
}
}

bool validFixedDepositTerms(double principal, double annualRate, int durationMonths) {
    return std::isfinite(principal) && std::isfinite(annualRate) &&
           principal >= 1000.0 && annualRate > 0.0 && annualRate <= 100.0 &&
           durationMonths >= 3 && durationMonths <= 120 && durationMonths % 3 == 0;
}

double calculateFdMaturityAmount(double principal, double annualRatePercent, int durationMonths) {
    if (!validFixedDepositTerms(principal, annualRatePercent, durationMonths)) return 0.0;
    const double quarterlyRate = (annualRatePercent / 100.0) / 4.0;
    const int quarters = durationMonths / 3;
    return std::round(principal * std::pow(1.0 + quarterlyRate, quarters) * 100.0) / 100.0;
}

double calculateFdInterest(double principal, double annualRatePercent, int durationMonths) {
    const double maturity = calculateFdMaturityAmount(principal, annualRatePercent, durationMonths);
    return maturity > 0.0 ? std::round((maturity - principal) * 100.0) / 100.0 : 0.0;
}

std::vector<FixedDeposit> loadFixedDeposits() {
    std::vector<FixedDeposit> result;
    std::ifstream input(FIXED_DEPOSIT_FILE);
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        FixedDeposit fd;
        if (!(row >> std::quoted(fd.fdId) >> std::quoted(fd.accountNumber)
                  >> fd.principal >> fd.annualInterestRate >> fd.durationMonths
                  >> std::quoted(fd.startDate) >> std::quoted(fd.maturityDate)
                  >> fd.maturityAmount >> std::quoted(fd.status))) continue;
        if (fd.fdId.empty() || fd.accountNumber.empty() ||
            !validFixedDepositTerms(fd.principal, fd.annualInterestRate, fd.durationMonths) ||
            !validDate(fd.startDate) || !validDate(fd.maturityDate) ||
            fd.maturityAmount < fd.principal ||
            (fd.status != "ACTIVE" && fd.status != "MATURED" && fd.status != "CLOSED")) continue;
        result.push_back(fd);
    }
    return result;
}

bool saveFixedDeposits(const std::vector<FixedDeposit>& deposits) {
    const std::string temp = FIXED_DEPOSIT_FILE + ".tmp";
    std::ofstream output(temp, std::ios::trunc);
    if (!output) return false;
    for (const auto& fd : deposits) {
        output << std::quoted(fd.fdId) << ' ' << std::quoted(fd.accountNumber) << ' '
               << std::setprecision(17) << fd.principal << ' ' << fd.annualInterestRate
               << ' ' << fd.durationMonths << ' ' << std::quoted(fd.startDate) << ' '
               << std::quoted(fd.maturityDate) << ' ' << std::fixed << std::setprecision(2)
               << fd.maturityAmount << ' ' << std::quoted(fd.status) << '\n';
    }
    output.close();
    if (!output) { std::remove(temp.c_str()); return false; }
    if (std::rename(temp.c_str(), FIXED_DEPOSIT_FILE.c_str()) != 0) {
        std::remove(temp.c_str());
        return false;
    }
    return true;
}

std::string nextFixedDepositId(const std::vector<FixedDeposit>& deposits) {
    unsigned long long largest = 0;
    for (const auto& fd : deposits) {
        if (fd.fdId.size() <= 2 || fd.fdId.substr(0, 2) != "FD") continue;
        try { largest = std::max(largest, std::stoull(fd.fdId.substr(2))); }
        catch (...) { }
    }
    std::ostringstream out;
    out << "FD" << std::setfill('0') << std::setw(6) << largest + 1;
    return out.str();
}

bool createFixedDepositRecord(const FixedDeposit& fd) {
    if (!validFixedDepositTerms(fd.principal, fd.annualInterestRate, fd.durationMonths) ||
        !validDate(fd.startDate) || !validDate(fd.maturityDate) ||
        fd.maturityAmount != calculateFdMaturityAmount(fd.principal, fd.annualInterestRate,
                                                       fd.durationMonths)) return false;
    auto deposits = loadFixedDeposits();
    for (const auto& old : deposits) if (old.fdId == fd.fdId) return false;
    deposits.push_back(fd);
    return saveFixedDeposits(deposits);
}

void createFixedDeposit(Account& account) {
    if (account.frozen || account.closed || account.deleted || account.locked) {
        std::cout << "This account cannot create an FD.\n";
        return;
    }
    double principal = 0.0;
    int annualRateInput = 0, months = 0;
    if (!readPositiveAmount("FD principal (minimum Rs. 1000): Rs. ", principal)) return;
    if (!readInteger("Annual interest rate in percent (e.g. 7): ", annualRateInput)) return;
    if (!readInteger("Tenure in months (3-120, multiple of 3): ", months)) return;
    const double rate = static_cast<double>(annualRateInput);
    if (!validFixedDepositTerms(principal, rate, months)) {
        std::cout << "Invalid FD terms. Principal >= Rs. 1000, rate > 0 and <= 100, tenure 3-120 months in multiples of 3.\n";
        return;
    }
    if (principal > account.balance) {
        std::cout << "Insufficient available balance.\n";
        logAudit("FD_CREATE_REJECTED", account.accountNumber, "reason=insufficient_balance");
        return;
    }

    auto deposits = loadFixedDeposits();
    FixedDeposit fd;
    fd.fdId = nextFixedDepositId(deposits);
    fd.accountNumber = account.accountNumber;
    fd.principal = principal;
    fd.annualInterestRate = rate;
    fd.durationMonths = months;
    fd.startDate = today();
    fd.maturityDate = addMonths(fd.startDate, months);
    fd.maturityAmount = calculateFdMaturityAmount(principal, rate, months);

    const double oldBalance = account.balance;
    account.balance -= principal;
    if (!saveAccounts()) {
        account.balance = oldBalance;
        std::cout << "Could not reserve principal; FD creation cancelled.\n";
        return;
    }
    deposits.push_back(fd);
    if (!saveFixedDeposits(deposits)) {
        account.balance = oldBalance;
        saveAccounts();
        std::cout << "Could not save FD; account balance was restored.\n";
        return;
    }
    writeAudit(account.accountNumber, "FD_CREATED", account.accountNumber,
               "fd_id=" + fd.fdId + "; principal=" + std::to_string(principal) +
               "; annual_rate=" + std::to_string(rate));
    std::cout << "Fixed deposit created successfully.\n";
    printFd(fd);
}

void viewFixedDeposits(const Account& account) {
    const auto deposits = loadFixedDeposits();
    bool found = false;
    std::cout << "\n========== YOUR FIXED DEPOSITS ==========\n";
    for (const auto& fd : deposits) {
        if (fd.accountNumber == account.accountNumber) { printFd(fd); found = true; }
    }
    if (!found) std::cout << "No fixed deposits found.\n";
    logAudit("FD_LIST_VIEWED", account.accountNumber);
}

void calculateFixedDepositReturns() {
    double principal = 0.0;
    int rateInput = 0, months = 0;
    if (!readPositiveAmount("Principal amount: Rs. ", principal)) return;
    if (!readInteger("Annual interest rate in percent: ", rateInput)) return;
    if (!readInteger("Tenure in months (multiple of 3): ", months)) return;
    const double rate = static_cast<double>(rateInput);
    if (!validFixedDepositTerms(principal, rate, months)) {
        std::cout << "Invalid FD terms. Check principal, rate and tenure.\n";
        return;
    }
    std::cout << std::fixed << std::setprecision(2)
              << "Principal: Rs. " << principal
              << "\nAnnual interest rate: " << rate << "%"
              << "\nCompounding: Quarterly"
              << "\nInterest earned: Rs. " << calculateFdInterest(principal, rate, months)
              << "\nMaturity amount: Rs. " << calculateFdMaturityAmount(principal, rate, months) << '\n';
}

void withdrawFixedDeposit(Account& account) {
    auto deposits = loadFixedDeposits();
    std::string id;
    std::cout << "Enter FD ID: ";
    if (!(std::cin >> id)) return;
    auto it = std::find_if(deposits.begin(), deposits.end(), [&](const FixedDeposit& fd) {
        return fd.fdId == id && fd.accountNumber == account.accountNumber;
    });
    if (it == deposits.end() || it->status != "ACTIVE") {
        std::cout << "Active FD not found.\n";
        return;
    }
    if (today() < it->maturityDate) {
        std::cout << "This FD has not matured. Premature closure is not enabled.\n";
        writeAudit(account.accountNumber, "FD_PREMATURE_WITHDRAWAL_REJECTED",
                   account.accountNumber, "fd_id=" + id + "; reason=policy_not_configured");
        return;
    }
    const double oldBalance = account.balance;
    account.balance += it->maturityAmount;
    if (!saveAccounts()) { account.balance = oldBalance; return; }
    if (!recordMaturity(*it, account)) {
        account.balance = oldBalance;
        saveAccounts();
        std::cout << "Maturity credit could not be recorded; balance restored.\n";
        return;
    }
    it->status = "MATURED";
    if (!saveFixedDeposits(deposits)) {
        account.balance = oldBalance;
        saveAccounts();
        std::cout << "Could not update FD status; balance restored. Please inspect transaction history before retrying.\n";
        return;
    }
    std::cout << "Maturity amount credited: Rs. " << std::fixed << std::setprecision(2)
              << it->maturityAmount << '\n';
}

void processMaturedFixedDeposits() {
    auto deposits = loadFixedDeposits();
    bool changed = false;
    for (auto& fd : deposits) {
        if (fd.status != "ACTIVE" || today() < fd.maturityDate) continue;
        const int index = findAccountIndex(fd.accountNumber);
        if (index < 0) continue;
        Account& account = accounts[static_cast<std::size_t>(index)];
        if (account.frozen || account.closed || account.deleted) continue;
        const double oldBalance = account.balance;
        account.balance += fd.maturityAmount;
        if (!saveAccounts()) { account.balance = oldBalance; continue; }
        if (!recordMaturity(fd, account)) {
            account.balance = oldBalance;
            saveAccounts();
            continue;
        }
        fd.status = "MATURED";
        changed = true;
    }
    if (changed) saveFixedDeposits(deposits);
}

void showFixedDepositMenu(Account& account) {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n========== FIXED DEPOSIT MENU ==========\n"
                  << "1. Create fixed deposit\n"
                  << "2. View my fixed deposits\n"
                  << "3. Calculate FD returns\n"
                  << "4. Withdraw matured FD\n"
                  << "0. Back\n";
        if (!readMenuChoice("Choice: ", choice)) return;
        switch (choice) {
            case 1: createFixedDeposit(account); break;
            case 2: viewFixedDeposits(account); break;
            case 3: calculateFixedDepositReturns(); break;
            case 4: withdrawFixedDeposit(account); break;
            case 0: break;
            default: std::cout << "Invalid choice.\n";
        }
    }
}
