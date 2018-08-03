Feature: A user opens an rCredits company account
AS a member
I WANT to open an rCredits company account
SO my company can accept rCredits payments
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | id   | fullName | email | zip   | federalId   | phone        | flags |*
  | .ZZA | Abe One  | a@    | 01330 | 111-22-0001 | +14136280000 | ok,member |
  | .ZZC | Our Pub  | c@    | 01301 | 111-22-3334 | +14136280003 | ok,member |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |

Scenario: A member registers a company
  When member ".ZZA" confirms form "signup-company" with values:
  | relation | flow |*
  |        0 |    0 |
  Then signup args:
  | personal | owner | employee | flow | helper  | fullName | phone |*
  |        0 |     1 |        1 |    0 | NEWZZA |          |       |
  And we show "Open a Company Account" with:
  |~nameDescription      |
  | properly capitalized |
  And with:
  |~acctType            |
  | partnership         |
  | private corporation |
  Given invitation to email "a@" from member ".ZZC" is "c0D3"
  When member "?" confirms form "signup/code=c0D3&helper=NEWZZA&flow=0&owner=1&employee=1" with values:
  | fullName | email       | phone | zip | federalId   | acctType        | company  | companyPhon | companyOptions | address | city    | state | postalAddr                 | tenure | owns |*
  | AAAme Co | aco@ | 413-253-9876 | 01002      | 111-22-0001 | %CO_CORPORATION | | | | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 |
  Then members:
  | id   | fullName | email | zip | phone        | city    | flags        | floor | helper |*
  | .AAA | AAAme Co | aco@  | 01002      | +14132539876 | Amherst | co,confirmed |     0 | .ZZA   |
  And relations:
  | main | agent | permission | employee | owner | draw |*
  | .AAA | .ZZA  | manage     |        1 |     1 |    0 |
  And balances:
  | id   | balance | rewards |*
  | .AAA |       0 |       0 |
  And we say "status": "company is ready"

Scenario: A member registers a company while managing another account
  When member "C:A" confirms form "signup-company" with values:
  | relation | flow |*
  |        0 |    0 |
  Then signup args:
  | personal | owner | employee | flow | helper  | fullName | phone |*
  |        0 |     1 |        1 |    0 | NEWZZA |          |       |

Scenario: A member registers a company, having given company info
  Given member ".ZZA" has company info:
  | company | companyPhone | owner | employee |*
  | Acme Co | +14132222222 |     1 |        0 |
  When member ".ZZA" confirms form "signup-company" with values:
  | relation | flow |*
  |        1 |    0 |
  Then signup args:
  | personal | owner | employee | flow | helper  | fullName | phone        |*
  |        0 |     0 |        0 |    0 | NEWZZA | Acme Co  | +14132222222 |