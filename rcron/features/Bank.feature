Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags                   |
  | .ZZA | Abe One  |     0 |     100 | dft,personal,company,ok |
  
Scenario: a member is barely below minimum
  Given balances:
  | id   | r  | usd   | rewards |
  | .ZZA | 50 | 49.99 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount       |
  | .ZZA | -%R_BANK_MIN |
  And we notice "minmax status|banked" to member ".ZZA" with subs:
  | action    | status                    | amount       |
  | draw from | under the minimum you set | $%R_BANK_MIN |
  
Scenario: a member is at minimum
  Given balances:
  | id   | r  | usd | rewards |
  | .ZZA | 50 |  50 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member is well below minimum
  Given balances:
  | id   | r  | usd | rewards |
  | .ZZA | 20 |   6 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |
  | .ZZA  |    -74 |
  And we notice "minmax status|banked" to member ".ZZA" with subs:
  | action    | status                    | amount |
  | draw from | under the minimum you set |    $74 |

Scenario: a member is under minimum but already requested barely enough funds from the bank
  Given balances:
  | id   | r   | usd    | rewards |
  | .ZZA | 10  |     10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |
  | .ZZA  |    -80 |
  When cron runs "bank"
# (again)  
  Then bank transfer count is 1
  
Scenario: a member is under minimum and has requested insufficient funds from the bank
  Given balances:
  | id   | r    | usd    | rewards |
  | .ZZA |   10 |     10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |
  | .ZZA  |    -80 |
  Given balances:
  | id   | r    | usd | rewards |
  | .ZZA | 9.99 |  10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount       |
  | .ZZA  | -%R_BANK_MIN |

