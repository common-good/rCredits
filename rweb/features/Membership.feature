Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

Setup:
  Given members:
  | id      | fullName   | phone    | address | city  | state | postalCode | country | floor | flags         |
  | NEW.ZZA | Abe One    | +2000001 | POB 1 | Atown | AK    | 01000 | US      |     0 | dft,personal,company |
  | NEW.ZZB | Bea Two    | +2000002 | POB 2 | Btown | UT    | 02000 | US      |  -200 | dft,personal,member  |
  | NEW.ZZC | Corner Pub | +2000003 | POB 3 | Ctown | Cher  |       | France  |     0 | dft,company          |
  And relations:
  | id      | main    | agent   | permission |
  | NEW:ZZA | NEW.ZZC | NEW.ZZA | manage     |
  
Scenario: A member clicks on the membership link
  When member "NEW.ZZA" visits page "membership"
  Then we show "Congratulations on signing up" with:
  | Step 1         | Step 2                      | Step 3              | Step 4            |
  | Upload a photo | Sign the rCredits Agreement | Make a contribution | Choose two people |
  And with done ""

Scenario: A company agent clicks on the membership link
  When member "NEW:ZZA" visits page "membership"
  Then we show "Congratulations on signing up" with:
  | Step 1         | Step 2                      | Step 3              |
  | Upload a photo | Sign the rCredits Agreement | Make a contribution |
  And we show "Congratulations on signing up" without:
  | Step 4            |
  | Choose two people |
  And with done ""

Scenario: A member does it all
  Given member "NEW.ZZA" has done step "agreement"
  When member "NEW.ZZA" visits page "membership"
  Then we show "Congratulations on signing up"
  And with done "2"
  When member "NEW.ZZA" has done step "contribution"
  And member "NEW.ZZA" visits page "membership"
  Then with done "23"
  When member "NEW.ZZA" has done step "photo"
  And member "NEW.ZZA" visits page "membership"
  Then with done "123"
  When member "NEW.ZZA" has done step "proxies"
  And member "NEW.ZZA" visits page "membership"
  Then we show "Missing Contact Information"
  When member "NEW.ZZA" supplies "physical": "planet Earth"
  And member "NEW.ZZA" visits page "membership"
  Then we show "You have completed the rCredits membership" with:
  | Step 1                | Step 2                | Step 3                       | Step 4                    |
  | Open a Dwolla account | your driver's license | Set your Account Preferences | Invite someone to sign up |
  And with done ""
  And we tell staff "event" with:
  | fullName | quid    | status |
  | Abe One  | NEW.ZZA | member |
  When member "NEW.ZZA" has done step "dwolla"
  And member "NEW.ZZA" visits page "membership"
  Then with done "1"
  When member "NEW.ZZA" has done step "preferences"
  And member "NEW.ZZA" visits page "membership"
  Then with done "13"
  When member "NEW.ZZA" has done step "proof"
  And member "NEW.ZZA" visits page "membership"
  Then we show "You have completed the rCredits membership" with:
  | note             |
  | pending approval |
  And with done "123"
  When member "NEW.ZZA" has done step "invitation"
  And member "NEW.ZZA" visits page "membership"
  Then we show "You have completed all membership steps" without:
  | content |
  | preferences |
  And we tell staff "event" with:
  | fullName | quid    | status |
  | Abe One  | NEW.ZZA | ready  |
  When member "NEW.ZZA" has permission "ok"
  And member "NEW.ZZA" visits page "membership"
  Then we show "Your account is Activated" without:
  | note             |
  | pending approval |
  