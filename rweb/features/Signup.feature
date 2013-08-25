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
  | errorPhrase         |
  | you must be invited |

Scenario: An invited newbie visits the registration page
  Given invitation to email "a@" is "c0D3"
  When member "?" visits page "signup/code=c0D3"
  Then we show "Sign up for rCredits" with:
  | nameDescription      |
  | properly capitalized |
  And without options:
  | acctType                             |
  | commercial (but not publicly traded) |  

Scenario: A newbie registers
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | code |
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | c0D3 |
  Then members:
  | id   | fullName | email | phone     | postalCode | country | state | city   | flags        | floor |
  | .AAC | Abe One  | a@ | +14132530000 | 01001      | US      | MA    | Agawam | dft,personal | 0     |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | region | pass     | bonus           |
  | Abe One  | abeone | NEW.AAC | new    | (varies) |  |
  And member ".AAC" one-time password is set
  And we show "Welcome" with:
  | oldpass      | pass1        | pass2                |
  | Tmp password | New password | Confirm new password |

Scenario: A newbie registers with no case
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType     | code |
  | abe one  | a@ | 413-253-0000 | 01002      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | c0D3 |
  Then members:
  | id   | fullName | email | phone     | postalCode | state | city    | flags        | floor |
  | .AAC | Abe One  | a@ | +14132530000 | 01002      | MA    | Amherst | dft,personal | 0     |

Scenario: A member registers bad email
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email     | postalCode | acctType    | code |
  | Abe One   | %whatever | 01001      | %R_PERSONAL | c0D3 |
  Then we say "error": "bad email" with subs:
  | email     |
  | %whatever |

Scenario: A member registers bad name
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone     | postalCode | federalId   | dob      | acctType | code |
  | ™ %random | a@ | 413-253-0000 | 01001-3829 | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | c0D3 |
  Then we say "error": "illegal char" with subs:
  | field    |
  | fullName |
  
Scenario: A member registers bad zip
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | phone | postalCode | federalId   | dob      | acctType    | code |
  | Abe One  | a@ | 413-253-0001 | %random    | 111-22-3333 | 1/2/1990 | %R_PERSONAL | c0D3 |
  Then we say "error": "bad zip"
  
Scenario: A member registers again
  Given invitation to email "a@" is "c0D3"
  Given members:
  | id   | fullName  | phone  | email | city  | state | 
  | .ZZA | Abe One    | +20001 | a@   | Atown | AK    |
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone      | postalCode | federalId   | dob      | acctType    | code |
  | Bea Two   | a@ | 413-253-0002 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | c0D3 |
  Then we say "error": "duplicate email|forgot password" with subs:
  | duplicateAccount | emailTagged            | passwordLink                            |
  | Abe One          | a+whatever@example.com | %BASE_PATHpassword/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

Scenario: A member registers with an existing company
  Given members:
  | id   | fullName | email | postalCode | phone        | city     | flags        |
  | .AAD | AAAme Co | myco@ | 01330      | +14136280000 | Ashfield | dft,company  |
  And invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhone  | companyOptions           |
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | AAAme Co | (413)628-0000 | isOwner=>1,contractor=>1 |
  Then members:
  | id   | fullName | email | postalCode | state | city    | flags        |
  | .AAC | Abe One  | a@    | 01002      | MA    | Amherst | dft,personal |
  And relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner | amount | draw |
  | 1  | .AAD | .AAC  |            |          0 |          1 |       1 |      0 |    0 |

Scenario: A member registers with an unknown company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhone  | companyOptions |
  | Abe One  | a@    | 413-253-9876 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | AAAme Co | (413)628-0000 | employeeOk=>1  |
  Then members:
  | id   | fullName | email | postalCode | phone        | city    | flags        |
  | .AAC | Abe One  | a@    | 01002      | +14132539876 | Amherst | dft,personal |
  And no relation:
  | main | agent |
  | .AAD | .AAC  |

Scenario: A member registers with a company with no relation
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone        | postalCode | federalId  | dob  | acctType    | company  | companyPhone  | companyOptions |
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | AAAme Co | (413)628-0000 |               |
  Then we say "error": "what relation"

Scenario: A member registers with a missing company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@    | 413-253-0002 | 01001 | 111-22-3333 | 1/2/1990 | %R_PERSONAL |       | (413)628-0000 | isOwner=>1     |
  Then we say "error": "missing field" with subs:
  | field   |
  | company |

Scenario: A member registers with a missing company phone
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhone | companyOptions |
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | AAAme Co |             | isOwner=>1      |
  Then we say "error": "missing field" with subs:
  | field   |
  | companyPhone |

Scenario: A member registers with a bad company
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhone | companyOptions |
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | 2sp  ces | (413)628-0000 | isOwner=>1    |
  Then we say "error": "multiple spaces" with subs:
  | field   |
  | Company |

Scenario: A member registers with a bad company phone
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email  | phone    | postalCode | federalId   | dob      | acctType    | company  | companyPhone | companyOptions |
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | AAAme Co | %random      | isOwner=>1     |
  Then we say "error": "bad company phone" with subs: ""

Scenario: A member registers a company
  Given members:
  | id   | fullName | email | postalCode | federalId   | phone        | flags        |
  | .AAC | Abe One  | a@    | 01330      | 111-22-3333 | +14136280000 | dft,personal |
  And invitation to email "a@" is "c0D3"
  When member "?" visits page "signup/code=c0D3&by=NEW.AAC&flow=from&isOwner=1&employeeOk=1"
  Then we show "Sign up for rCredits" with:
  | nameDescription      |
  | properly capitalized |
  And with options:
  | acctType                             |
  | commercial (but not publicly traded) |
  When member "?" confirms form "signup/code=c0D3&by=NEW.AAC&flow=from&isOwner=1&employeeOk=1" with values:
  | fullName | email       | phone | postalCode | federalId   | acctType      | company  | companyPhone | companyOptions |
  | AAcme Co | aco@ | 413-253-9876 | 01002      | 111-22-3333 | %R_COMMERCIAL | | | |
  Then members:
  | id   | fullName | email | postalCode | phone        | city    | flags       | floor |
  | .AAD | AAcme Co | aco@  | 01002      | +14132539876 | Amherst | dft,company |     0 |
  And relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner | amount | draw |
  | 1  | .AAD | .AAC  | manage     |          1 |          1 |       1 |      0 |    1 |
  And balances:
  | id   | r | usd | rewards |
  | .AAD | 0 |   0 |       0 |
  And we say "status": "company is ready"
  And we show "Welcome" with: ""
  
Scenario: A newbie registers from elsewhere
  Given invitation to email "a@" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone       | postalCode | federalId   | dob      | acctType    | code |
  | Abe One  | a@ | (333) 253-0000 | 03768-2345 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | c0D3 |
 Then members:
  | id      | fullName | email | phone     | postalCode | state | city | flags        | 
  | NEN.AAA | Abe One  | a@ | +13332530000 | 03768-2345 | NH    | Lyme | dft,personal |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | region | pass     |
  | Abe One  | abeone | NEN.AAA | new    | (varies) |
  And we show "Welcome" with: ""
# Should check for name defaulting to "abeone" (but doesn't work yet in test)
# Formatting and links are ignored