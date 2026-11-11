#include <sstream>
#include <iostream>
#include "test_framework.h"
#include "security.h"

class ScopedCinRedirect {
private:
    std::streambuf* originalBuffer;

public:
    explicit ScopedCinRedirect(std::istream& input)
        : originalBuffer(std::cin.rdbuf(input.rdbuf())) {}

    ~ScopedCinRedirect() {
        std::cin.rdbuf(originalBuffer);
    }

    ScopedCinRedirect(const ScopedCinRedirect&) = delete;
    ScopedCinRedirect& operator=(const ScopedCinRedirect&) = delete;
};

TEST(SecurityAcceptsOnlyFourDigitPins) {
    CHECK(validPIN("1234"));
    CHECK(validPIN("0000"));
    CHECK(!validPIN("123"));
    CHECK(!validPIN("12345"));
    CHECK(!validPIN("12a4"));
    CHECK(!validPIN(""));
}

TEST(SecurityValidatesAccountNumbers) {
    CHECK(validAccountNumber("100001"));
    CHECK(!validAccountNumber(""));
    CHECK(!validAccountNumber("10A001"));
    CHECK(!validAccountNumber("100 01"));
}

TEST(SecurityRejectsBlankCustomerNames) {
    CHECK(validCustomerName("Kiran BG"));
    CHECK(validCustomerName(" A "));
    CHECK(!validCustomerName(""));
    CHECK(!validCustomerName("   \t\n"));
}
TEST(SecurityAcceptsValidMenuChoice) {
    std::istringstream input("2\n");
    ScopedCinRedirect redirect(input);

    int choice = 0;
    CHECK(readMenuChoice("Choice: ", choice));
    CHECK(choice == 2);
}

TEST(SecurityRejectsNonNumericMenuChoice) {
    std::istringstream input("abc\n");
    ScopedCinRedirect redirect(input);

    int choice = 0;
    CHECK(!readMenuChoice("Choice: ", choice));
}

TEST(SecurityAcceptsPositiveAmount) {
    std::istringstream input("250.50\n");
    ScopedCinRedirect redirect(input);

    double amount = 0.0;
    CHECK(readPositiveAmount("Amount: ", amount));
    CHECK(amount == 250.50);
}

TEST(SecurityRejectsNonNumericAmount) {
    std::istringstream input("abc\n");
    ScopedCinRedirect redirect(input);

    double amount = 0.0;
    CHECK(!readPositiveAmount("Amount: ", amount));
}

TEST(SecurityRejectsZeroAmount) {
    std::istringstream input("0\n");
    ScopedCinRedirect redirect(input);

    double amount = 0.0;
    CHECK(!readPositiveAmount("Amount: ", amount));
}

TEST(SecurityRejectsNegativeAmount) {
    std::istringstream input("-100\n");
    ScopedCinRedirect redirect(input);

    double amount = 0.0;
    CHECK(!readPositiveAmount("Amount: ", amount));
}