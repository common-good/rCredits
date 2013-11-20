Feature: Identify QR
AS an individual or company member
I WANT to identify another member's QR code
SO I can safely pay or charge them.

AND

I WANT to see the server's picture of a potential customer
SO I can verify the customer's identity visually

Setup:
  Given members:
  | id      | fullName  | phone  | email         | city  | state  | country       |
  | NEW.ZZA | Abe One    | +20001 | a@ | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@ | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@ | Ctown | Corse  | France        |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  And relations:
  | id      | main    | agent   | permissions  |
  | NEW.ZZA | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW.ZZB | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | created   | type       | amount | from      | to      | purpose | taking |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZA | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZB | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZC | signup  | 0      |

Scenario: Member asks us to identify a QR
# Actually the member asks to pay or charge. That process begins with identification of the other party's QR.
  When member "NEW.ZZA" asks device "codeA" to identify QR "NEW.ZZB" 
  Then we respond with:
  | success | fullName | location  |
  | 1       | Bea Two   | Btown, UT |
#op="identify"
#other_balance (current balance for the identified person)

Scenario: Member asks us to identify the member's own QR
  When member "NEW.ZZA" asks device "codeA" to identify QR "NEW.ZZA" 
  Then we respond with:
  | success | message         |
  | 0       | no self-trading |

Scenario: Member asks us to identify a foreign QR
  When member "NEW.ZZA" asks device "codeA" to identify QR "NEW.ZZC" 
  Then we respond with:
  | success | fullName  | location             |
  | 1       | Corner Pub | Ctown, Corse, France |
  
Scenario: Member asks us to identify a QR and member can show balances
  Given member "NEW.ZZA" can charge unilaterally
  When member "NEW.ZZA" asks device "codeA" to identify QR "NEW.ZZB" 
  Then we respond with:
  | success | fullName | location  | other_balance |
  | 1       | Bea Two   | Btown, UT | 250           |

Scenario: Member asks us to identify a QR for a company agent
  When member "NEW.ZZA" asks device "codeA" to identify QR "NEW.ZZA"
  Then we respond with:
  | success | fullName | location             | company_name |
  | 1       | Bea Two   | Ctown, Corse, France | Corner Pub   |

Scenario: Device asks for a picture to go with the QR
  Given member "NEW.ZZB" has picture %picture1
  When member "NEW.ZZA" asks device "codeA" for a picture of member "NEW.ZZB"
  Then we respond to member "NEW.ZZA" with picture %picture1
#op="photo"
Response: just the picture file (Content-Type will probably be image/png, rather than application/whatever)

Scenario: Device asks for a picture but there isn't one
  Given member "NEW.ZZB" has no picture
  When member "NEW.ZZA" asks device "codeA" for a picture of member "NEW.ZZB"
  Then we respond to member "NEW.ZZA" with picture "no-photo-available"

