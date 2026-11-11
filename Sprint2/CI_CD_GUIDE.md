# CI/CD and Automated Testing Guide

## 1. Purpose

The project uses GitHub Actions to automatically build both
Sprint 1 and Sprint 2 applications, execute automated regression
tests, and generate a code coverage summary.

## 2. Continuous Integration

Continuous Integration (CI) checks whether changes can be built
and tested successfully.

The workflow is triggered by:
- Pushes to main.
- Pushes to feature/tests-audit-reports.
- Pull requests targeting main.
- Manual workflow dispatch.

## 3. Workflow Stages

### Stage 1: Checkout
Retrieves the repository source code for the workflow.

### Stage 2: Install Tools
Installs GCC and gcov for compilation and coverage measurement.

### Stage 3: Build Sprint 1
Compiles the original application using C++17 and warning flags.

### Stage 4: Build Sprint 2
Compiles all Sprint 2 source modules into the application.

### Stage 5: Build Automated Tests
Compiles the custom test framework, test files and required
implementation modules with coverage instrumentation.

### Stage 6: Execute Tests
Runs the automated test executable from an isolated runtime
directory so test data does not modify normal application data.

### Stage 7: Generate Coverage
Uses gcov to produce a coverage summary for Sprint 2 source files.

## 4. Automated Test Framework

The project uses a lightweight custom C++ test framework.

TEST(name) registers a test case.

CHECK(condition) verifies an expected condition. If the condition
fails, the framework throws an error containing the failed
condition and source line.

The test registry allows the test runner to execute registered
tests and report their results.

## 5. Test Categories

The regression suite covers:

- Account and transaction persistence.
- Legacy transaction compatibility.
- Security and input validation.
- Administrative account operations.
- Account status persistence.
- Audit event generation.
- Account summaries and transaction reports.
- Statement export and file creation failures.

## 6. Code Coverage

The workflow uses GCC coverage instrumentation and gcov.

Coverage measurements include:

- Line coverage: executable source lines reached by tests.
- Function coverage: functions entered during test execution.
- Branch coverage: conditional branches exercised by tests.

Coverage is a measurement of test execution, not proof that
the application is completely correct.

## 7. Running Tests Locally

Run these commands from the Sprint2 directory:

```bash
mkdir -p test-runtime

g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests \
tests/test_main.cpp tests/test_audit.cpp tests/test_storage.cpp \
tests/test_reports.cpp tests/test_security.cpp tests/test_admin.cpp \
src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp \
src/admin.cpp src/auth.cpp \
-o test-runtime/tests

cd test-runtime
./tests
