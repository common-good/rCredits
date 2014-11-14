Feature: Get rCredits/USD
AS a member
I WANT to transfer credit to my bank account
SO I can pay it to non-members
OR
I WANT to transfer credit from my bank account
SO I can spend it through the rCredits system or hold it in the rCredits system as savings.

Setup:
  Given members:
  | id   | fullName | floor | minimum | flags   | risks   |*
  | .ZZA | Abe One  |   -50 |      10 | ok,bona | hasBank |
  | .ZZB | Bea Two  |     0 |      10 | ok      | hasBank |
  | .ZZC | Our Pub  |     0 |     100 | co,ok   |         |
  | .ZZD | Dee Four |     0 |     100 | ok      | hasBank |
  And transactions:
  | xid | created    | type   | amount | from | to   | purpose |*
  | 1   | %today-10d | signup |     20 | ctty | .ZZA | signup  |
  | 2   | %today-10d | grant  |    100 | ctty | .ZZB | grant   |
  | 3   | %today-10d | signup |     10 | ctty | .ZZC | signup  |
  | 4   | %today-10d | signup |     20 | ctty | .ZZD | signup  |
  And usd transfers:
  | txid | payer | payee | amount | tid | created   | completed |*
  | 5001 |  .ZZA |     0 |    -99 |   1 | %today-7d | %today-5d |
  | 5002 |  .ZZA |     0 |   -100 |   2 | %today-5d |         0 |
  | 5003 |  .ZZA |     0 |     13 |   3 | %today-2d | %today-2d |
  | 5004 |  .ZZB |     0 |      4 |   1 | %today-2d | %today-2d |
  | 5005 |  .ZZC |     0 |    -30 |   1 | %today-2d | %today-2d |
  | 5006 |  .ZZD |     0 |   -140 |   1 | %today-2d | %today-2d |
  Then balances:
  | id   | r    | rewards |*
  | .ZZA |  106 |      20 |
  | .ZZB |   96 |       0 |
  | .ZZC |   40 |      10 |
  | .ZZD |  160 |      20 |

Scenario: a member moves credit to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount |*
  | put |     86 |
  Then usd transfers:
  | payer | payee | amount | tid | created   | completed |*
  |  .ZZA |     0 |     86 |   4 | %today    | %today    |
  And we say "status": "banked" with subs:
  | action     | amount |*
  | deposit to | $86    |
  And balances:
  | id   | r  |*
  | .ZZA | 20 |

Scenario: a member draws credit from the bank with zero floor
  When member ".ZZB" completes form "get" with values:
  | op  | amount    |*
  | get | %R_ACHMIN |
  Then usd transfers:
  | txid | payer | payee | amount     | tid | created | completed |*
  | 5007 |  .ZZB |     0 | -%R_ACHMIN |   2 | %today  |         0 |
  And balances:
  | id   | r   |*
  | .ZZA | 106 |
  And we say "status": "banked|bank tx number" with subs:
  | action     | amount     | checkNum |*
  | draw from  | $%R_ACHMIN |     5007 |

Scenario: a member draws credit from the bank with adequate floor
  When member ".ZZA" completes form "get" with values:
  | op  | amount    |*
  | get | %R_ACHMIN |
  Then usd transfers:
  | txid | payer | payee | amount     | tid | created | completed |*
  | 5007 |  .ZZA |     0 | -%R_ACHMIN |   4 | %today  |    %today |
  And balances:
  | id   | r                |*
  | .ZZA | %(106+%R_ACHMIN) |
  And we say "status": "banked|bank tx number|available now" with subs:
  | action     | amount     | checkNum |*
  | draw from  | $%R_ACHMIN |     5007 |
  
Scenario: a member moves too little to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount           |*
  | put | %(%R_ACHMIN-.01) |
  Then we say "error": "bank too little"

Scenario: a member tries to cash out rewards and/or pending withdrawals
  When member ".ZZA" completes form "get" with values:
  | op  | amount |*
  | put |     87 |
  Then we say "error": "short put|short cash help" with subs:
  | max |*
  | $86 |

Scenario: a member moves too much to the bank
  When member ".ZZB" completes form "get" with values:
  | op  | amount |*
  | put |    200 |
  Then we say "error": "short put|short cash help" with subs:
  | max |*
  | $96 |
  # one chunk each from ctty, A, and D. Only $2 from C.

Scenario: a member tries to go below their minimum
  When member ".ZZD" completes form "get" with values:
  | op  | amount |*
  | put |     61 |
  Then we say "error": "change min first"

Scenario: a member asks to do two transfers out in one day
  Given usd transfers:
  | payer | payee | amount | tid | created   |*
  |  .ZZD |     0 |      6 |   0 | %today    |
  When member ".ZZD" completes form "get" with values:
  | op  | amount |*
  | put |     10 |
  Then we show "Bank Transfer" with:
  |_Pending |
  | You have total pending transfer requests of $6 to your bank account. |
  And we say "error": "short put|short cash help" with subs:
  | max |*
  | $0  |
