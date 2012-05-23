Feature: Open an account for the other party to a transaction
  AS a player
  I WANT to do an informational transaction with someone I sometimes pay or get paid by
  SO I will get a rebate or bonus

Scenario: The caller asks to pay an identifiable total newbie
# by phone number, email address, or domain
  Given phone %number1 is a player
  And phone %number2 has no account
  And phone %number2 can be identified as "Corner Pub"
  When phone %number1 says "123.00 to %number2 for groceries"
  Then we say to phone %number1 "confirm payment"
  | @name | @amount | @whatfor |
  | Corner Pub | $123 | groceries |
  # Pay Corner Pub $123 for groceries?
  And phone %number2 has an account

Scenario: The caller asks to pay a non-identifiable total newbie by phone number, email address, or domain
  Given phone %number1 is a player
  And phone %number2 has no account
  And phone %number2 cannot be identified
  When phone %number1 says "123.00 to %number2 for groceries"
  Then we say to phone %number1 "whose phone?"
  | @number |
  | %number2 |
  # Whose phone number is %number2? Please type the full name.

Scenario: The caller supplies a full name for the recipient, as requested
  Given we just asked for a full name for phone %number2
  And the original request was "123.00 to %number2 for groceries"
  And phone %number1 current balance is $200.
  And phone %number1 last transaction number is 7
  When caller says "Corner Pub"
  Then phone %number2 account name is "Corner Pub"
  And we say to phone %number1 "transaction report"
  | @type | @amount | @who | @bonus_type | @bonus_amount | @balance | @transaction_id |
  | INFO payment | $123 | Corner Pub | rebate | $6.15 | $206.15 | 8 |
  # "INFO payment: $123 to Corner Pub (rebate: $6.15). Your new balance is $206.15. Transaction #8"
