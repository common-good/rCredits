Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | id  | fullName | phone | email | city  | state | postalCode | floor | flags   | pass |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000      |     0 | dw      | %whatever |
  | .ZZB | Bea Two |     2 | b@    | Btown | UT    | 02000      |  -200 | member  | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000      |     0 | co,dw   | |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZC | .ZZA  | manage     |

Scenario: A member signs in for the first time
  Given member is logged out
  And invitation to email "d@" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3&dwok=1" with values:
  | fullName  | email | phone | dupOk | country | postalCode | federalId | dob | acctType    | code | address | city       | state | postalAddr                |*
  | Dee Four  | d@ | 413-253-0000 | 0 |US  | 01002    | 123-45-6789 | 1/2/1993 | %R_PERSONAL | c0D3 | 1 A St. | Amherst | MA    | 1 A St., Amherst, MA 01002 |
  Then members:
  | id      | fullName | email   | country | postalCode | state | city    | flags | *
  | NEW.AAC | Dee Four | d@      | US      | 01002      | MA    | Amherst | dw |
  And we say "status": "your account is ready"
  And we email "welcome" to member "d@" with subs:
  | fullName | name    | quid    | site        | code  |*
  | Dee Four | deefour | NEW.AAC | %R_SITE_URL | %name |

  When member "?" visits page "reset/id=deefour&code=%name"
  Then we show "Choose a New Password"
  When member "?" confirms form "reset/id=deefour&code=%name" with values:
  | newPass   | strong |*
  | %whatever | 1      |
#  | name    | pass      | pass1  | pass2  | pin  |
#  | deefour | %whatever | Aa1!.. | Aa1!.. | 1234 |
  Then member ".AAC" is logged in
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
  | 6 | Bank Account |
  And with done ""

Scenario: A member without a bank account clicks the membership link
  When member ".ZZB" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Choose two people |
  | 4 | Preferences |
  | 5 | Photo |
  And with done ""

Scenario: A company agent clicks on the membership link
  When member ":ZZA" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Preferences |
  | 4 | Photo |
  | 5 | Company Info |
  | 6 | Relations |
  And without:
  || Bank Account |
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
  And member ".ZZA" visits page "status"
  Then with done "12345"
  And we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Donation |
  | 3 | Choose two people |
  | 4 | Preferences |
  | 5 | Photo |
  | 6 | Verify |  
  
  Given member ".ZZA" has done step "dw"
  # dw is temporary until we get free of Dwolla
  When member ".ZZA" has done step "verify"

  Then we tell staff "event" with subs:
  | fullName | quid | status |
  | Abe One  | .ZZA | member |
  When member ".ZZA" visits page "summary"
  Then we say "status": "setup complete"
  And we say "status": "adjust settings"

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
  | 5 | Company |  
  | 6 | Relations |
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
  
  When member ".ZZC" has done step "company"
  And member ":ZZA" visits page "status"
  Then with done "12345"
  
  Given member ".ZZC" has done step "dw"
  When member ":ZZA" visits page "account/relations"
  # dw is temporary until Dwolla's Reg API works
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

Scenario: A member types the wrong account info (name, ssn, or dob)
  Given members have:
  | id   | federalId | dob      |*
  | .ZZA | 999999999 | 1/1/1991 |

  Given members have:
  | id   | address | postalAddr        | state |*
  | .ZZA | 1 A St. | 1 A St., AK 01000 | AK    |

  When member ".ZZA" visits page "status"
  # Dwolla bug (unexpected error on Address step completion) requires extra visit to status page
  And member ".ZZA" visits page "status"
  Then we show "Retry Verification"
  And we say "error": "redo info"
  
  When member ".ZZA" confirms form "account/basic" with values:
  | first | last | federalId   | dob      |*
  | Abe   | One  | 001-01-0001 | 1/1/1990 |
  Then we show "You're getting there"
  And with done "2"

Scenario: A member company types the wrong account info (name, ein, or business structure)
  Given members have:
  | id   | federalId |*
  | .ZZC | 999999999 |
  # Dwolla's magic ssn for account info failure
  And member ".ZZC" chose to verify phone by "Voice"
  # When member ".ZZC" visits page "status"
  # Then member ".ZZC" is on step "Phone" within 30 seconds

  # When member ".ZZC" visits page "status"
  # Then we show "Verify Phone"
  When member ".ZZC" confirms form "account/verify-phone" with values:
  | code  |*
  | 99999 |
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        | state |*
  | .ZZC | 3 C St. | 3 C St., CA 03000 | CA    |

  When member ".ZZC" visits page "status"
  # Dwolla bug (unexpected error on Address step completion) requires extra visit to status page
  And member ".ZZC" visits page "status"
  Then we show "Retry Verification"
  And we say "error": "redo info"
  
  When member ".ZZC" confirms form "account/basic" with values:
  | org          | federalId   | acctType       |*
  | Corner Store | 001-01-0001 | CO_PARTNERSHIP |
  Then we show "You're getting there"
  And with done "2"

Scenario: A member has to submit a photo ID
  Given members have:
  | id   | federalId | dob      | state |*
  | .ZZA | 888888888 | 1/1/1991 | AK    |
  # Dwolla's magic ssn for account info failure
  And member ".ZZA" chose to verify phone by "Voice"
  When member ".ZZA" visits page "status"
  Then member ".ZZA" is on step "Phone" within 30 seconds

  When member ".ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZA" confirms form "account/verify-phone" with values:
  | code  |*
  | 99999 |
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        |*
  | .ZZA | 1 A St. | 1 A St., AK 01000 |
  When member ".ZZA" visits page "status"
  Then we show "Photo ID Verification"
  #And we say "error": "could not verify"
  
  When member ".ZZA" confirms form "account/photo-id" with values:
  | idProof           |*
  | rcredits-test.jpg |
  Then we show "Contact Information"