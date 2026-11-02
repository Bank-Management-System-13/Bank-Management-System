# Sprint 2 regression checklist

## Sprint 1 regression
- [ ] Create an account with a name containing spaces.
- [ ] Restart the program and confirm the account remains available.
- [ ] Log in with the correct PIN.
- [ ] Confirm an incorrect PIN is rejected.
- [ ] Deposit a positive amount and verify the balance.
- [ ] Withdraw an amount within the balance and verify the balance.
- [ ] Attempt to withdraw more than the balance; confirm the balance is unchanged.
- [ ] Try zero, negative, non-numeric, and non-finite amounts; confirm they are rejected.
- [ ] Exit and restart; confirm the final balance persists.

## SCRUM-8 — PIN attempt lockout
- [ ] Enter an incorrect PIN once and verify remaining-attempt message.
- [ ] Enter an incorrect PIN twice and verify remaining-attempt message.
- [ ] Enter an incorrect PIN a third time and verify account lockout.
- [ ] Restart and verify lockout persists.
- [ ] Admin unlocks the account; verify failed-attempt count resets and login works.
- [ ] Verify correct login resets an earlier failed-attempt count.

## SCRUM-9 — audit logging
- [ ] Verify account creation, successful/failed login, logout, balance enquiry, deposit, withdrawal, admin actions, and report generation append timestamped entries to `audit.log`.
- [ ] Verify each entry identifies actor, action, account number, and details.

## SCRUM-10 / SCRUM-11 — admin account management
- [ ] Search for an existing account and view customer name, balance, and status.
- [ ] Search for a nonexistent account.
- [ ] Freeze an account and verify customer login is blocked.
- [ ] Unfreeze an account and verify login works.
- [ ] Close an account and verify customer login is blocked.
- [ ] Reopen an account and verify login works.
- [ ] Soft-delete an account and verify it is hidden from normal search/login while historical records remain.

## SCRUM-12 — reports
- [ ] Generate account report and verify customer details, account number, balance, and account statuses.
- [ ] Generate transaction report and verify deposits/withdrawals, amounts, timestamps, and balance-after values.
- [ ] Verify old Sprint 1 transaction rows without timestamps appear as legacy records.

## Integration
- [ ] Compile all `.cpp` files together with the documented command.
- [ ] Run the complete customer and admin workflows.
- [ ] Confirm all files are created in the expected working directory.
