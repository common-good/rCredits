Feature: A user signs up for rCredits
AS a newbie
I WANT to get access to the rCredits Participants section
SO I can start pretending
# Note that "member" in the scenarios below means new member (newbie).
# The first member is AAB because CGF is AAA.

Setup:
  Given member is logged out

Scenario: A newbie visits the registration page with no invite
  Given invitation to email "a@example.com" is ""
  When member "?" visits page "/user/register"
  Then we show page "/user/register" with:
  | errorPhrase         |
  | you must be invited |

Scenario: An invited newbie visits the registration page
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" visits page "/user/register/code=s0M3_rAnd0M_c0D3"
  Then we show page "/user/register" with:
  | nameDescription      |
  | properly capitalized |
  
Scenario: A newbie registers
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | country | postalCode | state | city  | acctType | code        |
  | Abe One   | a@example.com | US    | 01001       | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
 Then members:
  | id      | fullName | email         | country | postalCode | state | city    | flags | 
  | NEW.AAB | Abe One   | a@example.com | US | 01001       | MA    | Amherst | dft,personal |
  And we say "status": "your account is ready" with subs:
  | quid    | balance |
  | NEW.AAB | $250    |
  And we email "welcome" to member "a@example.com" with subs:
  | fullName | name   | quid    | region | pass     |
  | Abe One  | abeone | NEW.AAB | new    | (varies) |
  And we show page "/user/login" with:
  | title          |
  | rCredits Login |
# Should check for name defaulting to "abeone" (but doesn't work yet in test)
# Formatting and links are ignored

Scenario: A member registers bad email
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email     | country | postalCode | state | city    | acctType | code |
  | Abe One   | %whatever | US      | 01001       | MA    | Amherst | %R_PERSONAL  | s0M3_rAnd0M_c0D3 |
  Then we say "error": "bad email" with subs:
  | email     |
  | %whatever |

Scenario: A member registers again
  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
  Given members:
  | id      | fullName  | phone  | email         | city  | state | country | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | AK    | US      |
  When member "?" confirms form "/user/register/code=s0M3_rAnd0M_c0D3" with values:
  | fullName | email         | country | postalCode | state | city    | acctType | code |
  | Bea Two   | a@example.com | US      | 01001       | MA    | Amherst | %R_PERSONAL | s0M3_rAnd0M_c0D3 |
  Then we say "error": "duplicate email|forgot password" with subs:
  | duplicateAccount | emailTagged            | passwordLink                            |
  | Abe One          | a+whatever@example.com | %BASE_PATHuser/password/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

##Scenario: A member registers with a company
##  Given invitation to email "a@example.com" is "s0M3_rAnd0M_c0D3"
##  When member "?" confirms form "user/register/code=s0M3_rAnd0M_c0D3" with values:
##  | fullName | email         | postalCode | city    | acctType | company  | company_phone | company_options |
##  | Abe One   | a@example.com | 01001       | Amherst | personal     | AAAme Co | (413)628-0000 | isOwner=>1     |
##  Then members:
##  | id      | fullName | email         | postalCode | phone        | city    | flags        |
##  | NEW.AAB | Abe One   | a@example.com | 01001       |              | Amherst | dft,personal |
##  | NEW.AAC | AAAme Co  |               |             | +14136280000 |         | dft,company  |
##  And relations:
##  | id | main | agent | permission | employerOk | employeeOk | isOwner |
##  | 1  | .AAC | .AAB  |            |             |             | 1        |
##
##Scenario: A member registers with a company with no employee or owner
##  Given invitation to email "a@example.com" is %whatever20
##  When member "?" confirms form "user/register" with values:
##  | fullName | email         | postalCode | city    | acctType | company  | company_phone | company_options |
##  | Abe One   | a@example.com |  01001      | Amherst | personal     | AAAme Co | (413)628-0000 |                 |
##  Then we say "error": "what relation" with subs: ""
#
##Scenario: A member registers with a company but no relation
##  When member "?" confirms form "user/register" with values:
##  | fullName | email         | country | postalCode | state | city    | acctType | company  |
##  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | AAAme Co |
##  Then we say "error": "bad company phone" with subs: ""
##
##Scenario: A member registers with a company with bad phone
##  When member "?" confirms form "user/register" with values:
##  | fullName | email         | country | postalCode | state | city    | acctType | company  | company_phone |
##  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | AAAme Co | random       |
##  Then we say "error": "bad company phone" with subs: ""
##
##Scenario: A member registers with a company but no relation
##  When member "?" confirms form "user/register" with values:
##  | fullName | email         | country | postalCode | state | city    | acctType | company  | company_phone |
##  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | AAAme Co | (413)628-0000 |
##  Then we say "error": "what relation" with subs: ""
##