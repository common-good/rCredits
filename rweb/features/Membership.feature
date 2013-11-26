Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#You can receive emails at any non-@dwolla.com email address.  That means you can make new accounts without me needing to forward the confirmation emails to you.
#Phones are automatically verified, so you don’t need to call VerifyPhone anymore.  You can cause the Verify Phone stage to fail by setting the phone number to “5551234567” 
#You can fail the SSN now with specific “always fail” SSNs:
#777777777 - will put user in Kba if personal account (if used for commercial account on AuthorizedRep call, will put them in PhotoId since there is no Kba for business acts)
#888888888 - will put user in PhotoId
#999999999 - will put user in AccountInfo

Setup:
  Given members:
  | id   | fullName   | phone    | address | city  | state | postalCode | country | floor | flags         |
  | .ZZA | Abe One    | +2000001 | 1 A St. | Atown | AK    | 01000 | US      |     0 | dft,person,company |
  | .ZZB | Bea Two    | +2000002 | 2 B St. | Btown | UT    | 02000 | US      |  -200 | dft,person,member  |
  | .ZZC | Corner Pub | +2000003 | 3 C St. | Ctown | Cher  | A1B23 | France  |     0 | dft,company          |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZC | .ZZA  | manage     |
  
Scenario: A member clicks on the membership link
#  Given member ".ZZA" supplies "postalAddr": "planet Earth"
  When member ".ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Contact Info |
  | 3 | Contribution |
  | 4 | Choose two people |
  | 5 | Preferences |
  | 6 | Photo |
  | 7 | Bank Account |
  And with done ""

Scenario: A company agent clicks on the membership link
#  Given member ".ZZC" supplies "postalAddr": "planet Earth"
  When member ":ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Contact Info |
  | 3 | Contribution |
  | 4 | Preferences |
  | 5 | Photo |
  | 6 | Bank Account |
  | _ | optional |
  | 7 | Company Info |
  | 8 | Relations |
  And without:
  | Choose two people |
  And with done ""

Scenario: A member does it all
#  Given member ".ZZA" supplies "postalAddr": "planet Earth"
  And member ".ZZA" has done step "agreement"
  When member ".ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "1"

  When member ".ZZA" has done step "contribution"
  And member ".ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "13"

  When member ".ZZA" has done step "photo"
  And member ".ZZA" visits page "status"
  Then with done "136"

  When member ".ZZA" has done step "contact"
  And member ".ZZA" visits page "status"
  Then with done "1236"

  When member ".ZZA" has done step "proxies"
  And member ".ZZA" visits page "status"
  Then with done "12346"
#  And members:
#  | id   | floor                               |
#  | .ZZA | %(%R_SIGNUP_BONUS - %R_SIGNUP_GIFT) |
  # card and letter sent to new member 
  # mentioning how to spend their $5 with the card?

  When member ".ZZA" has done step "preferences"
  And member ".ZZA" visits page "status"
  Then with done "123456"

  When member ".ZZA" has done step "connect"
  And member ".ZZA" visits page "status"
  Then we show "Your Account Setup Is Complete"
  And with done ""
  And we tell staff "event" with subs:
  | fullName | quid | status |
  | Abe One  | .ZZA | member |

  When member ".ZZA" has permission "ok"
  And member ".ZZA" visits page "status"
  Then we show "Your account is Activated"
  And with done ""
