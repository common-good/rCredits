Feature: Open an account for the other party to a transaction
  AS a player
  I WANT to do an informational transaction with someone I sometimes pay or get paid by
  SO I will get a rebate or bonus

Scenario: The caller asks to pay an identifiable total newbie by name
  Given players:
  | @name     | @memberid | @cell    |
  | Abe One   | neabcdea  | %number1 | 
  And local phonelist:
  | @name      | @phone  |
  | Bob's Gas  | %number2 |
  | Corner Pub | %number3 |
  When phone %number1 says "123.00 to .cornerpub for groceries"
  Then phone %number3 has an account
  And we say to phone %number1 "confirm payment" with subs:
  | @name      | @amount | @for      |
  | Corner Pub | $123    | groceries |
  # Pay Corner Pub $123 for goods and services?

Scenario: The caller asks to pay a non-identifiable total newbie by phone
# or by email address or domain
  Given players:
  | @name     | @memberid | @cell    |
  | Abe One   | neabcdea  | %number1 |
  And phone %number2 has no account
  And phone %number2 cannot be identified
  When phone %number1 says "123.00 to %number2 for groceries"
  Then we say to phone %number1 "whose phone?" with subs:
  | @number  |
  | %number2 |
  # Whose phone number is %number2? Please type the full name.

Scenario: The caller supplies a full name for the recipient, as requested
  Given players:
  | @name   | @memberid | @cell    | @rebate |
  | Abe One | neabcdea  | %number1 | 5       |  
  | Bea Two | neabcdeb  | %number2 | 5       |
  | %random | neabcdec  | %number3 | 5       |
  And we just asked phone %number1 for a full name for phone %number3
  And phone %number1 requested "100 to %number3 for groceries"
  And transactions:
  | @date     | @type  | @amount | @from    | @to      | @for          |
  | %today-1d | signup | 250.00  | community| neabcdea | signup reward |
  When phone %number1 says "Corner Pub"
  Then phone %number3 account name is "Corner Pub"
  And we say to phone %number1 "report transaction" with subs:
  | @type | @tofrom | @amount | @who       | @for      | @reward_type | @reward_amount | @balance | @transaction |
  | paid  | to      | 100     | Corner Pub | groceries | rebate       | 5              | 155      | 2            |
  # "You paid Corner Pub $100 (rebate: $5). Your new balance is $155. Transaction #2"
