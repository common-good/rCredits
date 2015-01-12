Feature: A user signs up for rCredits
AS a newbie
I WANT to open an rCredits account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given member is logged out

Scenario: A newbie visits the registration page with no invite
  Given invitation to email "a@" is ""
  When member "?" visits page "signup"
  Then we show "Sign up for rCredits" with:
  |_errorPhrase         |
  | you must be invited |

Scenario: A newbie visits the registration page with bad invite
  Given invitation to email "a@" is "c0D3"
  When member "?" visits page "signup/code=WhAtEvEr"
  Then we show "Sign up for rCredits" with:
  |_errorPhrase         |
  | you must be invited |

Scenario: A newbie visits the registration page with expired invite
  Given invitation to email "a@" is "c0D3"
  And invitation "c0D3" was sent on "%today-5w"
  When member "?" visits page "signup/code=c0D3"
  Then we show "Sign up for rCredits" with:
  |_errorPhrase            |
  | invitation has expired |

Scenario: A newbie visits the registration page with a used invite
  Given invitation to email "a@" is "c0D3"
  And member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 |
  When member "?" visits page "signup/code=c0D3"
  Then we show "Sign up for rCredits" with:
  |_errorPhrase                      |
  | invitation has already been used |

Scenario: A newbie registers in Western Massachusetts
  Given invitation to email "a@" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags | floor | address | postalAddr                | tenure | owns |*
  | .AAC | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | 0     |    1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAC | %R_SITE_URL | %name |
  And member ".AAC" one-time password is set
  #And we show "Empty"

Scenario: A newbie registers with a different legal name
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | legalName | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns |*
  | Abe One   | Abey One | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags | name    |*
  | .AAC | Abey One | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | abeyone |

Scenario: A newbie registers elsewhere
  Given invitation to email "a@" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns |*
  | Abe One  | a@ | 212-253-0000 | US      | 10001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | New York | NY    | 1 A St., New York, NY 10001 |     18 |    1 |
  Then members:
  | id      | fullName | email | phone     | postalCode | country | state | city     | flags |*
  | NYA.AAA | Abe One  | a@ | +12122530000 | 10001      | US      | NY    | New York |       |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NYA.AAA | %R_SITE_URL | %name |
  And member "NYA.AAA" one-time password is set

Scenario: A newbie registers with no case
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType     | address | city    | state | postalAddr                | tenure | owns |*
  | abe one  | a@ | 413-253-0000 | 01002      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | email | phone     | postalCode | state | city    | flags | floor | postalAddr |*
  | .AAC | Abe One  | a@ | +14132530000 | 01002      | MA    | Amherst |       | 0     | 1 A ST., Amherst, MA 01001 |

Scenario: A member registers bad email
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | postalCode | acctType    |*
  | Abe One  | %whatever | 01001      | %R_PERSONAL |
  Then we say "error": "bad email"

Scenario: A member registers bad name
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone     | postalCode | federalId   | dob      | acctType     | tenure |*
  | ™ %random | a@ | 413-253-0000 | 01001-3829 | 111-22-3333 | 1/2/1990 | %R_PERSONAL  |     18 |
  Then we say "error": "illegal char" with subs:
  | field    |*
  | fullName |

Scenario: A member registers bad zip
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | phone | postalCode | federalId   | dob      | acctType    |*
  | Abe One  | a@ | 413-253-0001 | %random    | 111-22-3333 | 1/2/1990 | %R_PERSONAL |
  Then we say "error": "bad zip"
  
Scenario: A member registers again
  Given invitation to email "a@" is "c0D3"
  Given members:
  | id   | fullName  | phone  | email | city  | state |*
  | .ZZA | Abe One    | +20001 | a@   | Atown | AK    |
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone      | postalCode | federalId   | dob      | acctType    |*
  | Bea Two  | a@ | 413-253-0002 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL |
  Then we say "error": "duplicate email|forgot password" with subs:
  | who     | emailTagged            | a                                       |*
  | Abe One | a+whatever@example.com | a href=account/password/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

Scenario: A member registers with an existing company
  Given members:
  | id   | fullName | email | postalCode | phone        | city     | flags  |*
  | .AAD | Aacme Co | myco@ | 01330      | +14136280000 | Ashfield | co    |
  And invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhon  | copts                    | address | city    | state | postalAddr                 | tenure | owns |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 | isOwner=>1,contractor=>1 | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | email | postalCode | state | city    | flags |*
  | .AAC | Abe One  | a@    | 01002      | MA    | Amherst |       |
  And relations:
  | id   | main | agent | permission | employee | isOwner | draw |*
  | :AAA | .AAD | .AAC  |            |        0 |       1 |    0 |

Scenario: A member registers with an unknown company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhon  | copts          | address | city    | state | postalAddr                 | tenure | owns |*
  | Abe One  | a@    | 413-253-9876 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 | employeeOk=>1  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 |
  Then members:
  | id   | legalName | email | postalCode | phone        | city    | flags |*
  | .AAC | Abe One   | a@    | 01002      | +14132539876 | Amherst |       |
  And no relation:
  | main | agent |*
  | .AAD | .AAC  |
  And signup company info for account ".AAC" is remembered

Scenario: A member registers with a company with no relation
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone        | postalCode | federalId  | dob  | acctType    | company  | companyPhon  | companyOptions | tenure | owns |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 |               |     18 |    1 |
  Then we say "error": "what relation"
Skip (requirement relaxed)
Scenario: A member registers with a missing company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType | company  | companyPhon | companyOptions | tenure | owns |*
  | Abe One  | a@    | 413-253-0002 | 01001 | 111-22-3333 | 1/2/1990 | %R_PERSONAL |       | (413)628-0000 | isOwner=>1     |     18 |    1 |
  Then we say "error": "missing field" with subs:
  | field   |*
  | company |
Skip (requirement relaxed)
Scenario: A member registers with a missing company phone
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co |             | isOwner=>1     |     18 |    1 |
  Then we say "error": "missing field" with subs:
  | field   |*
  | companyPhon |
Resume
Scenario: A member registers with a bad company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | 2sp  ces | (413)628-0000 | isOwner=>1   |     18 |    1 |
  Then we say "error": "multiple spaces" with subs:
  | field   |*
  | Company |

Scenario: A member registers with a bad company phone
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email  | phone    | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | %random      | isOwner=>1    |     18 |    1 |
  Then we say "error": "bad company phone" with subs: ""

Scenario: A member registers a company
  Given members:
  | id   | fullName | email | postalCode | federalId   | phone        | flags |*
  | .AAC | Abe One  | a@    | 01330      | 111-22-3333 | +14136280000 | ok    |
#  And invitation to email "a@" is "c0D3"
#  When member "?" visits page "signup/code=c0D3&personal=&helper=NEW.AAC&flow=from&isOwner=1&employeeOk=1"
  When member ".AAC" confirms form "another" with values:
  | relation | flow |*
  |        1 |    0 |
  Then we show "Sign up for rCredits" with:
  |_nameDescription      |
  | properly capitalized |
  And with options:
  |_acctType            |
  | partnership         |
  | private corporation |
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3&helper=NEW.AAC&flow=from&isOwner=1&employeeOk=1" with values:
  | fullName | email       | phone | postalCode | federalId   | acctType        | company  | companyPhon | companyOptions | address | city    | state | postalAddr                 | tenure | owns |*
  | AAcme Co | aco@ | 413-253-9876 | 01002      | 111-22-3333 | %CO_CORPORATION | | | | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | email | postalCode | phone        | city    | flags | floor |*
  | .AAD | AAcme Co | aco@  | 01002      | +14132539876 | Amherst | co    |     0 |
  And relations:
  | id   | main | agent | permission | employee | isOwner | draw |*
  | :AAA | .AAD | .AAC  | manage     |        1 |       1 |    1 |
  And balances:
  | id   | r | usd | rewards |*
  | .AAD | 0 |   0 |       0 |
  And we say "status": "company is ready"
# (does not work yet because svar('myid'...) is not recognized by tests)  And we show "You're getting there!"

Scenario: A newbie registers from elsewhere
  Given invitation to email "a@" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone       | postalCode | federalId   | dob      | acctType    | address | city | state | postalAddr                   | tenure | owns |*
  | Abe One  | a@ | (333) 253-0000 | 03768-2345 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | 1 A ST. | Lyme | NH    | 1 A ST., Lyme, NH 03768-2345 |     18 |    1 |
 Then members:
  | id      | fullName | email | phone     | postalCode | state | city | flags | *
  | NEN.AAA | Abe One  | a@ | +13332530000 | 03768-2345 | NH    | Lyme |       |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEN.AAA | %R_SITE_URL | %name |
  # And we show "Empty"
