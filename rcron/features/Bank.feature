Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.
and
I WANT credit to flow to my bank account
SO I can pay it to non-members.

Setup:
  Given members:
  | id      | fullName   | floor | minimum | maximum | flags                           |
  | NEW.ZZA | Abe One    |     0 |     100 |     200 | dft,personal,company,ok,to_bank |

Scenario: a member is barely below minimum
  Given balances:
  | id      | r  | usd   | rewards |
  | NEW.ZZA | 50 | 49.99 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -10 |
  
Scenario: a member is at minimum
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 50 |  50 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member is well below minimum
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 20 |   5 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -80 |

Scenario: a member is over maximum
  Given balances:
  | id      | r   | usd | rewards |
  | NEW.ZZA | 100 | 110 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |     10 |

Scenario: a member is barely over maximum
  Given balances:
  | id      | r   | usd    | rewards |
  | NEW.ZZA | 100 | 109.99 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0

Scenario: a member is under minimum but already requested barely enough funds from the bank
  Given balances:
  | id      | r   | usd    | rewards |
  | NEW.ZZA |  15 |      5 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -80 |
  When cron runs "bank"
  Then bank transfer count is 1
  
Scenario: a member is under minimum and has requested insufficient funds from the bank
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 15 |   5 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -80 |
  Given balances:
  | id      | r  | usd  | rewards |
  | NEW.ZZA | 15 | 4.99 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -10 |

Scenario: a member is over maximum but already requested that barely enough funds go to the bank
  Given balances:
  | id      | r   | usd | rewards |
  | NEW.ZZA | 150 | 110 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |     60 |
  When cron runs "bank"
  Then bank transfer count is 1
  
Scenario: a member is over maximum and has requested insufficient funds to go to the bank
  Given balances:
  | id      | r   | usd | rewards |
  | NEW.ZZA | 150 | 110 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |     60 |
  Given balances:
  | id      | r      | usd | rewards |
  | NEW.ZZA | 150.01 | 110 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |     10 |

Scenario: a member is over maximum but has requested funds FROM the bank
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 20 |   5 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |    -80 |
  Given balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 20 | 500 |      20 |
  When cron runs "bank"
  Then bank transfer count is 1

Scenario: a member has no maximum
  Given members:
  | id      | fullName   | floor | minimum | maximum | flags                           |
  | NEW.ZZB | Bea Two    |     0 |     100 |       0 | dft,personal,ok,to_bank         |
  And balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 50 |  50 |      20 |
  | NEW.ZZB | 20 | 500 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0
  
Scenario: a member pays virtually
  Given members:
  | id      | fullName   | floor | minimum | maximum | flags                           |
  | NEW.ZZC | Corner Pub |     0 |     100 |      10 | dft,company,ok,virtual,to_bank  |
  And balances:
  | id      | r  | usd | rewards |
  | NEW.ZZA | 50 |  50 |      20 |
  | NEW.ZZC | 20 | 500 |      20 |
  When cron runs "bank"
  Then bank transfer count is 0

Scenario: a member is over maximum but mostly in rCredits
  Given balances:
  | id      | r   | usd | rewards |
  | NEW.ZZA | 500 |  25 |      20 |
  When cron runs "bank"
  Then bank transfers:
  | payer   | amount |
  | NEW.ZZA |     25 |
