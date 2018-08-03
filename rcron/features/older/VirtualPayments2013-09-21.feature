Feature: Payment Exchanges
AS a member company
I WANT to pay my employees and suppliers rCredits automatically
SO I don't have to do change my current payroll and accounting system
and SO I don't build up a pile of credit I can't use yet.

Setup:
  Given members:
  | id   | fullName   | email         |floor | flags                        |
  | .ZZA | Abe One    | a@ |    0 | dft,person,ok,bona         |
  | .ZZB | Bea Two    | b@ |    0 | dft,person,company,ok,bona |
  | .ZZC | Corner Pub | c@ |    0 | dft,company,payex,ok,bona  |
  | .ZZD | Dee Four   | d@ |    0 | dft,person,ok,bona         |
  | .ZZE | Ezra Five  | e@ |    0 | dft,person,member          |
  And relations:
  | id      | main | agent | employerOk | permission | amount |
  | NEW.ZZA | .ZZC | .ZZA  |          1 | sell       |   1800 |
  | NEW.ZZB | .ZZC | .ZZB  |          0 |            |    200 |
  | NEW.ZZE | .ZZC | .ZZE  |          1 |            |   2000 |
# 90/10 split (E is not an rTrader, so doesn't count in the split)
  
Scenario: a member company pays suppliers virtually
  Given balances:
  | id   | r                 | usd | rewards | minimum |
  | .ZZA |                10 |  40 |       5 |     100 |
  | .ZZB |                60 |   0 |      20 |     100 |
  | .ZZC | %(12 + 10*%chunk) |   0 |      20 |      12 |
  | .ZZD |                 0 | 100 |      20 |     100 |
  | .ZZE |                 5 | 500 |       5 |     200 |
  When cron runs "paySuppliers"
  Then transactions:
  | xid   | created | type     | state | amount        | r             | from | to   | purpose               |
  | .AAAB | %today  | transfer | done  |             0 |        %chunk | ctty | .ZZA | rCredits/USD exchange |
  | .AAAC | %today  | transfer | done  |             0 |        %chunk | .ZZB | ctty | rCredits/USD exchange |
  | .AAAD | %today  | transfer | done  |             0 |        %chunk | .ZZC | .ZZB | payment exchange       |
  | .AAAE | %today  | rebate   | done  | %(.05*%chunk) | %(.05*%chunk) | ctty | .ZZC | rebate on #1          |
  | .AAAF | %today  | bonus    | done  | %(.10*%chunk) | %(.10*%chunk) | ctty | .ZZB | bonus on #2           |
  And usd transfers:
  | payer | payee | amount |
  | .ZZA  |  .ZZC | %chunk |
  And usd transfer count is 1
  And we notice "payment exchanges offered" to member ".ZZC" with subs:
  | offers | total        | whom      |
  |      1 | $%(%chunk) r | suppliers |
  And we notice "payment exchange received" to member ".ZZB" with subs:
  | amount       | fullName   | bonus          |
  | $%(%chunk) r | Corner Pub | $%(.10*%chunk) |

Scenario: a member company pays employees virtually
  Given balances:
  | id   | r               | usd           | rewards | minimum |
  | .ZZA |             200 | %(%chunk - 1) |       5 |      10 |
  | .ZZB |              60 |             0 |      20 |     100 |
  | .ZZC | %(10 + %chunk4) |             0 |      20 |      10 |
  | .ZZD |               0 |           100 |      20 |     100 |
  | .ZZE |               5 |           500 |       5 |     200 |
  # %chunk4 is 4 * %chunk
  When cron runs "payEmployees"
  Then transactions:
  | xid   | created | type     | state | amount         | r              | from | to   | purpose               |
  | .AAAB | %today  | transfer | done  |              0 | %chunk         | ctty | .ZZD | rCredits/USD exchange |
  | .AAAC | %today  | transfer | done  |              0 | %chunk         | ctty | .ZZD | rCredits/USD exchange |
  | .AAAD | %today  | transfer | done  |              0 | %(%chunk + 1)  | ctty | .ZZD | rCredits/USD exchange |
  | .AAAE | %today  | transfer | done  |              0 | %(%chunk3 + 1) | .ZZA | ctty | rCredits/USD exchange |
  | .AAAF | %today  | transfer | done  |              0 | %chunk4        | .ZZC | .ZZA | payment exchange       |
  | .AAAG | %today  | rebate   | done  | %(.05*%chunk4) | %(.05*%chunk4) | ctty | .ZZC | rebate on #1          |
  | .AAAH | %today  | bonus    | done  | %(.10*%chunk4) | %(.10*%chunk4) | ctty | .ZZA | bonus on #2           |
  And usd transfers:
  | payer | payee | amount        |
  | .ZZD  |  .ZZC |        %chunk |
  | .ZZD  |  .ZZC |        %chunk |
  | .ZZD  |  .ZZC | %(%chunk + 1) |
  | .ZZA  |  .ZZC | %(%chunk - 1) |
  And we notice "payment exchanges offered" to member ".ZZC" with subs:
  | offers | total         | whom      |
  |      1 | $%(%chunk4) r | employees |
  And we notice "payment exchange received" to member ".ZZA" with subs:
  | amount        | fullName   | bonus           |
  | $%(%chunk4) r | Corner Pub | $%(.10*%chunk4) |
