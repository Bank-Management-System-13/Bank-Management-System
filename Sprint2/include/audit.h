#ifndef BANK_AUDIT_H
#define BANK_AUDIT_H
#include <string>

// SCRUM-9: timestamped, append-only audit log (audit.log).
//
// Line format:
//   [YYYY-MM-DD HH:MM:SS] EVENT_NAME | acct=<account number or -> | optional details
//
// Never pass PINs or other secrets in `details`.
// Pass an empty accountNumber for events that are not tied to an account
// (for example APP_START / APP_EXIT).
bool logAudit(const std::string& event,
              const std::string& accountNumber,
              const std::string& details = "");

// Current local time as "YYYY-MM-DD HH:MM:SS".
// Can also be used to fill Transaction::timestamp when a transaction is saved.
std::string auditTimestamp();

#endif
