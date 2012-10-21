Feature: Transact
AS a member
I WANT to transfer rCredits to or from another member
SO I can buy and sell stuff.

Setup:
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com | Ctown | Corse  | France        |
  And relations:
  | id      | main    | agent   | permission   |
  | NEW:ZZA | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW:ZZB | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | created   | type       | amount | from      | to      | purpose | taking |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZA | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZB | signup  | 0      |
  | %today-6m | %TX_SIGNUP | 250    | community | NEW.ZZC | signup  | 0      |

Scenario: 





Transact (differences from Normal Startup are highlighted)
Request: 
my_id (account ID of agent -- defaults to owner_id)
code (permanent code received in First Time response)
op=”transact”
type (“charge” or “pay”)

account_id (value of scanned QR code)

amount (numeric dollar amount)
goods=TRUE or FALSE (true unless user checks “cash, loan, etc.”)
purpose (description of goods and services)
Response: 
success=TRUE or FALSE
message (error message or success message)
tx_id (transaction ID number, if success, otherwise empty string)
my_balance (user’s new balance)
other_balance (new balance for the other party -- do not show the “Show Customer Balance” button if this is omitted)

Undo - needing confirmation (differences from Normal Startup are highlighted)
Request: 
my_id (account ID of agent -- defaults to owner_id)
code (permanent code received in First Time response)

op=”undo”
tx_id (ID number of transaction to undo)

confirmed=FALSE
Response: 
success=TRUE or FALSE
message (error message or message requesting confirmation)

Undo - definitive (differences from Normal Startup are highlighted)
Request: 
my_id (account ID of agent -- defaults to owner_id)
code (permanent code received in First Time response)
op=”undo”
confirmed=TRUE
 tx_id (ID number of transaction to undo)
Response: 
success=TRUE or FALSE
message (error message or success message)
tx_id (ID number of offsetting transaction, if any (which could in turn be undone). tx_id not set means transaction was simply deleted, so there is no longer any transaction that can be undone.)
my_balance (user’s new balance)
other_balance (new balance for the other party -- do not show the “Show Customer Balance” button if this is omitted)

