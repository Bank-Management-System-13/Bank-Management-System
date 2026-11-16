# Fixed Deposit module

## Business rules
- The annual rate is a percentage per annum (7.0 means 7% p.a.).
- Interest compounds quarterly; periodic rate = annual rate / 4.
- This initial release supports whole-quarter tenures from 3 to 120 months.
- Principal and earned interest are paid together at maturity; no interim payout.
- Principal is reserved from the customer's available account balance when the FD is opened.
- The maturity amount is credited once when maturity is processed.
- FD records are separate from ordinary account and transaction records.

## Formula
For principal P, annual rate R as a percentage, and q = durationMonths / 3:

A = P * (1 + (R / 100) / 4)^q

Maturity amount is rounded to two decimal places. Interest = maturity amount - principal.

## Persistence
Each line stores quoted FD ID, quoted account number, principal, annual percentage rate, duration months, quoted start date, quoted maturity date, maturity amount, and quoted status.

## Deliberate limitations
Premature closure is rejected until an explicit bank policy defines early-closure interest and penalties. No penalty rate is invented. Auto-renewal and timed background processing are not included. This is an academic file-backed prototype, not production banking software.
