\# Bank Management System – Sprint 2 Demonstration Guide



\## 1. Project Overview



The Bank Management System is a console-based banking application developed using C++ as part of the Software Engineering mini-project at PES University.



The project was developed across two sprints.



\- \*\*Sprint 1:\*\* Implemented the fundamental banking operations, including customer account creation, customer login, PIN validation, deposits, withdrawals, balance enquiry, and file-based persistence.

\- \*\*Sprint 2:\*\* Extended the application with a modular architecture, enhanced security controls, administrative account management, audit logging, transaction reporting, and automated testing.



The objective of Sprint 2 was to improve the functionality, security, maintainability, and reliability of the original banking application.



\## 2. Technology Stack



| Technology | Purpose |

|---|---|

| C++17 | Core application development |

| GCC (g++) | Compilation |

| File handling | Persistent account and transaction storage |

| Git | Version control |

| GitHub | Repository hosting and collaboration |

| GitHub Actions | Continuous Integration and automated builds |

| Custom C++ test framework | Automated regression testing |

| gcov | Source-code coverage analysis |

| Jira | Sprint planning and task tracking |



\## 3. Sprint 2 Architecture



Sprint 2 follows a modular architecture in which different responsibilities are separated into individual modules.



\### Module Responsibilities



| Module | Responsibility |

|---|---|

| `main.cpp` | Application entry point, menus, and workflow integration |

| `storage.cpp` | Account and transaction persistence |

| `auth.cpp` | Customer account creation and authentication |

| `security.cpp` | PIN masking, input validation, and administrator PIN verification |

| `admin.cpp` | Administrative account management |

| `audit.cpp` | Timestamped audit logging |

| `reports.cpp` | Account and transaction reports |



The corresponding header files are maintained inside `include/`.



\### Application Data Flow



1\. The user interacts with the application through the console menus.

2\. The main application invokes the appropriate module.

3\. Authentication and security functions validate user input and access conditions.

4\. Storage functions retrieve or update account and transaction information.

5\. Important operations generate audit log entries.

6\. Reporting functions retrieve stored information and generate reports.



\### Persistence



The application uses file-based persistence.



\- `accounts.txt` stores account records.

\- `transactions.txt` stores transaction records.

\- `audit.log` stores application audit events.

\- Exported statements are written to individual text files.



The implementation maintains compatibility with the original Sprint 1 account format and supports older transaction records without timestamps.



\## 4. Sprint 2 Features



\### 4.1 Customer Authentication and Security



\- Four-digit customer PIN validation.

\- Masked PIN entry in the console.

\- Three consecutive incorrect login attempts trigger account lockout.

\- Failed-attempt count and lock status persist across application restarts.

\- Successful authentication resets the failed-attempt counter.

\- Frozen and closed accounts cannot be accessed through normal customer login.



\### 4.2 Input Validation



\- Validates account numbers.

\- Rejects blank customer names.

\- Validates four-digit PINs.

\- Rejects invalid menu choices.

\- Rejects non-numeric, zero, negative, and non-finite transaction amounts.



\### 4.3 Administrative Account Management



The administrator can:



\- Search for customer accounts.

\- View account details.

\- View multiple accounts.

\- Freeze and unfreeze accounts.

\- Close and reopen accounts.

\- Soft-delete accounts.

\- Unlock accounts after login lockout.



Soft deletion preserves the account record and its historical information instead of physically removing the record.



\### 4.4 Audit Logging



The application records important customer and administrative activities.



Examples include:



\- Account creation.

\- Successful and failed login attempts.

\- Account lockout.

\- Deposits and withdrawals.

\- Rejected transactions.

\- Account freezing and unfreezing.

\- Account closure and reopening.

\- Account deletion.

\- Administrative operations.

\- Report generation.



Audit entries include timestamps, event types, account identifiers, and relevant details.



\### 4.5 Reporting



The reporting module provides:



\- Account summaries.

\- Transaction history.

\- Account-specific transaction filtering.

\- Mini statements.

\- Exported account statements.



Reports are generated using the stored account and transaction information.



\## 5. Demonstration Sequence



The following sequence can be used during the faculty demonstration.



\### Step 1: Launch the Application



Open the MSYS2 UCRT64 terminal and navigate to the Sprint 2 directory.



Compile:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/\*.cpp -o bank\_management\_system.exe

```



Run:



```bash

./bank\_management\_system.exe

```



Explain that the application starts through `main.cpp`, which coordinates the different modules.



\### Step 2: Demonstrate Customer Account Creation



1\. Select the account creation option.

2\. Enter the required customer details.

3\. Enter a valid four-digit PIN.

4\. Show the account number generated by the application.



Explain that account creation is handled by the authentication module and the account is persisted through the storage module.



\### Step 3: Demonstrate Customer Login



1\. Select customer login.

2\. Enter the generated account number.

3\. Enter the customer PIN.

4\. Show that the PIN is masked during entry.



Explain how authentication validates the credentials before granting access.



\### Step 4: Demonstrate Banking Operations



Demonstrate:



\- Balance enquiry.

\- Deposit.

\- Withdrawal.

\- Updated balance.



Also demonstrate rejection of a withdrawal when the requested amount exceeds the available balance, if time permits.



Explain that transaction operations update the stored account balance and maintain transaction history.



\### Step 5: Demonstrate PIN Lockout



1\. Log out of the customer account.

2\. Attempt login using an incorrect PIN three consecutive times.

3\. Show that the account becomes locked.

4\. Restart the application and demonstrate that the lock status persists.

5\. Use the administrator unlock functionality to restore access.



Explain the failed-attempt counter and persistent account-lockout mechanism.



\### Step 6: Demonstrate Administrative Operations



Log in using the configured administrator PIN.



The demonstration administrator PIN is currently `2468`, as configured in `src/security.cpp`.



Demonstrate:



\- Searching for an account.

\- Viewing account details.

\- Freezing an account.

\- Unfreezing an account.

\- Closing an account.

\- Reopening an account.

\- Soft-deleting an account.

\- Unlocking a locked account.



Explain how account-status changes restrict or restore customer access.



\### Step 7: Demonstrate Audit Logging



Open `audit.log` after performing a few operations.



Show the recorded events and explain how the audit module maintains a history of important application activities.



\### Step 8: Demonstrate Reports



Use the reporting menu to demonstrate:



\- Account summary.

\- Transaction history.

\- Mini statement.

\- Statement export.



Explain how the reports retrieve and organize information from the stored records.



\### Step 9: Demonstrate Persistence



1\. Perform a deposit or withdrawal.

2\. Exit the application.

3\. Launch the application again.

4\. Log in to the same account.

5\. Check the updated balance.



Explain that file-based persistence allows account and transaction information to remain available after application restarts.



\## 6. Automated Testing



Sprint 2 includes a custom-built C++ testing framework.



\### Framework Components



\- `TestCase`: Stores the test name and function pointer.

\- `testRegistry()`: Maintains the registered test cases.

\- `TestRegistrar`: Automatically registers test functions.

\- `TEST(name)`: Declares and registers a test.

\- `CHECK(condition)`: Evaluates assertions and reports failures.



\### Test Coverage Areas



The automated test suite covers:



\- Security and input validation.

\- Customer PIN validation.

\- Account-number and customer-name validation.

\- Account persistence.

\- Transaction persistence.

\- Legacy transaction compatibility.

\- Administrative operations.

\- Account status changes.

\- Audit logging.

\- Account summaries and transaction reports.

\- Statement export.



\### Running Automated Tests



From the `Sprint2/` directory:



```bash

mkdir -p test-runtime

```



Compile the test suite:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests tests/test\_main.cpp tests/test\_audit.cpp tests/test\_storage.cpp tests/test\_reports.cpp tests/test\_security.cpp tests/test\_admin.cpp src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp src/admin.cpp src/auth.cpp -o test-runtime/tests.exe

```



Execute the tests:



```bash

cd test-runtime

./tests.exe

```



The test runner displays:



\- PASS for successful test cases.

\- FAIL for unsuccessful test cases.

\- Passed test count.

\- Failed test count.

\- Total executed test count.



A successful test run should report zero failures.



\### Coverage Analysis



The project uses GCC's `gcov` tool to inspect source-code coverage.



Coverage instrumentation is enabled during test compilation using `--coverage`.



Coverage reports can be generated using `gcov` with the appropriate source and generated `.gcda` files.



\## 7. CI/CD Workflow



GitHub Actions automates the build and testing process.



The workflow:



1\. Checks out the repository.

2\. Installs the required C++ compiler and coverage tools.

3\. Builds the Sprint 1 application.

4\. Builds the Sprint 2 application.

5\. Compiles the automated test suite with coverage instrumentation.

6\. Executes the tests in an isolated runtime directory.

7\. Generates a source-code coverage summary.



This helps identify build errors and test failures whenever changes are pushed or pull requests are created for configured branches.



\## 8. Git and Team Collaboration



The project uses Git and GitHub for collaborative development.



The development workflow includes:



1\. Dividing Sprint 2 tasks among team members.

2\. Developing features in separate branches.

3\. Committing and pushing changes.

4\. Creating pull requests for code review.

5\. Integrating approved changes.

6\. Using GitHub Actions for automated build and testing.



Jira is used for sprint planning, task assignment, and progress tracking.



\## 9. Security Limitations and Future Improvements



The current implementation is an academic prototype and is not intended for production banking use.



Known limitations include:



\- The administrator PIN is hardcoded in the source code.

\- Customer PINs remain in the legacy plaintext storage format for Sprint 1 compatibility.

\- The application uses local text files instead of a secure database.

\- The application does not implement production-grade authorization, encryption, or secure credential management.



Potential future improvements include:



\- Salted password hashing.

\- Secure database-backed persistence.

\- Stronger administrator authentication.

\- Role-based access control.

\- Encryption of sensitive stored information.

\- More comprehensive automated integration testing.



\## 10. Conclusion



Sprint 2 extends the original Bank Management System by introducing modular software architecture, improved authentication and security controls, administrative account management, audit logging, reporting, automated regression testing, and a CI workflow.



The project demonstrates the practical application of C++ programming, modular design, file handling, version control, collaborative development, software testing, and continuous integration.



The two-sprint development process demonstrates how an initially basic banking application can be incrementally extended into a more structured and maintainable software system.



