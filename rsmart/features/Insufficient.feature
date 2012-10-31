Feature: Insufficient Balance
AS a member
I WANT to do partial transactions in rCredits and be told when that is not possible
SO I can use rCredits as much as possible

Summary:
  
#Variants:
#  | %TX_DONE     |
#  | %TX_DISPUTED |

#Variants: given/taken
#  | 00000 |
#  | 1     |

Setup:
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com | Ctown | Corse  | France        |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  | NEW.ZZB | codeB |
  | NEW.ZZC | codeC |
  And relations:
  | id      | main    | agent   | permission   |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA |              |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | tx_id    | created   | type         | state       | amount | from      | to      | purpose      | taking |
  | NEW.AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 000000 |
  | NEW.AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 000000 |
  | NEW.AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 000000 |
  | NEW.AAAE | %today-3w | %TX_TRANSFER | %TX_DONE    |    200 | NEW.ZZA   | NEW.ZZB | whatever E   | 000000 |
  | NEW.AAAF | %today-3w | %TX_REBATE   | %TX_DONE    |     10 | community | NEW.ZZA | rebate on #2 | 000000 |
  | NEW.AAAG | %today-3w | %TX_BONUS    | %TX_DONE    |     20 | community | NEW.ZZB | bonus on #2  | 000000 |
  | NEW.AAAH | %today-3d | %TX_TRANSFER | %TX_DONE    |    100 | NEW.ZZC   | NEW.ZZA | labor H      | 000000 |
  | NEW.AAAI | %today-3d | %TX_REBATE   | %TX_DONE    |      5 | community | NEW.ZZC | rebate on #2 | 000000 |
  | NEW.AAAJ | %today-3d | %TX_BONUS    | %TX_DONE    |     10 | community | NEW.ZZA | bonus on #3  | 000000 |
  | NEW.AAAK | %today-2d | %TX_TRANSFER | %TX_DONE    |    100 | NEW.ZZA   | NEW.ZZB | cash I       | 000000 |
  Then balances:
  | id        | balance |
  | community |    -795 |
  | NEW.ZZA   |      70 |
  | NEW.ZZB   |     570 |
  | NEW.ZZC   |     155 |

#Variants: with/without an agent
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # member to member (pro se) |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # agent to member           |
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # member to agent           |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # agent to agent            |

Scenario: A member asks to undo a completed payment, with insufficient balance
  When member "NEW.ZZC" asks device "codeC" to undo transaction "NEW.AAAH", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom | other_name | purpose |
  | %today-3d | $100   | to     | Abe One    | labor H |
  And balances:
  | id        | balance |
  | community |    -795 |
  | NEW.ZZA   |      70 |
  | NEW.ZZC   |     155 |

Scenario: A member confirms request to undo a completed payment, with insufficient balance
  When member "NEW.ZZC" asks device "codeC" to undo transaction "NEW.AAAH", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAL" my_balance 525 other_balance 12.5 and message "report undo|report invoice" with subs:
  | action  | other_name | amount | balance | tid |
  | charged | Abe One    | $100   | $525    | 3   |
  And we email "new-invoice" to member "a@example.com" with subs:
  | created | full_name  | other_name | amount | payee_purpose |
  | %today  | Abe One    | Corner Pub | $100   | reverses #3   |
  And balances:
  | id        | balance |
  | community |    -795 |
  | NEW.ZZA   |      70 |
  | NEW.ZZC   |     155 |

Scenario: A member asks to undo a completed payment unilaterally, with insufficient balance
  Given member "NEW.ZZA" can charge unilaterally
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAB", with the request "unconfirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 12.5 other_balance "" and message "report short transaction" with subs:
  | action | other_name | amount | short | balance | tid |
  | paid   | Corner Pub | $250   | $50   | $12.50  | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member confirms request to undo a completed payment unilaterally, with insufficient balance
  Given member "NEW.ZZA" can charge unilaterally
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAB", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 12.5 other_balance "" and message "report short transaction" with subs:
  | action | other_name | amount | short | balance | tid |
  | paid   | Corner Pub | $250   | $50   | $12.50  | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member asks to undo a completed charge, with insufficient balance
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "unconfirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 12.5 other_balance "" and message "report short transaction" with subs:
  | action | other_name | amount | short | balance | tid |
  | paid   | Corner Pub | $250   | $50   | $12.50  | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member confirms request to undo a completed charge, with insufficient balance
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 12.5 other_balance "" and message "report short transaction" with subs:
  | action | other_name | amount | short | balance | tid |
  | paid   | Corner Pub | $250   | $50   | $12.50  | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |