Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags        | achMin | risks   |*
  | .ZZA | Abe One  |     0 |     100 | co,ok,refill | 30     | hasBank |
  | .ZZB | Bea Two  |   -50 |     100 | ok,refill    | 30     |         |
  And relations:
  | main | agent | draw |*
  |.ZZA | .ZZB  | 1    |
  
Scenario: a member is barely below minimum
  Given balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZA |      20 |          0 | 99.99   |
  When cron runs "bank"
  Then usd transfers:
  | txid | payee | amount | channel  |*
  |    1 | .ZZA  |     30 | %TX_CRON |
  And we notice "under min|banked|bank tx number" to member ".ZZA" with subs:
  | action    | amount | checkNum |*
  | draw from | $30    |        1 |

Scenario: a member has a negative balance
  Given balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZA |      20 |          0 | -50     |
  When cron runs "bank"
  Then usd transfers:
  | txid | payee | amount | channel  |*
  |    1 | .ZZA  |  150   | %TX_CRON |
  And we notice "under min|banked|bank tx number" to member ".ZZA" with subs:
  | action    | amount | checkNum |*
  | draw from | $150   |        1 |

Scenario: an unbanked member barely below minimum draws on another account
  Given balances:
  | id   | balance |*
  | .ZZA | 200   |
  | .ZZB | 99.99 |
  When cron runs "bank"
  Then transactions:
  | xid | type     | amount | from | to   | goods         | taking | purpose      |*
  |   1 | transfer |     30 | .ZZA | .ZZB | %FOR_NONGOODS |      1 | automatic transfer to NEWZZB,automatic transfer from NEWZZA |
  And we notice "under min|drew" to member ".ZZB" with subs:
  | amount |*
  | $30    |
  
Scenario: an unbanked member barely below minimum cannot draw on another account
  Given balances:
  | id   | balance |*
  | .ZZA | 0      |
  | .ZZB | 99.99  |
  When cron runs "bank"
  Then we notice "under min|cannot draw" to member ".ZZB"

Scenario: a member is at minimum
  Given balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZA |      20 |          0 |     100 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member is well below minimum
  Given balances:
  | id   | rewards | savingsAdd | balance | minimum |*
  | .ZZA |      25 |          0 |      50 |     151 |
  When cron runs "bank"
  Then usd transfers:
  | txid | payee | amount              | channel  |*
  |    1 | .ZZA  | %(100 + %R_ACHMIN) | %TX_CRON |
  And we notice "under min|banked|bank tx number" to member ".ZZA" with subs:
  | action    | amount              | checkNum |*
  | draw from | $%(100 + %R_ACHMIN) |        1 |

Scenario: a member is under minimum but already requested barely enough funds from the bank
  Given balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZA |      20 |          0 |      20 |
  | .ZZB |      20 |          0 |     100 |
  When cron runs "bank"
  Then usd transfers:
  | payee | amount | channel  |*
  | .ZZA  |     80 | %TX_CRON |
  When cron runs "bank"
# (again)  
  Then bank transfer count is 1
  
Scenario: a member is under minimum and has requested insufficient funds from the bank
# This works only if member requests more than R_USDTX_QUICK the first time (hence ZZD, whose minimum is 300)
  Given members:
  | id   | fullName | floor | minimum | flags     | achMin | risks   |*
  | .ZZD | Dee Four |   -50 |     300 | ok,refill | 30     | hasBank |
  And balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZD |      20 |          0 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payee | amount | deposit |*
  | .ZZD  |    280 |       0 |
  Given balances:
  | id   | rewards | savingsAdd | balance |*
  | .ZZD |      20 |          0 |   19.99 |
  When cron runs "bank"
  Then usd transfers:
  | payee | amount |*
  | .ZZD  | %(280+R_ACHMIN) |

Scenario: a member is under minimum only because rewards are reserved
  Given members:
  | id   | fullName | minimum | flags     | achMin | risks   |*
  | .ZZD | Dee Four |     100 | ok,refill | 10     | hasBank |
  And balances:
  | id   | balance | rewards | savingsAdd |*
  | .ZZD | 80      |      30 |          0 |
  When cron runs "bank"
  Then usd transfers:
  | payee | amount |*
  | .ZZD  |     20 |
  
Scenario: a member member with zero minimum has balance below minimum
  Given balances:
  | id   | minimum | balance |*
  | .ZZA |       0 | -10     |
  When cron runs "bank"
  Then usd transfers:
  | payee | amount |*
  | .ZZA  |     30 |
  
Scenario: an unbanked member with zero minimum has balance below minimum
  Given balances:
  | id   | minimum | balance |*
  | .ZZA |       0 |   0 |
  | .ZZB |       0 | -10 |
  When cron runs "bank"
  Then bank transfer count is 0

Scenario: a member has a deposited but not completed transfer
  Given balances:
  | id   | balance |*
  | .ZZA |  80 |
  | .ZZB | 100 |
  And usd transfers:
  | txid | payee | amount | created   | completed | deposit    |*
  | 5001 | .ZZA  |     50 | %today-4d |         0 | %(%today-%R_USDTX_DAYS*%DAY_SECS-9) |
  # -9 in case the test takes a while (elapsed time is slightly more than R_USDTX_DAYS days)
  When cron runs "bank"
  Then bank transfer count is 1

Scenario: an account has a minimum but no refills
  Given members have:
  | id   | flags |*
  | .ZZB | ok    |
  And balances:
  | id   | rewards | balance |*
  | .ZZA |       0 |     100 |
  | .ZZB |      20 |     -50 |
  When cron runs "bank"
  Then bank transfer count is 0
