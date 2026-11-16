# Bank Management System — Sprint 2 modular implementation

C++17 console application. This version keeps the Sprint 1 `accounts.txt` six-field format readable and writes three additional account-status fields when saved. Old transaction rows (four fields) remain readable; new rows include a timestamp.

## Build (from this folder)

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/*.cpp -o bank_management_system
```

Run on Linux/macOS: `./bank_management_system`
Run on Windows: `bank_management_system.exe`

## Files

- `include/models.h`: shared Account and Transaction structures.
- `include/storage.h`, `src/storage.cpp`: account and transaction file persistence.
- `include/security.h`, `src/security.cpp`: PIN masking/validation and input validation.
- `include/auth.h`, `src/auth.cpp`: account creation, customer login lockout and customer operations.
- `include/audit.h`, `src/audit.cpp`: timestamped audit trail in `audit.log`.
- `include/admin.h`, `src/admin.cpp`: account search/view and freeze/unfreeze/close/reopen/soft-delete/unlock.
- `include/reports.h`, `src/reports.cpp`: account and transaction report generation.
- `include/fixed_deposit.h`, `src/fixed_deposit.cpp`: fixed deposits with annual interest rates, quarterly compounding, maturity crediting, and customer FD reports.
- `src/main.cpp`: menu and module integration.

## Admin PIN

The demo admin PIN is `2468`, configured in `src/security.cpp`. Change it before your demonstration. This is an academic console prototype, not production banking software: customer PINs are still stored in the legacy plaintext format to preserve Sprint 1 data. A real system must use a vetted salted password-hashing scheme, secure secret management, authorization controls, and stronger transaction integrity.

## Generated files

`accounts.txt`, `transactions.txt`, `audit.log`, and `fixed_deposits.txt` are created in the current working directory. Exporting a customer statement creates `statement_<account number>.txt`.

## Automated tests

The regression suite is in `tests/`. GitHub Actions compiles and runs the tests in an isolated `test-runtime/` working directory, keeping the application's normal data files separate. The workflow prints gcovr line, function, and branch coverage and uploads an HTML coverage report named `sprint2-coverage-report`.

The test executable can be built from this folder on a system with GCC:

```bash
mkdir -p test-runtime
g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests \
  tests/test_main.cpp tests/test_audit.cpp tests/test_storage.cpp \
  tests/test_reports.cpp tests/test_security.cpp tests/test_admin.cpp tests/test_fixed_deposit.cpp \
  tests/test_fixed_deposit_workflows.cpp tests/test_auth.cpp tests/test_storage_edges.cpp \
  src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp src/admin.cpp src/fixed_deposit.cpp src/auth.cpp \
  -o test-runtime/tests
(cd test-runtime && ./tests)
```

## Fixed Deposits

Customers can create and view fixed deposits, preview returns, and receive a maturity credit. Rates are annual percentages and compounding is quarterly. This release supports 3–120 month tenures in whole-quarter increments, with principal plus interest paid at maturity. See [FIXED_DEPOSIT_DESIGN.md](FIXED_DEPOSIT_DESIGN.md) for the formula, data format, and current limitations.
