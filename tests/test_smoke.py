import subprocess
import sys

executable = sys.argv[1]

result = subprocess.run(
    [executable],
    input="0\n",
    text=True,
    capture_output=True,
    timeout=10
)

output = result.stdout + result.stderr

assert result.returncode == 0, (
    f"Application exited with code {result.returncode}\n{output}"
)

assert "SPRINT 2" in output, (
    "Sprint 2 main menu was not displayed"
)

assert "Thank you for using the Bank Management System" in output, (
    "Application did not exit correctly"
)

print("PASS: Sprint 2 application starts and exits successfully.")
