Feature: Bona
AS a member
I WANT to receive incentive rewards for signing up and for inviting/helping others sign up
SO I can do well by doing good.

Setup:
  Given members:
  | id   | fullName | minimum | flags   | achMin | risks   | helper | rebate |*
  | .ZZA | Abe One  |     100 | ok,bona | 30     |         |        |      5 |
  | .ZZB | Bea Two  |     100 | ok      | 30     | hasBank | .ZZA   |     10 |
  | .ZZC | Our Pub  |       0 | ok,bona | 30     |         | .ZZB   |     10 |
  | .ZZD | Dee Four |     100 | ok      | 30     | hasBank | .ZZB   |     10 |
  And relations:
  | main | agent | employee |*
  | .ZZC | .ZZD  |        1 |
  
Scenario: a member gets money for the first time
  Given member field values:
  | id   | field | value |*
  | .ZZB | floor | -100  |
  When cron runs "bank"
  Then usd transfers:
  | txid | payer | amount | completed |*
  |    1 | .ZZB  | -100   | %today    |
  And we notice "under min|banked|bank tx number" to member ".ZZB" with subs:
  | action    | amount | checkNum |*
  | draw from | $100   |        1 |
  And we notice "transfer complete" to member ".ZZB" with subs:
  | transfer           | amount |*
  | automatic transfer | $100   |
  And we message member ".ZZB" with topic "account funded" and subs: ""

  Given transactions:
  | xid | created | type     | amount | from | to   | purpose | taking |*
  |   1 | %today  | transfer |     50 | .ZZB | .ZZC | stuff   |      1 |
  When cron runs "bona"
  Then we notice "got funding" to member ".ZZB" with subs:
  | amount           | purpose      | thing  |*
  | $%R_SIGNUP_BONUS | signup bonus | reward |
  And we notice "got funding" to member ".ZZA" with subs:
  | amount           | purpose                                      | thing  |*
  | $%R_HELPER_BONUS | inviting and/or assisting new member Bea Two | reward |
  And members:
  | id   | flags          |*
  | .ZZB | member,ok,bona |
  And balances:
  | id   | r                     | rewards             |*
  | .ZZA | %R_HELPER_BONUS       | %R_HELPER_BONUS     |
  | .ZZB | %(55+%R_SIGNUP_BONUS) | %(5+R_SIGNUP_BONUS) |
  | .ZZC | 55                    | 5                   |

Scenario: an employee gets money for the first time
  Given member field values:
  | id   | field | value |*
  | .ZZD | floor | -100  |
  When cron runs "bank"
  Then usd transfers:
  | txid | payer | amount | completed |*
  |    1 | .ZZB  | -100   |         0 |
  |    2 | .ZZD  | -100   | %today    |
  And we notice "under min|banked|bank tx number" to member ".ZZD" with subs:
  | action    | amount | checkNum |*
  | draw from | $100   |        2 |
  And we notice "transfer complete" to member ".ZZD" with subs:
  | transfer           | amount |*
  | automatic transfer | $100   |
  And we message member ".ZZD" with topic "account funded" and subs: ""

  Given transactions:
  | xid | created | type     | amount | from | to   | purpose | taking |*
  |   1 | %today  | transfer |     50 | .ZZD | .ZZC | stuff   |      1 |
  When cron runs "bona"
  Then we notice "got funding" to member ".ZZD" with subs:
  | amount           | purpose      | thing  |*
  | $%R_SIGNUP_BONUS | signup bonus | reward |
  And we notice "got funding" to member ".ZZB" with subs:
  | amount           | purpose                                       | thing  |*
  | $%R_HELPER_BONUS | inviting and/or assisting new member Dee Four | reward |
  And members:
  | id   | flags          |*
  | .ZZD | member,ok,bona |
  When cron runs "employees"
  Then we notice "got funding" to member ".ZZA" with subs:
  | amount            | purpose                                                       | thing  |*
  | $%R_COUNTED_BONUS | inviting and/or assisting a member's manager (at Our Pub -- $%R_COUNTED_BONUSr per employee) | reward |
  And members:
  | id   | flags                  |*
  | .ZZD | member,ok,bona,counted |
  And balances:
  | id   | r                     | rewards              |*
  | .ZZA | %R_COUNTED_BONUS      | %R_COUNTED_BONUS     |
  | .ZZB | %R_HELPER_BONUS       | %R_HELPER_BONUS      |
  | .ZZC | 55                    | 5                    |
  | .ZZD | %(55+%R_SIGNUP_BONUS) | %(5+%R_SIGNUP_BONUS) |
