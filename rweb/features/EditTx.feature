Feature: Edit Transaction
AS a member
I WANT to change the details of a payment to or from me
SO I can make it right.

Setup:
  Given members:
  | id   | fullName   | rebate | flags      |*
  | .ZZA | Abe One    |      5 | ok,confirmed,bona    |
  | .ZZB | Bea Two    |     10 | ok,confirmed,bona    |
  | .ZZC | Corner Pub |     10 | ok,confirmed,co,bona |
  And relations:
  | id   | main | agent | permission |*
  | :ZZA | .ZZC | .ZZA  | sell       |
  | :ZZB | .ZZC | .ZZB  | buy        |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose      |*
  |   1 | %today-6m | signup   |    250 | ctty | .ZZA | signup       |
  |   2 | %today-6m | signup   |    250 | ctty | .ZZB | signup       |
  |   3 | %today-6m | signup   |    250 | ctty | .ZZC | signup       |
  |   4 | %today    | transfer |     20 | .ZZA | .ZZB | stuff        |
  |   5 | %today    | rebate   |      1 | ctty | .ZZA | rebate on #2 |
  |   6 | %today    | bonus    |      2 | ctty | .ZZB | bonus on #2  |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -753 |         |
  | .ZZA |     231 |     251 |
  | .ZZB |     272 |     252 |
  | .ZZC |     250 |     250 |

Scenario: A buyer changes the transaction description
  Given member ".ZZA" edits transaction "4" with values:
  | amount | goods        | purpose |*
  |     20 | %R_FOR_GOODS | things  |
  When member ".ZZA" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |_tid | Date | Name    | From you | To you | Status | _ | Purpose | Reward/Fee |
  |   2 | %dm  | Bea Two |    20.00 |     -- | %chk   | X | things  |       1.00 |

Scenario: A buyer increases a payment amount
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods        | purpose |*
  |     40 | %R_FOR_GOODS | stuff   |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -756 |         |
  | .ZZA |     212 |     252 |
  | .ZZB |     294 |     254 |
  | .ZZC |     250 |     250 |
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
  |     20 | %R_FOR_USD | stuff   |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -750 |         |
  | .ZZA |     230 |     250 |
  | .ZZB |     270 |     250 |
  | .ZZC |     250 |     250 |
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
  |     10 | %R_FOR_GOODS | stuff   |
  Then we say "error": "illegal amount change" with subs:
  | amount | action   | a |*
  | $20    | decrease | ? |

Scenario: A buyer disputes a charge
  Given transactions:
  | xid | created   | type     | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    |      8 | ctty | .ZZC | bonus    | 0      |
  When member ".ZZA" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status | _ | Purpose | Reward/Fee |
  |   3 | %dm-5d | Corner Pub | 80.00    | --     | %chk   | X | this CF | 4.00       |
  When member ".ZZA" clicks "X" on transaction 100
  Then we show "tx summary|confirm tx action" with subs:
  | amount | otherName  | otherDid | purpose | created   | txAction            |*
  | $80    | Corner Pub | charged  | this CF | %today-5d | DISPUTE this charge |
  When member ".ZZA" confirms "X" on transaction 100
#  When member ".ZZA" confirms form "history/transactions/period=5&do=no&xid=100" with values: ""
  Then we say "status": "report undo" with subs:
  | solution |*
  | marked ''disputed'' |
  And we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status   | _ | Purpose | Reward/Fee |
  |   3 | %dm-5d | Corner Pub | 80.00    | --     | disputed |   | this CF | 4.00       |
  
Scenario: A seller reverses a charge
  Given transactions:
  | xid | created   | type     | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    |      8 | ctty | .ZZC | bonus    | 0      |
  When member ":ZZB" visits page "history/transactions/period=5"
  Then we show "Transaction History" with:
  |_tid | Date   | Name    | From you | To you | Status | _ | Purpose | Reward/Fee |
  |   2 | %dm-5d | Abe One | --       | 80.00  | %chk   | X | this CF | 8.00       |
  When member ":ZZB" clicks "X" on transaction 100
  Then we show "tx summary|confirm tx action" with subs:
  | amount | otherName | otherDid | purpose | created   | txAction            |*
  | $80    | Abe One   | paid     | this CF | %today-5d | REVERSE this charge |
  
Skip
Scenario: A member confirms OK
  Given transactions:
  | xid | created   | type     | state   | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer | pending |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   | pending |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    | pending |      8 | ctty | .ZZC | bonus    | 0      |
  And next DO code is "whatever"
  When member ".ZZA" confirms form "history/transactions/period=5&do=ok&xid=100" with values: ""
  Then we say "status": "report tx|reward" with subs:
  | did    | otherName  | amount | why                | rewardAmount |*
  | paid   | Corner Pub | $80    | goods and services | $4           |
  And we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status | _ | Purpose | Reward/Fee |
  | 12  | %dm    | Corner Pub | 80.00    | --     | %chk   | X | this CF | 4.00   |
  And we notice "new payment|reward other" to member ".ZZC" with subs:
  | created | fullName   | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |*
  | %today  | Corner Pub | Abe One   | $80 | this CF | reward | $8 |
  And that "notice" has link results:
  | _name   |*
  | Abe One |
  # etc. (has to be vertical if just one value)
  # notice must postcede we show Transaction History (so as not to overwrite formOut['text']) -- fix that
  
Scenario: A member confirms OK for a disputed transaction
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |*
  | 100 | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  When member ".ZZA" confirms form "history/transactions/period=5&do=ok&xid=100" with values: ""
  Then we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status | _ | Purpose | Reward |
  | 11  | %dm-5d | Corner Pub | 80.00    | --     | %chk   | X | this CF | 4.00   |
  And we say "status": "charge accepted" with subs:
  | who     |*
  | Abe One |
