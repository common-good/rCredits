Feature: Gifts
AS a member
I WANT my recent requested contribution to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | email         | flags           |
  | .ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | a@example.com | dft,ok,personal |
  And balances:
  | id   | usd  | r   | rewards |
  | cgf  |    0 |   0 |       0 |
  | .ZZA |  100 |  20 |      20 |

Scenario: A contribution can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid   | created | type     | state | amount | from      | to      | purpose      | r    |
  | .AAAB | %today  | transfer | done  |     10 | .ZZA      | cgf     | contribution |   10 |
  | .AAAC | %today  | rebate   | done  |   0.50 | community | .ZZA    | rebate on #1 | 0.50 |
  | .AAAD | %today  | bonus    | done  |   1.00 | community | cgf     | bonus on #1  | 1.00 |
  And gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "new payment" to member "cgf" with subs:
  | otherName | amount | payeePurpose |
  | Abe One   |    $10 | contribution |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |
  |    $10 |        $0.50 |
  And we tell staff "gift accepted" with subs:
  | amount | often | myName  | rewardType |
  |     10 |     1 | Abe One | rebate     |
  # and many other fields

Scenario: A recurring contribution can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     Q | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid   | created | type     | state | amount | from      | to      | purpose      | r    |
  | .AAAB | %today  | transfer | done  |     10 | .ZZA      | cgf     | contribution |   10 |
  | .AAAC | %today  | rebate   | done  |   0.50 | community | .ZZA    | rebate on #1 | 0.50 |
  | .AAAD | %today  | bonus    | done  |   1.00 | community | cgf     | bonus on #1  | 1.00 |
  And gifts:
  | id   | giftDate      | amount | often | honor  | honored | completed |
  | .ZZA | %yesterday    |     10 |     Q | memory | Jane Do | %today    |
  | .ZZA | %yesterday+3m |     10 |     Q |        |         |         0 |
  And we notice "new payment" to member "cgf" with subs:
  | otherName | amount | payeePurpose |
  | Abe One   |    $10 | contribution |
  And we tell staff "gift accepted" with subs:
  | amount | often | myName  | rewardType |
  |     10 |     1 | Abe One | rebate     |
  # and many other fields
