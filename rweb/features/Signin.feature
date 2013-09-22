Feature: A user signs in to their rCredits account
AS a member
I WANT to sign in to my rCredits account
SO I can view or change settings, view or handle past transactions, and/or pay or charge another account

Setup:
  Given member is logged out

Scenario: A member signs in for the first time
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3&dwok=1" with values:
  | fullName | email | phone | country | postalCode | federalId | dob      | acctType     | code |
  | Abe One  | a@ | 413-253-0000 | US  | 01002    | 123-45-6789 | 1/2/1993 | %R_PERSONAL  | c0D3 |
  Then members:
  | id      | fullName | email   | country | postalCode | state | city    | flags        | 
  | NEW.AAC | Abe One  | a@      | US      | 01002      | MA    | Amherst | dft,person    |
  And member "NEW.AAC" one-time password is set
  Given member "NEW.AAC" one-time password is %whatever
  When member "?" visits page "/user/login"
  Then we show "Welcome" with:
  | Tmp password |
  | New password |
  | Confirm new password |
  | security code |
  When member "?" confirms form "/user/login" with values:
  | name   | pass      | pass1  | pass2  | pin  |
  | abeone | %whatever | Aa1!.. | Aa1!.. | 1234 |
  Then we show "Account Summary"
  And member "NEW.AAC" has a dwolla account, step "Email"
  And we say "status": "take a step"
  
Scenario: A member gives the wrong password
  Given members:
  | id      | fullName   | acctType    | flags           | pass       |
  | NEW.ZZA | Abe One    | %R_PERSONAL | dft,ok,person    | %whatever1 |
  And member "NEW.ZZA" one-time password is %whatever2
  When member "?" visits page "/user/login"
  And member "?" confirms form "/user/login" with values:
  | name   | pass    | pass1  | pass2  | pin  |
  | abeone | %random | Aa1!.. | Aa1!.. | 1234 |
  And we say "error": "wrong pass"
