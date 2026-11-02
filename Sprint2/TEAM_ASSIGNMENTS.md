# Sprint 2 — teammate file ownership

Keep the shared interfaces in `include/` consistent. Do not create duplicate `Account` or `Transaction` structures in teammate branches.

## Kruthik (KR) — SCRUM-8 and SCRUM-13
- `include/auth.h`, `src/auth.cpp`: three-attempt PIN lockout, persistent failed-attempt count, reset on successful login, blocked login for frozen/closed accounts.
- `include/security.h`, `src/security.cpp`: PIN format validation/masking, account-number/name/amount/menu validation, admin PIN verification.

## Kiran (KC) — SCRUM-9 and SCRUM-12
- `include/audit.h`, `src/audit.cpp`: timestamped append-only audit log.
- `include/reports.h`, `src/reports.cpp`: account report and transaction report.
- Coordinate with KR on the `Transaction` structure in `include/models.h` and the transaction file format in `include/storage.h` / `src/storage.cpp`.

## Charan (KB) — SCRUM-10 and SCRUM-11
- `include/admin.h`, `src/admin.cpp`: admin account search/view and freeze, unfreeze, close, reopen, soft-delete, and login-unlock actions.

## Shared integration — SCRUM-14
- Integrator owns `include/models.h`, `include/storage.h`, `src/storage.cpp`, `src/main.cpp`, and final merge/build/regression testing.
- All members contribute test cases and verify that Sprint 1 account creation, login, deposit, withdrawal, balance enquiry, and persistence still work.

## Integration rule

Each teammate should work on a separate branch and commit only their owned files plus tests/documentation. Merge the shared model/storage interface first, then feature branches. The integrator resolves conflicts and performs the final full build.
