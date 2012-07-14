Feature: Exchange for cash, pre-launch
AS a player
I WANT to simulate an exchange of rCredits from my account for someone's cash or vice versa
SO I can understand better how that will work once I am an Active Participant

Scenario: The caller confirms a trade of rCredits for cash
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdea | signup reward |
  When phone %number1 confirms "100 to neabcdec for cash"
  Then the community has r$-250
  And phone %number3 has r$100
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for |
  | %today | payment | 100     | neabcdea | neabcdec | cash |
  And we say to phone %number1 "report cash trade" with subs:
  | @type  | @amount | @who       | @way            | @balance | @transaction |
  | traded | 100     | Corner Pub | credit for cash | 150      | 2            |
  # "You traded Corner Pub $100 credit for cash. Your new balance is $150. Transaction #2"
  
Scenario: The caller confirms a trade of cash for rCredits
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdec | signup reward |
  When phone %number1 confirms "100 from neabcdec for cash"
  Then the community has r$-250
  And phone %number3 has r$150
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for |
  | %today | charge  | 100     | neabcdec | neabcdea | cash |
  And we say to phone %number1 "report cash trade" with subs:
  | @type  | @amount | @who       | @way            | @balance | @transaction |
  | traded | 100     | Corner Pub | cash for credit | 100      | 1            |
  # "You traded Corner Pub $100 cash for credit. Your new balance is $100. Transaction #1"

Scenario: The caller confirms an implicit trade of rCredits for cash
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdea | signup reward |
  When phone %number1 confirms "100 to neabcdec"
  Then the community has r$-250
  And phone %number3 has r$100
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for |
  | %today | payment | 100     | neabcdea | neabcdec | cash |
  And we say to phone %number1 "report cash trade" with subs:
  | @type | @amount | @who       | @way                                   | @balance | @transaction |
  | gave  | 100     | Corner Pub | credit (perhaps as a gift or for cash) | 150      | 2            |
  # "You gave Corner Pub $100 credit (perhaps as a gift or for cash). Your new balance is $150. Transaction #2"
  
Scenario: The caller confirms an implicit trade with insufficient balance
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 100.00  | community| neabcdea | signup reward |
  When phone %number1 confirms "250 to neabcdec"
  Then the community has r$-115
  And phone %number3 has r$110
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for |
  | %today | payment | 100     | neabcdea | neabcdec | cash |
  And we say to phone %number1 "partial | report cash trade" with subs:
  | @type | @amount | @who       | @way                                   | @balance | @transaction |
  | gave  | 100     | Corner Pub | credit (perhaps as a gift or for cash) | 0        | 2            |
  # "PARTIAL SUCCESS. You gave Corner Pub $100 credit (perhaps as a gift or for cash). Your new balance is $0. Transaction #2"
  