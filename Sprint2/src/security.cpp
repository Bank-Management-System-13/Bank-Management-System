#include "security.h"
#include <cctype>
#include <cmath>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {
constexpr const char* ADMIN_PIN = "2468"; // Change before demonstration; academic prototype only.
}

bool validPIN(const std::string& pin) {
    if (pin.size() != 4) return false;
    for (unsigned char c : pin) if (!std::isdigit(c)) return false;
    return true;
}

std::string readPIN(const std::string& prompt) {
    std::string pin;
    std::cout << prompt << std::flush;
#ifdef _WIN32
    while (true) {
        int c = _getch();
        if (c == '\r' || c == '\n') { std::cout << '\n'; break; }
        if ((c == '\b' || c == 127) && !pin.empty()) { pin.pop_back(); std::cout << "\b \b"; }
        else if (c >= '0' && c <= '9' && pin.size() < 4) { pin.push_back(static_cast<char>(c)); std::cout << '*'; }
    }
#else
    termios oldState{}, newState{};
    bool changed = (tcgetattr(STDIN_FILENO, &oldState) == 0);
    if (changed) { newState = oldState; newState.c_lflag &= static_cast<tcflag_t>(~ECHO); tcsetattr(STDIN_FILENO, TCSANOW, &newState); }
    while (true) {
        int c = std::cin.get();
        if (c == EOF || c == '\n' || c == '\r') break;
        if ((c == '\b' || c == 127) && !pin.empty()) { pin.pop_back(); std::cout << "\b \b"; }
        else if (c >= '0' && c <= '9' && pin.size() < 4) { pin.push_back(static_cast<char>(c)); std::cout << '*'; }
    }
    if (changed) tcsetattr(STDIN_FILENO, TCSANOW, &oldState);
    std::cout << '\n';
#endif
    return pin;
}

bool validAccountNumber(const std::string& number) {
    if (number.empty()) return false;
    for (unsigned char c : number) if (!std::isdigit(c)) return false;
    return true;
}

bool validCustomerName(const std::string& name) {
    for (unsigned char c : name) if (!std::isspace(c)) return true;
    return false;
}

bool readMenuChoice(const std::string& prompt, int& choice) {
    std::cout << prompt;
    if (std::cin >> choice) return true;
    if (std::cin.eof()) return false;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid input. Enter a whole number.\n";
    return false;
}

bool readPositiveAmount(const std::string& prompt, double& amount) {
    std::cout << prompt;
    if (!(std::cin >> amount)) {
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid amount.\n"; return false;
    }
    if (!std::isfinite(amount) || amount <= 0.0) { std::cout << "Amount must be greater than zero.\n"; return false; }
    return true;
}

bool verifyAdminPIN() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string entered = readPIN("Enter admin PIN: ");
    return entered == ADMIN_PIN;
}
