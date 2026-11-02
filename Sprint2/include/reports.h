#ifndef BANK_REPORTS_H
#define BANK_REPORTS_H
#include "models.h"

// SCRUM-12: account and transaction reports for the logged-in customer.
// All functions are read-only: they never change the account or its history.

// Last 5 transactions of the account.
void showMiniStatement(const Account& account);

// Every transaction of the account, with deposit/withdrawal totals.
void showFullHistory(const Account& account);

// One-page summary: status, balance, counts and totals, first/latest activity.
void showAccountSummary(const Account& account);

// Writes the full statement to statement_<account number>.txt.
void exportStatement(const Account& account);

// Sub-menu that offers the four reports above.
// The customer menu only needs one entry, e.g. "4. Reports" -> showReportsMenu(...).
void showReportsMenu(const Account& account);

#endif
