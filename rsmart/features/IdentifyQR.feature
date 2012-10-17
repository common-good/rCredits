Feature: Identify QR
AS an individual or company member
I WANT to identify another member's QR code
SO I can safely pay or charge them.

AND

I WANT to see the server's picture of a potential customer
SO I can verify the customer's identity visually

Setup:
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com | Ctown | Corse  | France        |
  And relations:
  | id      | main    | agent   | permissions  |
  | NEW:ZZA | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW:ZZB | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | created   | type       | amount | from      | to      | purpose | taking |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZA | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZB | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZC | signup  | 0      |

Scenario: Member asks us to identify a QR
# Actually the member asks to pay or charge. That process begins with identification of the other party's QR.
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When member "NEW.ZZA" asks device %whatever1 to identify QR "NEW.ZZB" 
  Then we respond with:
  | success | full_name | location    | company_name | message |
  | 1       | Bea Two   | Btown, Utah |              |         |
#op=”identify”
#other_balance (current balance for the identified person)

Scenario: Member asks us to identify the member's own QR
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When member "NEW.ZZA" asks device %whatever1 to identify QR "NEW.ZZA" 
  Then we respond with:
  | success | message         |
  | 0       | no self-trading |

Scenario: Member asks us to identify a foreign QR
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When member "NEW.ZZA" asks device %whatever1 to identify QR "NEW.ZZC" 
  Then we respond with:
  | success | full_name  | location             | company_name | message |
  | 1       | Corner Pub | Ctown, Corse, France |              |         |
  
Scenario: Member asks us to identify a QR and member can show balances
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  And member "NEW.ZZA" can show balances
  When member "NEW.ZZA" asks device %whatever1 to identify QR "NEW.ZZB" 
  Then we respond with:
  | success | full_name | location    | company_name | message | other_balance |
  | 1       | Bea Two   | Btown, Utah |              |         | $250          |

Scenario: Member asks us to identify a QR for a company agent
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When member "NEW.ZZA" asks device %whatever1 to identify QR "NEW:ZZA"
  Then we respond with:
  | success | full_name | location             | company_name | message |
  | 1       | Bea Two   | Ctown, Corse, France | Corner Pub   |         |

Scenario: Device asks for picture to go with QR
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When member "NEW.ZZA" asks device %whatever1 for a picture of member "NEW.ZZB"
  Then we respond with a picture of member "NEW.ZZB"
#op=”photo”
Response: (Content-Type will probably be image/png, rather than application/whatever)
just the picture file
