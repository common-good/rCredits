Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | zip | country | postalAddr | flags        |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK | ok,confirmed |
  And balances:
  | id   | balance | floor |*
  | cgf  |       0 |     0 |
  | .ZZA |     100 |   -20 |

Scenario: A donation can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |*
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid | created | type     | amount | from      | to      | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA      | cgf     | donation |
  And gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |*
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | donation     | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
  | ~footer | %PROJECT |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount |*
  |    $10 |
#  And we tell admin "gift accepted" with subs:
#  | amount | myName  | often | rewardType | *
#  |     10 | Abe One |     1 | reward     |
  # and many other fields

Scenario: A donation can be completed even if the member has never yet made an rCard purchase
  Given member ".ZZA" has no photo ID recorded
  And gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |*
  | .ZZA | %yesterday |     10 |     1 | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid | created | type     | amount | from      | to      | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA      | cgf     | donation |
 
Scenario: A recurring donation can be completed
  Given gifts:
  | id   | giftDate   | amount | often | honor  | honored | share | completed |*
  | .ZZA | %yesterday |     10 |     Q | memory | Jane Do |    10 |         0 |
  When cron runs "gifts"
  Then transactions:
  | xid | created | type     | amount | from | to   | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf  | regular donation (quarterly gift #1) |
  And gifts:
  | id   | giftDate      | amount | often | honor  | honored | completed |*
  | .ZZA | %yesterday    |     10 |     Q | memory | Jane Do | %today    |
  | .ZZA | %yesterday+3m |     10 |     Q |        |         |         0 |
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose                 | aPayLink |*
  | Abe One   | $10    | regular donation (quarterly gift #1) | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
#  And we tell admin "gift accepted" with subs:
#  | amount | myName  | often | rewardType |*
#  |     10 | Abe One |     Q | reward     |
  # and many other fields
