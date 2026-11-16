#include "test_framework.h"
#include "audit.h"

#include <cstdio>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <regex>
#include <string>

TEST(AuditLogCreatesTimestampedEntry) {
    std::remove("audit.log");

    CHECK(logAudit("TEST_EVENT", "100001", "test details"));

    std::ifstream input("audit.log");
    CHECK(input.is_open());

    std::string line;
    CHECK(static_cast<bool>(std::getline(input, line)));
    CHECK(line.find("TEST_EVENT") != std::string::npos);
    CHECK(line.find("acct=100001") != std::string::npos);
    CHECK(line.find("test details") != std::string::npos);

    const std::regex timestamp(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\])");
    CHECK(std::regex_search(line, timestamp));
}

TEST(AuditWriteHelperIncludesActor) {
    std::remove("audit.log");

    CHECK(writeAudit("CUSTOMER", "LOGIN_SUCCESS", "100002", "authenticated"));

    std::ifstream input("audit.log");
    CHECK(input.is_open());

    std::string line;
    CHECK(static_cast<bool>(std::getline(input, line)));
    CHECK(line.find("actor=CUSTOMER") != std::string::npos);
    CHECK(line.find("LOGIN_SUCCESS") != std::string::npos);
    CHECK(line.find("acct=100002") != std::string::npos);
}


TEST(AuditLogReturnsFalseWhenAuditPathIsNotAFile) {
    std::error_code ignored;
    std::filesystem::remove_all("audit.log", ignored);
    CHECK(std::filesystem::create_directory("audit.log"));
    CHECK(!logAudit("SHOULD_FAIL", "100099", "path is a directory"));
    std::filesystem::remove_all("audit.log", ignored);
}
