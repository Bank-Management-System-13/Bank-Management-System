# Bank Management System - Sprint 2 Security Notes

## 1. Overview

Sprint 2 introduces additional security controls to protect customer account access and restrict administrative operations.

## 2. Customer PIN Security

- Customer PINs must contain exactly four digits.
- PIN entry is masked in the console.
- Invalid PIN formats are rejected.
- Three consecutive incorrect login attempts lock the account.
- Failed-attempt counts and lock status persist across application restarts.
- Successful authentication resets the failed-attempt counter.

## 3. Account Validation

- Account numbers are validated before account operations.
- Empty customer names are rejected.
- Invalid menu choices and monetary inputs are handled.
- Non-positive and non-finite transaction amounts are rejected.

## 4. Administrative Access

- Administrative operations require administrator authentication.
- The administrator can unlock customer accounts after login lockout.
- Account status restrictions prevent access to frozen or closed accounts.
- Administrative actions are recorded in the audit log.

## 5. Audit Logging

Important security-related events are recorded, including:

- Successful and failed customer login attempts.
- Account lockout events.
- Administrator account-management actions.
- Rejected transactions and other relevant operations.

## 6. Known Security Limitations

This project is an academic prototype and is not intended for production banking use.

- The administrator PIN is currently hardcoded in the source code.
- Customer PINs remain in the legacy plaintext storage format for Sprint 1 compatibility.
- The application uses local file-based storage rather than a secure database.
- Production deployment would require salted password hashing, secure credential management, stronger access controls, and protection of stored data.

## 7. Security Testing

Automated tests cover PIN validation, account-number validation, customer-name validation, administrative operations, account lockout, persistence, and audit logging.
