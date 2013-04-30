Feature: A user signs up for rCredits
AS a newbie
I WANT to open an rCredits account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given member is logged out

Scenario: A newbie visits the registration page with no invite
  Given invitation to email "a@example.com" is ""
  When member "?" visits page "/user/register"
  Then we show "Sign up for rCredits" with:
  | errorPhrase         |
  | you must be invited |

Scenario: An invited newbie visits the registration page
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" visits page "/user/register/code=s0M3_rAnd0M_c0D3"
  Then we show "Sign up for rCredits" with:
  | nameDescription      |
  | properly capitalized |
  And without options:
  | acctType                             |
  | commercial (but not publicly traded) |  

Scenario: A newbie registers
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | postalCode | state | city  | acctType | code        |
  | Abe One | a@example.com | (413) 253-0000 | US | 01001 | MA | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then members:
  | id      | fullName | email         | phone        | country | postalCode | state | city    | flags | 
  | NEW.AAC | Abe One  | a@example.com | +14132530000 | US | 01001       | MA    | Amherst | dft,personal |
  And we say "status": "your account is ready" with subs:
  | quid    |
  | NEW.AAC |
  And we email "welcome" to member "a@example.com" with subs:
  | fullName | name   | quid    | region | pass     |
  | Abe One  | abeone | NEW.AAC | new    | (varies) |
  And member "NEW.AAC" one-time password is set
  And we show "Sign In" with:
  | oldpass      | pass1        | pass2                |
  | Old password | New password | Confirm new password |
# Should check for name defaulting to "abeone" (but doesn't work yet in test)
# Formatting and links are ignored

Scenario: A member registers bad email
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email     | phone | country | postalCode | state | city    | acctType | code |
  | Abe One   | %whatever | 413-253-0001 | US      | 01001       | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then we say "error": "bad email" with subs:
  | email     |
  | %whatever |

Scenario: A member registers bad phone
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email     | phone | country | postalCode | state | city    | acctType | code |
  | Abe One   | a@example.com | %random | US      | 01001       | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then we say "error": "bad phone"

Scenario: A member registers bad name
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName  | email     | phone | country | postalCode | state | city    | acctType | code |
  | ™ %random | a@example.com | 413-253-0001 | US      | 01001-3829 | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then we say "error": "illegal char" with subs:
  | field    |
  | fullName |
  
Scenario: A member registers bad zip
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email     | phone | country | postalCode | state | city    | acctType | code |
  | Abe One  | a@example.com | 413-253-0001 | US      | %random | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then we say "error": "bad zip"
  
Scenario: A member registers again
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  Given members:
  | id      | fullName  | phone  | email         | city  | state | country | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | AK    | US      |
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | postalCode | state | city    | acctType | code |
  | Bea Two   | a@example.com | 413-253-0002 | US | 01001 | MA | Amherst | %R_PERSONAL | s0M3_rAnd0M_c0D3 |
  Then we say "error": "duplicate email|forgot password" with subs:
  | duplicateAccount | emailTagged            | passwordLink                            |
  | Abe One          | a+whatever@example.com | %BASE_PATHuser/password/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

Scenario: A member registers with an existing company
  Given members:
  | id      | fullName | email         | postalCode | phone        | country | city     | flags        |
  | NEW.AAD | AAAme Co | myco@example.com | 01330   | +14136280000 | US      | Ashfield | dft,company  |
  And invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | AAAme Co | (413)628-0000 | isOwner=>1,contractor=>1 |
  Then members:
  | id      | fullName | email         | postalCode | phone         | city    | flags        |
  | NEW.AAC | Abe One  | a@example.com | 01001       | +14132539876 | Amherst | dft,personal |
  And relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner | amount | draw |
  | 1  | .AAD | .AAC  |            |          0 |          1 |       1 |      0 |    0 |

Scenario: A member registers with an unknown company
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | AAAme Co | (413)628-0000 | employeeOk=>1     |
  Then members:
  | id      | fullName | email         | postalCode | phone         | city    | flags        |
  | NEW.AAC | Abe One  | a@example.com | 01001       | +14132539876 | Amherst | dft,personal |
  And no relation:
  | main | agent |
  | .AAD | .AAC  |

Scenario: A member registers with a company with no relation
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | AAAme Co | (413)628-0000 | |
  Then we say "error": "what relation"

Scenario: A member registers with a missing company
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email      | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL |          | (413)628-0000 | isOwner=>1 |
  Then we say "error": "missing field" with subs:
  | field   |
  | company |

Scenario: A member registers with a missing company phone
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email      | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | AAAme Co |             | isOwner=>1 |
  Then we say "error": "missing field" with subs:
  | field   |
  | companyPhone |

Scenario: A member registers with a bad company
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email      | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | 2sp  ces | (413)628-0000 | isOwner=>1 |
  Then we say "error": "multiple spaces" with subs:
  | field   |
  | Company |

Scenario: A member registers with a bad company phone
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email  | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | Abe One  | a@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_PERSONAL | AAAme Co | %random | isOwner=>1 |
  Then we say "error": "bad company phone" with subs: ""

Scenario: A member registers a company
  Given members:
  | id      | fullName | email         | postalCode | phone        | country | city     | flags        |
  | NEW.AAC | Abe One  | a@example.com | 01330   | +14136280000 | US      | Ashfield | dft,personal  |
  And invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" visits page "/user/register/code=s0M3_rAnd0M_c0D3&by=NEW.AAC&flow=from&isOwner=1&employeeOk=1"
  Then we show "Sign up for rCredits" with:
  | nameDescription      |
  | properly capitalized |
  And with options:
  | acctType                             |
  | commercial (but not publicly traded) |
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3&by=NEW.AAC&flow=from&isOwner=1&employeeOk=1" with values:
  | fullName | email         | phone | country | state | postalCode | city    | acctType | company  | companyPhone | companyOptions |
  | AAcme Co | aco@example.com | 413-253-9876 | US | MA | 01001 | Amherst | %R_COMMERCIAL | | | |
  Then members:
  | id      | fullName | email         | postalCode | phone         | city    | flags        |
  | NEW.AAD | AAcme Co | aco@example.com | 01001       | +14132539876 | Amherst | dft,company |
  And relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner | amount | draw |
  | 1  | .AAD | .AAC  | manage     |          1 |          1 |       1 |      0 |    1 |
  And we say "status": "company is ready" with subs:
  | quid    |
  | NEW.AAD |
  And we show "Sign In" with: ""
  
Scenario: A newbie registers from elsewhere
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | phone | country | postalCode | state | city  | acctType | code        |
  | Abe One | a@example.com | (333) 253-0000 | US | 03768-2345 | NH | Lyme | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
 Then members:
  | id      | fullName | email         | phone        | country | postalCode | state | city    | flags | 
  | NEN.AAA | Abe One  | a@example.com | +13332530000 | US | 03768-2345 | NH | Lyme | dft,personal |
  And we say "status": "your account is ready" with subs:
  | quid    |
  | NEN.AAA |
  And we email "welcome" to member "a@example.com" with subs:
  | fullName | name   | quid    | region | pass     |
  | Abe One  | abeone | NEN.AAA | new    | (varies) |
  And we show "Sign In" with: ""
# Should check for name defaulting to "abeone" (but doesn't work yet in test)
# Formatting and links are ignored