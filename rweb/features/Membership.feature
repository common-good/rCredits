Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#Special Dwolla magic:
#You can receive emails at any non-@dwolla.com email address.  That means you can make new accounts without me needing to forward the confirmation emails to you.
#Phones are automatically verified, so you don’t need to call VerifyPhone anymore.  You can cause the Verify Phone stage to fail by setting the phone number to 5551234567
#You can fail the SSN now with specific “always fail” SSNs:
#777777777 - will put user in Kba if personal account (if used for commercial account on AuthorizedRep call, will put them in PhotoId since there is no Kba for business acts)
#888888888 - will put user in PhotoId
#999999999 - will put user in AccountInfo

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | id  | fullName | phone | email | city  | state | postalCode | floor | flags              | pass |
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000      |     0 | dft,person,dw      | %whatever |
  | .ZZB | Bea Two |     2 | b@    | Btown | UT    | 02000      |  -200 | dft,person,member  | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000      |     0 | dft,company,dw     | |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZC | .ZZA  | manage     |

Scenario: A member signs in for the first time
  Given member is logged out
  And invitation to email "d@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3&dwok=1" with values:
  | legalName | email | phone | dupOk | country | postalCode | federalId | dob | acctType    | code | verifyBy |
  | Dee Four  | d@ | 413-253-0000 | 0 |US  | 01002    | 123-45-6789 | 1/2/1993 | %R_PERSONAL | c0D3 |        1 |
  Then members:
  | id      | fullName | email   | country | postalCode | state | city    | flags         | 
  | NEW.AAC | Dee Four | d@      | US      | 01002      | MA    | Amherst | dft,person,dw |
  And member "NEW.AAC" one-time password is set
  Given member "NEW.AAC" one-time password is %whatever
  When member "?" visits page "/user/login"
  Then we show "Welcome" with:
  | Tmp password |
  | New password |
  | Confirm new password |
  | security code |
  When member "?" confirms form "/user/login" with values:
  | name    | pass      | pass1  | pass2  | pin  |
  | deefour | %whatever | Aa1!.. | Aa1!.. | 1234 |
  Then we show "Account Summary"
  # (sometimes it goes to Phone too fast!) And member ".AAC" has a dwolla account, step "Email"
  And we say "status": "take a step"
  Skip (dwolla changed this Jan2014 so it doesn't work on their test server)
  And member ".AAC" is on step "Phone" within 10 seconds
  Resume

Scenario: A member gives the wrong password
  Given member "NEW.ZZA" one-time password is %whatever2
  When member "?" visits page "/user/login"
  And member "?" confirms form "/user/login" with values:
  | name   | pass    | pass1  | pass2  | pin  |
  | abeone | %random | Aa1!.. | Aa1!.. | 1234 |
  And we say "error": "wrong pass"

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
  
Scenario: A member without a Dwolla account clicks on the membership link
  When member ".ZZB" visits page "status"
  Then we show "Membership Steps" with:
  | 1 | Agreement |
  | 2 | Contact Info |
  | 3 | Contribution |
  | 4 | Choose two people |
  | 5 | Preferences |
  | 6 | Photo |
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
  Given members have:
  | id   | federalId   | dob      |
  | .ZZA | 001-01-0001 | 1/1/1990 |
  When member ".ZZA" visits page "status"
  Then we show "Membership Steps"
  And with done ""
  # (sometimes it goes to Phone too fast!) And member ".ZZA" has a dwolla account, step "Email"
  And member ".ZZA" is on step "Phone" within 10 seconds

  When member ".ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZA" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
# Then member ".ZZA" has a dwolla account, step "Address" (NOPE, might already be on to step Ssn)
  Then we show "Contact Information"

  When member ".ZZA" confirms form "account/contact" with values:
  | legalName | email | phone | address | state | country | postalCode | verifyBy | postalAddr  | faxetc    |
  | Abe One   | a@    |     1 | 1 A St. | AK    | US      | 01002      |        1 | 1 A St., AK | %whatever |
#  When member ".ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "2"

  When member ".ZZA" has done step "agreement"
  And member ".ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "12"
  
  When member ".ZZA" has done step "contribution"
  And member ".ZZA" visits page "status"
  Then with done "123"

  When member ".ZZA" has done step "photo"
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

Scenario: A member opens a business account
  Given account ".ZZC" was set up by member ".ZZA"
  And members have:
  | id   | federalId   | dob      |
  | .ZZA | 001-01-0001 | 1/1/1990 |
  | .ZZC | 01-0000001  |          |
  When member ":ZZA" visits page "status"
  Then we show "Membership Steps"
  And with done ""
  # (sometimes it goes to Phone too fast!) And member ".ZZC" has a dwolla account, step "Email"
  And member ".ZZC" is on step "Phone" within 10 seconds

  When member ":ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ":ZZA" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
# Then member ".ZZA" has a dwolla account, step "Address" (NOPE, might already be on to step Ssn)
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        | state |
  | .ZZA | 1 A St. | 1 A St., AK 01000 | AK    |  
  When member ":ZZA" confirms form "account/contact" with values:
  | fullName | email | phone | address | state | country | postalCode | verifyBy | postalAddr  | faxetc    |
  | Our Pub  | c@    |     3 | 3 C St. | CA    | US      | 01003      |        1 | 3 C St., CA | %whatever |
#  When member ":ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "2"

  When member ".ZZC" has done step "agreement"
  And member ":ZZA" visits page "status"
  Then we show "You're getting there"
  And with done "12"
  
  When member ".ZZC" has done step "contribution"
  And member ":ZZA" visits page "status"
  Then with done "123"

  When member ".ZZC" has done step "photo"
  And member ":ZZA" visits page "status"
  Then with done "1235"

  When member ".ZZC" has done step "preferences"
  And member ":ZZA" visits page "status"
  Then with done "12345"

  When member ".ZZC" has done step "connect"
  And member ":ZZA" visits page "status"
  Then with done "123456"
  
  When member ".ZZC" has done step "company"
  And member ":ZZA" visits page "status"
  Then with done "1234567"
  
  When member ":ZZA" visits page "account/relations"
  And member ":ZZA" visits page "status"
  Then we show "Your Account Setup Is Complete"
  And with done ""
  And we tell staff "event" with subs:
  | fullName | quid | status |
  | Our Pub  | .ZZC | member |

  When member ".ZZC" has permission "ok"
  And member ":ZZA" visits page "status"
  Then we show "Your account is Activated"
  And with done ""
Skip
Scenario: A member types the wrong phone verification code
  Given members have:
  | id   | federalId   | dob      | phone        |
  | .ZZA | 001-01-0001 | 1/1/1990 | +15551234567 |
  # Dwolla's magic phone number for phone verification failure
  And member ".ZZA" chose to verify phone by "Voice"
  When member ".ZZA" visits page "status"
  Then member ".ZZA" is on step "Phone" within 30 seconds

  When member ".ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZA" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
  Then we show "Verify Phone"
  And we say "error": "bad nonce"
Resume
Scenario: A member types the wrong account info (name, ssn, or dob)
  Given members have:
  | id   | federalId | dob      |
  | .ZZA | 999999999 | 1/1/1991 |
  # Dwolla's magic ssn for account info failure
  And member ".ZZA" chose to verify phone by "Voice"
  When member ".ZZA" visits page "status"
  Then member ".ZZA" is on step "Phone" within 30 seconds

  When member ".ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZA" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        | state |
  | .ZZA | 1 A St. | 1 A St., AK 01000 | AK    |

  When member ".ZZA" visits page "status"
  # Dwolla bug (unexpected error on Address step completion) requires extra visit to status page
  And member ".ZZA" visits page "status"
  Then we show "Retry Verification"
  And we say "error": "redo info"
  
  When member ".ZZA" confirms form "account/basic" with values:
  | first | last | federalId   | dob      |
  | Abe   | One  | 001-01-0001 | 1/1/1990 |
  Then we show "You're getting there"
  And with done "2"
  
Scenario: A member company types the wrong account info (name, ein, or business structure)
  Given members have:
  | id   | federalId |
  | .ZZC | 999999999 |
  # Dwolla's magic ssn for account info failure
  And member ".ZZC" chose to verify phone by "Voice"
  When member ".ZZC" visits page "status"
  Then member ".ZZC" is on step "Phone" within 30 seconds

  When member ".ZZC" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZC" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        | state |
  | .ZZC | 3 C St. | 3 C St., CA 03000 | CA    |

  When member ".ZZC" visits page "status"
  # Dwolla bug (unexpected error on Address step completion) requires extra visit to status page
  And member ".ZZC" visits page "status"
  Then we show "Retry Verification"
  And we say "error": "redo info"
  
  When member ".ZZC" confirms form "account/basic" with values:
  | org          | federalId   | acctType       |
  | Corner Store | 001-01-0001 | CO_PARTNERSHIP |
  Then we show "You're getting there"
  And with done "2"
Skip
Scenario: A member has to submit a photo ID
  Given members have:
  | id   | federalId | dob      | state |
  | .ZZA | 888888888 | 1/1/1991 | AK    |
  # Dwolla's magic ssn for account info failure
  And member ".ZZA" chose to verify phone by "Voice"
  When member ".ZZA" visits page "status"
  Then member ".ZZA" is on step "Phone" within 30 seconds

  When member ".ZZA" visits page "status"
  Then we show "Verify Phone"
  When member ".ZZA" confirms form "account/verify-phone" with values:
  | code  |
  | 99999 |
  Then we show "Contact Information"

  Given members have:
  | id   | address | postalAddr        |
  | .ZZA | 1 A St. | 1 A St., AK 01000 |
  When member ".ZZA" visits page "status"
  Then we show "Photo ID Verification"
  #And we say "error": "could not verify"
  
  When member ".ZZA" confirms form "account/photo-id" with values:
  | idProof           |
  | rcredits-test.jpg |
  Then we show "Contact Information"