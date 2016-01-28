Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | id  | fullName | phone | email | city  | state | postalCode | floor | flags  | pass      |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000      |     0 |        | %whatever |
  | .ZZB | Bea Two |     2 | b@    | Btown | UT    | 02000      |  -200 | member | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000      |     0 | co     | |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZC | .ZZA  | manage     |

Scenario: A member signs in for the first time
  Given member is logged out
  And invitation to email "d@" from member ".ZZA" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone | country | postalCode | federalId | dob | acctType    | code | address | city    | state | tenure | owns | postalAddr                |*
  | Dee Four  | d@ | 413-253-0000 | US | 01002    | 123-45-6789 | 1/2/1993 | %R_PERSONAL | c0D3 | 1 A St. | Amherst | MA    |     25 |    0 | 1 A St., Amherst, MA 01002 |
  Then members:
  | id   | fullName | email   | country | postalCode | state | city    | flags     | tenure | risks | helper |*
  | .AAA | Dee Four | d@      | US      | 01002      | MA    | Amherst | confirmed |     25 | rents | .ZZA   |
  And we say "status": "your account is ready"
  And we email "welcome" to member "d@" with subs:
  | fullName | name    | quid    | site        | code  |*
  | Dee Four | deefour | NEW.AAA | %BASE_URL | %name |

  When member "?" visits page "reset/id=deefour&code=%name"
  Then we show "Choose a New Password"
  When member "?" confirms form "reset/id=deefour&code=%name" with values:
  | pass1     | pass2     | strong |*
  | %whatever | %whatever | 1      |
#  | name    | pass      | pass1  | pass2  | pin  |
#  | deefour | %whatever | Aa1!.. | Aa1!.. | 1234 |
  Then member ".AAA" is logged in
  And we show "Account Summary"
  And we say "status": "take a step"

Scenario: A member clicks the membership link
  When member ".ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Choose two people |
  | 4 | Preferences |
  | 5 | Photo |
  | 6 | Connect |
  And with done ""

Scenario: A company agent clicks on the membership link
  When member ":ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Preferences |
  | 4 | Photo |
  | 5 | Connect |
  | 6 | Company Info |
  | 7 | Relations |
  And without:
  || Choose two people |
  And with done ""

Scenario: A member does it all
  Given members have:
  | id   | federalId   | dob      |*
  | .ZZA | 001-01-0001 | 1/1/1990 |
  And member ".ZZA" has done step "contact"
  # temporary starting 5/8/2014 while some newbies catch up
  When member ".ZZA" visits page "status"
  Then we show "Membership Steps"
  And with done ""

  When member ".ZZA" has done step "sign"
  And member ".ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "1"
  
  When member ".ZZA" has done step "donate"
  And member ".ZZA" visits page "status"
  Then with done "12"

  When member ".ZZA" has done step "photo"
  And member ".ZZA" visits page "status"
  Then with done "125"

  When member ".ZZA" has done step "proxies"
  And member ".ZZA" visits page "status"
  Then with done "1235"

  When member ".ZZA" has done step "prefs"
  And member ".ZZA" visits page "status"
  Then with done "12345"

  When member ".ZZA" has done step "connect"
  Then we tell staff "event" with subs:
  | fullName | quid | status |
  | Abe One  | .ZZA | member |
  When member ".ZZA" visits page "summary"
  Then we say "status": "setup complete"
#  And we say "status": "adjust settings"

  When member ".ZZA" visits page "status"
  Then we show "Your Account Setup Is Complete"
  And with done ""
  
  When member ".ZZA" has permission "ok"
  And member ".ZZA" visits page "status"
  Then we show "Your account is Activated"
  And with done ""

Scenario: A member opens a business account
  Given account ".ZZC" was set up by member ".ZZA"
  And members have:
  | id   | federalId   | dob      |*
  | .ZZA | 001-01-0001 | 1/1/1990 |
  | .ZZC | 01-0000001  |          |
  And member ".ZZC" has done step "contact"
  When member ":ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Preferences |
  | 4 | Photo |
  | 5 | Connect |
  | 6 | Company Info |
  | 7 | Relations |
  And with done ""

  When member ".ZZC" has done step "sign"
  And member ":ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "1"

  When member ".ZZC" has done step "donate"
  And member ":ZZA" visits page "status"
  Then with done "12"

  When member ".ZZC" has done step "photo"
  And member ":ZZA" visits page "status"
  Then with done "124"

  When member ".ZZC" has done step "prefs"
  And member ":ZZA" visits page "status"
  Then with done "1234"
  
  When member ".ZZC" has done step "connect"
  And member ":ZZA" visits page "status"
  Then with done "12345"

  When member ".ZZC" has done step "company"
  And member ":ZZA" visits page "status"
  Then with done "123456"
  
  When member ":ZZA" visits page "settings/relations"
  And member ":ZZA" visits page "status"
  Then we show "Your Account Setup Is Complete"
  And with done ""
  And we tell staff "event" with subs:
  | fullName | quid | status |*
  | Our Pub  | .ZZC | member |

  When member ".ZZC" has permission "ok"
  And member ":ZZA" visits page "status"
  Then we show "Your account is Activated"
  And with done ""