Feature: A user signs in to their rCredits account
AS a member
I WANT to sign in to my rCredits account
SO I can view or change settings, view or handle past transactions, and/or pay or charge another account

Setup:
  Given member is logged out

Scenario: A member signs in for the first time
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | postalCode | state | city  | acctType | code        |
  | Abe One | a@example.com | (413) 253-0000 | US | 01001 | MA | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then members:
  | id      | fullName | email         | phone        | country | postalCode | state | city    | flags | 
  | NEW.AAC | Abe One  | a@example.com | +14132530000 | US | 01001       | MA    | Amherst | dft,personal |
  And member "NEW.AAC" one-time password is set
  Given member "NEW.AAC" one-time password is "thingy"
  When member "?" visits page "/user/login"
  Then we show "Sign In" with:
  | oldpass      | pass1        | pass2                |
  | Old password | New password | Confirm new password |
  When member "?" confirms form "/user/login" with values:
  | name   | pass   | pass1  | pass2  |
  | abeone | thingy | Aa1!.. | Aa1!.. |
  Then we show "Account Summary"
  And we say "status": "take a step"