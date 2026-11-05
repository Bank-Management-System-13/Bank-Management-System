#ifndef BANK_TEST_FRAMEWORK_H
#define BANK_TEST_FRAMEWORK_H

#include <stdexcept>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    void (*function)();
};

inline std::vector<TestCase>& testRegistry() {
    static std::vector<TestCase> registry;
    return registry;
}

struct TestRegistrar {
    TestRegistrar(const char* name, void (*function)()) {
        testRegistry().push_back({name, function});
    }
};

#define TEST(name) \
    void name(); \
    static TestRegistrar registrar_##name(#name, name); \
    void name()

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Check failed: ") + #condition + \
                                     " at line " + std::to_string(__LINE__)); \
        } \
    } while (false)

#endif
