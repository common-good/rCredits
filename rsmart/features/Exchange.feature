Feature: Exchange
AS a company agent
I WANT to transfer rCredits to or from another member in exchange for cash
SO my company can accept cash deposits and give customers cash.

# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc. and foreigner (member on a different server) to member, etc.
# cc means cardCode
# while testing, bonus is artificially set at twice the rebate amount

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags      |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |     10 | ok,bona    |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |     10 | ok,bona    |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,co,bona |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 |     10 | ok,bona    |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 |     10 | ok,bona,secret |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      |      5 | ok,co,bona |
  And devices:
  | id   | code |*
  | .ZZC | devC |
  And selling:
  | id   | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | id   | flags        |*
  | .ZZC | refund,r4usd |
  And relations:
  | id   | main | agent | permission |*
  | :ZZA | .ZZC | .ZZA  | buy        |
  | :ZZB | .ZZC | .ZZB  | scan       |
  | :ZZD | .ZZC | .ZZD  | read       |
  | :ZZE | .ZZF | .ZZE  | sell       |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  | 1   | %today-6m | signup   |    350 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup   |    150 | ctty | .ZZB | signup  |
  | 3   | %today-6m | signup   |    250 | ctty | .ZZC | signup  |
  | 4   | %today-5m | transfer |    100 | .ZZC | .ZZB | cash    |
  | 5   | %today-5m | transfer |    200 | .ZZA | .ZZC | cash    |
  | 6   | %today-4m | grant    |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | id   | balance | rewards |*
  | ctty |   -1000 |         |
  | .ZZA |     150 |     350 |
  | .ZZB |     250 |     150 |
  | .ZZC |     350 |     250 |
  | .ZZF |     250 |       0 |

#Variants: with/without an agent
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent  |

Scenario: A cashier asks to charge someone for cash
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $100 for "cash": "cash out" at %now
  Then we respond ok txid 7 created %now balance 150 rewards 150
  And with message "report tx" with subs:
  | did     | otherName | amount | why         |*
  | charged | Bea Two   | $100   | other money |
  And with did
  | did     | amount | forCash  |*
  | charged | $100   | for cash |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | from   | Bea Two   |
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | cash out     |
  And balances:
  | id   | balance |*
  | ctty |   -1000 |
  | .ZZA |     150 |
  | .ZZB |     150 |
  | .ZZC |     450 |

Scenario: A cashier asks to refund someone
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $-100 for "cash": "cash in" at %now
  Then we respond ok txid 7 created %now balance 350 rewards 150
  And with message "report tx" with subs:
  | did      | otherName | amount | why         |*
  | credited | Bea Two   | $100   | other money |
  And with did
  | did      | amount | forCash  |*
  | credited | $100   | for cash |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | to     | Bea Two   |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payeePurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | cash in      |
  And balances:
  | id   | balance |*
  | ctty |   -1000 |
  | .ZZA |     150 |
  | .ZZB |     350 |
  | .ZZC |     250 |

Scenario: A cashier asks to charge another member, with insufficient balance
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $300 for "cash": "cash out" at %now
  Then we return error "short from" with subs:
  | otherName | short |*
  | Bea Two   | $200  |

Scenario: A cashier asks to refund another member, with insufficient balance
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $-400 for "cash": "cash in" at %now
  Then we return error "short to" with subs:
  | short |*
  | $300  |

Scenario: A cashier asks to pay self
  When agent ":ZZA" asks device "devC" to charge ".ZZC-ccC" $300 for "cash": "cash out" at %now
  Then we return error "shoulda been login"

Scenario: Device gives no member id
  When agent ":ZZA" asks device "devC" to charge "" $300 for "cash": "cash out" at %now
  Then we return error "missing member"
  
Scenario: Device gives bad account id
  When agent ":ZZA" asks device "devC" to charge %whatever $300 for "cash": "cash out" at %now
  Then we return error "bad customer"

Scenario: Device gives no amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $"" for "cash": "cash out" at %now
  Then we return error "bad amount"
  
Scenario: Device gives bad amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $%whatever for "cash": "cash out" at %now
  Then we return error "bad amount"
  
Scenario: Device gives too big an amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $10,000,000 for "cash": "cash out" at %now
  Then we return error "amount too big" with subs:
  | max           |*
  | %R_MAX_AMOUNT |

Scenario: Device gives no purpose for goods and services
  When agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $100 for "goods": "" at %now
  Then we return error "missing description"

Scenario: Seller agent lacks permission to buy
  When agent ":ZZB" asks device "devC" to charge ".ZZB-ccB" $-100 for "goods": "refund" at %now
  Then we return error "no perm" with subs:
  | what    |*
  | refunds |

Scenario: Seller agent lacks permission to scan and sell
  When agent ":ZZD" asks device "devC" to charge ".ZZA-ccA" $100 for "cash": "cash out" at %now
  Then we return error "no perm" with subs:
  | what  |*
  | sales |
  
Scenario: Buyer agent lacks permission to buy
  When agent ":ZZA" asks device "devC" to charge ":ZZE-ccE2" $100 for "cash": "cash out" at %now
  Then we return error "other no perm" with subs:
  | otherName | what      |*
  | Eve Five  | purchases |
  
Scenario: Device sends wrong card code
  When agent ":ZZA" asks device "devC" to charge ".ZZB-whatever" $100 for "cash": "cash out" at %now
  Then we return error "bad customer"
  