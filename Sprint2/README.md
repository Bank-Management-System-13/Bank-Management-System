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
- `src/main.cpp`: menu and module integration.

## Admin PIN

The demo admin PIN is `2468`, configured in `src/security.cpp`. Change it before your demonstration. This is an academic console prototype, not production banking software: customer PINs are still stored in the legacy plaintext format to preserve Sprint 1 data. A real system must use a vetted salted password-hashing scheme, secure secret management, authorization controls, and stronger transaction integrity.

## Generated files

`accounts.txt`, `transactions.txt`, `audit.log`, `account_report.txt`, and `transaction_report.txt` are created in the current working directory.
