Feature: Gift
AS a member
I WANT to donate to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | zip | postalAddr | rebate | flags   |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000 | 1 A, A, AK |      5 | ok,confirmed      |
  And balances:
  | id     | balance |*
  | cgf    |       0 |
  | .ZZA   |     100 |

Scenario: A member donates
  Given next DO code is "whatever"
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | often | honor  | honored | share |*
  |   -1 |     10 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | xid | created | type     | amount | from | to   | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf  | donation |
  And we say "status": "gift successful" with subs:
  | amount |*
  |    $10 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |*
  | .ZZA | %today   |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |*
  |    $10 |        $0.50 | 
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | donation     | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
#  And we tell admin "gift accepted" with subs:
#  | amount | often | txField  |*
#  |     10 |     1 | payerTid |
  # and many other fields

Scenario: A member donates with insufficient funds
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | often | honor  | honored | share |*
  |   -1 |    200 |     1 | memory | Jane Do |    10 |
  Then we say "status": "gift successful|gift transfer later" with subs:
  | amount |*
  |   $200 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |*
  | .ZZA | %today   |    200 |     1 | memory | Jane Do |    10 |         0 |
  And we tell admin "gift" with subs:
  | amount | often |*
  |    200 |     1 |
