Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.
and
I WANT credit to flow to my bank account
SO I can pay it to non-members.

Setup:
  Given members:
  | id   | fullName | floor | minimum | maximum | flags                           |
  | .ZZA | Abe One  |     0 |     100 |     200 | dft,personal,company,ok         |
  
Scenario: a member is barely below minimum
  Given balances:
  | id   | r  | usd   | rewards |
  | .ZZA | 50 | 49.99 |      20 |
  When cron runs "bank"
  Then bank transfers:
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
  Then bank transfers:
  | payer | amount |
  | .ZZA |    -74 |
  And we notice "minmax status|banked" to member ".ZZA" with subs:
  | action    | status                    | amount |
  | draw from | under the minimum you set |    $74 |

Scenario: a member is over maximum
  Given balances:
  | id   | r   | usd | rewards |
  | .ZZA | 100 | 110 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA |     10 |
  And we notice "minmax status|banked" to member ".ZZA" with subs:
  | action     | status                   | amount |
  | deposit to | over the maximum you set |    $10 |

Scenario: a member is barely over maximum
  Given balances:
  | id   | r           | usd    | rewards |
  | .ZZA | %R_BANK_MIN | 199.99 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0

Scenario: a member is under minimum but already requested barely enough funds from the bank
  Given balances:
  | id   | r   | usd    | rewards |
  | .ZZA | 10  |     10 |      20 |
  When cron runs "bank"
  Then bank transfers:
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
  Then bank transfers:
  | payer | amount |
  | .ZZA  |    -80 |
  Given balances:
  | id   | r    | usd | rewards |
  | .ZZA | 9.99 |  10 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount       |
  | .ZZA  | -%R_BANK_MIN |

Scenario: a member is over maximum but already requested that barely enough funds go to the bank
  Given balances:
  | id   | r | usd    | rewards |
  | .ZZA | 0 | 260.01 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA  |  60.01 |
  Given balances:
  | id   | r           | usd | rewards |
  | .ZZA | %R_BANK_MIN | 260 |      20 |
  When cron runs "bank"
  Then bank transfer count is 1
  
Scenario: a member is over maximum and has requested insufficient funds to go to the bank
  Given balances:
  | id   | r | usd | rewards |
  | .ZZA | 0 | 260 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA  |     60 |
  Given balances:
  | id   | r           | usd | rewards |
  | .ZZA | %R_BANK_MIN | 260 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount      |
  | .ZZA  | %R_BANK_MIN |

Scenario: a member is over maximum and has requested funds FROM the bank
  Given balances:
  | id   | r  | usd | rewards |
  | .ZZA | 20 |   5 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA  |    -75 |
  Given balances:
  | id   | r  | usd | rewards |
  | .ZZA | 20 | 500 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA  |    395 |

Scenario: a member has no maximum
  Given members:
  | id   | fullName   | floor | minimum | maximum | flags                           |
  | .ZZB | Bea Two    |     0 |     100 |      -1 | dft,personal,ok                 |
  And balances:
  | id   | r  | usd | rewards |
  | .ZZA | 50 |  50 |      20 |
  | .ZZB | 20 | 500 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member pays virtually
  Given members:
  | id   | fullName   | floor | minimum | maximum | flags                           |
  | .ZZC | Corner Pub |     0 |      10 |       5 | dft,company,ok,payex          |
  And balances:
  | id   | r  | usd | rewards |
  | .ZZA | 50 |  50 |      20 |
  | .ZZC | 20 | 100 |      70 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZC  |     50 |

Scenario: a member is over maximum but mostly in rCredits
  Given balances:
  | id   | r   | usd   | rewards |
  | .ZZA | 500 | 25.67 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer | amount |
  | .ZZA  |  25.67 |

Scenario: a member is over plenty over maximum but not enough over floor
  Given members:
  | id   | fullName   | floor | minimum | maximum | flags                   |
  | .ZZC | Corner Pub |   100 |     100 |      10 | dft,company,ok          |
  And balances:
  | id   | r           | usd   | rewards |
  | .ZZA |          50 |    50 |      20 |
  | .ZZC | %R_BANK_MIN | 99.99 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
