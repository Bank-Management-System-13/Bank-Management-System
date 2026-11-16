# Fixed Deposit test coverage

Automated tests cover quarterly compound-interest arithmetic, invalid term boundaries, valid terms, sequential FD identifiers, persistence/reload, duplicate identifiers, and invalid persisted rows.

Manual integration checks:
1. Create an account and fund it.
2. Open an FD; verify that principal reduces the ordinary balance.
3. Verify customer-specific listing and persistence after restart.
4. Verify a non-matured FD cannot be withdrawn.
5. In isolated test data, set maturity date to today and verify credit and status.
6. Verify frozen, closed, and soft-deleted accounts are not credited by automatic maturity processing.
