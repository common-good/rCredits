Feature: adhoc
As a member
I WANT to temporarily change the agent or account associated with a device
SO I can use it (or assign someone) to buy and sell with rCredits, on behalf of another member.



Setup:
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com | Ctown | Corse  | France        |
  And devices:
  | id      | code  | default_agent |
  | NEW.ZZA | codeA | NEW.ZZA       |
  | NEW.ZZB | codeB | NEW.ZZB       |
  | NEW.ZZC | codeC | NEW.ZZB       |
  And relations:
  | id      | main    | agent   | permission        |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell      |
  | NEW:ZZB | NEW.ZZB | NEW.ZZC | sell              |
  | NEW:ZZC | NEW.ZZC | NEW.ZZA | read transactions |
  | NEW:ZZD | NEW.ZZC | NEW.ZZB |                   |
  | NEW:ZZE | NEW.ZZB | NEW.ZZA | read transactions |
  | NEW:ZZF | NEW.ZZA | NEW.ZZC | sell              |

Scenario: A member changes account
  Given device "codeA" account is "NEW.ZZB" and agent is "NEW.ZZA"
  When a member asks device "codeA" to change "account" to "NEW.ZZC"
  Then we respond success 1, my_id "NEW:ZZC", account_name "Corner Pub~Agent: Abe One", show_buttons 0, and message "changed account", with subs:
  | what    | account_name |
  | account | Corner Pub   |
