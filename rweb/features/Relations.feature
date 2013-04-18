Feature: Relations
AS a member
I WANT to manage my account's relations with other rCredits accounts
SO I can buy and sell stuff on behalf of other accounts, and they on mine.

Setup:
  Given members:
  | id   | fullName   | acctType         | flags                   |
  | .ZZA | Abe One    | %R_PERSONAL      | dft,ok,personal         |
  | .ZZB | Bea Two    | %R_SELF_EMPLOYED | dft,ok,personal,company |
  | .ZZC | Corner Pub | %R_COMMERCIAL    | dft,ok,company          |
  | .ZZD | Dee Four   | %R_PERSONAL      | dft,ok,personal         |
  And transactions: 
  | tx_id | created   | type       | amount | from      | to   | purpose | taking |
  | :AAAB | %today-6m | %TX_SIGNUP |    250 | community | .ZZA | signup  | 0      |
  | :AAAC | %today-6m | %TX_SIGNUP |    250 | community | .ZZB | signup  | 0      |
  | :AAAD | %today-6m | %TX_SIGNUP |    250 | community | .ZZC | signup  | 0      |
  Then balances:
  | id        | balance |
  | community |    -750 |
  | .ZZA      |     250 |
  | .ZZB      |     250 |
  | .ZZC      |     250 |

Scenario: Member has an employee, confirmed
  Given relations:
  | id | main | agent | permission   | amount | employerOk | employeeOk | isOwner | draw |
  | 1  | .ZZA | .ZZD  | sell         |     10 | 1          | 1          | 1       | 0    |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | Draw | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Dee Four   | No   | No           | Yes          | Yes     | sell         | request rCard |

Scenario: Member has an employee, unconfirmed
  Given relations:
  | id | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | buy        |     50 | 1           | 0           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Dee Four   | No           | Yes          | No      | buy and sell |          |

Scenario: Member has a relation with a contractor
  Given relations:
  | id | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZB  | buy        |     50 | 1           | 0           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Bea Two    | No           | --           | No      | buy and sell |          |
  
Scenario: Member has an employee, claimed
  Given relations:
  | id | main | agent | permission   | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | sell         |     10 | 0           | 1           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Dee Four   | No           | No           | No        | sell       |          |
  
Scenario: Employee can only read
  Given relations:
  | id | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | read       |     10 | 1           | 1           | 1        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission        | Request rCard |
  | Dee Four   | No           | Yes          | Yes     | read transactions |          |
  
Scenario: Member has an employer
  Given relations:
  | id | main | agent | permission   | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZB | .ZZA  | sell         |     10 | 1           | 1           | 1        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Bea Two    | Yes          | --           | No      | no access    | --       |
  
Scenario: Member has access to employee account
  Given relations:
  | id | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  |            |     10 | 1           | 1           | 1        |
  | 2  | .ZZD | .ZZA  | sell       |     20 | 0           | 0           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Dee Four   | No           | Yes          | Yes     | no access    | --       |
  When member ".ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Abe One    | Yes          | No           | No      | sell         | --            |

Scenario: Member company has relations
  Given relations:
  | id    | main | agent | permission   | amount | employerOk | employeeOk | isOwner |
  | :ZZA  | .ZZC | .ZZA  | sell         |     10 | 1           | 1           | 1        |
  When member ":ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person  | Amount | My employee? | Is owner? | Permission | Request rCard      |
  | Abe One | 10     | Yes          | Yes       | sell       | request rCard |
  And we show "Relations" without:
  | Header       |
  | My employer? |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Corner Pub | Yes          | --           | No      | --           | --            |

Scenario: It's complicated
  Given relations:
  | id   | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | :ZZA | .ZZA | .ZZD  | sell       |     10 | 1           | 0           | 1        |
  | :ZZB | .ZZD | .ZZA  |            |     20 | 0           | 1           | 0        |
  | :ZZC | .ZZA | .ZZC  | buy        |     30 | 0           | 1           | 0        |
  | :ZZD | .ZZC | .ZZA  | manage     |     40 | 1           | 0           | 0        |
  | :ZZE | .ZZA | .ZZB  | sell       |     10 | 1           | 0           | 1        |
  | :ZZF | .ZZB | .ZZA  |            |     20 | 1           | 1           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Bea Two    | Yes          | --           | Yes     | sell         | --       |
  | Corner Pub | No           | --           | No      | --           | --       |
  | Dee Four   | Yes          | Yes          | Yes     | sell         | --       |
  When member ".ZZB" visits page "account/relations"
  Then we show "Relations" with:
  | Person  | Amount | My employee? | Family? | Permission | Request rCard      |
  | Abe One | 20     | Yes          | No      | no access  | --            |
  When member ".ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Person  | My employer? | My employee? | Family? | Permission | Request rCard      |
  | Abe One | No           | No           | No      | no access  | --            |
  And we show "Relations" with:
  | Header  |
  | Family? |
  When member ":ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Person  | Amount | My employee? | Is owner? | Permission     | Request rCard      |
  | Abe One | 40     | Yes          | No        | manage account | --            |
  And we show "Relations" without:
  | Header    |
  | employer? |

Scenario: A member adds a relation
  When member ".ZZA" visits page "account/relations"
  And member ".ZZA" confirms form "account/relations" with values:
  | newPerson |
  | beatwo    |
  Then we show "Relations" with:
  | Person     | Draw | My employer? | My employee? | Family? | Permission   | Request rCard |
  | Bea Two    | No   | No           | No           | No      | --           |               |
  