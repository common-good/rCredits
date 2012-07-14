Feature: Transaction pre-launch
AS a player
I WANT to simulate a transfer of rCredits from my account to someone else's or vice versa
SO I will get a rebate or bonus that I can spend once I am an active participant

Scenario: The caller asks to pay a member id
  Given players:
  | @name      | @memberid | @cell    |
  | Abe One    | neabcdea  | %number1 |
  | Bea Two    | neabcdeb  | %number2 |
  | Corner Pub | neabcdec  | %number3 |
  When phone %number1 says "123.45 to neabcdeb for pie"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type | @amount | @who    | @for |
  | Pay   | 123.45  | Bea Two | pie  |
  # "Pay Bea Two $123.45 for goods and services? Type MANGO to confirm."

Scenario: The caller asks to pay a player by name
# or charge. Nickname matches the full name of a local business, organization, or individual account
# note the required dot (.) before the nickname.
  Given players:
  | @name      | @memberid | @cell    |
  | Abe One    | neabcdea  | %number1 |
  | Bea Two    | neabcdeb  | %number2 |
  | Corner Pub | neabcdec  | %number3 |
# And phone %number1 chosen nicknames do not include "cornerpub"
  When phone %number1 says "123.45 to .cornerpub for groceries"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type | @amount | @who       | @whatfor           |
  | Pay   | 123.45  | Corner Pub | goods and services |
  # "Pay Corner Pub $123.45 for goods and services? Type MANGO to confirm."

Scenario: The caller asks to charge a member id
  Given players:
  | @name      | @memberid | @cell    |
  | Abe One    | neabcdea  | %number1 |
  | Bea Two    | neabcdeb  | %number2 |
  | Corner Pub | neabcdec  | %number3 |
  When phone %number1 says "123.45 from neabcdec for labor"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type  | @amount | @who       | @for  |
  | Charge | 123.45  | Corner Pub | labor |
  # "Charge Corner Pub $123.45 for goods and services? Type MANGO to confirm."

Scenario: The caller confirms a payment
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdea | signup reward |
  When phone %number1 confirms "100 to neabcdec for groceries"
  Then the community has r$-265
  And phone %number3 has r$110
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for         |
  | %today | payment | 100     | neabcdea | neabcdec | groceries    |
  | %today | rebate  | 5       | community| neabcdea | rebate on #2 |
  | %today | bonus   | 10      | community| neabcdec | bonus on #1  |
  And we say to phone %number1 "report transaction" with subs:
  | @type | @tofrom | @amount | @who       | @for      | @reward_type | @reward_amount | @balance | @transaction |
  | paid  | to      | 100     | Corner Pub | groceries | rebate       | 5              | 155      | 2            |
  # "You paid Corner Pub $100 (rebate: $5). Your new balance is $155. Transaction #2"
  
Scenario: The caller confirms a charge
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdec | signup reward |
  When phone %number1 confirms "100 from neabcdec for labor"
  Then the community has r$-265
  And phone %number3 has r$155
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for         |
  | %today | charge  | 100     | neabcdec | neabcdea | labor        |
  | %today | rebate  | 5       | community| neabcdec | rebate on #2 |
  | %today | bonus   | 10      | community| neabcdea | bonus on #1  |
  And we say to phone %number1 "report transaction" with subs:
  | @type   | @tofrom | @amount | @who       | @for  | @reward_type | @reward_amount | @balance | @transaction |
  | charged | from    | 100     | Corner Pub | labor | bonus        | 10             | 110      | 1            |
  # "You charged Corner Pub $100 (bonus: $10). Your new balance is $110. Transaction #1"

Scenario: The caller confirms a payment with insufficient balance
  Given players:
  | @name      | @memberid | @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 100.00  | community| neabcdea | signup reward |
  When phone %number1 confirms "250 to neabcdec for groceries"
  Then the community has r$-115
  And phone %number3 has r$110
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for         |
  | %today | payment | 100     | neabcdea | neabcdec | groceries    |
  | %today | rebate  | 5       | community| neabcdea | rebate on #2 |
  | %today | bonus   | 10      | community| neabcdec | bonus on #1  |
  And we say to phone %number1 "report transaction short" with subs:
  | @type | @tofrom | @amount | @who       | @for      | @reward_type | @reward_amount | @balance | @transaction |
  | paid  | to      | 100     | Corner Pub | groceries | rebate       | 5              | 155      | 2            |
  # "SPLIT TRANSACTION! You paid Corner Pub $100 (rebate: $5). You will need to use US Dollars for the remainder. Your new balance is $5. Transaction #2"
  