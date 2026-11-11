# Sprint 2 regression and automated test plan

## Automated tests

The C++ test suite is in `tests/`. It uses a small in-repository test harness and runs from a separate working directory so generated files do not touch real banking records.

Covered automatically:
- Audit log creation, timestamp format, event, account number, details, and actor helper.
- Account persistence, account status fields, transaction persistence, legacy transaction rows, and soft-delete lookup behavior.
- Account summary totals, transaction-history account filtering, and statement export.
- PIN, account-number, and customer-name validation.
- Admin freeze and soft-delete operations.

## Manual regression checks

The following workflows still require manual interaction with the console application:
- Create an account with a name containing spaces and verify it after restart.
- Correct and incorrect customer PIN login; three-attempt lockout; restart persistence; admin unlock.
- Successful and rejected deposits/withdrawals, including zero, negative, non-numeric, and non-finite values.
- Balance enquiry and logout audit events.
- Admin search, unfreeze, close, reopen, and login behavior for frozen/closed accounts.
- Mini statement last-five behavior and interactive reports-menu navigation.

## Audit logging
- Automated checks verify the timestamped line format, event name, account number, details, and actor metadata.
- Manual checks verify application workflows append their expected events.

## Reports
- Automated checks verify summary fields and totals, account-specific transaction filtering, and exported statement contents.
- Manual checks verify mini statement output and interactive menu navigation.

## Integration and CI
- GitHub Actions builds Sprint 1 and the Sprint 2 application.
- GitHub Actions compiles and runs automated tests in an isolated runtime directory.
- GitHub Actions generates a gcovr coverage summary for Sprint 2 source files.

## Running Automated Tests Locally

Open the MSYS2 UCRT64 terminal and navigate to the Sprint2 directory.

### 1. Create the test runtime directory

Run: `mkdir -p test-runtime`

### 2. Compile the automated tests

Run from the Sprint2 directory:

`g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests tests/test_main.cpp tests/test_audit.cpp tests/test_storage.cpp tests/test_reports.cpp tests/test_security.cpp tests/test_admin.cpp src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp src/admin.cpp src/auth.cpp -o test-runtime/tests.exe`

### 3. Execute the tests

Run:

`cd test-runtime`

`./tests.exe`

### 4. Understand the results

- PASS indicates that a test completed successfully.
- FAIL indicates that an assertion or expected condition failed.
- Passed shows the number of successful tests.
- Failed shows the number of unsuccessful tests.
- Total shows the total number of executed tests.

A successful test run should report zero failures.

### 5. Test isolation and coverage

Tests execute in a separate runtime directory to prevent test-generated
account, transaction, and audit data from modifying normal application data.

Coverage instrumentation is enabled during compilation. GCC gcov can
be used to inspect source-code coverage.
