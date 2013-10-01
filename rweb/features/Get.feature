Feature: Get rCredits/USD
AS a member
I WANT to transfer credit to my bank account
SO I can pay it to non-members
OR
I WANT to transfer credit from my bank account
SO I can spend it through the rCredits system or hold it in the rCredits system as savings.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags                 |
  | .ZZA | Abe One  |     0 |      10 | dft,person,ok,bank    |
  | .ZZB | Bea Two  |     0 |      10 | dft,person,ok,bank    |
  | .ZZC | Our Pub  |     0 |     100 | dft,person,company,ok |
  | .ZZD | Dee Four |     0 |     100 | dft,person,ok,bank    |
  And balances:
  | id   | r    | usd | rewards |
  | ctty | -320 | 500 |       0 |
  | .ZZA |    0 | 100 |      20 |
  | .ZZB |  320 |   0 |      20 |
  | .ZZC |    0 |  30 |      20 |
  | .ZZD |    0 | 140 |      20 |
  And usd transfers:
  | payer | payee | amount | tid | created   |
  |  .ZZA |     0 |      1 |   0 | %today-5d |
  |  .ZZA |  .ZZB |      2 |   0 | %today-3d |
  |  .ZZA |     0 |      3 |   0 | %today-2d |
  |  .ZZB |     0 |      4 |   5 | %today-2d |

Scenario: a member moves credit to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount |
  | put |     12 |
  Then usd transfers:
  | payer | payee | amount | tid | created   |
  |  .ZZA |     0 |      1 |   1 | %today-5d |
  |  .ZZA |  .ZZB |      2 |   0 | %today-3d |
  |  .ZZA |     0 |      3 |   2 | %today-2d |
  |  .ZZA |     0 |     12 |   3 | %today    |
  And we say "status": "banked" with subs:
  | action     | amount |
  | deposit to | $12    |

Scenario: a member draws credit from the bank
  When member ".ZZB" completes form "get" with values:
  | op  | amount      |
  | get | %R_BANK_MIN |
  Then usd transfers:
  | payer | payee | amount       | tid |
  |  .ZZB |     0 | -%R_BANK_MIN |   6 |
  And we say "status": "banked" with subs:
  | action     | amount       |
  | draw from  | $%R_BANK_MIN |

Scenario: a member moves too little to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount             |
  | put | %(%R_BANK_MIN-.01) |
  Then we say "error": "bank too little"

Scenario: a member tries to cash out rewards
  When member ".ZZA" completes form "get" with values:
  | op  | amount |
  | put |     81 |
  Then we say "error": "short deposit" with subs:
  | max |
  | $80 |

Scenario: a member moves inconveniently much to the bank
  When member ".ZZB" completes form "get" with values:
  | op  | amount |
  | put |    200 |
  Then we say "error": "short deposit" with subs:
  | max                     |
  | $%(4*%DW_FEE_THRESHOLD) |

Scenario: a member tries to go below their minimum
  When member ".ZZD" completes form "get" with values:
  | op  | amount |
  | put |     50 |
  Then we say "error": "change min first"

Scenario: a member asks to do two transfers out in one day
  Given usd transfers:
  | payer | payee | amount | tid | created   |
  |  .ZZD |     0 |      6 |   0 | %today    |
  When member ".ZZD" completes form "get" with values:
  | op  | amount |
  | put |     10 |
  Then we show "Bank Transfer" with:
  | Pending: |
  | You have total pending transfer requests of $6 to your bank account. |
  And we say "error": "short deposit" with subs:
  | max |
  | $0  |
