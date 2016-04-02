Feature: A user signs in to their rCredits account
AS a member
I WANT to sign in to my rCredits account
SO I can view or change settings, view or handle past transactions, and/or pay or charge another account

Setup:
  Given members:
  | id   | fullName | pass | email |*
  | .ZZA | Abe One  | a1   | a@    |
  And member is logged out

Scenario: A member visits the member site
  When member "?" visits page "signin"
  Then we show "Welcome to rCredits" with:
  | Account ID | account ID, email, or username |
  | Password   | Password problems? |
  |_promo      | Not yet a member? |

Scenario: A member signs in with username on the member site
  When member "?" confirms form "signin" with values:
  | name   | pass |*
  | abeone | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with account ID on the member site
  When member "?" confirms form "signin" with values:
  | name    | pass |*
  | new.zza | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with email on the member site
  When member "?" confirms form "signin" with values:
  | name          | pass |*
  | a@example.com | a1   |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member types the wrong password
  When member "?" confirms form "signin" with values:
  | name   | pass |*
  | abeone | a2   |
  Then we say "error": "bad login"

Scenario: A member types an unknown username/ID
  When member "?" confirms form "signin" with values:
  | name  | pass |*
  | bogus | a1   |
  Then we say "error": "bad login"

#.........................................................
Skip (remote signin is no longer allowed)
Scenario: A member signs in with username from rCredits.org
  When a member posts to "signinx" with values:
  | id     | pw |*
  | abeone | a1 |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with account ID from rCredits.org
  When a member posts to "signinx" with values:
  | id      | pw |*
  | new.zza | a1 |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member signs in with email from rCredits.org
  When a member posts to "signinx" with values:
  | id            | pw |*
  | a@example.com | a1 |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

Scenario: A member types the wrong password from rCredits.org
  When a member posts to "signinx" with values:
  | id     | pw |*
  | abeone | a2 |
  Then we show "Miscellaneous"
  And we say "error": "bad login"

Scenario: A member types an unknown username/ID from rCredits.org
  When a member posts to "signinx" with values:
  | id    | pw |*
  | bogus | a1 |
  Then we show "Miscellaneous"
  And we say "error": "bad login"
#.........................................................
Resume
Scenario: A member asks for a new password for username
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name   |*
  | abeone |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |
  
Scenario: A member asks for a new password for account ID
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name    |*
  | new.zza |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |
  
Scenario: A member asks for a new password for email
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  Then we email "password-reset" to member "a@example.com" with subs:
  | fullName | site        | name   | code     |*
  | Abe One  | %BASE_URL | abeone | wHatEveR |

Scenario: A member asks for a new password for an unknown account
  When member "?" completes form "settings/password" with values:
  | name  |*
  | bogus |
  Then we say "error": "bad account id"
  
Scenario: A member asks for a new password for a company
  Given members:
  | id   | fullName | pass | email | flags |*
  | .ZZC | Our Pub  | c1   | c@    | co    |
  When member "?" completes form "settings/password" with values:
  | name    |*
  | new.zzc |
  Then we say "error": "no co pass" with subs:
  | company |*
  | Our Pub |
  
#.........................................................

Scenario: A member clicks a link to reset password
  Given next random code is "wHatEveR"
  When member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  And member "?" visits page "reset/id=abeone&code=wHatEveR"
  Then we show "Choose a New Password"

  When member "?" confirms form "reset/id=abeone&code=wHatEveR" with values:
  | pass1      | pass2      | strong |*
  | %whatever  | %whatever  | 1      |
  Then member ".ZZA" is logged in
  And we show "Account Summary"

  Given member is logged out
  When member "?" confirms form "signin" with values:
  | name   | pass      |*
  | abeone | %whatever |
  Then we show "Account Summary"
  And member ".ZZA" is logged in

Scenario: A member clicks a link to reset password with wrong code
  Given next random code is "wHatEveR"
  And member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  When member "?" visits page "reset/id=abeone&code=NOTwHatEveR"
  Then we say "error": "bad login"
  And we show "Miscellaneous"

Scenario: A member clicks a link to reset password for unknown account
  Given next random code is "wHatEveR"
  And member "?" completes form "settings/password" with values:
  | name          |*
  | a@example.com |
  When member "?" visits page "reset/id=abeone&code=NOTwHatEveR"
  Then we say "error": "bad login"
  And we show "Miscellaneous"