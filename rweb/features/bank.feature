Feature: Get rCredits/USD
AS a member
I WANT to transfer credit to my bank account
SO I can pay it to non-members
OR
I WANT to transfer credit from my bank account
SO I can spend it through the rCredits system or hold it in the rCredits system as savings.

Setup:
  Given members:
  | id   | fullName | minimum | floor | flags             | risks   |*
  | .ZZA | Abe One  |       0 |   -20 | ok,debt,bankOk    | hasBank |
  | .ZZB | Bea Two  |       0 |     0 | ok                | hasBank |
  | .ZZC | Our Pub  |      40 |   -10 | co,ok,debt,bankOk | hasBank |
  | .ZZD | Dee Four |      80 |   -20 | ok,refill,debt    | hasBank |
	And relations:
	| main | other | permission |*
	| .ZZC | .ZZB  |     manage |
  And transactions:
  | xid | created    | type   | amount | from | to   | purpose |*
  | 1   | %today-10d | signup |     20 | ctty | .ZZA | signup  |
  | 2   | %today-10d | grant  |    100 | ctty | .ZZB | grant   |
  | 3   | %today-10d | signup |     10 | ctty | .ZZC | signup  |
  | 4   | %today-10d | signup |     20 | ctty | .ZZD | signup  |
  And usd transfers:
  | txid | payee | amount | created   | completed | deposit   |*
  | 5001 |  .ZZA |     99 | %today-7d | %today-5d | %today-1d |
  | 5002 |  .ZZA |    100 | %today-5d |         0 | %today-1d |
  | 5003 |  .ZZA |    -13 | %today-2d | %today-2d | %today-1d |
  | 5004 |  .ZZB |     -4 | %today-2d | %today-2d | %today-1d |
  | 5005 |  .ZZC |     30 | %today-2d | %today-2d | %today-1d |
  | 5006 |  .ZZD |    140 | %today-2d | %today-2d | %today-1d |
  Then balances:
  | id   | balance |*
  | .ZZA |      86 |
  | .ZZB |      96 |
  | .ZZC |      30 |
  | .ZZD |     140 |

Scenario: a member moves credit to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount |*
  | put |     86 |
  Then usd transfers:
  | payee | amount | created   | completed | channel |*
  |  .ZZA |    -86 | %today    | %today    | %TX_WEB |
  And we say "status": "banked" with subs:
  | action     | amount | why             |*
  | deposit to | $86    | at your request |
  And balances:
  | id   | balance |*
  | .ZZA |       0 |

Scenario: a member draws credit from the bank with zero floor
  When member ".ZZB" completes form "get" with values:
  | op  | amount    |*
  | get | %R_ACHMIN |
  Then usd transfers:
  | txid | payee | amount    | created   | completed | channel |*
  | 5007 |  .ZZB | %R_ACHMIN | %tomorrow |         0 | %TX_WEB |
  And balances:
  | id   | balance |*
  | .ZZA |      86 |
  And we say "status": "banked|bank tx number" with subs:
  | action     | amount     | checkNum | why             |*
  | draw from  | $%R_ACHMIN |     5007 | at your request |

Scenario: a member draws credit from the bank with adequate floor
  When member "C:B" completes form "get" with values:
  | op  | amount |*
  | get |     10 |
  Then usd transfers:
  | txid | payee | amount | created | completed | channel |*
  | 5007 |  .ZZC |     10 | %today  |    %today | %TX_WEB |
  And balances:
  | id   | balance |*
  | .ZZC | 40      |
  And we say "status": "banked|bank tx number|available now" with subs:
  | action     | amount | checkNum | why             |*
  | draw from  |    $10 |     5007 | at your request |
  
Scenario: a member moves too little to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount           |*
  | put | %(%R_ACHMIN-.01) |
  Then we say "error": "bank too little"

#Scenario: a member tries to cash out rewards and/or pending withdrawals
#  When member ".ZZA" completes form "get" with values:
#  | op  | amount |*
#  | put |     87 |
#  Then we say "error": "short put|short cash help" with subs:
#  | max |*
#  | $86 |

Scenario: a member moves too much to the bank
  When member ".ZZB" completes form "get" with values:
  | op  | amount |*
  | put |    200 |
  Then we say "error": "short put" with subs:
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
  | payee | amount | created   |*
  |  .ZZD |     -6 | %today    |
  When member ".ZZD" completes form "get" with values:
  | op  | amount |*
  | put |     10 |
  Then we show "Exchange US Dollars for Common Good Credits" with:
  |~Pending |
  | You have total pending exchange requests of $6 to your bank account. |
  And we say "error": "short put" with subs:
  | max |*
  | $0  |
