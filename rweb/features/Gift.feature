Feature: Gift
AS a member
I WANT to donate to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | postalAddr | rebate | flags   |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | 1 A, A, AK |      5 | ok,confirmed,bona |
  And balances:
  | id     | r   | rewards |*
  | cgf    |   0 |       0 |
  | .ZZA   | 120 |      20 |

Scenario: A member donates
  Given next DO code is "whatever"
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | often | honor  | honored | share |*
  |    0 |     10 |     1 | memory | Jane Do |    10 |
  Then transactions:
  | xid | created | type     | amount | from | to   | purpose      |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf  | donation |
  |   2 | %today  | rebate   |   0.50 | ctty | .ZZA | rebate on #1 |
  |   3 | %today  | bonus    |   1.00 | ctty | cgf  | bonus on #1  |
  And we say "status": "gift successful" with subs:
  | amount |*
  |    $10 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |*
  | .ZZA | %today   |     10 |     1 | memory | Jane Do |    10 | %today    |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |*
  |    $10 |        $0.50 | 
  And we notice "new payment|reward other" to member "cgf" with subs:
  | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |*
  | Abe One   | $10 | donation | reward | $1 |
Skip messages don't show up in get_file_contents simulation and I haven't figured out how to test this yet
  And that "notice" has link results:
  | _name | Abe One |
  | _postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
Resume
  And we tell staff "gift accepted" with subs:
  | amount | often | txField  |*
  |     10 |     1 | payerTid |
  # and many other fields

Scenario: A member donates with insufficient funds
  When member ".ZZA" completes form "community/donate" with values:
  | gift | amount | often | honor  | honored | share |*
  |    0 |    200 |     1 | memory | Jane Do |    10 |
  Then we say "status": "gift successful|gift transfer later" with subs:
  | amount |*
  |   $200 |
  And gifts:
  | id   | giftDate | amount | often | honor  | honored | share | completed |*
  | .ZZA | %today   |    200 |     1 | memory | Jane Do |    10 |         0 |
  And we tell staff "gift" with subs:
  | amount | often |*
  |     10 |     1 |
