#ifndef BANK_AUTH_H
#define BANK_AUTH_H
#include "models.h"
int customerLogin(); // returns account index on success, -1 otherwise
void customerMenu(int accountIndex);
void createAccount();
#endif
