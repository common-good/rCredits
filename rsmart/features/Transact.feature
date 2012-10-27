Feature: Transact
AS a member
I WANT to transfer rCredits to or from another member (acting on their own behalf)
SO I can buy and sell stuff.
# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc.
# And foreigner (member on a different server) to member, etc.

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
  | tx_id    | created   | type       | amount | from      | to      | purpose | taking |
  | NEW.AAAB | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZA | signup  | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZB | signup  | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZC | signup  | 0      |
  Then balances:
  | id        | balance |
  | community |    -750 |
  | NEW.ZZA   |     250 |
  | NEW.ZZB   |     250 |
  | NEW.ZZC   |     250 |

Variants:
  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # member to member (pro se) |
  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW.ZZA" $ | "NEW.ZZC" $ | # agent to member           |
  | "NEW.ZZA" asks device "codeA" | "NEW.ZZC" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # member to agent           |
  | "NEW.ZZB" asks device "codeA" | "NEW.ZZB" asks device "codeC" | "NEW:ZZA" $ | "NEW:ZZC" $ | # agent to agent            |

Scenario: A member asks to charge another member
  When member "NEW.ZZA" asks device "codeA" to do this: "charge" "NEW.ZZC" $100 ("goods": "labor")
  # cash exchange would be ("cash": "")
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 250 other_balance "" and message "report invoice" with subs:
  | action  | other_name | amount | tid |
  | charged | Corner Pub | $100   | 2   |
  # "You charged Corner Pub $100 (bonus: $10). Your balance is unchanged, pending payment. Invoice #2"
  And we email "new-invoice" to member "c@example.com" with subs:
  | created | full_name  | other_name | amount | payer_purpose |
  | %today  | Corner Pub | Abe One    | $100   | labor         |
  And balances:
  | id        | balance |
  | community |    -750 |
  | NEW.ZZA   |     250 |
  | NEW.ZZC   |     250 |

Scenario: A member asks to pay another member
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $100 ("goods": "groceries")
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 155 other_balance "" and message "report transaction" with subs:
  | action | other_name | amount | reward_type | reward_amount | balance | tid |
  | paid   | Corner Pub | $100   | rebate      | $5            | $155    | 2   |
  # "You paid Corner Pub $100 (rebate: $5). Your new balance is $155. Transaction #2"
  And we email "new-payment" to member "c@example.com" with subs:
  | created | full_name  | other_name | amount | payee_purpose   |
  | %today  | Corner Pub | Abe One    | $100   | groceries       |
  And balances:
  | id        | balance |
  | community |    -765 |
  | NEW.ZZA   |     155 |
  | NEW.ZZC   |     360 |
#Request: 
#my_id (account ID of agent -- defaults to owner_id)
#code (permanent code received in First Time response)
#op=”transact”
#type (“charge” or “pay”)
#account_id (value of scanned QR code)
#amount (numeric dollar amount)
#goods=TRUE or FALSE (true unless user checks “cash, loan, etc.”)
#purpose (description of goods and services)
#Response: 
#success=TRUE or FALSE
#message (error message or success message)
#tx_id (transaction ID number, if success, otherwise empty string)
#my_balance (user’s new balance)
#other_balance (new balance for the other party -- do not show the “Show Customer Balance” button if this is omitted)

Scenario: A member asks to charge another member unilaterally
  Given member "NEW.ZZC" can charge unilaterally
  When member "NEW.ZZC" asks device "codeC" to do this: "charge" "NEW.ZZA" $100 ("goods": "groceries")
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 360 other_balance 155 and message "report transaction" with subs:
  | action  | other_name | amount | reward_type | reward_amount | balance | tid |
  | charged | Abe One    | $100   | bonus       | $10           | $360    | 2   |
  # "You charged Corner Pub $100 (bonus: $10). Your new balance is $360. Transaction #2"
  And we email "new-charge" to member "a@example.com" with subs:
  | created | full_name | other_name | amount | payer_purpose   |
  | %today  | Abe One   | Corner Pub | $100   | groceries       |
  And balances:
  | id        | balance |
  | community |    -765 |
  | NEW.ZZA   |     155 |
  | NEW.ZZC   |     360 |

Scenario: A member asks to charge another member unilaterally, with insufficient balance
  Given member "NEW.ZZC" can charge unilaterally
  When member "NEW.ZZC" asks device "codeC" to do this: "charge" "NEW.ZZA" $300 ("goods": "groceries")
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 525 other_balance 12.5 and message "report short transaction" with subs:
  | action  | other_name | amount | short | balance | tid |
  | charged | Abe One    | $250   | $50   | $525    | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member asks to pay another member, with insufficient balance
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $300 ("goods": "groceries")
  Then we respond success 1 tx_id "NEW.AAAE" my_balance 12.5 other_balance "" and message "report short transaction" with subs:
  | action | other_name | amount | short | balance | tid |
  | paid   | Corner Pub | $250   | $50   | $12.50  | 2   |
  # "SPLIT TRANSACTION! You paid Corner Pub $250 (rebate: $12.50). You will need to use US Dollars for the remaining $50. Your new balance is $12.50. Transaction #2"
  And balances:
  | id        | balance |
  | community | -787.50 |
  | NEW.ZZA   |   12.50 |
  | NEW.ZZC   |  525.00 |

Scenario: A member asks to pay self
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZA" $300 ("goods": "groceries")
  Then we respond with:
  | success | message         |
  | 0       | no self-trading |

Scenario: Device gives no account id
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "" $300 ("goods": "groceries")
  Then we respond with:
  | success | message            |
  | 0       | missing account id |
  
Scenario: Device gives bad account id
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" %whatever $300 ("goods": "groceries")
  Then we respond with:
  | success | message        |
  | 0       | bad account id |

Scenario: Device gives no amount
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $"" ("goods": "groceries")
  Then we respond with:
  | success | message        |
  | 0       | missing amount |
  
Scenario: Device gives bad amount
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $%whatever ("goods": "groceries")
  Then we respond with:
  | success | message    |
  | 0       | bad amount |
  
Scenario: Device gives nonpositive amount
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $-100 ("goods": "groceries")
  Then we respond with:
  | success | message    |
  | 0       | nonpositive transfer |

Scenario: Device gives too big an amount
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $10,000,000 ("goods": "groceries")
  Then we respond with:
  | success | message    |
  | 0       | nonpositive transfer |

Scenario: Device gives no type
  When member "NEW.ZZA" asks device "codeA" to do this: "" "NEW.ZZC" $300 ("goods": "groceries")
  Then we respond with:
  | success | message                  |
  | 0       | missing transaction type |
  
Scenario: Device gives bad type
  When member "NEW.ZZA" asks device "codeA" to do this: %whatever "NEW.ZZC" $300 ("goods": "groceries")
  Then we respond with:
  | success | message |
  | 0       | bad transaction type |

Scenario: Device gives no purpose for goods and services
  When member "NEW.ZZA" asks device "codeA" to do this: "pay" "NEW.ZZC" $300 ("goods": "")
  Then we respond with:
  | success | message         |
  | 0       | missing purpose |

Scenario: Buyer agent lacks permission to buy
  When member " NEW.ZZA " asks device "codeC" to do this: "pay" "NEW.ZZB" $300 ("goods": "groceries")
  Then we respond with:
  | success | message         |
  | 0       | no buy and sell |

Scenario: Seller agent lacks permission to sell
  When member " NEW.ZZA " asks device "codeB" to do this: "charge" "NEW.ZZC" $300 ("goods": "groceries")
  Then we respond with:
  | success | message |
  | 0       | no sell |
