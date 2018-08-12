Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state | zip   | country | postalAddr | flags        | risks   |*
  | .ZZA | Abe One    | 1 A St. | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed | hasBank |
  And balances:
  | id   | balance | floor |*
  | cgf  |       0 |     0 |
  | .ZZA |     100 |   -20 |

Scenario: A brand new recurring donation can be completed
  Given these "recurs":
  | created    | payer | payee | amount | period |*
  | %yesterday | .ZZA  | cgf   |     10 |      M |
  When cron runs "gifts"
  Then transactions:
  | xid | created | type     | amount | from | to  | purpose                            | flags          |*
  |   1 | %today  | transfer |     10 | .ZZA | cgf | regular donation (monthly gift #1) | gift,patronage |
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose                       | aPayLink |*
  | Abe One   | $10    | regular donation (monthly gift #1) | ?        |
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
	And count "txs" is 1
	And count "usd" is 0
	And count "invoices" is 0
	When cron runs "gifts"
	Then count "txs" is 1
	And count "usd" is 0
	And count "invoices" is 0

Scenario: A second recurring donation can be completed
  Given these "recurs":
  | created   | payer | payee | amount | period |*
  | %today-3m | .ZZA  | cgf   |     10 |      M |
  And transactions:
  | xid | created    | type     | amount | from | to  | purpose                            | flags          |*
  |   1 | %today-32d | transfer |     10 | .ZZA | cgf | regular donation (monthly gift #1) | gift,patronage |
  When cron runs "gifts"
  Then transactions:
  | xid | created | type     | amount | from | to  | purpose                            | flags          |*
  |   2 | %today  | transfer |     10 | .ZZA | cgf | regular donation (monthly gift #2) | gift,patronage |
	
Scenario: A donation invoice can be completed
# even if the member has never yet made an rCard purchase
  Given invoices:
  | nvid | created   | status       | amount | from | to  | for      | flags |*
  |    2 | %today    | %TX_APPROVED |     50 | .ZZA | cgf | donation | gift  |
  And member ".ZZA" has no photo ID recorded
  When cron runs "invoices"
  Then transactions: 
  | xid | created | type     | amount | from | to  | purpose                      | flags |*
  |   1 | %today  | transfer |     50 | .ZZA | cgf | donation (Common Good inv#2) | gift  |
	And invoices:
  | nvid | created   | status | amount | from | to  | for      | flags |*
  |    2 | %today    | 1      |     50 | .ZZA | cgf | donation | gift  |	
	
Scenario: A recurring donation cannot be completed
  Given these "recurs":
  | created   | payer | payee | amount | period |*
  | %today-3m | .ZZA  | cgf   |    200 |      M |
  When cron runs "gifts"
	Then invoices:
  | nvid | created   | status       | amount | from | to  | for                                | flags          |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | cgf | regular donation (monthly gift #1) | gift,patronage |	
	And count "txs" is 0
	And count "usd" is 0
	And count "invoices" is 1

  When cron runs "invoices"
	Then count "txs" is 0
  And count "usd" is 1
  And count "invoices" is 1
  And	invoices:
  | nvid | created   | status       | amount | from | to  | for                                | flags                  |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA | cgf | regular donation (monthly gift #1) | gift,patronage,funding |	

	When cron runs "gifts"
	Then count "txs" is 0
  And count "usd" is 1
  And count "invoices" is 1

Scenario: A non-member chooses a donation
  Given members:
  | id   | fullName | flags  | risks   | activated | balance |*
  | .ZZD | Dee Four |        | hasBank |         0 |       0 |
  | .ZZE | Eve Five | refill | hasBank | %today-9m |     200 |
  Given these "recurs":
  | created   | payer | payee | amount | period |*
  | %today-3y | .ZZD  | cgf   |      1 |      Y |
  | %today-3m | .ZZE  | cgf   |    200 |      M |
  When cron runs "gifts"
	Then count "txs" is 0
	And count "usd" is 0
	And count "invoices" is 0
	
	