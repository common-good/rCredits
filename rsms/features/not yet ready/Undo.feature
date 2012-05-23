Feature: Undo
# the most recent transaction
AS a player
I WANT to undo a transaction recently completed on my account
SO I can easily correct a mistake

Scenario: Undoing a payment
  Given the last transaction for phone %nummber was 
  | type | amount | who | whatfor |
  | payment | 43.21 | neabcdef | whatever |
  And member "neabcdef" name is "Corner Pub"
  When phone %number1 says "undo"
  Then we say to phone %number1 "confirm undo"
  | type | amount | who | whatfor |
  | payment | 43.21 | Corner Pub | whatever |
  # "Undo INFO payment of 43.21 to Corner Pub?"

Scenario: Undo the most recent transaction with someone specific
  Given transactions: 
  | date      | type    | amount | from | to | whatfor   |
  | 11-22-2011 | charge  | 11.11  | 2    | 1  | whatever1 |
  | 01-03-2012 | charge  | 22.22  | 3    | 1  | whatever2 |
  | 01-05-2012 | payment | 33.33  | 1    | 2  | whatever3 |
  | 01-08-2012 | payment | 44.44  | 2    | 3  | whatever4 |
  And players:
  | uid | name       | memberid | cell     |
  | 1   | Abe       | neabcdea | %number1 |
  | 2   | Bea       | neabcdeb | %number2 |
  | 3   | Corner Pub | neabcdec | %number3 |
  When phone %number1 says "undo neabcdeb"
  Then we say to phone %number1 "confirm undo" with subs:
  | date      | type    | amount | who | whatfor   |
  | 01-05-2012 | payment | 33.33  | Bea | whatever3 |
  # "Undo 01-05-2012 INFO payment of 33.33 to Corner Pub?"

Scenario: Undo the most recent transfer of funds into my account
  Given transactions: 
  | date      | type    | amount | from | to | whatfor   |
  | 11-22-2011 | charge  | 11.11  | 2    | 1  | whatever1 |
  | 01-03-2012 | charge  | 22.22  | 3    | 1  | whatever2 |
  | 01-05-2012 | payment | 33.33  | 1    | 2  | whatever3 |
  | 01-08-2012 | payment | 44.44  | 2    | 3  | whatever4 |
  And players:
  | uid | name       | memberid | cell     |
  | 1   | Abe       | neabcdea | %number1 |
  | 2   | Bea       | neabcdeb | %number2 |
  | 3   | Corner Pub | neabcdec | %number3 |
  When phone %number1 says "undo from"
  Then we say to phone %number1 "confirm undo" with subs:
  | date      | type    | amount | who | whatfor   |
  | 01-03-2012 | charge  | 22.22  | Bea | whatever3 |
  # "Undo 01-03-2012 INFO charge of 22.22 from Bea?"

Scenario: Undo the most recent transfer of funds from my account to someone specific
  Given transactions: 
  | date      | type    | amount | from | to | whatfor   |
  | 11-22-2011 | charge  | 11.11  | 2    | 1  | whatever1 |
  | 01-03-2012 | charge  | 22.22  | 3    | 1  | whatever2 |
  | 01-05-2012 | payment | 33.33  | 1    | 2  | whatever3 |
  | 01-08-2012 | payment | 44.44  | 2    | 3  | whatever4 |
  And players:
  | uid | name       | memberid | cell     |
  | 1   | Abe       | neabcdea | %number1 |
  | 2   | Bea       | neabcdeb | %number2 |
  | 3   | Corner Pub | neabcdec | %number3 |
  When phone %number1 says "undo from"
  Then we say to phone %number1 "confirm undo" with subs:
  | date      | type    | amount | who | whatfor   |
  | 01-03-2012 | charge  | 22.22  | Bea | whatever3 |
  # "Undo 01-03-2012 INFO charge of 22.22 from Bea?"

Feature: Info transaction pre-launch
AS a player
I WANT to use cell texting to simulate a transfer of rCredits from my account to someone else's
SO I will get a rebate that I can spend once I am a participant

Scenario: The caller asks to pay a valid member id
# (or info pay)
  Given phone %number1 is a player
  And neabcdef is an account id for Corner Pub
  And phone %number1's rebate percentage is 5%
  And phone %number1 is a non-participant or neabcdef is a non-participant or phone %number1's current balance minus unavailable is greater than or equal to $123.45 divided by [one plus phone %number1's rebate percentage] ($117.14285)
the next random dictionary word to be used is "mango"
  When phone %number1 says "123.00 to neabcdef for groceries"
  Then ask phone %number1 to confirm: "Straw pay Corner Pub $123.45 for groceries? Type MANGO to confirm."
  And remember we are waiting for "mango" to confirm "123.00 to neabcdef for groceries"

Scenario: The caller asks to pay or charge (or straw pay or charge) a nickname that matches the full name of a local business, organization, or individual account
  Given phone %number1 is a player
  And "cornerpub" is not among phone %number1's chosen nicknames
  And "Corner Pub" is the full name on an account in phone %number1's zipcode or neighboring zipcode And that account id is neabcdef
  And phone %number1's rebate percentage is 5%
  And phone %number1 is a non-participant or neabcdef is a non-participant or phone %number1's current balance minus unavailable is greater than or equal to $123.45 divided by [one plus phone %number1's rebate percentage] ($117.14285)
the next random dictionary word to be used is "mango"
  When phone %number1 says "123.00 to .cornerpub for groceries"
  Then ask phone %number1 to confirm: "Straw pay Corner Pub $123.45 for groceries? Type MANGO to confirm."
  And remember we are waiting for "mango" to confirm "123.00 to neabcdef for groceries"

Scenario: The caller confirms a straw payment
  Given phone %number1 is a player
  And phone %number1's current balance is $200.
  And neabcdef is an account whose actual name is "Corner Pub"
  And phone %number1 is a non-participant
  And phone %number1 has 517 transactions so far
  And phone %number1's rebate percentage is 5%
  When phone %number1 confirms "123.00 to neabcdef for groceries"
  Then record the transaction as a straw transaction
  And transfer $6.15 (5% of $123) to phone %number1's account from the community account
  And increase phone %number1's unavailable by that much
  And say "STRAW payment: $123.45 to Corner Pub (rebate: $6.15). Your new balance is $206.15. Transaction ID #518"

Scenario: The caller asks to charge (or straw charge) a valid account id
  Given phone %number1 is a player
  And neabcdef is an account id for Corner Pub
  And phone %number1's bonus percentage is 10%
  And phone %number1 is a non-participant or neabcdef is a non-participant
the next random dictionary word to be used is "mango"
  When phone %number1 says "123.00 from neabcdef for labor"
  Then ask phone %number1 to confirm: "Straw charge Corner Pub $123.45 for labor?" Type MANGO to confirm."
  And remember we are waiting for "mango" to confirm "123.00 from neabcdef for labor"

Scenario: The caller confirms a straw charge
  Given phone %number1 is a player
  And phone %number1's current balance is $200.
  And neabcdef is an account whose actual name is "Corner Pub"
  And phone %number1 is a non-participant
  And phone %number1 has 517 transactions so far
  And phone %number1's bonus percentage is 10%
  When phone %number1 confirms "123.00 from neabcdef for labor"
  Then record the transaction as a straw transaction
  And transfer $12.30 (10% of $123) to phone %number1's account from the community account
  And increase phone %number1's unavailable by that much
  And say "STRAW charge: $123.45 from Corner Pub (bonus: $10.30). Your new balance is $210.30. Transaction ID #518"


FROM HERE ON BY KURT 
(for post-straw-phase)

Feature: Direct Payment
AS a participant
I WANT to use cell texting to transfer rCredits from my account to someone else's
SO they will give me goods and services and I will get a rebate

Scenario: (1) Direct payment participant to participant by SMS with balance sufficient
  Given participant payer uses SMS
  And receiver is a participant
  When payer sends valid receiver identifier with valid amount and description
  And amount less than his/her positive balance
  Then the amount is added to receiver balance
  And receiver receives applicable rebates and bonuses (this needs clarification)
  And receiver receives notification
  And the amount is subtracted from payer balance
  And payer receives applicable rebates and bonuses (this needs clarification)
  And payer receives confirmation

Scenario: (2) Direct payment participant to non-participant by SMS (straw transaction)
  Given participant payer uses SMS
  And receiver is a non-participant with a cell phone
  When payer sends valid receiver cell number with valid amount and description
  Then receiver receives credit for future applicable rebates And bonuses (this needs clarification)
  And receiver receives notification
  And payer receives credit for future applicable rebates and bonuses (this needs clarification)
  And payer receives confirmation and is advised to pay with external instrument

Scenario: (3) Direct payment participant to participant by SMS with after-balance insufficient but within negative limit
  Given participant payer uses SMS
  And receiver is a participant
  When payer sends valid receiver identifier with valid amount and description
  And amount less than his/her positive balance + negative balance limit
  Then the amount is added to receiver balance
  And receiver receives applicable rebates and bonuses (this needs clarification)
  And receiver receives notification
  And the amount is subtracted from payer balance
  And payer receives applicable rebates and bonuses (this needs clarification)
  And payer receives confirmation

Scenario: (4) Direct payment participant to participant by SMS with after-balance beyond negative balance limit
  Given participant payer uses SMS
  And receiver is a participant
  When payer sends valid receiver identifier with valid amount and description
  And amount greater than his/her positive balance + negative balance limit
  Then the system computes the available amount
  And the payer is asked whether a) to cancel or b) to partial pay the available amountand use another instrument for the remainder

Scenario: (5) Direct payment participant to participant by SMS with after-balance beyond negative balance limit and payer has responded in Scenario (4) with choice to "partial pay".
  Given participant payer has just responded with "partial pay" to the choice requested in scenario (4)
  When payer sends the choice of "partial pay"
  Then the available amount (from Scenario (4)) is added to receiver balance
  And receiver receives applicable rebates and bonuses (this needs clarification)
  And receiver receives notification
  And the available amount is subtracted from payer balance
  And payer receives applicable rebates and bonuses (this needs clarification)
  And payer receives confirmation

Scenario: (6) Direct payment participant to participant by SMS with after-balance beyond negative balance limit and payer has responded in Scenario (4) with choice to "cancel".
  Given participant payer has just responded with "cancel" to the choice requested inscenario (4)
  When payer sends the choice of "cancel"
  Then he/she receives a "canceled" message.
… more scenarios, e.g. Web-based

Note: I believe that scenario 1 can be dropped in favor of #3.