# Sprint 2 — Unit, System Testing and Code Coverage Report

**Project:** Bank Management System  
**Module:** Sprint 2 (C++17 modular implementation)  
**Report type:** Test strategy, execution summary and source-code coverage  
**Coverage evidence:** GitHub Actions run [#43805968921](https://github.com/Bank-Management-System-13/Bank-Management-System/actions/runs/43805968921)  
**Verified commit:** `97b77d0a35a1601bd1d8d984de04c4acefc92f62`

---

## 1. Executive summary

The Sprint 2 application was built and its automated regression suite completed successfully in GitHub Actions. The verified run reported **102 tests passed and 0 failed**. The source-code coverage summary was:

| Metric | Covered | Total | Coverage |
|---|---:|---:|---:|
| Line coverage | 708 | 730 | **97.0%** |
| Function coverage | 56 | 56 | **100.0%** |
| Branch coverage | 1,146 | 1,281 | **89.5%** |

The CI workflow enforces minimums of **80% line coverage** and **80% branch coverage**. The measured result is above both gates.

> These figures are the recorded output from the coverage run. They measure execution of the selected Sprint 2 source files; they do not prove that the application is defect-free.

## 2. Testing objectives

The test suite was expanded to check that the banking application:

1. Validates customer details, account numbers, PINs, transaction amounts and fixed-deposit terms.
2. Persists account, transaction, fixed-deposit and audit information correctly.
3. Applies account status restrictions to login and transactions.
4. Calculates account reports and fixed-deposit returns correctly.
5. Handles invalid input, malformed records, file errors and rollback paths safely.
6. Prevents duplicate fixed-deposit maturity credits.
7. Continues to build and pass its regression suite through GitHub Actions.

## 3. Test levels and scope

### 3.1 Unit tests

Unit tests exercise individual functions or modules with controlled inputs and verify their return values, state changes, file output or console output.

| Module / test source | Main areas checked |
|---|---|
| `test_security.cpp` | PIN, account-number and customer-name validation; input edge cases |
| `test_storage.cpp` | Account and transaction persistence, lookup, status fields and legacy-format compatibility |
| `test_storage_edges.cpp` | Missing files, malformed rows, invalid records and filesystem write failures |
| `test_audit.cpp` | Audit event creation, timestamp and actor details, and log-write failure |
| `test_reports.cpp` | Account summaries, transaction filtering, mini statements, totals, export and menu edge cases |
| `test_admin.cpp` | Account search, freeze/unfreeze, close/reopen, soft delete and unlock operations |
| `test_fixed_deposit.cpp` | Quarterly compound-interest calculation, term validation, identifiers and persistence |
| `test_fixed_deposit_workflows.cpp` | Maturity-date calculation, FD creation/listing, maturity processing, withdrawal restrictions and failure recovery |
| `test_auth.cpp` | Account creation, login, failed PIN attempts, lockout, customer menu and transaction rollback |

### 3.2 Integration and system-workflow tests

These checks exercise connected operations across multiple modules. The tests use captured console input/output and a separate test working directory so test records do not overwrite normal application data.

| Workflow | Expected result verified |
|---|---|
| Customer account creation | Valid account is created and persisted; an audit event is written |
| Customer login | Valid login succeeds; malformed, unknown, locked, frozen and closed accounts are rejected |
| PIN lockout | Three incorrect attempts lock the account; successful login resets failed attempts |
| Deposit and withdrawal | Valid transactions update the balance and transaction history |
| Transaction failure recovery | A failed account save or transaction write cancels the operation and restores the previous balance |
| Reports navigation | Customer can reach report functions and return through menu choices |
| Fixed-deposit opening | Principal is reserved from available balance; FD record, ledger entry and audit event are created |
| Fixed-deposit maturity | Maturity amount is credited and the FD status changes to matured |
| Duplicate maturity protection | Reprocessing does not credit the same FD a second time |
| Account restrictions | Blocked account states prevent restricted transactions and FD actions |
| Admin account controls | Status changes and unlock actions are persisted and reflected in account behavior |

**Testing terminology:** These are automated integration/workflow tests executed against the C++ modules and file-backed storage. They exercise complete application workflows, but they are not a separate deployed production-environment or external-client end-to-end test.

### 3.3 Manual system regression checks

The following checks are useful when demonstrating the executable interactively. They should be performed on the target operating system and recorded separately; they are not included as additional automated test cases in the 102-test count.

| Manual check | Procedure / expected observation |
|---|---|
| Fresh application start | Start the executable with an empty test data directory; confirm menus appear without a crash |
| Restart persistence | Create an account, exit, restart and verify the account can be loaded |
| Customer authentication | Try correct and incorrect PINs; verify the three-attempt lockout and admin unlock flow |
| Deposit and withdrawal | Try valid, zero, negative, non-numeric and excessive amounts; verify rejected transactions do not change the balance |
| Account status controls | Freeze, unfreeze, close and reopen an account; verify the applicable login and transaction restrictions |
| Fixed-deposit lifecycle | Create an FD, restart the program, view it and verify maturity processing when it is due |
| Report export | Export a statement and open the generated statement file to check its contents |
| Audit trail | Complete representative customer and admin actions; inspect `audit.log` for the corresponding events |

## 4. Code coverage results

The verified coverage summary is:

| Coverage type | Result | Uncovered count |
|---|---:|---:|
| Lines | **97.0% (708 / 730)** | 22 |
| Functions | **100.0% (56 / 56)** | 0 |
| Branches | **89.5% (1,146 / 1,281)** | 135 |

### Coverage interpretation

- **Line coverage (97.0%)** means 708 of the 730 measured executable source lines were run by the automated tests.
- **Function coverage (100.0%)** means all 56 measured functions were entered at least once.
- **Branch coverage (89.5%)** means 1,146 of the 1,281 measured branch outcomes were exercised.

The report is generated with GCC coverage instrumentation and gcovr. The source filter is `src/.*`, so this metric covers the selected Sprint 2 implementation files under `Sprint2/src`; it does not represent coverage of every file in the repository. The command excludes exception-only branches and unreachable compiler-generated branches using gcovr's `--exclude-throw-branches` and `--exclude-unreachable-branches` options. Application source files are not individually removed from the measurement.

## 5. Continuous Integration verification

The GitHub Actions workflow performs these steps:

1. Installs the C++ compiler and gcovr.
2. Builds the Sprint 1 application.
3. Builds the Sprint 2 application.
4. Compiles the automated test executable with coverage instrumentation.
5. Runs the tests from the isolated `Sprint2/test-runtime` directory.
6. Generates the line, function and branch coverage summary.
7. Uploads the detailed HTML coverage report.

**Recorded CI result:** The workflow run completed successfully. The build, test execution, coverage generation and artifact upload steps all passed.

- [Open the successful GitHub Actions run](https://github.com/Bank-Management-System-13/Bank-Management-System/actions/runs/43805968921)
- Artifact name: `sprint2-coverage-report`
- The artifact contains the detailed HTML coverage report generated by gcovr.

### Coverage quality gates

The workflow uses:

```bash
gcovr --root . --filter 'src/.*' \
  --exclude-throw-branches --exclude-unreachable-branches \
  --html-details coverage.html --print-summary \
  --fail-under-line 80 --fail-under-branch 80
```

The CI job fails if line coverage falls below 80% or branch coverage falls below 80%.

## 6. Running the tests locally

Run these commands from the `Sprint2/` directory in an environment with GCC and gcovr installed.

### Step 1 — Create an isolated test directory

```bash
mkdir -p test-runtime
```

### Step 2 — Compile the tests with coverage instrumentation

```bash
g++ -std=c++17 -Wall -Wextra -pedantic --coverage -Iinclude -Itests \
  tests/test_main.cpp tests/test_audit.cpp tests/test_storage.cpp \
  tests/test_reports.cpp tests/test_security.cpp tests/test_admin.cpp \
  tests/test_fixed_deposit.cpp tests/test_fixed_deposit_workflows.cpp \
  tests/test_auth.cpp tests/test_storage_edges.cpp \
  src/audit.cpp src/storage.cpp src/reports.cpp src/security.cpp \
  src/admin.cpp src/fixed_deposit.cpp src/auth.cpp \
  -o test-runtime/tests
```

### Step 3 — Execute the automated suite

```bash
(cd test-runtime && ./tests)
```

A successful run should finish with 102 passed and 0 failed for the verified source revision.

### Step 4 — Generate the coverage report

Run from `Sprint2/` after the instrumented test executable has completed:

```bash
gcovr --root . --filter 'src/.*' \
  --exclude-throw-branches --exclude-unreachable-branches \
  --html-details coverage.html --print-summary \
  --fail-under-line 80 --fail-under-branch 80
```

Open `coverage.html` to inspect file-level and line-level coverage details.

## 7. Limitations and recommendations

- Coverage is evidence of which measured code paths were executed; it is not proof that all requirements or possible input combinations are correct.
- The tests use file-backed storage and an in-process console harness. A production deployment would need additional environment-level, security, concurrency, recovery and operational testing.
- Manual checks should still be performed on the operating systems used for demonstration, especially around file replacement, console input and restart behavior.
- The current prototype stores customer PINs in the legacy plaintext format. This is a known security limitation and must be addressed before any real banking use.
- Future tests can target the remaining uncovered line and branch outcomes, prioritizing business-critical money movement, account restrictions and persistence failure paths.

## 8. Conclusion

For the verified Sprint 2 revision, the automated suite completed with **102 passed and 0 failed**. The measured implementation achieved **97.0% line coverage, 100.0% function coverage and 89.5% branch coverage**, exceeding the configured 80% line and branch gates. The CI workflow also produced and uploaded a detailed HTML coverage artifact.

The results support the quality of the tested Sprint 2 code paths while leaving manual system regression, security hardening and production-readiness validation as separate activities.
