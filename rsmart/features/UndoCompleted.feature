Feature: Undo Completed Transaction
AS a player
I WANT to undo a transaction recently completed on my account
SO I can easily correct a mistake

Summary:
  A member asks to undo a completed payment
  A member confirms request to undo a completed payment
  A member asks to undo a completed charge
  A member confirms request to undo a completed charge
  A member asks to undo a completed cash payment
  A member confirms request to undo a completed cash payment
  A member asks to undo a completed cash charge
  A member confirms request to undo a completed cash charge

#Variants:
#  | %TX_DONE     |
#  | %TX_DISPUTED |

Setup:
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
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
  | NEW.AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 0      |
  | NEW.AAAE | %today-2m | %TX_TRANSFER | %TX_DONE    |  11.11 | NEW.ZZB   | NEW.ZZA | cash taken   | 1      |
  | NEW.AAAF | %today-3w | %TX_TRANSFER | %TX_DONE    |  22.22 | NEW.ZZC   | NEW.ZZA | usd taken    | 1      |
  | NEW.AAAG | %today-3d | %TX_TRANSFER | %TX_DONE    |  33.33 | NEW.ZZA   | NEW.ZZB | whatever43   | 0      |
  | NEW.AAAH | %today-3d | %TX_REBATE   | %TX_DONE    |   1.67 | community | NEW.ZZA | rebate on #4 | 0      |
  | NEW.AAAI | %today-3d | %TX_BONUS    | %TX_DONE    |   3.33 | community | NEW.ZZB | bonus on #3  | 0      |
  | NEW.AAAJ | %today-2d | %TX_TRANSFER | %TX_DONE    |  44.44 | NEW.ZZB   | NEW.ZZC | cash given   | 0      |
  | NEW.AAAK | %today-1d | %TX_TRANSFER | %TX_DONE    |  55.55 | NEW.ZZA   | NEW.ZZC | whatever54   | 0      |
  | NEW.AAAL | %today-1d | %TX_REBATE   | %TX_DONE    |   2.78 | community | NEW.ZZA | rebate on #5 | 0      |
  | NEW.AAAM | %today-1d | %TX_BONUS    | %TX_DONE    |   5.56 | community | NEW.ZZC | bonus on #4  | 0      |
  Then balances:
  | id        | balance |
  | community | -763.34 |
  | NEW.ZZA   |  198.90 |
  | NEW.ZZB   |  231.11 |
  | NEW.ZZC   |  333.33 |

#Variants:
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # member to member (pro se) |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # agent to member           |
#  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # member to agent           |
#  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # agent to agent            |

Scenario: A member asks to undo a completed payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "NEW.AAAN", with the request "unconfirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-1d | $55.55 | to      | Corner Pub | whatever43 |
  # "Undo 01-02-2012 payment of $55.55 to Corner Pub for whatever?"
#Request:
#op = ”undo”
#tx_id (ID number of transaction to undo)
#confirmed = 0
#Response: 
#success = 0 or 1
#message (error message or message requesting confirmation)

Scenario: A member confirms request to undo a completed payment
  When member "NEW.ZZA" asks device "codeA" to undo transaction "", with the request "confirmed"
  Then we respond with success 1, message "confirm undo", and subs:
  | created   | amount | tofrom  | other_name | purpose    |
  | %today-1d | $55.55 | to      | Corner Pub | whatever43 |
  # "Undo 01-02-2012 payment of $55.55 to Corner Pub for whatever?"
#Request: 
#op = ”undo”
#confirmed = 1
#tx_id (ID number of transaction to undo)
#Response: 
#success = 0 or 1
#message (error message or success message)
#tx_id (ID number of offsetting transaction, if any (which could in turn be undone). tx_id not set means transaction was simply deleted, so there is no longer any transaction that can be undone.)
#my_balance (user’s new balance)
#other_balance (new balance for the other party -- do not show the “Show Customer Balance” button if this is omitted)


