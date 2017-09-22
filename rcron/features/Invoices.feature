Feature: Invoices
AS a member
I WANT to charge other members and pay invoices from other members automatically at night, if necessary
SO I can buy and sell stuff.

Setup:
  Given members:
  | id   | fullName | address | city  | state      | risks   | rebate | minimum | flags               |*
  | .ZZA | Abe One  | 1 A St. | Atown | Alaska     | hasBank | 5      |     500 | ok,confirmed,refill |
  | .ZZB | Bea Two  | 2 B St. | Btown | Utah       |         | 10     |     100 | ok,confirmed        |
  | .ZZC | Our Pub  | 3 C St. | Ctown | California |         | 10     |       0 | ok,confirmed,co     |
  And relations:
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  And invoices:
  | nvid | created   | status       | amount | from | to   | for   |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZA | .ZZC | one   |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZC | two   |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB | .ZZC | three |
  |    4 | %today-1w | %TX_PENDING  |    400 | .ZZA | .ZZC | four  |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |       0 |     250 |
  | .ZZB |       0 |     250 |
  | .ZZC |       0 |     250 |

  Scenario: Unpaid invoices get handled
  When cron runs "invoices"
  Then transactions: 
  | xid | created | type     | amount | from | to   | purpose             | taking |*
  |   4 | %today  | transfer |    100 | .ZZA | .ZZC | one (%PROJECT inv#1) | 0      |
  And usd transfers:
  | txid | payee | amount | created | completed |*
  |    1 | .ZZA  |    800 | %today  |         0 |
  And invoices:
  | nvid | created   | status       | amount | from | to   | for   |*
  |    1 | %today    | 4            |    100 | .ZZA | .ZZC | one   |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZC | two   |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB | .ZZC | three |
  |    4 | %today-1w | %TX_PENDING  |    400 | .ZZA | .ZZC | four  |
  And we notice "banked|bank tx number" to member ".ZZA" with subs:
  | action    | amount | checkNum |*
  | draw from | $800   |        1 |
  And we notice "short invoice|when funded" to member ".ZZB" with subs:
  | short | payeeName |*
  | $50   | Our Pub   |
  And we message "stale invoice" to member ".ZZA" with subs:
  | daysAgo | amount | purpose | nvid | payeeName |*
  |       7 | $400   | four    |    4 | Our Pub   |
  And we message "stale invoice report" to member ".ZZC" with subs:
  | daysAgo | amount | purpose | nvid | payerName | created |*
  |       7 | $400   | four    |    4 | Abe One   | %mdy-1w |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |    -100 |     255 |
  | .ZZB |       0 |     250 |
  | .ZZC |     100 |     260 |
