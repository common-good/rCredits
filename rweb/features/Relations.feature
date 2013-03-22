Feature: Relations
AS a member
I WANT to manage my account's relations with other rCredits accounts
SO I can buy and sell stuff on behalf of other accounts, and they on mine.

Setup:
  Given members:
  | id   | full_name  | account_type     | flags         |
  | .ZZA | Abe One    | %R_PERSONAL      | %BIT_DEFAULTS |
  | .ZZB | Bea Two    | %R_SELF_EMPLOYED | %BIT_MEMBER   |
  | .ZZC | Corner Pub | %R_COMMERCIAL    | %BIT_RTRADER  |
  | .ZZD | Dee Four   | %R_PERSONAL      | 0             |
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
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | sell         |     10 | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Dee Four   | No           | Yes          | Yes     | sell         | print ID Card |

Scenario: Member has an employee, unconfirmed
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | buy and sell |     50 | 1           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Dee Four   | No           | Yes          | No      | buy and sell |          |

Scenario: Member has a relation with a contractor
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | buy and sell |     50 | 1           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Bea Two    | No           | --           | No      | buy and sell |          |
  
Scenario: Member has an employee, claimed
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | sell         |     10 | 0           | 1           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Dee Four   | No           | No           | No        | sell       |          |
  
Scenario: Employee can only read
  Given relations:
  | id | main | agent | permission        | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | read transactions |     10 | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission        | Print ID |
  | Dee Four   | No           | Yes          | Yes     | read transactions |          |
  
Scenario: Member has an employer
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZB | .ZZA  | sell         |     10 | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Bea Two    | Yes          | --           | No      | no access    | --       |
  
Scenario: Member has access to employee account
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | no access    |     10 | 1           | 1           | 1        |
  | 2  | .ZZD | .ZZA  | sell         |     20 | 0           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Dee Four   | No           | Yes          | Yes     | no access    | --       |
  When member ".ZZD" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Abe One    | Yes          | No           | No      | sell         | --            |

Scenario: Member company has relations
  Given relations:
  | id | main | agent | permission   | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZC | .ZZA  | sell         |     10 | 1           | 1           | 1        |
  When member ".ZZC" visits page "relations"
  Then we show page "relations" with:
  | Person  | Amount | My employee? | Is owner? | Permission | Print ID      |
  | Abe One | 10     | Yes          | Yes       | sell       | print ID Card |
  And we show page "relations" without:
  | Header       |
  | My employer? |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Corner Pub | Yes          | --           | No      | no access    | --       |

Scenario: It's complicated
  Given relations:
  | id | main | agent | permission     | amount | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZD  | sell           |     10 | 1           | 0           | 1        |
  | 2  | .ZZD | .ZZA  | no access      |     20 | 0           | 1           | 0        |
  | 3  | .ZZA | .ZZC  | buy and sell   |     30 | 0           | 1           | 0        |
  | 4  | .ZZC | .ZZA  | manage account |     40 | 1           | 0           | 0        |
  | 5  | .ZZA | .ZZB  | sell           |     10 | 1           | 0           | 1        |
  | 6  | .ZZB | .ZZA  | no access      |     20 | 1           | 1           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | My employer? | My employee? | Family? | Permission   | Print ID |
  | Bea Two    | Yes          | --           | Yes     | sell         | --       |
  | Corner Pub | No           | --           | No      | buy and sell | --       |
  | Dee Four   | Yes          | Yes          | Yes     | sell         | --       |
  When member ".ZZB" visits page "relations"
  Then we show page "relations" with:
  | Person  | Amount | My employee? | Family? | Permission | Print ID      |
  | Abe One | 20     | Yes          | No      | no access  | --            |
  When member ".ZZD" visits page "relations"
  Then we show page "relations" with:
  | Person  | My employer? | My employee? | Family? | Permission | Print ID      |
  | Abe One | No           | No           | No      | no access  | --            |
  And we show page "relations" with:
  | Header  |
  | Family? |
  When member ".ZZC" visits page "relations"
  Then we show page "relations" with:
  | Person  | Amount | My employee? | Is owner? | Permission     | Print ID      |
  | Abe One | 40     | Yes          | No        | manage account | --            |
  And we show page "relations" without:
  | Header    |
  | employer? |
