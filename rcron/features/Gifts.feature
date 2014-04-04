Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | postalAddr | email | flags |
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK | a@    | ok    |
  And balances:
  | id   | usd  | r   | rewards |
  | cgf  |    0 |   0 |       0 |
  | .ZZA |  100 |  20 |      20 |

Scenario: A donation can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid   | created | type     | amount | from      | to      | purpose      |
  | .AAAB | %today  | transfer |     10 | .ZZA      | cgf     | donation |
  | .AAAC | %today  | rebate   |   0.50 | community | .ZZA    | rebate on #1 |
  | .AAAD | %today  | bonus    |   1.00 | community | cgf     | bonus on #1  |
  And gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "new payment|reward other" to member "cgf" with subs:
  | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |
  | Abe One   | $10    | donation | reward          | $1                |
  And that "notice" has link results:
  | _name | Abe One |
  | _postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
  | _footer | Common Good Finance |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |
  |    $10 |        $0.50 |
  And we tell staff "gift accepted" with subs:
  | amount | myName  | often | rewardType | 
  |     10 | Abe One |     1 | reward     |
  # and many other fields

Scenario: A recurring donation can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |
  | .ZZA | %yesterday |     10 |     Q | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid   | created | type     | amount | from | to   | purpose      |
  | .AAAB | %today  | transfer |     10 | .ZZA | cgf  | donation (quarterly gift #1) |
  | .AAAC | %today  | rebate   |   0.50 | ctty | .ZZA | rebate on #1 |
  | .AAAD | %today  | bonus    |   1.00 | ctty | cgf  | bonus on #1  |
  And gifts:
  | id   | giftDate      | amount | often | honor  | honored | completed |
  | .ZZA | %yesterday    |     10 |     Q | memory | Jane Do | %today    |
  | .ZZA | %yesterday+3m |     10 |     Q |        |         |         0 |
  And we notice "new payment|reward other" to member "cgf" with subs:
  | otherName | amount | payeePurpose                 | otherRewardType | otherRewardAmount |
  | Abe One   | $10    | donation (quarterly gift #1) | reward          | $1                |
  And that "notice" has link results:
  | _name | Abe One |
  | _postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
  And we tell staff "gift accepted" with subs:
  | amount | myName  | often | rewardType |
  |     10 | Abe One |     Q | reward     |
  # and many other fields
