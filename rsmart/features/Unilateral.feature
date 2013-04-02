Feature: Insufficient Balance
AS a member
I WANT to do partial transactions in rCredits and be told when that is not possible
SO I can use rCredits as much as possible

Summary:
  
Variants:
  | %TX_DONE     |
  | %TX_DISPUTED |

#Variants: given/taken
  | 00000 |
  | 1     |

Setup:
  Given members:
  | id      | fullName  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com | Btown | Utah   | United States |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com | Ctown | Corse  | France        |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  | NEW.ZZB | codeB |
  | NEW.ZZC | codeC |
  And relations:
  | id      | main    | agent   | permission   |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA |              |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy and sell |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell         |
  And transactions: 
  | tx_id    | created   | type         | state       | amount | from      | to      | purpose      | taking |
  | NEW:AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 000000 |
  | NEW:AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 000000 |
  | NEW:AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 000000 |
  | NEW:AAAE | %today-3w | %TX_TRANSFER | %TX_DONE    |    200 | NEW.ZZA   | NEW.ZZB | whatever E   | 000000 |
  | NEW:AAAF | %today-3w | %TX_REBATE   | %TX_DONE    |     10 | community | NEW.ZZA | rebate on #2 | 000000 |
  | NEW:AAAG | %today-3w | %TX_BONUS    | %TX_DONE    |     20 | community | NEW.ZZB | bonus on #2  | 000000 |
  | NEW:AAAH | %today-3d | %TX_TRANSFER | %TX_DONE    |    100 | NEW.ZZC   | NEW.ZZA | labor H      | 000000 |
  | NEW:AAAI | %today-3d | %TX_REBATE   | %TX_DONE    |      5 | community | NEW.ZZC | rebate on #2 | 000000 |
  | NEW:AAAJ | %today-3d | %TX_BONUS    | %TX_DONE    |     10 | community | NEW.ZZA | bonus on #3  | 000000 |
  | NEW:AAAK | %today-2d | %TX_TRANSFER | %TX_DONE    |    100 | NEW.ZZA   | NEW.ZZB | cash I       | 000000 |
  Then balances:
  | id        | balance |
  | community |    -795 |
  | NEW.ZZA   |      70 |
  | NEW.ZZB   |     570 |
  | NEW.ZZC   |     155 |

#Variants: with/without an agent
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # member to member (pro se) |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # agent to member           |
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # member to agent           |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # agent to agent            |

