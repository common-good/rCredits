Feature: Edit Transaction
AS a member
I WANT to change the details of a payment to or from me
SO I can make it right.

Setup:
  Given members:
  | id   | fullName   | rebate | flags      |*
  | .ZZA | Abe One    |      5 | ok,confirmed         |
  | .ZZB | Bea Two    |     10 | ok,confirmed         |
  | .ZZC | Corner Pub |     10 | ok,confirmed,co      |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | sell       |
  | .ZZC | .ZZB  |   2 | manage     |
  And transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose      |*
  |   1 | %today-6m | signup   |      0 |      0 |   250 | ctty | .ZZA | signup       |
  |   2 | %today-6m | signup   |      0 |      0 |   250 | ctty | .ZZB | signup       |
  |   3 | %today-6m | signup   |      0 |      0 |   250 | ctty | .ZZC | signup       |
  |   4 | %today    | transfer |     20 |      1 |     2 | .ZZA | .ZZB | stuff        |
  Then balances:
  | id   | balance |*
  | .ZZA |     -20 |
  | .ZZB |      20 |
  | .ZZC |       0 |

Scenario: A buyer changes the transaction description
  Given member ".ZZA" edits transaction "4" with values:
  | amount | goods        | purpose |*
  |     20 | %FOR_GOODS | things  |
  When member ".ZZA" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |~tid | Date | Name    | Purpose | Amount | ~ |
  |   2 | %mdy  | Bea Two | things  | -20.00 | X |

Scenario: A buyer increases a payment amount
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods        | purpose |*
  |     40 | %FOR_GOODS | stuff   |
  Then balances:
  | id   |  balance |*
  | .ZZA |      -40 |
  | .ZZB |       40 |
  | .ZZC |        0 |
  And we say "status": "info saved"
  And we notice "tx edited|new tx amount" to member ".ZZA" with subs:
  | tid | who     | amount |*
  | 2   | you     | $40    |
  And we notice "tx edited|new tx amount" to member ".ZZB" with subs:
  | tid | who     | amount |*
  | 2   | Abe One | $40    |

Scenario: A buyer changes the goods status
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods      | purpose |*
  |     20 | %FOR_USD | stuff   |
  Then balances:
  | id   | balance |*
  | .ZZA |     -20 |
  | .ZZB |      20 |
  | .ZZC |       0 |
  And we say "status": "info saved"
  And we notice "tx edited|new tx goods" to member ".ZZA" with subs:
  | tid | who     | what        |*
  | 2   | you     | exchange of US Dollars or other currency |
  And we notice "tx edited|new tx goods" to member ".ZZB" with subs:
  | tid | who     | what  |*
  | 2   | Abe One | exchange of US Dollars or other currency |

Scenario: A buyer tries to decrease a payment amount
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods        | purpose |*
  |     10 | %FOR_GOODS | stuff   |
  Then we say "error": "illegal amount change" with subs:
  | amount | action   | a |*
  | $20    | decrease | ? |

Scenario: A buyer disputes a charge
  Given transactions:
  | xid | created   | type     | amount | from | to   | purpose  | flags  |*
  | 100 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | taking |
  When member ".ZZA" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |~tid | Date    | Name       | Purpose | Amount | ~ |
  |   3 | %mdy-5d | Corner Pub | this CF | -80.00 | X |
  # Status was %chk 
  When member ".ZZA" clicks "X" on transaction 100
  Then we scrip "reverse-tx" with subs:
  | title   |  msg                      |*
  | Reverse | Reverse this transaction? |
#  Then we show "tx desc passive|purpose|when|.|confirm tx action" with subs:
#  | amount | otherName  | otherDid | purpose     | created   | txAction                        |*
#  | $80    | Corner Pub | charged  | "this CF" | %today-5d | request a refund of this charge |
  When member ".ZZA" confirms "X" on transaction 100
#  When member ".ZZA" confirms form "history/transactions/period=5&do=no&xid=100" with values: ""
Skip (test doesn't work yet)
  Then we say "status": "report undo" with subs:
  | solution |*
  | marked "disputed" |
  And we show "Transaction History" with:
  |~tid | Date   | Name       | Purpose | Amount | ~ |
  |   3 | %mdy-5d | Corner Pub | this CF | -80.00 |   |
  # Status was disputed
Resume
  
Scenario: A seller reverses a charge
  Given transactions:
  | xid | created   | type     | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | 1      |
  When member "C:B" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |~tid | Date   | Name    | Purpose | Amount | ~ |
  |   2 | %mdy-5d | Abe One | this CF | 80.00  | X |
  When member "C:B" clicks "X" on transaction 100
  Then we scrip "reverse-tx" with subs:
  | title   |  msg                      |*
  | Reverse | Reverse this transaction? |
#  Then we show "tx desc active|purpose|when|.|confirm tx action" with subs:
#  | amount | otherName | did     | purpose   | created   | txAction            |*
#  | $80    | Abe One   | charged | "this CF" | %today-5d | reverse this charge |

Skip no dispute feature at the moment  
Scenario: A member confirms OK for a disputed transaction
  Given transactions:
  | xid | created   | type     | flags    | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  When member ".ZZA" completes form "history/transactions/period=5&do=ok&xid=100" with values: ""
  Then we say "status": "confirm accept" with subs:
  | who        | did     | amount | for       | date    | do                  |*
  | Corner Pub | charged | $80    | "this CF" | %dmy-5d | accept this charge? |
Resume
# This test doesn't work yet.
#  When member ".ZZA" confirms form "history/transactions/period=5&do=ok&xid=100" with values: ""
#  Then we show "Transaction History" with:
#  |~tid | Date   | Name       | Purpose | Amount | Reward | ~ |
#  | 11  | %mdy-5d | Corner Pub | this CF | -80.00 |   4.00 | X |
#  And we say "status": "charge accepted" with subs:
#  | who     |*
#  | Abe One |
