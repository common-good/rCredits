Feature: Joint
AS a pair of members with a joint account
WE WANT to transfer money from our bank account to our joint account
SO BOTH OF US can make purchases with those funds.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags     | achMin | risks   | jid  |*
  | .ZZA | Abe One  |     0 |     100 | ok,refill | 30     | hasBank | .ZZB |
  | .ZZB | Bea Two  |     0 |       0 | ok        | 10     |         | .ZZA |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | joint      |
  | .ZZB | .ZZA  | joint      |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  | 1   | %today-6m | signup |    250 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup |    250 | ctty | .ZZB | signup  |
  Then balances:
  | id   | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |       0 |

Scenario: a joint account needs refilling
  Given balances:
  | id   | balance |*
  | .ZZA |   50.00 |
  | .ZZB |   49.99 |
  When cron runs "bank"
  Then usd transfers:
  | txid | payee | amount |*
  |    1 | .ZZA  |  30    |
  And we notice "under min|banked|bank tx number" to member ".ZZA" with subs:
  | action    | amount | checkNum |*
  | draw from | $30    |        1 |

Scenario: a joint account does not need refilling
  Given balances:
  | id   | balance |*
  | .ZZA |   50.01 |
  | .ZZB |   49.99 |
  When cron runs "bank"
  Then bank transfer count is 0