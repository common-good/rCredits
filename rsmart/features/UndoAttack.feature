Feature: Undo Attack
AS member
I WANT to be thoroughly protected from technological glitches and malicious attacks
SO I can have confidence in the security of my credit

Summary: 
  Post-setup variants for unconfirmed and confirmed)
  
  Device gives no transaction id
  Device gives bad transaction id
  Device gives nonexistent transaction id
  Device gives no confirmation status
  Device gives bad confirmation status
  Member asks to undo someone else's transaction
  Buyer agent lacks permission to reverse sale
  Seller agent lacks permission to reverse purchase

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
  | id      | main    | agent   | permission        |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell      |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA | read transactions |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy and sell      |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell              |
  And transactions: 
  | tx_id    | created   | type         | state       | amount | from      | to      | purpose      | taking |
  | NEW:AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 0      |
  | NEW:AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 0      |
  | NEW:AAAD | %today-2d | %TX_TRANSFER | %TX_DONE    |      5 | NEW.ZZB   | NEW.ZZC | cash given   | 0      |
  | NEW:AAAE | %today-1d | %TX_TRANSFER | %TX_DONE    |     80 | NEW.ZZA   | NEW.ZZC | whatever54   | 1      |
  Then balances:
  | id        | balance |
  | community |    -500 |
  | NEW.ZZA   |     170 |
  | NEW.ZZB   |     245 |
  | NEW.ZZC   |      85 |

Variants:
  | unconfirmed |
  | confirmed   |

Variants:
  | "NEW.ZZA" asks device "codeA" | (pro se) |
  | "NEW.ZZB" asks device "codeA" | agent    |

Scenario: Device gives no transaction id
  When member "NEW.ZZA" asks device "codeA" to undo transaction "", with the request "unconfirmed"
  Then we respond with:
  | success | message                |
  | 0       | missing transaction id |
  
Scenario: Device gives bad transaction id
  When member "NEW.ZZA" asks device "codeA" to undo transaction %random, with the request "unconfirmed"
  Then we respond with:
  | success | message            |
  | 0       | bad transaction id |

Scenario: Device gives nonexistent transaction id
  When member " NEW.ZZA" asks device "codeA" to undo transaction "NEW:AAAZ", with the request "confirmed"
  #no variant on first member because (1) balance should be shown when confirmed and (3) showing balance requires B_MANAGE 
  Then we respond with:
  | success | message       | my_balance |
  | 0       | undo no match | $170       |

Scenario: Device gives no confirmation status
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW:AAAB", with the request ""
  Then we respond with:
  | success | message                 |
  | 0       | bad confirmation status |

Scenario: Device gives bad confirmation status
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW:AAAB", with the request %random
  Then we respond with:
  | success | message                 |
  | 0       | bad confirmation status |

Scenario: Member asks to undo someone else's transaction
  When member " NEW.ZZA" asks device "codeA" to undo transaction "NEW:AAAC", with the request "confirmed"
  #no variant on first member because (1) balance should be shown when confirmed and (3) showing balance requires B_MANAGE 
  Then we respond with:
  | success | message       | my_balance |
  | 0       | undo no match | $170       |
  
Scenario: Buyer agent lacks permission to reverse sale
  When member "NEW.ZZA" asks device "codeC" to undo transaction "NEW:AAAE", with the request "unconfirmed"
  Then we respond with:
  | success | message         |
  | 0       | no buy          |

Scenario: Seller agent lacks permission to reverse purchase
  When member "NEW.ZZA" asks device "codeB" to undo transaction "NEW:AAAD", with the request "unconfirmed"
  Then we respond with:
  | success | message |
  | 0       | no sell |