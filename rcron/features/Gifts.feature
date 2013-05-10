Feature: Gifts
AS a member
I WANT my recent requested contribution to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id      | fullName   | address | city  | state  | postalCode | country | email         | flags           |
  | NEW.ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | a@example.com | dft,ok,personal |
  And balances:
  | id        | usd  | r   | rewards |
  | cgf       |    0 |   0 |       0 |
  | NEW.ZZA   |  100 |  20 |      20 |
  And gifts:
  | id      | giftDate  | amount | often | honor  | honored | share | completed |
  | NEW.ZZA | %today-1d |     10 |     1 | memory | Jane Do |    10 |         0 |

Scenario: A contribution can be completed
  When cron runs "gifts"
  Then transactions:
  | xid      | created | type         | state    | amount | from      | to      | purpose      | r    |
  | NEW.AAAB | %today  | %TX_TRANSFER | %TX_DONE |     10 | NEW.ZZA   | cgf     | contribution |   10 |
  | NEW.AAAC | %today  | %TX_REBATE   | %TX_DONE |   0.50 | community | NEW.ZZA | rebate on #1 | 0.50 |
  | NEW.AAAD | %today  | %TX_BONUS    | %TX_DONE |   1.00 | community | cgf     | bonus on #1  | 1.00 |
  And gifts:
  | id      | giftDate  | amount | often | honor  | honored | share | completed |
  | NEW.ZZA | %today-1d |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "new payment" to member "cgf" with subs:
  | otherName | amount | payeePurpose |
  | Abe One   |    $10 | contribution |
  And we tell staff "gift accepted" with:
  | amount | often | myName  | rewardType |
  |     10 |     1 | Abe One | rebate     |
  # and many other fields
