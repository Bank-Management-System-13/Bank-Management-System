# Admin Automated Test Evidence

**Project:** Bank Management System (Sprint 2)
**Scope:** Admin account search and viewing, freeze/unfreeze, close/reopen, soft delete, login unlock and admin authentication
**Branch:** `feature/tests-admin`
**Test file:** `Sprint2/tests/test_admin.cpp`
**Code under test:** `Sprint2/src/admin.cpp` (and `verifyAdminPIN()` in `Sprint2/src/security.cpp`)
**Date of run:** 2026-11-10

## 1. Summary

| Item | Result |
| --- | --- |
| Admin tests in the suite | 27 (2 existing + 25 new) |
| Passed | 27 |
| Failed | 0 |
| `admin.cpp` line coverage | 100% (49 of 49 lines) |
| `admin.cpp` function coverage | 100% (3 of 3 functions) |
| `admin.cpp` branch coverage | 62.8% (120 of 191 branches) |

Branch coverage is lower than line coverage because gcov also counts the hidden branches inside the C++ standard library (stream and string operations) that are generated from each line of `admin.cpp`. The behaviours that matter to the user are each exercised by at least one test: every menu option, the not-found case, the invalid-action case, non-numeric input, the save-failure case and both the success and failure paths of admin login. One path is not tested: the admin menu closing when input ends unexpectedly (end of input while reading a menu choice).

## 2. How the tests work

- The tests use the project's own framework (`tests/test_framework.h`): `TEST(name)` registers a test and `CHECK(condition)` fails it.
- Each test builds its own accounts in memory, feeds the admin screen scripted keyboard input by replacing `std::cin`'s buffer, and captures the screen text from `std::cout`.
- A test then checks the screen text, the account fields, `accounts.txt` (by reloading it with `loadAccounts()`) and `audit.log`.
- Each new test starts and ends with `accounts.txt`, `audit.log` and the in-memory accounts removed, even when a check fails, so tests do not affect each other.
- The admin authentication tests (23 to 27) read the PIN through `std::cin`, which works on Linux, where GitHub Actions runs. On Windows the PIN is read straight from the console (`_getch`), which a test cannot feed, so these five tests are compiled only on Linux (`#ifndef _WIN32`).

## 3. Test cases, expected results and outcome

Requirement groups follow the Sprint 2 testing assignment: (1) search and view, (2) freeze and unfreeze, (3) close and reopen, (4) soft delete, (5) admin authentication including wrong-PIN handling.

| ID | Group | Test name | Steps (input) | Expected result | Result |
| --- | --- | --- | --- | --- | --- |
| 01 | 2 | `AdminCanFreezeAnExistingAccount` (existing) | Account 300001, choose Freeze | `frozen` is true | PASS |
| 02 | 4 | `AdminCanSoftDeleteAnAccount` (existing) | Account 300002, choose Soft delete | `deleted` is true, record kept, account no longer found | PASS |
| 03 | 1 | `Admin_SearchShowsDetailsOfOneAccount` | Search 300101 | Number, name, balance `Rs. 1250.50`, locked/frozen/closed flags shown; other accounts not shown | PASS |
| 04 | 1 | `Admin_SearchAllListsEveryActiveAccount` | Search `ALL` | Every active account listed | PASS |
| 05 | 1 | `Admin_SearchAllSkipsSoftDeletedAccounts` | Three accounts, one deleted, search `ALL` | The two active accounts are listed, the deleted one is not | PASS |
| 06 | 1 | `Admin_SearchUnknownAccountReportsNoMatch` | Search 999999 | "No matching active record found." | PASS |
| 07 | 1 | `Admin_SearchHidesSoftDeletedAccount` | Search a soft-deleted account | "No matching active record found." | PASS |
| 08 | 1 | `Admin_SearchShowsFrozenClosedAndLockedFlags` | Search an account that is frozen, closed and locked | Frozen, Closed and Login locked all show `Yes` | PASS |
| 09 | 1 | `Admin_SearchIsWrittenToAuditLog` | Search 300101 | `audit.log` has an `ACCOUNT_SEARCH` line for `acct=300101` | PASS |
| 10 | 2 | `Admin_UnfreezeClearsFrozenFlag` | Frozen account, choose Unfreeze | `frozen` is false, balance unchanged, "Account status updated." | PASS |
| 11 | 2 | `Admin_FreezingTwiceKeepsAccountFrozen` | Choose Freeze twice | Account stays frozen | PASS |
| 12 | 3 | `Admin_CloseAndReopenChangeOnlyClosedFlag` | Close, then Reopen a frozen account with balance 750 | Close sets `closed`; Reopen clears it; `frozen` and balance never change | PASS |
| 13 | 4 | `Admin_SoftDeleteKeepsRecordAndBalance` | Soft delete an account with balance 500 | Record kept in the list, `deleted` true, name and balance unchanged | PASS |
| 14 | 4 | `Admin_SoftDeletedAccountCannotBeManagedAgain` | Try to freeze a soft-deleted account | "Account not found.", no change | PASS |
| 15 | 4 | `Admin_SoftDeleteIsPersistedAfterReload` | Soft delete, then reload `accounts.txt` | Account still marked deleted, balance kept, not found by lookup | PASS |
| 16 | 2 | `Admin_UnlockLoginResetsLockAndFailedAttempts` | Locked account with 3 failed attempts, choose Unlock | `locked` false, `failedAttempts` 0, balance unchanged | PASS |
| 17 | 2, 3 | `Admin_StatusChangesArePersistedAfterReload` | Freeze, close, then reload `accounts.txt` | Reloaded account is frozen and closed, not deleted | PASS |
| 18 | 1 to 4 | `Admin_StatusChangesAreAuditLogged` | Freeze, unfreeze, close, reopen, unlock, soft delete | `audit.log` has `ACCOUNT_FROZEN`, `ACCOUNT_UNFROZEN`, `ACCOUNT_CLOSED`, `ACCOUNT_REOPENED`, `LOGIN_UNLOCKED`, `ACCOUNT_SOFT_DELETED`, with the account number and `actor=ADMIN` | PASS |
| 19 | 1 to 4 | `Admin_UnknownAccountChangesNothing` | Manage account 999999 | "Account not found.", no account changed, nothing written to `accounts.txt` | PASS |
| 20 | 1 to 4 | `Admin_InvalidActionChangesNothing` | Choose action 9 | "Invalid action.", no flag changed, nothing saved | PASS |
| 21 | 1 to 4 | `Admin_NonNumericActionChangesNothing` | Enter `abc` as the action | "Invalid input.", no flag changed | PASS |
| 22 | 1 to 4 | `Admin_StatusChangeIsRolledBackWhenSaveFails` | Make saving fail, then choose Freeze | "Could not save status change.", account is not frozen (change rolled back) | PASS |
| 23 | 5 | `Admin_CorrectPinOpensMenuAndLogsLoginAndLogout` | Admin PIN correct, then Logout | Admin menu shown; `ADMIN_LOGIN_SUCCESS` and `ADMIN_LOGOUT` logged | PASS |
| 24 | 5 | `Admin_WrongPinIsRejected` | Wrong admin PIN | "Admin authentication failed.", no admin menu, `ADMIN_LOGIN_FAILED` logged, no `ADMIN_LOGIN_SUCCESS` | PASS |
| 25 | 5 | `Admin_PinsAreNeverWrittenToAuditLog` | One correct and one wrong admin login | Neither PIN appears anywhere in `audit.log` | PASS |
| 26 | 1 to 4 | `Admin_MenuReachesSearchAndStatusScreens` | Login, menu option 1 (search ALL), option 2 (freeze 300101), Logout | Search output shown, status updated, account frozen | PASS |
| 27 | 5 | `Admin_MenuRejectsInvalidChoice` | Login, enter menu choice 7 | "Invalid choice." | PASS |

## 4. Test run output

Built and run with the same compiler flags as the GitHub Actions workflow (`g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests`), on Ubuntu with g++ 13.3.0. The build produced no warnings.

```text
========== AUTOMATED TESTS ==========
[PASS] AdminCanFreezeAnExistingAccount
[PASS] AdminCanSoftDeleteAnAccount
[PASS] Admin_SearchShowsDetailsOfOneAccount
[PASS] Admin_SearchAllListsEveryActiveAccount
[PASS] Admin_SearchAllSkipsSoftDeletedAccounts
[PASS] Admin_SearchUnknownAccountReportsNoMatch
[PASS] Admin_SearchHidesSoftDeletedAccount
[PASS] Admin_SearchShowsFrozenClosedAndLockedFlags
[PASS] Admin_SearchIsWrittenToAuditLog
[PASS] Admin_UnfreezeClearsFrozenFlag
[PASS] Admin_FreezingTwiceKeepsAccountFrozen
[PASS] Admin_CloseAndReopenChangeOnlyClosedFlag
[PASS] Admin_SoftDeleteKeepsRecordAndBalance
[PASS] Admin_SoftDeletedAccountCannotBeManagedAgain
[PASS] Admin_SoftDeleteIsPersistedAfterReload
[PASS] Admin_UnlockLoginResetsLockAndFailedAttempts
[PASS] Admin_StatusChangesArePersistedAfterReload
[PASS] Admin_StatusChangesAreAuditLogged
[PASS] Admin_UnknownAccountChangesNothing
[PASS] Admin_InvalidActionChangesNothing
[PASS] Admin_NonNumericActionChangesNothing
[PASS] Admin_StatusChangeIsRolledBackWhenSaveFails
[PASS] Admin_CorrectPinOpensMenuAndLogsLoginAndLogout
[PASS] Admin_WrongPinIsRejected
[PASS] Admin_PinsAreNeverWrittenToAuditLog
[PASS] Admin_MenuReachesSearchAndStatusScreens
[PASS] Admin_MenuRejectsInvalidChoice

========== TEST SUMMARY ==========
Passed: 27
Failed: 0
Total:  27
```

## 5. Coverage of `src/admin.cpp`

Measured with `gcovr --root . --filter 'src/admin.cpp' --print-summary` after running the admin tests:

```text
File                                         Lines     Exec  Cover   Missing
src/admin.cpp                                   49       49   100%
lines: 100.0% (49 out of 49)
functions: 100.0% (3 out of 3)
branches: 62.8% (120 out of 191)
```

## 6. Checks that the tests can detect faults

To confirm the tests are not passing by accident, six deliberate faults were introduced one at a time into a temporary copy of `admin.cpp`. Each fault made at least one test fail, and the real code was restored afterwards.

| Fault introduced | Test(s) that failed |
| --- | --- |
| Unfreeze no longer clears the frozen flag | `Admin_UnfreezeClearsFrozenFlag` |
| Search also shows soft-deleted accounts | `Admin_SearchAllSkipsSoftDeletedAccounts`, `Admin_SearchHidesSoftDeletedAccount` |
| A failed save is not rolled back | `Admin_StatusChangeIsRolledBackWhenSaveFails` |
| Unlock does not reset failed attempts | `Admin_UnlockLoginResetsLockAndFailedAttempts` |
| Admin actions are not written to the audit log | `Admin_StatusChangesAreAuditLogged` |
| A wrong admin PIN is accepted | `Admin_WrongPinIsRejected`, `Admin_PinsAreNeverWrittenToAuditLog` |

The full suite was also built with the AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=address,undefined`) and ran with no errors reported.

## 7. How to run the tests

From the repository root (Linux, or Windows using the MSYS2 UCRT64 terminal):

```bash
cd Sprint2
mkdir -p test-runtime
g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests \
  tests/test_main.cpp tests/test_audit.cpp tests/test_storage.cpp \
  tests/test_reports.cpp tests/test_security.cpp tests/test_admin.cpp \
  src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp src/admin.cpp \
  -o test-runtime/tests
cd test-runtime
./tests
```

The tests run in the `test-runtime` folder so that `accounts.txt` and `audit.log` created by the tests never touch real data. GitHub Actions runs the same commands on every pull request.

## 8. Limitations

- The five admin authentication tests (23 to 27) run on Linux only, because the Windows PIN prompt reads the console directly. On Windows, admin login should be checked manually: choose the admin option, enter a wrong PIN and confirm "Admin authentication failed.", then enter the correct PIN and confirm the admin menu opens.
- The admin PIN is a fixed value in `security.cpp`, so the tests assume that value. If it is changed before the demonstration, tests 23, 25, 26 and 27 need the new PIN.
- The results in sections 4 and 5 were produced by building the team's code on Linux with the GitHub Actions flags. The result of the official run is shown in the Checks tab of the pull request for this branch.
