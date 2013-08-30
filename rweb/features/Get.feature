Feature: Get rCredits/USD
AS a member
I WANT to transfer credit to my bank account
SO I can pay it to non-members
OR
I WANT to transfer credit from my bank account
SO I can spend it through the rCredits system or hold it in the rCredits system as savings.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags                   |
  | .ZZA | Abe One  |     0 |     100 | dft,person,ok,bank    |
  | .ZZB | Bea Two  |     0 |     100 | dft,person,ok         |
  | .ZZC | Our Pub  |     0 |     100 | dft,person,company,ok |
  | .ZZD | Dee Four |     0 |     100 | dft,person,ok         |

Scenario: a member moves credit to the bank
  Given balances:
  | id   | r | usd | rewards |
  | .ZZA | 0 | 500 |      20 |
  When member ".ZZA" completes form "get" with values:
  | op  | amount |
  | put |      8 |
  Than bank transfers:
  | payer | payee | amount |
  | .ZZA  |     0 |      8 |

Scenario: a member draws credit from the bank
  Given balances:
  | id   | r | usd | rewards |
  | .ZZA | 0 |   0 |      20 |
  When member ".ZZA" completes form "get" with values:
  | op  | amount |
  | get |      8 |
  Than bank transfers:
  | payer | payee | amount |
  | .ZZA  |     0 |     -8 |
  
#  | .ZZA  | -%R_BANK_MIN |

