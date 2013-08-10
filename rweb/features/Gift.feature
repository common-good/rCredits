Feature: Gift
AS a member
I WANT to contribute to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | email         | flags                |
  | .ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | a@example.com | dft,ok,personal,bona |
  And balances:
  | id     | usd  | r   | rewards |
  | cgf    |    0 |   0 |       0 |
  | .ZZA   |  100 |  20 |      20 |

Scenario: A member contributes
  When member ".ZZA" completes form "contribute" with values:
  | gift | amount | often | honor  | honored | share |
  |    0 |     10 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | xid   | created | type     | state | amount | from | to   | purpose      | r    |
  | .AAAB | %today  | transfer | done  |     10 | .ZZA | cgf  | contribution |   10 |
  | .AAAC | %today  | rebate   | done  |   0.50 | ctty | .ZZA | rebate on #1 | 0.50 |
  | .AAAD | %today  | bonus    | done  |   1.00 | ctty | cgf  | bonus on #1  | 1.00 |
  And we say "status": "gift successful" with subs:
  | amount |
  |    $10 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |
  | .ZZA | %today   |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |
  |    $10 |        $0.50 | 
  And we notice "new payment|reward other" to member "cgf" with subs:
  | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |
  | Abe One   |    $10 | contribution |           bonus |                $1 |
  And we tell staff "gift accepted" with subs:
  | amount | often | txField  |
  |     10 |     1 | payerTid |
  # and many other fields

Scenario: A member contributes partly in USD
# Donations to CGF get full rewards, even if given in USD.
  When member ".ZZA" completes form "contribute" with values:
  | gift | amount | often | honor  | honored | share |
  |    0 |     50 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | xid   | created | type     | state | amount | from | to   | purpose      | r    |
  | .AAAB | %today  | transfer | done  |     50 | .ZZA | cgf  | contribution |   20 |
  | .AAAC | %today  | rebate   | done  |   2.50 | ctty | .ZZA | rebate on #1 | 2.50 |
  | .AAAD | %today  | bonus    | done  |   5.00 | ctty | cgf  | bonus on #1  | 5.00 |
  
Scenario: A member contributes with insufficient funds
  When member ".ZZA" completes form "contribute" with values:
  | gift | amount | often | honor  | honored | share |
  |    0 |    200 |     1 | memory | Jane Do |    10 |
  Then we say "status": "gift successful|gift transfer later" with subs:
  | amount |
  |   $200 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |
  | .ZZA | %today   |    200 |     1 | memory | Jane Do |    10 |         0 |
  And we tell staff "gift" with subs:
  | amount | often |
  |     10 |     1 |
