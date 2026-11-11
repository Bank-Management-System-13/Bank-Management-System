# Bank Management System

A console-based Bank Management System developed in **C++** as part of the Software Engineering mini project at PES University. The application provides basic banking operations and uses file-based storage to maintain account information between program executions.

## Features

- **Account Creation:** Create a customer account with the required details.
- **Customer Login:** Authenticate customers using their account number and PIN.
- **PIN Validation:** Validate PIN input and mask PIN characters during entry.
- **Deposit and Withdrawal:** Support basic money deposit and withdrawal operations.
- **Balance Enquiry:** Check the available account balance.
- **Input Validation:** Handle invalid input and reject invalid account details.
- **Persistent Storage:** Store account information in a file so that it can be loaded in subsequent executions.

*Note: Verify each feature against the latest integrated code before submission.*

## Technology Stack

- **Programming Language:** C++
- **Compiler:** GCC (`g++`)
- **Standard:** C++17
- **Version Control:** Git and GitHub
- **Continuous Integration:** GitHub Actions

## Prerequisites

Ensure the following are installed:

- GCC C++ compiler
- Git (optional, for cloning the repository)

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/Bank-Management-System-13/Bank_MANAGEMENT_SYSTEM_13.git
cd Bank_MANAGEMENT_SYSTEM_13
```

### 2. Compile the Program

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o bank_management_system
```

### 3. Run the Program

On Linux or macOS:

```bash
./bank_management_system
```

On Windows using the MSYS2 UCRT64 terminal:

```bash
./bank_management_system.exe
```

If compilation produces an executable with a different name, use that name when running the program.

## Project Structure

```text
Bank-Management-System/
├── .github/
│   └── workflows/
│       └── blank.yml
├── main.cpp
├── README.md
└── .gitignore
```

The repository may contain additional files, including project documentation and supporting files.

## Development Workflow

The project follows a collaborative GitHub workflow:

1. Tasks are divided among team members.
2. Features are developed on separate branches.
3. Changes are committed and pushed to GitHub.
4. Pull requests are used for code review and integration.
5. GitHub Actions checks that the C++ program compiles.

Sprint planning and task tracking can be managed using Jira, while GitHub is used for source code management and collaboration.

## Team

**Institution:** PES University  
**Department:** Computer Science and Engineering  
**Project:** Software Engineering Mini Project — Bank Management System

## Future Enhancements

- Account lockout after repeated incorrect PIN attempts
- Administrative account management
- Transaction history and audit logging
- Enhanced authentication and data protection
- Expanded automated testing

## Academic Project

This project demonstrates the application of C++ programming, input validation, file handling, version control, collaborative development, and software engineering practices to a basic banking application.

## Sprint 2 – Advanced Banking Features

Sprint 2 extends the original banking application with a modular C++17
implementation, administrative operations, security improvements, audit
logging, transaction reports, and automated regression tests.

### Additional Features

- **PIN Security:** Four-digit PIN validation, masked PIN entry, and
  account lockout after three consecutive incorrect login attempts.
- **Account Management:** Admin account search, account details,
  freeze/unfreeze, close/reopen, soft deletion, and account unlocking.
- **Audit Logging:** Timestamped records of important customer and
  administrator activities.
- **Customer Reports:** Mini statements, complete transaction history,
  account summaries, and statement export.
- **Persistent Storage:** Account status and transaction information
  are maintained across application restarts.
- **Automated Testing:** Regression tests cover administrative operations,
  security validation, reports, storage, and audit logging.

### Sprint 2 Architecture

The Sprint 2 implementation separates application responsibilities
into dedicated modules:

| Module | Responsibility |
|---|---|
| main.cpp | Application entry point and menu integration |
| storage.cpp | Account and transaction persistence |
| auth.cpp | Customer account creation and authentication |
| security.cpp | PIN masking and input validation |
| admin.cpp | Administrative account operations |
| audit.cpp | Timestamped audit logging |
| reports.cpp | Account and transaction reports |

Shared data structures and interfaces are maintained in the
corresponding header files under `include/`.

### Build and Run – Sprint 2

Open a terminal in the `Sprint2` directory.

Compile:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/*.cpp -o bank_management_system.exe
