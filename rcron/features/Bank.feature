Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags    | achMin |*
  | .ZZA | Abe One  |     0 |     100 | co,ok,dw | 30     |
  | .ZZB | Bea Two  |     0 |     100 | ok       | 30     |
  And relations:
  | id | main | agent | draw |*
  | 1  | .ZZA | .ZZB  | 1    |
  
Scenario: a member is barely below minimum
  Given balances:
  | id   | r  | usd   | rewards |*
  | .ZZA | 50 | 49.99 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |*
  | .ZZA | -30     |
  And we notice "under min|banked" to member ".ZZA" with subs:
  | action    | amount |*
  | draw from | $30    |

Scenario: an unbanked member barely below minimum draws on another account
  Given balances:
  | id   | r     | usd |*
  | .ZZA | 200   |   0 |
  | .ZZB | 99.99 |   0 |
  When cron runs "bank"
  Then transactions:
  | xid | type     | amount | from | to   | purpose      |*
  |   1 | transfer |     30 | .ZZA | .ZZB | automatic transfer to NEW.ZZB,automatic transfer from NEW.ZZA |
  And we notice "under min|drew" to member ".ZZB" with subs:
  | amount |*
  | $30    |
  
Scenario: an unbanked member barely below minimum cannot draw on another account
  Given balances:
  | id   | r     | usd |*
  | .ZZA | 0     |   0 |
  | .ZZB | 99.99 |   0 |
  When cron runs "bank"
  Then we notice "under min|cannot draw" to member ".ZZB"

Scenario: a member is at minimum
  Given balances:
  | id   | r  | usd | rewards |*
  | .ZZA | 50 |  50 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member is well below minimum
  Given balances:
  | id   | r  | usd | rewards | minimum |*
  | .ZZA | 50 | 0   | 25      | 151     |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount               |*
  | .ZZA  | %(-100 - %R_ACHMIN) |
  And we notice "under min|banked" to member ".ZZA" with subs:
  | action    | amount                |*
  | draw from | $%(100 + %R_ACHMIN) |

Scenario: a member is under minimum but already requested barely enough funds from the bank
  Given balances:
  | id   | r   | usd    | rewards |*
  | .ZZA | 10  |     10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |*
  | .ZZA  |    -80 |
  When cron runs "bank"
# (again)  
  Then bank transfer count is 1
  
Scenario: a member is under minimum and has requested insufficient funds from the bank
  Given balances:
  | id   | r    | usd    | rewards |*
  | .ZZA |   10 |     10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount |*
  | .ZZA  |    -80 |
  Given balances:
  | id   | r    | usd | rewards |*
  | .ZZA | 9.99 |  10 |      20 |
  When cron runs "bank"
  Then usd transfers:
  | payer | amount       |*
  | .ZZA  | -30 |

