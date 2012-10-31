Feature: Offer to exchange US Dollars for rCredits
AS a member
I WANT more rCredits than I have
SO I can spend more rCredits or store them

Scenario: Make or update an offer
  Given phone %number1 is a member
  And phone %number1 demand for rCredits is $100
  And the total demand for rCredits is $100,000
  When phone %number1 says "get r 123.45"
  Then phone %number1 demand for rCredits is $123.45
  And the total demand for rCredits is $100,023.45
  And we say to phone %number1 "your demand" with subs:
  | request |
  | $123.45 |
  # "Your total request for rCredits is now $123.45"
