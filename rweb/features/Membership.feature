Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

Setup:
  Given members:
  | id   | fullName   | phone    | address | city  | state | postalCode | country | floor | flags         |
  | .ZZA | Abe One    | +2000001 | POB 1 | Atown | AK    | 01000 | US      |     0 | dft,personal,company |
  | .ZZB | Bea Two    | +2000002 | POB 2 | Btown | UT    | 02000 | US      |  -200 | dft,personal,member  |
  | .ZZC | Corner Pub | +2000003 | POB 3 | Ctown | Cher  | A1B23 | France  |     0 | dft,company          |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZC | .ZZA  | manage     |
  
Scenario: A member has incomplete contact information
  When member ".ZZA" visits page "membership"
  Then we show "Missing Contact Information"

Scenario: A member clicks on the membership link
  Given member ".ZZA" supplies "physical": "planet Earth"
  When member ".ZZA" visits page "membership"
  Then we show "Congratulations on signing up" with:
  | Step 1 | Step 2    | Step 3       | Step 4            | Step 5 | Step 6   | Step 7      |
  | photo  | Agreement | contribution | Choose two people | Dwolla | identity | Preferences |
  And with done ""

Scenario: A company agent clicks on the membership link
  Given member ".ZZC" supplies "physical": "planet Earth"
  When member ":ZZA" visits page "membership"
  Then we show "Congratulations on signing up" with:
  | Step 1 | Step 2    | Step 3       | Step 4 | Step 5      |
  | photo  | Agreement | contribution | Dwolla | Preferences |
  And we show "Congratulations on signing up" without:
  | Step 4            |
  | Choose two people |
  And with done ""

Scenario: A member does it all
  Given member ".ZZA" supplies "physical": "planet Earth"
  And member ".ZZA" has done step "agreement"
  When member ".ZZA" visits page "membership"
  Then we show "You're getting there"
  And with done "2"

  When member ".ZZA" has done step "contribution"
  And member ".ZZA" visits page "membership"
  Then with done "23"

  When member ".ZZA" has done step "photo"
  And member ".ZZA" visits page "membership"
  Then with done "123"

  When member ".ZZA" has done step "proxies"
  And member ".ZZA" visits page "membership"
  Then with done "1234"
  And we tell staff "event" with subs:
  | fullName | quid | status |
  | Abe One  | .ZZA | member |
#  And members:
#  | id   | floor                               |
#  | .ZZA | %(%R_SIGNUP_BONUS - %R_SIGNUP_GIFT) |
  # card and letter sent to new member 
  # mentioning how to spend their $5 with the card?

  When member ".ZZA" has done step "dwolla"
  And member ".ZZA" visits page "membership"
  Then with done "12345"

  When member ".ZZA" has done step "proof"
  And member ".ZZA" visits page "membership"
  Then we show "You're getting there" with:
  | note             |
  | pending approval |
  And with done "123456"

  When member ".ZZA" has done step "preferences"
  And member ".ZZA" visits page "membership"
  Then we show "Your Account Setup Is Complete"
  And with done ""
  And we tell staff "event" with subs:
  | fullName | quid | status |
  | Abe One  | .ZZA | ready  |

  When member ".ZZA" has permission "ok"
  And member ".ZZA" visits page "membership"
  Then we show "Your account is Activated" without:
  | note             |
  | pending approval |
  
#  And we show "You have completed all membership steps" without:
#  | content |
#  | preferences |
