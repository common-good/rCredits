Feature: Change Account or Agent
As a member
I WANT to temporarily change the agent or account associated with a device
SO I can use it (or assign someone) to buy and sell with rCredits, on behalf of another member.



Setup:
  Given members:
  | id      | fullName  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@ | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@ | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@ | Ctown | Corse  | France        |
  And devices:
  | id      | code  | default_agent |
  | NEW.ZZA | codeA | NEW.ZZA       |
  | NEW.ZZB | codeB | NEW.ZZB       |
  | NEW.ZZC | codeC | NEW.ZZB       |
  And relations:
  | id      | main    | agent   | permission        |
  | NEW.ZZA | NEW.ZZA | NEW.ZZB | buy and sell      |
  | NEW.ZZB | NEW.ZZB | NEW.ZZC | sell              |
  | NEW.ZZC | NEW.ZZC | NEW.ZZA | read transactions |
  | NEW.ZZD | NEW.ZZC | NEW.ZZB |                   |
  | NEW.ZZE | NEW.ZZB | NEW.ZZA | read transactions |
  | NEW.ZZF | NEW.ZZA | NEW.ZZC | sell              |

#Variants: changing to compound account/agent
#  | NEW. |
#  | NEW. |

Scenario: A member changes agent
  Given device "codeB" account is "NEW.ZZB" and agent is "NEW.ZZB"
  When a member asks device "codeB" to change "agent" to "NEW.ZZC"
  Then we respond success 1, my_id "NEW.ZZB", account_name "Bea Two~Agent: Corner Pub", show_buttons 1, and message "changed agent", with subs:
  | what  | agentName  |
  | agent | Corner Pub |
# tilda is a stand-in for EOL

Scenario: A member changes account
  Given device "codeA" account is "NEW.ZZA" and agent is "NEW.ZZA"
  When a member asks device "codeA" to change "account" to "NEW.ZZC"
  Then we respond success 1, my_id "NEW.ZZC", account_name "Corner Pub~Agent: Abe One", show_buttons 0, and message "changed account", with subs:
  | what    | accountName |
  | account | Corner Pub  |

Scenario: A member changes to different account AND agent
  Given device "codeC" account is "NEW.ZZB" and agent is "NEW.ZZC"
  When a member asks device "codeC" to change "agent" to "NEW.ZZA"
  Then we respond success 1, my_id "NEW.ZZE", account_name "Bea Two~Agent: Abe One", show_buttons 0, and message "changed agent", with subs:
  | what  | agentName  |
  | agent | Abe One    |
  
Scenario: A member changes agent back to default account
  Given device "codeC" account is "NEW.ZZC" and agent is "NEW.ZZA"
  When a member asks device "codeC" to change "agent" to "NEW.ZZC"
  Then we respond success 1, my_id "NEW.ZZC", account_name "Corner Pub", show_buttons 3, and message "changed agent", with subs:
  | what  | agentName  |
  | agent | Corner Pub |

Scenario: A member changes account back to the default account
  Given device "codeA" account is "NEW.ZZC" and agent is "NEW.ZZA"
  When a member asks device "codeA" to change "account" to "NEW.ZZA"
  Then we respond success 1, my_id "NEW.ZZA", account_name "Abe One", show_buttons 3, and message "changed account", with subs:
  | what    | accountName |
  | account | Abe One     |

Scenario: A member changes agent to same as current non-default account
  Given device "codeC" account is "NEW.ZZA" and agent is "NEW.ZZB"
  When a member asks device "codeC" to change "agent" to "NEW.ZZA"
  Then we respond success 1, my_id "NEW.ZZA", account_name "Abe One", show_buttons 1, and message "changed agent", with subs:
  | what  | agentName  |
  | agent | Abe One    |

Scenario: A member changes account to same as current non-default agent
  Given device "codeC" account is "NEW.ZZA" and agent is "NEW.ZZB"
  When a member asks device "codeC" to change "account" to "NEW.ZZB"
  Then we respond success 1, my_id "NEW.ZZB", account_name "Bea Two", show_buttons 1, and message "changed account", with subs:
  | what    | accountName |
  | account | Bea Two     |

Scenario: A member omits the account_id
  Given device "codeC" account is "NEW.ZZA" and agent is "NEW.ZZB"
  When a member asks device "codeC" to change "account" to ""
  Then we respond with:
  | success | message            |
  | 0       | missing account_id |

Scenario: A member omits the type of change
  Given device "codeC" account is "NEW.ZZA" and agent is "NEW.ZZB"
  When a member asks device "codeC" to change "" to "NEW.ZZB"
  Then we respond with:
  | success | message  |
  | 0       | bad what |

Scenario: A member types a bad type of change
  Given device "codeC" account is "NEW.ZZA" and agent is "NEW.ZZB"
  When a member asks device "codeC" to change %random to "NEW.ZZB"
  Then we respond with:
  | success | message  |
  | 0       | bad what |
