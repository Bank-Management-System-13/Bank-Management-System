#include "test_framework.h"
#include "security.h"

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
