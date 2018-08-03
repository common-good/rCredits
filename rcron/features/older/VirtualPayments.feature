Feature: payment exchanges
AS a member company
I WANT to pay my employees and suppliers rCredits automatically
SO I don't have to do change my current payroll and accounting system
and SO I don't build up a pile of credit I can't use yet.

Setup:
  Given members:
  | id   | fullName   | email         |floor | minimum | maximum | flags                        |
  | .ZZA | Abe One    | a@example.com |    0 |      20 |     100 | dft,personal,ok,bona         |
  | .ZZB | Bea Two    | b@example.com |    0 |     100 |     200 | dft,personal,company,ok,bona |
  | .ZZC | Corner Pub | c@example.com |    0 |      10 |      10 | dft,company,payex,ok,bona  |
  | .ZZD | Dee Four   | d@example.com |    0 |     100 |     100 | dft,personal,ok,bona         |
  | .ZZE | Ezra Five  | e@example.com |    0 |     200 |      -1 | dft,personal,member          |
  And relations:
  | id      | main | agent | employerOk | permission | amount |
  | NEW.ZZA | .ZZC | .ZZA  |          1 | sell       |   1800 |
  | NEW.ZZB | .ZZC | .ZZB  |          0 |            |    200 |
  | NEW.ZZE | .ZZC | .ZZE  |          1 |            |   2000 |
# 90/10 split (E is not an rTrader, so doesn't count in the split)
  
Scenario: a member company pays suppliers virtually
  Given balances:
  | id   | r  | usd | rewards |
  | .ZZA | 10 |  40 |       5 |
  | .ZZB | 60 |   0 |      20 |
  | .ZZC | 50 |   0 |      20 |
  | .ZZD |  0 | 100 |      20 |
  | .ZZE |  5 | 500 |       5 |
  When cron runs "paySuppliers"
  Then transactions:
  | xid   | created | type     | state | amount | r    | from | to   | purpose               |
  | .AAAB | %today  | transfer | done  |      0 |    4 | ctty | .ZZA | rCredits/USD exchange |
  | .AAAC | %today  | transfer | done  |      0 |    4 | .ZZB | ctty | rCredits/USD exchange |
  | .AAAD | %today  | transfer | done  |      0 |    4 | .ZZC | .ZZB | payment exchange       |
  | .AAAE | %today  | rebate   | done  |   0.20 | 0.20 | ctty | .ZZC | rebate on #1          |
  | .AAAF | %today  | bonus    | done  |   0.40 | 0.40 | ctty | .ZZB | bonus on #2           |
  And we notice "payment exchanges offered" to member ".ZZC" with subs:
  | offers | total | whom      |
  |      1 |   $4r | suppliers |
  And we notice "payment exchange received" to member ".ZZB" with subs:
  | amount | fullName   | bonus |
  |    $4r | Corner Pub | $0.40 |

Scenario: a member company pays employees virtually
  Given balances:
  | id   | r               | usd | rewards | minimum | maximum |
  | .ZZA | %(21 - %chunk)  | 100 |       5 |      10 |      20 |
  | .ZZB | 60              |   0 |      20 |     100 |     200 |
  | .ZZC | %(10 + %chunk4) |   0 |      20 |      10 |      10 |
  | .ZZD |  0              | 100 |      20 |     100 |     150 |
  | .ZZE |  5              | 500 |       5 |     200 |      -1 |
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
  And we notice "payment exchanges offered" to member ".ZZC" with subs:
  | offers | total        | whom      |
  |      1 | $%(%chunk4)r | employees |
  And we notice "payment exchange received" to member ".ZZA" with subs:
  | amount       | fullName   | bonus              |
  | $%(%chunk4)r | Corner Pub | $%(.10*%chunk4).00 |
