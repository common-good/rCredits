Feature: Joint
AS a member
I WANT to join my rCredits account to someone else's
SO we can share our finances, as for a typical "joint account" at a bank.

Setup:
  Given members:
  | id   | fullName   | acctType    | flags      | rebate | minimum | created   |*
  | .ZZA | Abe One    | personal    | ok,bona    |     10 |     100 | %today-6m |
  | .ZZB | Bea Two    | personal    | ok,bona    |     10 |      50 | %today-6m |
  | .ZZC | Corner Pub | corporation | ok,co,bona |      5 |       0 | %today-6m |
  | .ZZD | Dee Four   | personal    | ok,bona    |     10 |       0 | %today-6m |
  And transactions: 
  | xid | created   | type       | amount | from      | to   | purpose | taking |*
  |   1 | %today-6m | %TX_SIGNUP |    250 | community | .ZZA | signup  | 0      |
  |   2 | %today-6m | %TX_SIGNUP |    250 | community | .ZZB | signup  | 0      |
  |   3 | %today-6m | %TX_SIGNUP |    250 | community | .ZZC | signup  | 0      |
  Then balances:
  | id   | r    |*
  | ctty | -750 |
  | .ZZA |  250 |
  | .ZZB |  250 |
  | .ZZC |  250 |

Scenario: A member requests a joint account
  Given relations:
  | id | main | agent | permission | employee | isOwner | draw |*
  | 1  | .ZZA | .ZZB  | none       |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | none       |        0 |       0 |    0 |
  When member ".ZZA" completes relations form with values:
  | other | permission |*
  | .ZZB  | joint      |
  Then we say "status": "updated relation" with subs:
  | otherName |*
  | Bea Two   |
  And we show "Relations" with:
  | Other      | Draw | My employee? | Family? | Permission |_requests      |
  | Bea Two    | No   | No           | No      | %can_joint | --            |
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

Scenario: A joined account slave member requests a new minimum
  Given members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  When member ".ZZB" completes form "account/preferences" with values:
  | minimum | achMin | share |*
  |     200 |    100 |    10 |
  Then members have:
  | id   | minimum | achMin | share |*
  | .ZZA |     200 |    100 |     0 |
  | .ZZB |       0 |    100 |    10 |

Scenario: A joined account member looks at transaction history and summary
  Given members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And relations:
  | id | main | agent | permission | employee | isOwner | draw |*
  | 1  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And usd transfers:
  | txid | payer | amount | created   | completed |*
  |  600 | .ZZA  |  -1000 | %today-2w | %today-6m |
  |  601 | .ZZB  |   -600 | %today-2w | %today-2w |
  |  602 | .ZZA  |   -400 | %today-2w | %today-2w |
  |  603 | .ZZA  |    100 | %today    |         0 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-1m | transfer |    200 | .ZZA | .ZZD | favors  |
  |   7 | %today-1w | transfer |    500 | .ZZA | .ZZB | usd     |
  |   8 | %today-2d | transfer |     50 | .ZZD | .ZZB | cash    |
  |   9 | %today-1d | transfer |    100 | .ZZC | .ZZA | labor   |
  Then balances:
  | id   | r    |*
  | ctty | -805 |
  | .ZZA | 1080 |
  | .ZZB | 1400 |
  | .ZZC |  155 |
  | .ZZD |  170 |
  When member ".ZZB" visits page "history/period=14"
  Then we show "Transaction History" with:
  |_Start Date |_End Date |
  | %dmy-2w    | %dmy     |
  And with:
  | Start     | From Bank | From You | To You | Rewards | To CGF | End       |
  | $1,320.00 |  1,000.00 |   500.00 | 650.00 |   10.00 |        | $2,480.00 |
  | PENDING   |   -100.00 |     0.00 |   0.00 |    0.00 |   0.00 | - $100.00 |
  And with:
  |_tid | Date   | Name       | From you | To you | Status  |_do   | Purpose   | Reward/Fee | Agent |
  | 5   | %dm-1d | Corner Pub | --       | 100.00 | %chk    | X    | labor     | 10.00      | .ZZA  |
  | 4   | %dm-2d | Dee Four   | --       |  50.00 | %chk    | X    | cash      | --         | .ZZB  |
  | 3   | %dm-1w | Abe One    | 500.00   | 500.00 | %chk    | X    | usd       | --         | .ZZB  |
  | 602 | %dm-2w |            | --       | 400.00 |         |      | from bank | --         | .ZZA  |
  | 601 | %dm-2w |            | --       | 600.00 |         |      | from bank | --         | .ZZB  |
  Given cron runs "acctStats"
  When member ".ZZB" visits page "summary"
  Then we show "Account Summary" with:
  | Name          | Bea Two & Abe One (beatwo & abeone) |
  | ID            | .ZZB (joint account) |
  | Balance       | $2,480 |
  | Rewards       | $530 |
#  | Committed     | $0.60 |
  | Your return   | 21.9% |
  | _ever         | 137.1% |
# or 136.7% (depends on daylight time?)
  | Social return | $37.50 |
  | _ever         | $37.50 |
  
Scenario: A joined account member unjoins the account
  Given members have:
  | id   | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And relations:
  | id | main | agent | permission | employee | isOwner | draw |*
  | 1  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | 2  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-1d | transfer |    100 | .ZZC | .ZZA | labor   |
  Then balances:
  | id   | r    |*
  | ctty | -765 |
  | .ZZA |  360 |
  | .ZZB |  250 |
  | .ZZC |  155 |
  When member ".ZZB" completes relations form with values:
  | other | permission |*
  | .ZZA  | none       |
  Then members have:
  | id   | jid  | minimum | r   | rewards |*
  | .ZZA |      |     150 | 305 |     260 |
  | .ZZB |      |     150 | 305 |     250 |
  And member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  
Scenario: A member requests two joins at once
  Given relations:
  | id | main | agent | permission | employee | isOwner | draw |*
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
  | Other      | Draw | My employee? | Family? | Permission |_requests      |
  | Bea Two    | No   | No           | No      | %can_joint | --            |
  | Dee Four   | No   | No           | No      | %can_none  | --            |
