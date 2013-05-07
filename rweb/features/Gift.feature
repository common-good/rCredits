Feature: Gift
AS a member
I WANT to contribute to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id      | fullName   | address | city  | state  | postalCode | country | email         | flags           |
  | NEW.ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | a@example.com | dft,ok,personal |
  And balances:
  | id        | usd  | r   | rewards |
  | cgf       |    0 |   0 |       0 |
  | NEW.ZZA   |  100 |  20 |      20 |

Scenario: A member contributes
  When member "NEW.ZZA" completes form "membership/contribute" with values:
  | gift | amount | often | honor  | honored | share |
  |    0 |     10 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | tx_id    | created | type         | state    | amount | from      | to      | purpose      | r    |
  | NEW.AAAB | %today  | %TX_TRANSFER | %TX_DONE |     10 | NEW.ZZA   | cgf     | contribution |   10 |
  | NEW.AAAC | %today  | %TX_REBATE   | %TX_DONE |   0.50 | community | NEW.ZZA | rebate on #1 | 0.50 |
  | NEW.AAAD | %today  | %TX_BONUS    | %TX_DONE |   1.00 | community | cgf     | bonus on #1  | 1.00 |
  And we say "status": "gift successful" with subs:
  | amount |
  |    $10 |
  And gifts:
  | id      | created | amount | often | honor  | honored | share | completed |
  | NEW.ZZA | %today  |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "new payment" to member "cgf" with subs:
  | otherName | amount | payeePurpose |
  | Abe One   |    $10 | contribution |
  And we email staff "gift accepted" with:
  | amount | often | payerTid |
  |     10 |     1 |        2 |

Scenario: A member contributes partly in USD
# Donations to CGF get full rewards, even if given in USD.
  When member "NEW.ZZA" completes form "membership/contribute" with values:
  | gift | amount | often | honor  | honored | share |
  |    0 |     50 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | tx_id    | created | type         | state    | amount | from      | to      | purpose      | r    |
  | NEW.AAAB | %today  | %TX_TRANSFER | %TX_DONE |     50 | NEW.ZZA   | cgf     | contribution |   20 |
  | NEW.AAAC | %today  | %TX_REBATE   | %TX_DONE |   2.50 | community | NEW.ZZA | rebate on #1 | 2.50 |
  | NEW.AAAD | %today  | %TX_BONUS    | %TX_DONE |   5.00 | community | cgf     | bonus on #1  | 5.00 |