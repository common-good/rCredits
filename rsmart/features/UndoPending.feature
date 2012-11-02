Feature: Undo Pending Transaction
AS a member
I WANT to undo a proposed transaction to or from my account
SO I can resolve a confict with another member or correct a mistake

Summary:

#Variants:
#  | %TX_PENDING |
#  | %TX_REFUSED |

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
  | NEW.AAAE | %today-3w | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZA   | NEW.ZZB | pie E        | 0      |
  | NEW.AAAF | %today-3w | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZC | rebate on #2 | 0      |
  | NEW.AAAG | %today-3w | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus on #2  | 0      |
  | NEW.AAAH | %today-2w | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZC   | NEW.ZZA | labor H      | 1      |
  | NEW.AAAI | %today-2w | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZC | rebate on #2 | 0      |
  | NEW.AAAJ | %today-2w | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus on #3  | 0      |
  | NEW.AAAK | %today-3d | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZA   | NEW.ZZB | cash given   | 0      |
  | NEW.AAAL | %today-2d | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZB   | NEW.ZZA | cash request | 1      |
  Then balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |

Scenario: A member asks to refuse payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | other_name | purpose |
  | %today-3w | $100   | to      | Bea Two    | pie E   |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |
  
Scenario: A member confirms request to refuse payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "confirmed"
  Then we respond success 1 tx_id "NEW.AAAM" my_balance 235 other_balance "" and message "report undo" with subs:
  | solution | action | other_name | amount | tid | reward_type | reward_amount | balance |
  | reversed | paid   | Abe One    | $80    | 5   | rebate      | $-8           | $235    |
  And we email "new-payment" to member "a@example.com" with subs:
  | created | full_name | other_name | amount | payee_purpose |
  | %today  | Abe One   | Corner Pub | $80    | reverses #5   |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |
  