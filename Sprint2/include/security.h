#ifndef BANK_SECURITY_H
#define BANK_SECURITY_H
#include <string>
#include <iostream>
#include <limits>

bool validPIN(const std::string& pin);
std::string readPIN(const std::string& prompt = "Enter 4-digit PIN: ");
bool validAccountNumber(const std::string& number);
bool validCustomerName(const std::string& name);
bool readMenuChoice(const std::string& prompt, int& choice);
bool readPositiveAmount(const std::string& prompt, double& amount);
bool verifyAdminPIN();
#endif
