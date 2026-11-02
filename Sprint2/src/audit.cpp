#include "audit.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

static const string AUDIT_FILE_NAME = "audit.log";

string auditTimestamp() {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

bool logAudit(const string& event,
              const string& accountNumber,
              const string& details) {
    ofstream file(AUDIT_FILE_NAME, ios::app);
    if (!file) {
        cerr << "Warning: Could not open the audit log.\n";
        return false;
    }

    file << '[' << auditTimestamp() << "] "
         << left << setw(22) << event
         << " | acct=" << (accountNumber.empty() ? "-" : accountNumber);
    if (!details.empty()) {
        file << " | " << details;
    }
    file << '\n';

    if (!file) {
        cerr << "Warning: Failed while writing to the audit log.\n";
        return false;
    }
    return true;
}
