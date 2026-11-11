# Bank Management System - Sprint 2 Test Case Matrix

## 1. Purpose

This document maps the automated test cases to their respective modules and describes the behaviour each test verifies.

## 2. Test Case Matrix

### A. Administration Module

| Test ID | Test Case | Expected Result |
|---|---|---|
| ADM-01 | AdminCanFreezeAnExistingAccount | Existing account is frozen successfully. |
| ADM-02 | AdminCanSoftDeleteAnAccount | Account is marked as soft-deleted. |
| ADM-03 | Admin_SearchShowsDetailsOfOneAccount | Correct account details are displayed. |
| ADM-04 | Admin_SearchAllListsEveryActiveAccount | All active accounts are listed. |
| ADM-05 | Admin_SearchAllSkipsSoftDeletedAccounts | Soft-deleted accounts are excluded. |
| ADM-06 | Admin_SearchUnknownAccountReportsNoMatch | Appropriate no-match message is displayed. |
| ADM-07 | Admin_SearchHidesSoftDeletedAccount | Soft-deleted account is not exposed. |
| ADM-08 | Admin_SearchShowsFrozenClosedAndLockedFlags | Account status flags are displayed correctly. |
| ADM-09 | Admin_SearchIsWrittenToAuditLog | Search activity is recorded. |
| ADM-10 | Admin_UnfreezeClearsFrozenFlag | Frozen status is cleared. |
| ADM-11 | Admin_FreezingTwiceKeepsAccountFrozen | Repeated freezing preserves frozen status. |
| ADM-12 | Admin_CloseAndReopenChangeOnlyClosedFlag | Only the closed status changes. |
| ADM-13 | Admin_SoftDeleteKeepsRecordAndBalance | Record and balance remain stored. |
| ADM-14 | Admin_SoftDeletedAccountCannotBeManagedAgain | Further management is rejected. |
| ADM-15 | Admin_SoftDeleteIsPersistedAfterReload | Deleted status survives reload. |
| ADM-16 | Admin_UnlockLoginResetsLockAndFailedAttempts | Lock and failed-attempt count are reset. |
| ADM-17 | Admin_StatusChangesArePersistedAfterReload | Status changes survive reload. |
| ADM-18 | Admin_StatusChangesAreAuditLogged | Status changes are recorded. |
| ADM-19 | Admin_UnknownAccountChangesNothing | Unknown account operation makes no changes. |
| ADM-20 | Admin_InvalidActionChangesNothing | Invalid action makes no changes. |
| ADM-21 | Admin_NonNumericActionChangesNothing | Non-numeric action is rejected safely. |
| ADM-22 | Admin_StatusChangeIsRolledBackWhenSaveFails | Failed persistence does not leave an incorrect status. |
| ADM-23 | Admin_CorrectPINOpensMenuAndLogsLoginAndLogout | Correct PIN permits access and login/logout are logged. |
| ADM-24 | Admin_WrongPINIsRejected | Incorrect administrator PIN is rejected. |
| ADM-25 | Admin_PinsAreNeverWrittenToAuditLog | PIN values are not exposed in audit logs. |
| ADM-26 | Admin_MenuReachesSearchAndStatusScreens | Relevant administration screens are reachable. |
| ADM-27 | Admin_MenuRejectsInvalidChoice | Invalid menu selection is rejected. |

### B. Audit Logging Module

| Test ID | Test Case | Expected Result |
|---|---|---|
| AUD-01 | AuditLogCreatesTimestampedEntry | Audit entry contains a timestamp. |
| AUD-02 | AuditWriteHelperIncludesActor | Audit entry contains actor information. |

### C. Reporting Module

| Test ID | Test Case | Expected Result |
|---|---|---|
| REP-01 | ReportsSummaryShowsCorrectAccountAndTotals | Summary contains correct account and transaction totals. |
| REP-02 | ReportsFullHistoryFiltersOtherAccounts | Transaction history is account-specific. |
| REP-03 | ReportsExportStatementCreatesExpectedFile | Statement export creates the expected file. |
| REP-04 | ReportsHandleAccountWithNoTransactions | Empty transaction history is handled correctly. |
| REP-05 | ReportsExportHandlesFileCreationFailure | File creation failure is handled safely. |
| REP-06 | ReportsMenuReturnsOnZero | Zero selection returns from the report menu. |
| REP-07 | ReportsMenuRejectsInvalidChoice | Invalid report-menu selection is rejected. |
| REP-08 | ReportsMenuHandlesNonNumericInput | Non-numeric report-menu input is handled. |

### D. Security and Input Validation Module

| Test ID | Test Case | Expected Result |
|---|---|---|
| SEC-01 | SecurityAcceptsOnlyFourDigitPins | Only valid four-digit PINs are accepted. |
| SEC-02 | SecurityValidatesAccountNumbers | Account-number format is validated. |
| SEC-03 | SecurityRejectsBlankCustomerNames | Empty or whitespace-only names are rejected. |

### E. Storage Module

| Test ID | Test Case | Expected Result |
|---|---|---|
| STO-01 | StorageSavesAndReloadsAccountFields | Account fields persist after saving and reloading. |
| STO-02 | StorageAppendsAndLoadsTimestampedTransactions | Timestamped transactions are stored and loaded. |
| STO-03 | StorageReadsLegacyTransactionsWithoutTimestamp | Legacy transaction records remain readable. |
| STO-04 | StorageDoesNotFindSoftDeletedAccounts | Soft-deleted accounts are not returned as active accounts. |

## 3. Test Execution

The test suite uses a custom C++ test framework with TEST and CHECK macros.

Tests are compiled with GCC and can be executed in an isolated runtime directory to prevent test-generated data from modifying normal banking records.

## 4. Test Result Interpretation

- PASS: The test completed successfully.
- FAIL: An assertion or expected condition failed.
- Passed: Number of successful test cases.
- Failed: Number of unsuccessful test cases.
- Total: Number of executed test cases.

## 5. Important Note

This matrix documents the registered automated test cases. Actual pass/fail results must be taken from the latest test execution output.
