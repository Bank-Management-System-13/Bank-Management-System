\# Contributing to the Bank Management System



\## 1. Introduction



This document defines the contribution guidelines and collaborative development workflow for the Bank Management System, developed as a Software Engineering mini-project at PES University.



The project uses Git and GitHub for source-code management, Jira for sprint planning and task tracking, and GitHub Actions for continuous integration.



All team members are expected to follow these guidelines to maintain code quality, collaboration, traceability, and consistency throughout development.



\## 2. Project Repository



\*\*GitHub Repository:\*\*  

https://github.com/Bank-Management-System-13/Bank-Management-System



\*\*GitHub Organization:\*\*  

Bank-Management-System-13



\*\*Jira Project:\*\*  

Team Astro (SCRUM)



\*\*Jira Board:\*\*  

https://github.com/orgs/Bank-Management-System-13/projects/2



\## 3. Team Responsibilities



The project is developed collaboratively by three team members.



| Team Member | Responsibilities |

|---|---|

| KR – Kruthik | Customer authentication, PIN security, integration coordination |

| KC – Kiran | Audit logging, transaction reports, testing |

| KB – Charan | Administrative account management |



Responsibilities are divided using Jira issues and subtasks. Each member should work primarily on the modules assigned to them.



Shared files and interfaces must be coordinated before making changes to avoid integration conflicts.



\## 4. Development Environment



The project uses the following tools and technologies:



\- C++17 for application development.

\- GCC/G++ for compilation.

\- MSYS2 UCRT64 on Windows for development.

\- Git for version control.

\- GitHub for repository hosting and collaboration.

\- Jira for sprint planning and progress tracking.

\- GitHub Actions for automated builds and testing.



\## 5. Branching Strategy



The project follows a feature-branch workflow.



\### 5.1 Main Branch



The `main` branch contains the primary project code and integrated documentation.



Direct development on `main` should be avoided. Changes should be developed on separate branches and integrated through pull requests.



\### 5.2 Integration Branch



The `integration/sprint2` branch is used to collect and review Sprint 2 contributions before final integration into `main`.



\### 5.3 Feature Branches



Each team member should create a separate branch for their assigned work.



Examples:



```bash

feature/sprint2-kr-auth-security

feature/sprint2-kc-audit-reports

feature/sprint2-kb-admin

```



Feature branches should be created from the latest appropriate integration branch.



\## 6. Development Workflow



The following workflow should be followed for every contribution.



\### Step 1: Update the Local Repository



Before starting new work, fetch the latest changes:



```bash

git fetch origin

```



Switch to the appropriate base branch and update it:



```bash

git switch integration/sprint2

git pull origin integration/sprint2

```



\### Step 2: Create a Feature Branch



Create a new branch for the assigned feature:



```bash

git switch -c feature/your-feature-name

```



Use descriptive branch names that identify the feature being implemented.



\### Step 3: Implement the Changes



\- Work only on the files assigned to the feature.

\- Follow the existing project architecture and coding conventions.

\- Reuse shared data structures and interfaces.

\- Avoid unnecessary modifications to unrelated modules.

\- Coordinate changes to shared files with the integration coordinator.



\### Step 4: Compile and Test



Compile the Sprint 2 application from the `Sprint2/` directory:



```bash

g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/\*.cpp -o bank\_management\_system.exe

```



Run the application:



```bash

./bank\_management\_system.exe

```



Run relevant automated tests whenever applicable.



Do not submit code that has known compilation errors or failing tests.



\### Step 5: Review Changes



Before committing, inspect the working tree:



```bash

git status --short

git diff --check

git diff

```



Check that:



\- Only intended files have been modified.

\- No generated executables or runtime data files are included.

\- No temporary files or debugging statements remain.

\- The changes follow the agreed module interfaces.



\### Step 6: Commit Changes



Use descriptive commit messages.



Examples:



```bash

git add Sprint2/src/auth.cpp Sprint2/include/auth.h

git commit -m "feat: implement customer authentication"

```



Other commit message prefixes include:



| Prefix | Purpose |

|---|---|

| `feat:` | New functionality |

| `fix:` | Bug fixes |

| `test:` | Automated or manual test additions |

| `docs:` | Documentation updates |

| `refactor:` | Code restructuring |

| `chore:` | Maintenance and configuration changes |



Each commit should represent a meaningful, logical change.



Avoid empty commits and artificial commit splitting.



\### Step 7: Push the Feature Branch



```bash

git push -u origin feature/your-feature-name

```



Subsequent pushes can use:



```bash

git push

```



\### Step 8: Create a Pull Request



Create a pull request on GitHub.



The pull request should:



\- Use the feature branch as its source.

\- Target `integration/sprint2` for Sprint 2 contributions.

\- Include a descriptive title and summary.

\- Reference the related Jira issue.

\- Explain the implemented functionality.

\- Mention the tests performed.

\- Identify any known limitations.



\### Step 9: Code Review and Integration



\- Another team member or the integration coordinator should review the changes.

\- Check code correctness, readability, module compatibility, and test results.

\- Resolve review comments before merging.

\- Confirm that GitHub Actions checks pass.

\- Merge only after the pull request is approved.



The integration coordinator is responsible for coordinating the final Sprint 2 integration.



\## 7. Jira Task Management



Jira is used to organize and track development work.



Each feature or development activity should be associated with the appropriate Jira issue.



The expected issue workflow is:



1\. \*\*To Do:\*\* Work has not started.

2\. \*\*In Progress:\*\* Development or testing is actively underway.

3\. \*\*In Review:\*\* Implementation is complete and awaiting review.

4\. \*\*Done:\*\* The work has been reviewed, integrated, and verified against its acceptance criteria.



Jira issues should be updated according to actual development progress.



Subtasks should be used when a larger issue requires contributions from multiple team members.



Pull requests should reference their corresponding Jira issue wherever possible.



\## 8. Coding Guidelines



All contributors should follow these guidelines:



\- Use meaningful variable, function, and class names.

\- Keep functions focused on a single responsibility.

\- Avoid unnecessary code duplication.

\- Use appropriate header files for module declarations.

\- Keep implementation logic in the corresponding source files.

\- Validate user input before performing operations.

\- Handle file access and runtime errors appropriately.

\- Preserve compatibility with the existing Sprint 1 data format.

\- Avoid hardcoding unnecessary values.

\- Add comments where they clarify non-obvious logic.



\## 9. Project Module Ownership



Sprint 2 follows a modular architecture.



| Module | Main Responsibility |

|---|---|

| `main.cpp` | Application entry point and menu integration |

| `storage.cpp` | Account and transaction persistence |

| `auth.cpp` | Customer authentication and account login |

| `security.cpp` | PIN masking and input validation |

| `admin.cpp` | Administrative account operations |

| `audit.cpp` | Timestamped audit logging |

| `reports.cpp` | Account and transaction reports |



Shared interfaces and data structures are maintained in the corresponding header files under `Sprint2/include/`.



Changes to shared interfaces must be communicated to all affected contributors before implementation.



\## 10. Testing Guidelines



Testing is an important part of the development process.



Contributors should:



\- Compile the application after relevant changes.

\- Test normal and invalid input scenarios.

\- Verify that account and transaction data persist across restarts.

\- Check that administrative operations enforce the correct account restrictions.

\- Verify that important operations generate the expected audit events.

\- Run relevant automated regression tests.

\- Check that existing functionality is not broken by new changes.



Generated test data should be kept separate from normal application data.



\## 11. Continuous Integration



GitHub Actions is used to automate build and testing activities.



The workflow is responsible for:



\- Checking out the repository.

\- Installing the required compiler and coverage tools.

\- Building the Sprint 1 application.

\- Building the Sprint 2 application.

\- Compiling the automated test suite.

\- Executing tests in an isolated runtime directory.

\- Generating a GCC coverage summary.



Pull requests should be checked for successful CI execution before integration.



A failing build or test workflow must be investigated and resolved before merging.



\## 12. Files That Should Not Be Committed



The following generated or temporary files should not be included in normal source-code contributions:



\- Compiled executables such as `.exe` files.

\- Object files such as `.o` files.

\- GCC coverage files such as `.gcda`, `.gcno`, and `.gcov`.

\- Runtime account and transaction data.

\- Generated audit logs.

\- Temporary test output.

\- IDE-specific temporary configuration files.



The repository's `.gitignore` should be maintained to exclude these files where appropriate.



\## 13. Pull Request Checklist



Before submitting a pull request, contributors should verify:



\- \[ ] The feature is associated with the correct Jira issue.

\- \[ ] The changes are committed to the appropriate feature branch.

\- \[ ] The application compiles successfully.

\- \[ ] Relevant automated tests pass.

\- \[ ] Existing functionality remains compatible.

\- \[ ] No generated runtime data or executables are included.

\- \[ ] The pull request has a meaningful title and description.

\- \[ ] Any shared interface changes have been communicated.

\- \[ ] GitHub Actions checks have been reviewed.



\## 14. Collaboration and Communication



All contributors are expected to:



\- Communicate changes that affect shared modules.

\- Avoid directly modifying another member's assigned implementation without coordination.

\- Review pull requests constructively.

\- Report bugs and integration problems promptly.

\- Keep Jira issue statuses accurate.

\- Maintain a clear and traceable development history.



The integration coordinator should ensure that independently developed modules work together as a complete application.



\## 15. Conclusion



Following these contribution guidelines helps maintain a structured development process and ensures that all team members follow consistent practices.



The combination of feature branches, meaningful commits, pull-request reviews, Jira task tracking, automated testing, and continuous integration supports collaborative development and reliable integration of the Bank Management System.

