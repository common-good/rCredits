Feature: Undo Completed Transaction
AS a player
I WANT to undo a transaction recently completed on my account
SO I can easily correct a mistake

Summary:
  A member asks to undo a completed payment
  A member confirms request to undo a completed payment
  A member asks to undo a completed charge
  A member confirms request to undo a completed charge
  A member asks to undo a completed cash payment
  A member confirms request to undo a completed cash payment
  A member asks to undo a completed cash charge
  A member confirms request to undo a completed cash charge
  A member confirms request to undo a completed payment unilaterally
  A member asks to undo a completed payment, with insufficient balance
  A member confirms request to undo a completed payment, with insufficient balance
  A member asks to undo a completed charge unilaterally, with insufficient balance  
  A member confirms request to undo a completed charge unilaterally, with insufficient balance  
  
#Variants:
#  | %TX_DONE     |
#  | %TX_DISPUTED |

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
  | NEW.AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 0      |
  | NEW.AAAE | %today-2m | %TX_TRANSFER | %TX_DONE    |     10 | NEW.ZZB   | NEW.ZZA | cash taken   | 1      |
  | NEW.AAAF | %today-3w | %TX_TRANSFER | %TX_DONE    |     20 | NEW.ZZC   | NEW.ZZA | usd taken    | 1      |
  | NEW.AAAG | %today-3d | %TX_TRANSFER | %TX_DONE    |     40 | NEW.ZZA   | NEW.ZZB | whatever43   | 0      |
  | NEW.AAAH | %today-3d | %TX_REBATE   | %TX_DONE    |      2 | community | NEW.ZZA | rebate on #4 | 0      |
  | NEW.AAAI | %today-3d | %TX_BONUS    | %TX_DONE    |      4 | community | NEW.ZZB | bonus on #3  | 0      |
  | NEW.AAAJ | %today-2d | %TX_TRANSFER | %TX_DONE    |      5 | NEW.ZZB   | NEW.ZZC | cash given   | 0      |
  | NEW.AAAK | %today-1d | %TX_TRANSFER | %TX_DONE    |     80 | NEW.ZZA   | NEW.ZZC | whatever54   | 0      |
  | NEW.AAAL | %today-1d | %TX_REBATE   | %TX_DONE    |      4 | community | NEW.ZZA | rebate on #5 | 0      |
  | NEW.AAAM | %today-1d | %TX_BONUS    | %TX_DONE    |      8 | community | NEW.ZZC | bonus on #4  | 0      |
  Then balances:
  | id        | balance |
  | community |    -768 |
  | NEW.ZZA   |     166 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     323 |

#Variants:
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # member to member (pro se) |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # agent to member           |
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # member to agent           |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # agent to agent            |

Scenario: A member asks to undo a completed payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAK", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-1d | $80    | to      | Corner Pub | whatever54 |
  # "Undo 01-02-2012 payment of $80 to Corner Pub for whatever?"
  And balances:
  | id        | balance |
  | community |    -768 |
  | NEW.ZZA   |     166 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     323 |
#Request:
#op = ”undo”
#tx_id (ID number of transaction to undo)
#confirmed = 0
#Response: 
#success = 0 or 1
#message (error message or message requesting confirmation)

Scenario: A member confirms request to undo a completed payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAK", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAN" my_balance 166 other_balance "" and message "report undo|report invoice" with subs:
  | solution | action  | other_name | amount | tid |
  | reversed | charged | Corner Pub | $80    | 6   |
  # "Undo 01-02-2012 payment of $55.55 to Corner Pub for whatever?"
  And we email "new-invoice" to member "c@example.com" with subs:
  | created | full_name  | other_name | amount | payer_purpose |
  | %today  | Corner Pub | Abe One    | $80    | reverses #4   |
  And balances:
  | id        | balance |
  | community |    -768 |
  | NEW.ZZA   |     166 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     323 |
#Request: 
#op = ”undo”
#confirmed = 1
#tx_id (ID number of transaction to undo)
#Response: 
#success = 0 or 1
#message (error message or success message)
#tx_id (ID number of offsetting transaction, if any (which could in turn be undone). tx_id not set means transaction was simply deleted, so there is no longer any transaction that can be undone.)
#my_balance (user’s new balance)
#other_balance (new balance for the other party -- do not show the “Show Customer Balance” button if this is omitted)

Scenario: A member asks to undo a completed charge
Scenario: A member confirms request to undo a completed charge
Scenario: A member asks to undo a completed cash payment
Scenario: A member confirms request to undo a completed cash payment
Scenario: A member asks to undo a completed cash charge
Scenario: A member confirms request to undo a completed cash charge
  
Scenario: A member confirms request to undo a completed payment unilaterally
  Given member "NEW.ZZA" can charge unilaterally
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAK", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAN" my_balance 242 other_balance 235 and message "report undo|report transaction" with subs:
  | solution | action  | other_name | amount | reward_type | reward_amount | balance | tid |
  | reversed | charged | Corner Pub | $80    | bonus       | $-4           | $242    | 6   |
  # "Undo 01-02-2012 payment of $55.55 to Corner Pub for whatever?"
  And we email "new-charge" to member "c@example.com" with subs:
  | created | full_name  | other_name | amount | payer_purpose |
  | %today  | Corner Pub | Abe One    | $80    | reverses #4   |
  And balances:
  | id        | balance |
  | community |    -756 |
  | NEW.ZZA   |     242 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     235 |
  
Scenario: A member asks to undo a completed payment, with insufficient balance
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAB", with the request "unconfirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 525 other_balance 12.5 and message "report short transaction" with subs:
  | action  | other_name | amount | short | balance | tid |
  | charged | Abe One    | $250   | $50   | $525    | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member confirms request to undo a completed payment, with insufficient balance
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAB", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 525 other_balance 12.5 and message "report short transaction" with subs:
  | action  | other_name | amount | short | balance | tid |
  | charged | Abe One    | $250   | $50   | $525    | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member asks to undo a completed charge unilaterally, with insufficient balance
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

Scenario: A member confirms request to undo a completed charge unilaterally, with insufficient balance
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
