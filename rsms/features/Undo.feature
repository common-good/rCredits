Feature: Undo
AS a member
I WANT to undo a transaction recently completed on my account
SO I can easily correct a mistake

Setup:
  Given members:
  | id   | full_name  | cell   | email         |
  | .ZZA | Abe One    | +20001 | a@example.com |
  | .ZZB | Bea Two    | +20002 | b@example.com |
  | .ZZC | Corner Pub | +20003 | c@example.com |
  And transactions: 
  | created   | type         | amount | from      | to   | purpose      | taking |
  | %today-6m | %TX_SIGNUP   | 250    | community | .ZZA | signup       | 0      |
  | %today-6m | %TX_SIGNUP   | 250    | community | .ZZB | signup       | 0      |
  | %today-6m | %TX_SIGNUP   | 250    | community | .ZZC | signup       | 0      |
  | %today-4m | %TX_TRANSFER | 11.11  | .ZZB      | .ZZA | cash for me  | 1      |
  | %today-3w | %TX_TRANSFER | 22.22  | .ZZC      | .ZZA | usd          | 1      |
  | %today-3d | %TX_TRANSFER | 33.33  | .ZZA      | .ZZB | whatever43   | 0      |
  | %today-3d | %TX_REBATE   | 1.67   | community | .ZZA | rebate on #4 | 0      |
  | %today-3d | %TX_BONUS    | 3.33   | community | .ZZB | bonus on #3  | 0      |
  | %today-2d | %TX_TRANSFER | 44.44  | .ZZB      | .ZZC | cash         | 0      |
  | %today-1d | %TX_TRANSFER | 55.55  | .ZZA      | .ZZC | whatever43   | 0      |
  | %today-1d | %TX_REBATE   | 2.78   | community | .ZZA | rebate on #5 | 0      |
  | %today-1d | %TX_BONUS    | 5.56   | community | .ZZC | bonus on #4  | 0      |
  Then the community has r$-763.34
  And phone +20001 has r$198.90
  And phone +20002 has r$231.11
  And phone +20003 has r$333.33

Scenario: Undo the last transaction
  When phone +20001 says "undo"
  Then we say to phone +20001 "confirm undo|please confirm" with subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-1d | $55.55 | to      | Corner Pub | whatever54 |
  # "Undo 01-02-2012 payment of $55.55 to Corner Pub for whatever?"

Scenario: Undo the last transaction with someone specific
  When phone +20001 says "undo .ZZB"
  Then we say to phone +20001 "confirm undo|please confirm" with subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-3d | $33.33 | to      | Bea Two    | whatever43 |
  # "Undo 05-17-2012 payment of 33.33 to Bea Two?"

Scenario: Undo the last transfer to me
  When phone +20001 says "undo from"
  Then we say to phone +20001 "confirm undo|please confirm" with subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-3w | $22.22 | from    | Corner Pub | usd        |
  # "Undo 01-03-2012 charge of 22.22 from Corner Pub?"

Scenario: Undo the last transfer to me from someone specific
  When phone +20001 says "undo from .ZZB"
  Then we say to phone +20001 "confirm undo|please confirm" with subs:
  | created   | amount | tofrom | other_name | purpose     |
  | %today-4m | $11.11 | from   | Bea Two    | cash for me |
  # "Undo 01-03-2012 charge of 33.33 to Bea?"

Scenario: The caller confirms undoing a charge
  When phone +20001 confirms "undo from .ZZB"
  Then the community has r$-763.34
  And phone +20002 has r$242.22
  And we say to phone +20001 "report undo|report exchange" with subs:
  | solution | action | other_name | amount | balance | tid |
  | reversed | gave   | Bea Two    | $11.11 | $187.79 | 6   |
  # "You gave Corner Pub $100 cash/loan/etc. Your new balance is $150. Transaction #2"

Scenario: The caller confirms undoing a payment
  When phone +20001 confirms "undo to .ZZB"
  And we say to phone +20001 "report undo|report invoice" with subs:
  | solution | action  | other_name | amount | tid |
  | reversed | charged | Bea Two    | $33.33 | 6   |
  # "You gave Corner Pub $100 cash/loan/etc. Your new balance is $150. Transaction #2"

Scenario: The caller refuses to pay the latest invoice
  Given transactions:
  | created   | state       | amount | from | to   | purpose  | taking |
  | %today    | %TX_PENDING | 100    | .ZZA | .ZZB | cleaning | 1      |
  When phone +20001 confirms "undo"
  Then we say to phone +20001 "report undo" with subs:
  | solution          |
  | marked ''denied'' |
  And we email "invoice-denied" to member "b@example.com" with subs:
  | created | full_name | other_name | amount | payee_purpose  |
  | %today  | Bea Two   | Abe One    | $100   | cleaning       |

Scenario: The caller refuses a pending payment
  Given transactions:
  | created   | state       | amount | from | to   | purpose | taking |
  | %today    | %TX_PENDING | 100    | .ZZC | .ZZA | wages   | 0      |
  When phone +20001 confirms "undo from .ZZC"
  Then we say to phone +20001 "report undo" with subs:
  | solution          |
  | marked ''denied'' |
  And we email "payment-denied" to member "c@example.com" with subs:
  | created | full_name  | other_name | amount | payer_purpose |
  | %today  | Corner Pub | Abe One    | $100   | wages         |
