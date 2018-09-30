Feature: Gift
AS a member
I WANT to donate to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state | zip | postalAddr | rebate | flags   |*
  | .ZZA | Abe One    | 1 A St. | Atown | AK    | 01000 | 1 A, A, AK |      5 | ok,confirmed      |
  And balances:
  | id     | balance |*
  | cgf    |       0 |
  | .ZZA   |     100 |

Scenario: A member donates
  Given next DO code is "whatever"
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | period | honor  | honored | share |*
  |   -1 |     10 |      1 | memory | Jane Do |    10 |
  Then transactions:
  | xid | created | type     | amount | from | to   | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf  | donation |
  And we say "status": "gift successful"
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
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
#  | amount | period | txField  |*
#  |     10 |     1 | payerTid |
  # and many other fields

Scenario: A member makes a recurring donation
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | period | honor  | honored | share |*
  |   -1 |     10 |      M | memory | Jane Do |    10 |
  Then transactions:
  | xid | created | type     | amount | from | to   | purpose                            |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf  | regular donation (monthly gift #1) |
  And we say "status": "gift successful"
	And these "recurs":
	| created | from | to  | amount | period |*
	| %today  | .ZZA | cgf |     10 |      M |
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |*
  |    $10 |        $0.50 | 
	
Scenario: A member donates with insufficient funds
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | period | honor  | honored | share |*
  |   -1 |    200 |      1 | memory | Jane Do |    10 |
  Then we say "status": "gift successful|gift transfer later"
  And invoices:
  | nvid | created | amount | from | to   | purpose  | flags |*
  |    1 | %today  |    200 | .ZZA | cgf  | donation | gift  |
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we tell admin "gift" with subs:
  | amount | period |*
  |    200 |      1 |
