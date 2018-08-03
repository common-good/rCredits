Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | id  | fullName | phone | email | city  | state | zip | floor | flags  | pass      |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000      |     0 |        | %whatever |
  | .ZZB | Bea Two |     2 | b@    | Btown | UT    | 02000      |  -200 | member | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000      |     0 | co     | |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |

Scenario: A member signs in for the first time
  Given member is logged out
  And invitation to email "d@" from member ".ZZA" is "c0D3"
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone | country | zip | federalId | dob | acctType    | code | address | city    | state | tenure | owns | postalAddr                |*
  | Dee Four  | d@ | 413-253-0000 | US | 01002    | 123-45-6789 | 1/2/1993 | %CO_PERSONAL | c0D3 | 1 A St. | Amherst | MA    |     25 |    0 | 1 A St., Amherst, MA 01002 |
  Then members:
  | id   | fullName | email   | country | zip | state | city    | flags     | tenure | risks | helper |*
  | .AAA | Dee Four | d@      | US      | 01002      | MA    | Amherst | confirmed |     25 | rents | .ZZA   |
  And we email "verify" to member "d@" with subs:
  | fullName | name    | quid    | site      | code      |*
  | Dee Four | deefour | NEW.AAA | %BASE_URL | WHATEVER |
  And member ".AAA" one-time password is set to "WHATEVER"
  And member ".AAA" is logged in
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "settings/verify" with values:
  | verify   | pass1      | pass2      | strong |*
  | WHATEVER | %whatever3 | %whatever3 |      1 |
  Then we show "Confirm Your Social Security Number"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "settings/ssn" with values:
  | federalId   | field     |*
  | 123-45-6789 | federalId |
  Then we show "%PROJECT Agreement"
  And we say "status": "info saved|step completed"
  
  When member ".AAA" completes form "community/agreement" with name "Dee Four" and all checkboxes
  Then we show "Donate"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "community/donate" with values:
  | gift | often | honor | honored | share |*
  |   50 |     M |     - |         |    25 |
  Then we show "Proxies"
  And we say "status": "info saved|step completed"
  
  Given proxies:
  | person | proxy | priority |*
  | .AAA   | .ZZA  |        1 |
  | .AAA   | .ZZB  |        1 |
  When member ".AAA" completes form "settings/proxies" with values:
  | op       |*
  | nextStep |
  Then we show "Account Preferences"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal |*
  |       1 |      2 | monthly | electronic |        0 |         1 |
  Then we show "Banking Settings"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "settings/connect" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  | submit |       1 |     053000196 |         123 |          123 |       0 |       0 |     $0 |    $20 |         $0 |  
  Then we show "Photo ID Picture"
  And we say "status": "info saved|step completed"

  When member ".AAA" completes form "settings/photo" with values:
  | op       |*
  | nextStep |
  Then we say "status": "setup complete|individual approval|join thanks"
  And we tell ".AAA" CO "New Member (Dee Four)" with subs:
  | fullName | quid | status |*
  | Dee Four | .AAA | member |

  When member ".AAA" visits page "summary"
  Then we show "Account Summary"
  And we say "status": "setup complete|individual approval|join thanks"
  
  When member ".ZZC" has permission "ok"
# then what?  