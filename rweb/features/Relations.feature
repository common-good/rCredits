Feature: Relations
AS a member
I WANT to manage my account's relations with other rCredits accounts
SO I can buy and sell stuff on behalf of other accounts, and they on mine.

Setup:
  Given members:
  | id   | full_name  | account_type  | flags         |
  | .ZZA | Abe One    | %R_PERSONAL   | %BIT_DEFAULTS |
  | .ZZB | Bea Two    | %R_PERSONAL   | %BIT_PARTNER  |
  | .ZZC | Corner Pub | %R_COMMERCIAL | %BIT_RTRADER  |
  And transactions: 
  | tx_id | created   | type       | amount | from      | to   | purpose | taking |
  | :AAAB | %today-6m | %TX_SIGNUP |    250 | community | .ZZA | signup  | 0      |
  | :AAAC | %today-6m | %TX_SIGNUP |    250 | community | .ZZB | signup  | 0      |
  | :AAAD | %today-6m | %TX_SIGNUP |    250 | community | .ZZC | signup  | 0      |
  Then "asif" balances:
  | id        | balance |
  | community |    -750 |
  | .ZZA      |     250 |
  | .ZZB      |     250 |
  | .ZZC      |     250 |

Scenario: Member has an employee, confirmed
  Given relations:
  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | sell       | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print id      |
  | Bea Two | No           | Yes          | Yes       | sell       | print ID Card |

Scenario: Member has an employee, unconfirmed
  Given relations:
  | id | main | agent | permission   | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | buy and sell | 1           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission   | Print ID |
  | Bea Two | No           | Yes          | No        | buy and sell |          |
  
Scenario: Member has an employee, claimed
  Given relations:
  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | sell       | 0           | 1           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Bea Two | No           | No           | No        | sell       |          |
  
Scenario: Employee can only read
  Given relations:
  | id | main | agent | permission        | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | read transactions | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission        | Print ID |
  | Bea Two | No           | Yes          | Yes       | read transactions |          |
  
Scenario: Member has an employer
  Given relations:
  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
  | 1  | .ZZB | .ZZA  | sell       | 1           | 1           | 1        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Bea Two | Yes          | No           | No        | no access  | --       |
  
Scenario: Member has access to employee account
  Given relations:
  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | no access  | 1           | 1           | 1        |
  | 2  | .ZZB | .ZZA  | sell       | 0           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Bea Two | No           | Yes          | Yes       | no access  | --       |
  When member ".ZZB" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Abe One | Yes          | No           | No        | sell       | --       |

Scenario: Member company has relations
  Given relations:
  | id | main | agent | permission | employer_ok | employee_ok | is_owner |
  | 1  | .ZZC | .ZZA  | sell       | 1           | 1           | 1        |
  When member ".ZZC" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employee? | Is owner? | Permission | Print ID      |
  | Abe One | Yes          | Yes       | sell       | print ID Card |
  And we show page "relations" without:
  | Header       |
  | Is employer? |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Corner Pub | Yes          | --           | No        | no access  | --       |

Scenario: It's complicated
  Given relations:
  | id | main | agent | permission     | employer_ok | employee_ok | is_owner |
  | 1  | .ZZA | .ZZB  | sell           | 1           | 0           | 1        |
  | 2  | .ZZB | .ZZA  | no access      | 0           | 1           | 0        |
  | 3  | .ZZA | .ZZC  | buy and sell   | 0           | 1           | 0        |
  | 4  | .ZZC | .ZZA  | manage account | 1           | 0           | 0        |
  When member ".ZZA" visits page "relations"
  Then we show page "relations" with:
  | Person     | Is employer? | Is employee? | Is owner? | Permission   | Print ID |
  | Bea Two    | Yes          | Yes          | Yes       | sell         | --       |
  | Corner Pub | No           | --           | No        | buy and sell | --       |
  When member ".ZZB" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employer? | Is employee? | Is owner? | Permission | Print ID |
  | Abe One | No           | No           | No        | no access  | --       |
  When member ".ZZC" visits page "relations"
  Then we show page "relations" with:
  | Person  | Is employee? | Is owner? | Permission     | Print ID |
  | Abe One | Yes          | No        | manage account | --       |
  And we show page "relations" without:
  | Header       |
  | Is employer? |
