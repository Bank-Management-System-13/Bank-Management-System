#include "test_framework.h"
#include "admin.h"
#include "storage.h"

#include <cstdio>
#include <iostream>
#include <sstream>

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
