Feature: Virtual Payments
AS a member company
I WANT to pay my employees and suppliers rCredits automatically
SO I don't have to do change my current payroll and accounting system.
and
SO I don't build up a pile of credit I can't use yet.

Setup:
  Given members:
  | id      | fullName   | email         |floor | minimum | maximum | flags                   |
  | NEW.ZZA | Abe One    | a@example.com |    0 |      20 |      20 | dft,personal,ok         |
  | NEW.ZZB | Bea Two    | b@example.com |    0 |     100 |     200 | dft,personal,company,ok |
  | NEW.ZZC | Corner Pub | c@example.com |    0 |      10 |       0 | dft,company,virtual,ok  |
  | NEW.ZZD | Dee Four   | d@example.com |    0 |     100 |     100 | dft,personal,ok         |
  And relations:
  | id      | main    | agent   | employerOk | permission | amount |
  | NEW:ZZA | NEW.ZZC | NEW.ZZA |          1 | sell       |   1800 |
  | NEW:ZZB | NEW.ZZC | NEW.ZZB |          0 |            |    200 |
#90/10 split
  
Scenario: a member company pays suppliers virtually
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 10 | 100 |       5 |
  | NEW.ZZB | 60 |   0 |      20 |
  | NEW.ZZC | 50 |   0 |      20 |
  | NEW.ZZD |  0 | 100 |      20 |
  When cron runs "paySuppliers"
  Then transactions:
  | tx_id    | created | type        | state    | amount | r    | from      | to      | purpose               |
  | NEW.AAAB | %today | %TX_TRANSFER | %TX_DONE |      4 |    4 | NEW.ZZB | community | rCredits/USD exchange |
  | NEW.AAAC | %today | %TX_TRANSFER | %TX_DONE |      4 |    4 | community | NEW.ZZA | rCredits/USD exchange |
  | NEW.AAAD | %today | %TX_TRANSFER | %TX_DONE |      0 |    4 | NEW.ZZC   | NEW.ZZB | virtual payment       |
  | NEW.AAAE | %today | %TX_REBATE   | %TX_DONE |   0.20 | 0.20 | community | NEW.ZZC | rebate on #1          |
  | NEW.AAAF | %today | %TX_BONUS    | %TX_DONE |   0.40 | 0.40 | community | NEW.ZZB | bonus on #2           |
  And we notice "virtual payments offered" to member "NEW.ZZC" with subs:
  | offers | total | whom      |
  |      1 |   $4r | suppliers |
  And we notice "virtual payment received" to member "NEW.ZZB" with subs:
  | amount | fullName   | bonus |
  |    $4r | Corner Pub | $0.40 |

Scenario: a member company pays employees virtually
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 10 | 100 |       5 |
  | NEW.ZZB | 60 |   0 |      20 |
  | NEW.ZZC | 50 |   0 |      20 |
  | NEW.ZZD |  0 | 100 |      20 |
  When cron runs "payEmployees"
  Then transactions:
  | tx_id    | created | type        | state    | amount | r    | from      | to      | purpose               |
  | NEW.AAAB | %today | %TX_TRANSFER | %TX_DONE |     30 |   30 | NEW.ZZA | community | rCredits/USD exchange |
  | NEW.AAAC | %today | %TX_TRANSFER | %TX_DONE |     30 |   30 | community | NEW.ZZD | rCredits/USD exchange |
  | NEW.AAAD | %today | %TX_TRANSFER | %TX_DONE |      0 |   40 | NEW.ZZC   | NEW.ZZA | virtual payment       |
  | NEW.AAAE | %today | %TX_REBATE   | %TX_DONE |   2.00 | 2.00 | community | NEW.ZZC | rebate on #1          |
  | NEW.AAAF | %today | %TX_BONUS    | %TX_DONE |   4.00 | 4.00 | community | NEW.ZZA | bonus on #2           |
  And we notice "virtual payments offered" to member "NEW.ZZC" with subs:
  | offers | total | whom      |
  |      1 |  $40r | employees |
  And we notice "virtual payment received" to member "NEW.ZZA" with subs:
  | amount | fullName   | bonus |
  |   $40r | Corner Pub | $4.00 |
  