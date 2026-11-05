#include "test_framework.h"

#include <iostream>

int main() {
    int passed = 0;
    int failed = 0;

    std::cout << "\n========== AUTOMATED TESTS ==========\n";
    for (const TestCase& test : testRegistry()) {
        try {
            test.function();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cout << "[FAIL] " << test.name << " : " << error.what() << '\n';
        } catch (...) {
            ++failed;
            std::cout << "[FAIL] " << test.name << " : unknown exception\n";
        }
    }

    std::cout << "\n========== TEST SUMMARY ==========\n"
              << "Passed: " << passed << '\n'
              << "Failed: " << failed << '\n'
              << "Total:  " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
