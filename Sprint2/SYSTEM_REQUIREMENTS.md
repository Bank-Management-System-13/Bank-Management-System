\# Bank Management System – Sprint 2 System Requirements



\## 1. Introduction



This document describes the hardware, software, development environment, and runtime requirements for the Bank Management System.



The application is a console-based banking system developed using C++17. Sprint 2 introduces a modular architecture with authentication, security, administration, storage, audit logging, reporting, and automated testing.



\## 2. Hardware Requirements



The application has modest hardware requirements because it is a lightweight console-based C++ application.



| Component | Minimum Requirement |

|---|---|

| Processor | Any modern dual-core processor |

| RAM | 2 GB |

| Available Storage | 100 MB for source code, compiler output, and runtime files |

| Display | Standard terminal or console |

| Input Devices | Keyboard |



These are recommended minimum requirements for running the academic prototype, not formally benchmarked limits.



\## 3. Software Requirements



\### 3.1 Operating System



The application can be built and executed in environments with a compatible GCC C++ toolchain.



Supported development environments include:



\- Windows using MSYS2 UCRT64.

\- Linux distributions with GCC installed.

\- macOS with a compatible GCC toolchain.



\### 3.2 Programming Language



\- \*\*Language:\*\* C++

\- \*\*Standard:\*\* C++17



The C++17 standard is used to support the language features and standard library functionality required by the application.



\### 3.3 Compiler



\- \*\*Compiler:\*\* GNU C++ Compiler (G++)

\- \*\*Required standard:\*\* C++17



The compiler must support the `-std=c++17` option.



\### 3.4 Development and Version Control Tools



| Tool | Purpose |

|---|---|

| MSYS2 UCRT64 | Windows development environment |

| GCC / G++ | Compilation |

| Git | Source-code version control |

| GitHub | Repository hosting and collaboration |

| GitHub Actions | Automated build and test workflow |

| Jira | Sprint planning and task tracking |



An IDE or text editor such as Visual Studio Code may be used, but is not mandatory.



\## 4. Dependencies



\### 4.1 Application Dependencies



The Sprint 2 application primarily uses:



\- C++ standard library headers.

\- Standard input and output facilities.

\- File-stream facilities for persistent storage.

\- Platform-specific terminal functionality for masked PIN input.



No external database server or third-party application framework is required for the core application.



\### 4.2 Automated Testing Dependencies



The automated test suite uses:



\- GCC / G++ for test compilation.

\- The custom C++ test framework maintained under `tests/`.

\- GCC coverage instrumentation for coverage analysis.



The custom testing framework avoids requiring an external unit-testing library.



\### 4.3 Coverage Analysis



The project uses `gcov`, distributed with GCC, to inspect source-code coverage.



Coverage instrumentation is enabled during compilation using:



```bash

\--coverage

```



The generated coverage files are used for analysis and should not be committed as source files.



\## 5. Installation and Setup



\### 5.1 Windows Setup



Install MSYS2 and configure the UCRT64 environment with the required compiler and development tools.



Install the required packages:



```bash

pacman -S --needed mingw-w64-ucrt-x86\_64-gcc mingw-w64-ucrt-x86\_64-gcc-libs git

```



Verify the compiler:



```bash

g++ --version

```



Verify Git:



```bash

git --version

```



\### 5.2 Linux Setup



Install GCC and Git using the distribution's package manager.



For Ubuntu or Debian-based systems:



```bash

sudo apt update

sudo apt install g++ gcc git

```



Verify the installation:



```bash

g++ --version

git --version

```



\### 5.3 Clone the Repository



```bash

git clone https://github.com/Bank-Management-System-13/Bank-Management-System.git

cd Bank-Management-System

```



\## 6. Build Requirements



\### 6.1 Sprint 1



From the repository root:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o bank\_management\_system

```



On Windows, the output executable can be named:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o bank\_management\_system.exe

```



\### 6.2 Sprint 2



Navigate to the Sprint 2 directory:



```bash

cd Sprint2

```



Compile the modular application:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/\*.cpp -o bank\_management\_system.exe

```



Run the application in MSYS2 UCRT64:



```bash

./bank\_management\_system.exe

```



On Linux or macOS, the executable can be named `bank\_management\_system` and run using:



```bash

./bank\_management\_system

```



\## 7. Runtime Requirements



The application creates and maintains local data files in its current working directory.



| File | Purpose |

|---|---|

| `accounts.txt` | Stores customer account information |

| `transactions.txt` | Stores transaction records |

| `audit.log` | Stores application audit events |

| `statement\_<account\_number>.txt` | Stores exported customer statements |



The application requires permission to read and write files in its working directory.



Runtime files are generated by the application and should be kept separate from source-code files.



Existing Sprint 1 account and transaction records should be backed up before testing changes to the persistence implementation.



\## 8. Automated Testing Requirements



The automated tests are maintained in `Sprint2/tests/`.



To compile the test suite, run the following from the `Sprint2/` directory:



```bash

mkdir -p test-runtime



g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests \\

tests/test\_main.cpp tests/test\_audit.cpp tests/test\_storage.cpp \\

tests/test\_reports.cpp tests/test\_security.cpp tests/test\_admin.cpp \\

src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp \\

src/admin.cpp src/auth.cpp \\

\-o test-runtime/tests.exe

```



Execute the tests:



```bash

cd test-runtime

./tests.exe

```



The test runner reports passed and failed test cases and returns a non-zero exit status when failures occur.



\## 9. Continuous Integration Requirements



The GitHub Actions workflow requires:



\- A GitHub-hosted Ubuntu runner.

\- Repository checkout access.

\- GCC and the coverage utilities.

\- Permission to execute build and test commands.



The workflow builds both Sprint 1 and Sprint 2 applications, compiles the automated test suite, executes the tests in an isolated runtime directory, and generates a coverage summary.



\## 10. Operational Considerations



\- Run the application from a directory where it has permission to create and modify runtime files.

\- Back up account and transaction data before testing persistence changes.

\- Avoid committing executables, generated coverage files, and runtime banking records.

\- Use test accounts when demonstrating administrative and customer operations.

\- Keep the administrator PIN and other demonstration credentials confidential outside the academic demonstration environment.



\## 11. Security Considerations



The application is an academic prototype and is not suitable for production banking use.



Known limitations include:



\- The administrator PIN is hardcoded in the source code.

\- Customer PINs remain in the legacy plaintext storage format for Sprint 1 compatibility.

\- Data is maintained in local text files.

\- Production deployment would require secure credential hashing, encryption, authorization controls, and protected persistent storage.



\## 12. Conclusion



The Bank Management System requires a standard C++17 development environment, a compatible GCC compiler, and basic file-system access.



Its modular design, lightweight dependencies, custom automated testing framework, and GitHub Actions workflow make it suitable for academic development, demonstration, and further extension.

