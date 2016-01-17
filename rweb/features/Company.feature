Feature: Company Information
AS a company member
I WANT to update my company information
SO I can complete my registration and/or publicize my goods and services to other rCredits members.

Setup:
  Given members:
  | id   | fullName | flags |*
  | .ZZA | Abe One  |       |
  | .ZZC | Our Pub  | co,ok |
  And relations:
  | id   | main | agent | permission |*
  | :ZZA | .ZZC | .ZZA  | manage     |
  
Scenario: A member visits the company info page
  When member ":ZZA" visits page "settings/company"
  Then we show "Company Information" with:
  |_rCredits page | rCredits web page |
  | Company name  | Our Pub |

Scenario: A member updates company info
  When member ":ZZA" confirms form "settings/company" with values:
  | private | selling | website     | description   | employees | gross | tips |*
  |         | stuff   | www.pub.com | we do vittles |         2 |   100 |    1 |
  Then members:
  | id   | selling | website     | description   | employees | gross | coFlags       |*
  | .ZZC | stuff   | www.pub.com | we do vittles |         2 |   100 | %(1<<%CO_TIP) |
  And we say "status": "options saved"
  
Scenario: A member gives a bad employee count
  When member ":ZZA" confirms form "settings/company" with values:
  | selling | website     | description   | employees | gross |*
  | stuff   | www.pub.com | we do vittles |        -2 |   100 |
  Then we say error in field "employees": "negative amount"

Scenario: A member gives a bad gross
  When member ":ZZA" confirms form "settings/company" with values:
  | selling | website     | description   | employees | gross |*
  | stuff   | www.pub.com | we do vittles |         2 |  junk |
  Then we say error in field "gross": "bad amount"
