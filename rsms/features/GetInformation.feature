Feature: Get information about account balance and system health
AS a player
I WANT information about my balance and the health of the system
SO I can make sound decisions about how much to pay and receive using the rCredits system

Scenario: Getting information
# Leave the cents off balance and unavailable rather than adding ".00". 
# Always round the demand to an integer. Round the demand down to three significant digits and use "million" once it passes a million.
  Given phone %number1 is a player
  And phone %number1 has r$100
  And phone %number1 unavailable is $7.50
  And the total demand for rCredits is $26,987.56
  When phone %number1 says "information"
  Then we say to phone %number1 "account info" with subs:
  | @balance | @unavailable | @demand    |
  | $100     | $7.50        | $26,987.56 |
  # "Your balance is $100, including $7.50 not yet available. The current demand for rCredits is $26,900."

Scenario: Getting information, larger amounts
  Given phone %number1 is a player
  And phone %number1 has r$100,000.01
  And phone %number1 unavailable is $99,908.00
  And the total demand for rCredits is $1,226,987.25
  When phone %number1 says "information"
  Then we say to phone %number1 "account info" with subs:
  | @balance    | @unavailable | @demand       |
  | $100,000.01 | $99,908      | $1.22 million |
  # "Your balance is $100,000.01, including $99,908 not yet available. The current demand for rCredits is $1.22 million."
