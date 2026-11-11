# Bank Management System - Sprint 2 Architecture

## 1. Overview

Sprint 2 extends the original Sprint 1 C++ console-based Bank
Management System. It introduces a modular architecture that
separates account persistence, authentication, security,
administrative operations, audit logging and reporting.

The application is built using C++17 and uses file-based storage
to maintain compatibility with the existing Sprint 1 data.

## 2. High-Level Architecture

The application follows a modular, layered design.

User
 |
 v
src/main.cpp
 |
 +--> Authentication Module
 |      src/auth.cpp
 |
 +--> Security and Input Validation Module
 |      src/security.cpp
 |
 +--> Administration Module
 |      src/admin.cpp
 |
 +--> Reporting Module
 |      src/reports.cpp
 |
 +--> Audit Module
 |      src/audit.cpp
 |
 +--> Storage Module
        src/storage.cpp
             |
             v
       Persistent Data Files

Shared data structures are defined in include/models.h.
Module interfaces are declared in the corresponding header files.

## 3. Module Responsibilities

### 3.1 Main Application
Files: src/main.cpp

Provides the main application entry point, displays menus,
and coordinates customer and administrator workflows.

### 3.2 Authentication
Files: src/auth.cpp, include/auth.h

Handles customer account creation and login, PIN verification,
failed login attempts, account lockout and login restrictions.

### 3.3 Security and Input Validation
Files: src/security.cpp, include/security.h

Provides PIN validation and masking, account number validation,
customer name validation, menu input validation, amount validation
and administrator PIN verification.

### 3.4 Administration
Files: src/admin.cpp, include/admin.h

Provides administrator operations including account search,
account viewing, freezing, unfreezing, closing, reopening,
soft deletion and login unlocking.

### 3.5 Storage
Files: src/storage.cpp, include/storage.h

Handles account and transaction persistence. Maintains compatibility
with legacy Sprint 1 account and transaction records.

### 3.6 Audit Logging
Files: src/audit.cpp, include/audit.h

Records application events with timestamps, event types,
account identifiers, details and actor information.

### 3.7 Reporting
Files: src/reports.cpp, include/reports.h

Provides mini statements, full transaction history, account
summaries and exported account statements.

## 4. Data Flow

1. The user interacts with the console menus.
2. The main application invokes the relevant module.
3. Authentication and security functions validate user input
   and access conditions.
4. Storage functions load or persist account and transaction data.
5. Relevant operations generate audit events.
6. Reporting functions retrieve transaction records and produce
   account-specific reports.

## 5. Persistence

The application uses file-based persistence.

- accounts.txt stores account records.
- transactions.txt stores transaction records.
- audit.log stores audit events.
- Exported statements are written to individual text files.

The Sprint 2 implementation preserves compatibility with the
existing Sprint 1 account format and supports legacy transaction
records without timestamps.

## 6. Automated Testing

The tests are maintained under tests/.

The custom test framework provides TEST and CHECK macros.
Tests cover administrative operations, audit logging, reports,
security validation and storage behaviour.

The regression suite is executed in an isolated working directory
to avoid modifying normal application data.

## 7. CI/CD Workflow

GitHub Actions performs the following operations:

1. Checks out the repository.
2. Installs the C++ compiler and coverage tools.
3. Builds the Sprint 1 application.
4. Builds the Sprint 2 application.
5. Compiles the automated tests with coverage instrumentation.
6. Executes the tests in an isolated runtime directory.
7. Generates a GCC coverage summary.

## 8. Build Instructions

Run from the Sprint2 directory:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/*.cpp -o bank_management_system.exe
./bank_management_system.exe
