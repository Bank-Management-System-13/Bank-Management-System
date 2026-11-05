#ifndef BANK_AUDIT_H
#define BANK_AUDIT_H

#include <string>

// Timestamped, append-only audit log. Never include PINs or other secrets.
bool logAudit(const std::string& event,
              const std::string& accountNumber,
              const std::string& details = "");

std::string auditTimestamp();

inline bool writeAudit(const std::string& actor,
                       const std::string& event,
                       const std::string& accountNumber,
                       const std::string& details = "") {
    std::string message = "actor=" + actor;
    if (!details.empty()) {
        message += "; " + details;
    }
    return logAudit(event, accountNumber, message);
}

#endif
