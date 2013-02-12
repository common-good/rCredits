Feature: A user signs up for rCredits
AS a newbie
I WANT to get access to the rCredits Participants section
SO I can start pretending
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given member is logged out

Scenario: A newbie visits the registration page with no invite
  Given invitation to email "a@example.com" is ""
  When member "?" visits page "\user_register_form"
  Then we show page "\user_register_form" with:
  | errorPhrase         |
  | you must be invited |

Scenario: An invited newbie visits the registration page
  Given invitation to email "a@example.com" is %whatever20
  When member "?" visits page "\user_register_form"
  Then we show page "\user_register_form" with:
  | nameDescription      |
  | properly capitalized |
  
Scenario: A newbie registers
  Given invitation to email "a@example.com" is %whatever20
  When member "?" confirms form "\user_register_form" with values:
  | full_name | email         | country | postal_code | state | city    | account_type | code        |
  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | %whatever20 |
  Then members:
  | id      | full_name | email         | country | postal_code | state | city    | account_type | flags         |
  | NEW.ZZA | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | %R_PERSONAL  | %BIT_DEFAULTS |
  And we say "status": "your account is ready" with "asif" subs:
  | quid    | balance |
  | NEW.ZZA | $250    |
  And we email "welcome" to member "a@example.com" with "asif" subs:
  | fullName | name   | quid    | oneTimeLoginUrl |
  | Abe One  | abeone | NEW.ZZA | (varies)        |
  And member is logged out
#Formatting and links are ignored

Scenario: A member registers bad email
  When member "?" confirms form "\user_register_form" with values:
  | full_name | email     | country | postal_code | state | city    | account_type |
  | Abe One   | %whatever | US      | 01001       | MA    | Amherst | personal     |
  Then we say "error": "bad email" with "asif" subs:
  | email     |
  | %whatever |

#Scenario: A member registers again
#  Given invitation to email "a@example.com" is %whatever20
#  Given members:
#  | id      | full_name  | phone  | email         | city  | state | country | 
#  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | AK    | US      |
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | country | postal_code | state | city    | account_type |
#  | Bea Two   | a@example.com | US      | 01001       | MA    | Amherst | %R_PERSONAL  |
#  Then we say "error": "duplicate email|forgot password" with "asif" subs:
#  | duplicateAccount | href                              |
#  | Abe One          | %BASE_PATHuser/password/a%40example.com |
#  And member is logged out
##That email is taken. Click here to get a new password.

#Scenario: A member registers with a company
#  Given invitation to email "a@example.com" is %whatever20
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | postal_code | city    | account_type | company  | company_phone | company_options |
#  | Abe One   | a@example.com | 01001       | Amherst | personal     | Aacme Co | (413)628-0000 | is_owner=>1     |
#  Then members:
#  | id      | full_name | email         | postal_code | phone        | city    | account_type  | flags         |
#  | NEW.ZZA | Abe One   | a@example.com | 01001       |              | Amherst | %R_PERSONAL   | %BIT_DEFAULTS |
#  | NEW.ZZB | Aacme Co  |               |             | +14136280000 |         | %R_COMMERCIAL | %BIT_DEFAULTS |
#  And relations:
#  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
#  | 1  | .ZZB | .ZZA  |            |             |             | 1        |
#
#Scenario: A member registers with a company with no employee or owner
#  Given invitation to email "a@example.com" is %whatever20
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | postal_code | city    | account_type | company  | company_phone | company_options |
#  | Abe One   | a@example.com |  01001      | Amherst | personal     | Aacme Co | (413)628-0000 |                 |
#  Then we say "error": "what relation" with "asif" subs: ""

#Scenario: A member registers with a company but no relation
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | country | postal_code | state | city    | account_type | company  |
#  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | Aacme Co |
#  Then we say "error": "bad company phone" with "asif" subs: ""
#
#Scenario: A member registers with a company with bad phone
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | country | postal_code | state | city    | account_type | company  | company_phone |
#  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | Aacme Co | random       |
#  Then we say "error": "bad company phone" with "asif" subs: ""
#
#Scenario: A member registers with a company but no relation
#  When member "?" confirms form "\user_register_form" with values:
#  | full_name | email         | country | postal_code | state | city    | account_type | company  | company_phone |
#  | Abe One   | a@example.com | US      | 01001       | MA    | Amherst | personal     | Aacme Co | (413)628-0000 |
#  Then we say "error": "what relation" with "asif" subs: ""
#