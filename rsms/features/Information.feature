Feature: Get information about account balance and system health
AS a player
I WANT information about my balance and the health of the system
SO I can make sound decisions about how much to pay and receive using the rCredits system

Setup:
  Given members:
  | id   | fullName   | number   | flags    |*
  | .ZZA | Abe One    | %number1 | ok,bona  |
  
Scenario: Getting information
  Given phone %number1 has r$200 including rewards of $2.50
  And the total demand for rCredits is $26,987.56
  When phone %number1 says "information"
  Then we say to phone %number1 "account info" with subs:
  | quid    | balance | rewards | totalDemand  |
  | NEW.ZZA | $200    | $2.50   | $26,987.56   |
  # "Your balance is $200, including $2.50 rewards. The current demand for rCredits is $26,900."
  # Leave the cents off balance and rewards rather than adding ".00". 
  # Always round the demand to an integer. Round the demand down to three significant digits and use "million" once it passes a million.

Scenario: Getting information, larger amounts
  Given phone %number1 has r$100,000.01 including rewards of $99,908.00
  And the total demand for rCredits is $1,226,987.25
  When phone %number1 says "information"
  Then we say to phone %number1 "account info" with subs:
  | quid    | balance     | rewards  | totalDemand   |
  | NEW.ZZA | $100,000.01 | $99,908  | $1.22 million |
  # "Your balance is $100,000.01, including $99,908 not yet available. The current demand for rCredits is $1.22 million."
