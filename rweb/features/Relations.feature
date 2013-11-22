Feature: Relations
AS a member
I WANT to manage my account's relations with other rCredits accounts
SO I can buy and sell stuff on behalf of other accounts, and they on mine.

Setup:
  Given members:
  | id   | fullName   | acctType         | flags                        |
  | .ZZA | Abe One    | %R_PERSONAL      | dft,ok,person,bona         |
  | .ZZB | Bea Two    | %R_SELF_EMPLOYED | dft,ok,person,company,bona |
  | .ZZC | Corner Pub | %R_COMMERCIAL    | dft,ok,company,bona          |
  | .ZZD | Dee Four   | %R_PERSONAL      | dft,ok,person,bona         |
  And transactions: 
  | xid | created   | type       | amount | from      | to   | purpose | taking |
  |   1 | %today-6m | %TX_SIGNUP |    250 | community | .ZZA | signup  | 0      |
  |   2 | %today-6m | %TX_SIGNUP |    250 | community | .ZZB | signup  | 0      |
  |   3 | %today-6m | %TX_SIGNUP |    250 | community | .ZZC | signup  | 0      |
  Then balances:
  | id        | balance |
  | community |    -750 |
  | .ZZA      |     250 |
  | .ZZB      |     250 |
  | .ZZC      |     250 |

Scenario: Member has an employee, confirmed
  Given relations:
  | id | main | agent | permission   | employerOk | employeeOk | isOwner | draw |
  | 1  | .ZZA | .ZZD  | sell         | 1          | 1          | 1       | 0    |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | Draw | My employer? | My employee? | Family? | Permission    |_request rCard |
  | Dee Four   | No   | No           | Yes          | Yes     | send invoices | request rCard |

Scenario: Member has an employee, unconfirmed
  Given relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | buy        | 1          | 0          | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Dee Four   | No           | Yes          | No      | buy and sell |          |

Scenario: Member has a relation with a contractor
  Given relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZB  | buy        | 1          | 0          | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Bea Two    | No           | --           | No      | buy and sell |               |
  
Scenario: Member has an employee, claimed
  Given relations:
  | id | main | agent | permission   | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | sell         | 0          | 1          | 0       |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission    |_request rCard |
  | Dee Four   | No           | No           | No      | send invoices |               |
  
Scenario: Employee can only read
  Given relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  | read       | 1          | 1          | 1       |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission        |_request rCard |
  | Dee Four   | No           | Yes          | Yes     | read transactions |          |
  
Scenario: Member has an employer
  Given relations:
  | id | main | agent | permission   | employerOk | employeeOk | isOwner |
  | 1  | .ZZB | .ZZA  | sell         | 1          | 1          | 1        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Bea Two    | Yes          | --           | No      | no access    | --       |
  
Scenario: Member has access to employee account
  Given relations:
  | id | main | agent | permission | employerOk | employeeOk | isOwner |
  | 1  | .ZZA | .ZZD  |            | 1          | 1          | 1        |
  | 2  | .ZZD | .ZZA  | sell       | 0          | 0          | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Dee Four   | No           | Yes          | Yes     | no access    | --            |
  When member ".ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission    |_request rCard |
  | Abe One    | Yes          | No           | No      | send invoices | --            |

Scenario: Member company has relations
  Given relations:
  | id   | main | agent | permission | employerOk | employeeOk | isOwner |
  | .ZZA | .ZZC | .ZZA  | manage     | 1          | 1          | 1        |
  When member ":ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other   | My employee? | Owner? | Permission     |_request rCard |
  | Abe One | Yes          | Yes    | manage account | request rCard |
  And without:
  |_Header       |
  | My employer? |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Corner Pub | Yes          | --           | No      | --           | --            |

Scenario: It's complicated
  Given relations:
  | id   | main | agent | permission | amount | employerOk | employeeOk | isOwner |
  | .ZZA | .ZZA | .ZZD  | sell       |     10 | 1           | 0           | 1        |
  | .ZZB | .ZZD | .ZZA  |            |     20 | 0           | 1           | 0        |
  | .ZZC | .ZZA | .ZZC  | buy        |     30 | 0           | 1           | 0        |
  | .ZZD | .ZZC | .ZZA  | manage     |     40 | 1           | 0           | 0        |
  | .ZZE | .ZZA | .ZZB  | sell       |     10 | 1           | 0           | 1        |
  | .ZZF | .ZZB | .ZZA  |            |     20 | 1           | 1           | 0        |
  When member ".ZZA" visits page "account/relations"
  Then we show "Relations" with:
  | Other      | My employer? | My employee? | Family? | Permission    |_request rCard |
  | Bea Two    | Yes          | --           | Yes     | send invoices | --       |
  | Corner Pub | No           | --           | No      | --            | --       |
  | Dee Four   | Yes          | Yes          | Yes     | send invoices | --       |
  When member ".ZZB" visits page "account/relations"
  Then we show "Relations" with:
  | Other   | My employee? | Family? | Permission |_request rCard |
  | Abe One | Yes          | No      | no access  | --            |
  When member ".ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Other   | My employer? | My employee? | Family? | Permission |_request rCard |
  | Abe One | No           | No           | No      | no access  | --            |
  And with:
  |_Header  |
  | Family? |
  When member ":ZZD" visits page "account/relations"
  Then we show "Relations" with:
  | Other   | My employee? | Owner? | Permission     |_request rCard |
  | Abe One | Yes          | No     | manage account | --            |
  And without:
  |_Header    |
  | employer? |

Scenario: A member adds a relation
  When member ".ZZA" completes form "account/relations" with values:
  | newPerson |
  | beatwo    |
  Then we say "status": "report new relation" with subs:
  | who     |
  | Bea Two |
  And we show "Relations" with:
  | Other      | Draw | My employer? | My employee? | Family? | Permission   |_request rCard |
  | Bea Two    | No   | No           | --           | No      | no access    |               |
  