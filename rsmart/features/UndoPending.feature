Feature: Undo Pending Transaction
AS a member
I WANT to undo a proposed transaction to or from my account
SO I can resolve a confict with another member or correct a mistake

Summary:

Variants:
  | %TX_PENDING | %TX_DONE     |
  | %TX_DENIED  | %TX_DISPUTED |

Setup:
  Given members:
  | id      | fullName  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@ | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@ | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@ | Ctown | Corse  | France        |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  | NEW.ZZB | codeB |
  | NEW.ZZC | codeC |
  And relations:
  | id      | main    | agent   | permission   |
  | NEW.ZZA | NEW.ZZA | NEW.ZZB | buy and sell |
  | NEW.ZZB | NEW.ZZB | NEW.ZZA |              |
  | NEW.ZZC | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW.ZZD | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | xid      | created   | type         | state       | amount | from      | to      | purpose      | taking |
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

Scenario: A member asks to refuse to pay invoice
  When member "NEW.ZZC" asks device "codeC" to undo transaction "NEW.AAAH", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | otherName | purpose |
  | %today-2w | $100   | to      | Abe One    | labor H |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |
  
Scenario: A member confirms request to refuse to pay invoice
  When member "NEW.ZZC" asks device "codeC" to undo transaction "NEW.AAAH", with the request "confirmed"
  Then we respond success 1 tx_id "" my_balance "$250" other_balance "" and message "report undo" with subs:
  | solution          |
  | marked ''denied'' |
  And we email "invoice-denied" to member "a@" with subs:
  | created    | fullName | otherName | amount | payeePurpose |
  | %today-2w  | Abe One  | Corner Pub | $100   | labor H       |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |

Scenario: A member asks to refuse payment offer
  When member "NEW.ZZB" asks device "codeB" to undo transaction "NEW.AAAE", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | otherName | purpose |
  | %today-3w | $100   | from    | Abe One    | pie E   |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |
  
Scenario: A member confirms request to refuse payment offer
  When member "NEW.ZZB" asks device "codeB" to undo transaction "NEW.AAAE", with the request "confirmed"
  Then we respond success 1 tx_id "" my_balance "$250" other_balance "" and message "report undo" with subs:
  | solution          |
  | marked ''denied'' |
  And we email "offer-refused" to member "a@" with subs:
  | created   | fullName | otherName | amount | payerPurpose |
  | %today-3w | Abe One  | Bea Two    | $100   | pie E         |
  And balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |

Scenario: A member asks to cancel an invoice
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAH", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom | otherName | purpose |
  | %today-2w | $100   | from   | Corner Pub | labor H |

Scenario: A member confirms request to cancel an invoice
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAH", with the request "confirmed"
  Then we respond success 1 tx_id "" my_balance "$250" other_balance "" and message "report undo" with subs:
  | solution |
  | deleted  |
  And we email "invoice-canceled" to member "c@" with subs:
  | created   | fullName   | otherName | amount | payerPurpose |
  | %today-2w | Corner Pub | Abe One    | $100   | labor H       |
  
Scenario: A member asks to cancel an offer
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom | otherName | purpose |
  | %today-3w | $100   | to     | Bea Two    | pie E |

Scenario: A member confirms request to cancel an offer
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAE", with the request "confirmed"
  Then we respond success 1 tx_id "" my_balance "$250" other_balance "" and message "report undo" with subs:
  | solution |
  | deleted  |
  And we email "offer-canceled" to member "b@" with subs:
  | created   | fullName | otherName | amount | payeePurpose |
  | %today-3w | Bea Two  | Abe One    | $100   | pie E       |