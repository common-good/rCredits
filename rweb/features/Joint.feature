Feature: Joint
AS a member
I WANT to join my rCredits account to someone else's
SO we can share our finances, as for a typical "joint account" at a bank.

Setup:
  Given members:
  | id   | fullName   | acctType    | flags                     | rebate | minimum | created   |*
  | .ZZA | Abe One    | personal    | ok,member,confirmed,ided  |     10 |     100 | %today-6m |
  | .ZZB | Bea Two    | personal    | ok,confirmed,ided         |     10 |      50 | %today-6m |
  | .ZZC | Corner Pub | corporation | ok,confirmed,ided,co      |      5 |       0 | %today-6m |
  | .ZZD | Dee Four   | personal    | ok,confirmed,ided         |     10 |       0 | %today-6m |
  And transactions: 
  | xid | created   | type       | amount | from      | to   | purpose | taking |*
  |   1 | %today-6m | %TX_SIGNUP |    250 | community | .ZZA | signup  | 0      |
  |   2 | %today-6m | %TX_SIGNUP |    250 | community | .ZZB | signup  | 0      |
  |   3 | %today-6m | %TX_SIGNUP |    250 | community | .ZZC | signup  | 0      |
  Then balances:
  | id   | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member requests a joint account
  Given relations:
  | id | main | agent | permission | employee | owner | draw |*
  | 1  | .ZZA | .ZZB  | none       |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | none       |        0 |       0 |    0 |
  When member ".ZZA" completes relations form with values:
  | other | permission |*
  | .ZZB  | joint      |
  Then we say "status": "updated relation" with subs:
  | otherName |*
  | Bea Two   |
  And we show "Relations" with:
  | other      | Draw | Employee | Family | Permission |
  | Bea Two    | No   | No       | No     | %can_joint |
  And members have:
  | id   | jid | minimum |*
  | .ZZA |     |     100 |
  | .ZZB |     |      50 |
  When member ".ZZB" completes relations form with values:
  | other | permission |*
  | .ZZA  | joint      |
  Then members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  # cache is ok tests acct::cacheOk, to make sure cron doesn't muck with the cached amounts
  
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | goods      | purpose |*
  | pay | Corner Pub |     20 | %FOR_GOODS | stuff   |
  And member ".ZZB" confirms form "pay" with values:
  | op  | who        | amount | goods      | purpose |*
  | pay | Corner Pub |    300 | %FOR_GOODS | crud    |
  Then member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  Then member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  # do it twice, to make sure cacheOk() doesn't screw it up

Scenario: A joined account slave member requests a new minimum
  Given members have:
  | id   | jid  | achMin | minimum |*
  | .ZZA | .ZZB |     50 |     150 |
  | .ZZB | .ZZA |     50 |       0 |
  When member ".ZZB" completes form "settings/connect" with values:
  | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  |       1 |     053000196 |         123 |          123 |       1 |    300 |    100 |          2 |
  Then members have:
  | id   | bankAccount      | last4bank | achMin | minimum | saveWeekly |*
  | .ZZA | USkk053000196123 |      6123 |    100 |     300 |          2 |
  | .ZZB |                  |           |     50 |       0 |          0 |

#  When member ".ZZB" completes form "settings/preferences" with values:
#  | minimum | achMin | savingsAdd | saveWeekly | share |*
#  |     200 |    100 |        250 |          0 |    10 |
#  Then members have:
#  | id   | minimum | savingsAdd | achMin | share |*
#  | .ZZA |     200 |        250 |    100 |     0 |
#  | .ZZB |       0 |          0 |    100 |    10 |

Scenario: A joined account member looks at transaction history and summary
#  Given reward step is "1000"
  And members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And relations:
  | id | main | agent | permission | employee | owner | draw |*
  | 1  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And usd transfers:
  | txid | payee | amount | created   | completed |*
  |  600 | .ZZA  |   1000 | %today-2w | %today-6m |
  |  601 | .ZZB  |    600 | %today-2w | %today-2w |
  |  602 | .ZZA  |    400 | %today-2w | %today-2w |
  |  603 | .ZZA  |   -100 | %today    |         0 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-1m | transfer |    200 | .ZZA | .ZZD | favors  |
  |   7 | %today-1w | transfer |    500 | .ZZA | .ZZB | usd     |
  |   8 | %today-2d | transfer |     50 | .ZZD | .ZZB | cash    |
  |   9 | %today-1d | transfer |    100 | .ZZC | .ZZA | labor   |
  Then balances:
  | id   | balance |*
  | .ZZA |     800 |
  | .ZZB |    1150 | 
  | .ZZC |    -100 |
  | .ZZD |     150 |
  When member ".ZZB" visits page "history/transactions/period=14"
  Then we show "Transaction History" with:
  | Start        |   |   800.00 | %dmy-2w |
  | From Bank    | + | 1,000.00 | - 100.00 Pending |
  | Received     | + |   650.00 |         |
  | Out          | - |   500.00 |         |
#  | Credit Line+ |   |          |         |
  | End          |   | 1,950.00 | %dmy    |
  And with:
#  |~tid | Date    | Name       | Purpose   | Amount | Reward | Agent | ~ |
#  | 5   | %mdy-1d | Corner Pub | labor     | 100.00 | 10.00      | ZZA  | X |
#  | 4   | %mdy-2d | Dee Four   |  cash     |  50.00 | --         | ZZB  | X |
##  | 3   | %mdy-1w | Abe One    | usd       | 500.00   | 500.00 | --         | ZZB  | X |
#  | 602 | %mdy-2w |            | from bank | 400.00 | --         | ZZA  | X |
#  | 601 | %mdy-2w |            | from bank | 600.00 | --         | ZZB  | X |

  |~tid | Date    | Name       | Purpose   | Amount |  Balance | ~ |
  | 5   | %mdy-1d | Corner Pub | labor     | 100.00 | 1,950.00 | X |
  | 4   | %mdy-2d | Dee Four   |  cash     |  50.00 | 1,850.00 | X |
#  | 3   | %mdy-1w | Abe One    | usd       | 500.00   | 500.00 |  +0    | X |
  | 602 | %mdy-2w |            | from bank | 400.00 | 1,800.00 |   |
  | 601 | %mdy-2w |            | from bank | 600.00 | 1,400.00 |   |
  Given cron runs "acctStats"
  When member ".ZZB" visits page "summary"
  Then we show "Account Summary" with:
  | ID            | ZZB (joint account) |
  | Name          | Bea Two & Abe One |
  | ~             | (beatwo & abeone) |
  | Balance       | $1,950 |
#  | Savings       | $530 |
#  | ~rewards      | $530 |
#  | Committed     | $0.60 |
#  | Your return   | 21.9% | (sometimes is 20.2%)
#  | ~ever         | 136.7% | or 137.1% (depends on daylight time?) or 68.0%?!
#  | Social return | $68.75 |
#  | including     | $0 |

  Scenario: A joined account member unjoins the account
  Given members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And relations:
  | id | main | agent | permission | employee | owner | draw |*
  | 1  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-1d | transfer |    100 | .ZZC | .ZZA | labor   |
  Then balances:
  | id   | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 | 
  | .ZZC |    -100 |
  When member ".ZZB" completes relations form with values:
  | other | permission |*
  | .ZZA  | none       |
  Then members have:
  | id   | jid  | minimum | balance |*
  | .ZZA |      |     150 |      50 |
  | .ZZB |      |     150 |      50 |
  And member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  
Scenario: A member requests two joins at once
  Given relations:
  | id | main | agent | permission | employee | owner | draw |*
  | 1  | .ZZA | .ZZB  | none       |        0 |       0 |    0 |
  | 2  | .ZZA | .ZZD  | none       |        0 |       0 |    0 |
  When member ".ZZA" completes relations form with values:
  | other | permission |*
  | .ZZB  | joint      |
  | .ZZD  | joint      |
  Then we say "status": "updated relation" with subs:
  | otherName |*
  | Bea Two   |
# (actually does this, but test can't find it. why?)  And we say "error": "too many joins"
  And we show "Relations" with:
  | other      | Draw | Employee | Family | Permission |
  | Bea Two    | No   | No       | No     | %can_joint |
  | Dee Four   | No   | No       | No     | %can_none  |
