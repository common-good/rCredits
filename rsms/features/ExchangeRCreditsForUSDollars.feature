Feature: Exchange rCredits for US Dollars
AS a participant
I WANT to exchange some rCredits for US Dollars
SO I can pay where rCredits are not yet accepted

Scenario: The caller can get US Dollars for rCredits
#  Given phone %number1 is a participant
  Given phone %number1 is set up for direct deposits
  And phone %number1 has Pr$150
#  And phone %number1 unavailable is $20
#  And phone %number1 incentive rewards to date is $5
  When phone %number1 says "get usd 123.45"
  Then we say to phone %number1 "confirm get usd" with subs:
  | @amount |
  | $123.45 |
  # Give r$123.45 in exchange for us$123.45? Type "mango" to confirm".

Scenario: Caller confirms request for US Dollars
  Given phone %number1 is set up for direct deposits
  And phone %number1 has Pr$160
#  And phone %number1 unavailable is $20
#  And phone %number1 incentive rewards to date is $15
  And the community has Pr$-10,000
  And the community has USD$200
  When phone %number1 confirms "get usd 123.45"
  Then we email to admin "send USD" with subs:
  | @phone   | @amount |
  | %number1 | $123.45 |
  And the community has Pr$-9,876.55
  And the community has USD$76.55
  And we say to phone %number1 "report got usd" with subs:
  | @amount |
  | 123.45  |
  # us$123.45 is being transferred to your USD bank account.