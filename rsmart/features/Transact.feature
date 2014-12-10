Feature: Transact
AS a company agent
I WANT to transfer rCredits to or from another member
SO my company can sell stuff and give refunds.

# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc. and foreigner (member on a different server) to member, etc.
# cc means cardCode
# while testing, bonus is artificially set at twice the rebate amount

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags      | *
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
  | xid | created   | type   | amount | from | to   | purpose |*
  | 1   | %today-6m | signup |    250 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup |    250 | ctty | .ZZB | signup  |
  | 3   | %today-6m | signup |    250 | ctty | .ZZC | signup  |
  | 4   | %today-6m | grant  |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | id   | balance |*
  | ctty |   -1000 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |
  | .ZZF |     250 |

#Variants: with/without an agent
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent  |

Scenario: A cashier asks to charge someone
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $100 for "goods": "food" at %now
  # cash exchange would be for "cash": "cash out"
  Then we respond ok txid 5 created %now balance 160 rewards 260
  And with message "report tx|reward" with subs:
  | did     | otherName | amount | why                | rewardAmount |*
  | charged | Bea Two   | $100   | goods and services | $5           |
  And with did
  | did     | amount | forCash |*
  | charged | $100   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | from   | Bea Two   |
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $10               |
  And balances:
  | id   | balance |*
  | ctty |   -1015 |
  | .ZZA |     250 |
  | .ZZB |     160 |
  | .ZZC |     355 |

Scenario: A cashier asks to refund someone
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $-100 for "goods": "food" at %now
  Then we respond ok txid 5 created %now balance 340 rewards 240
  And with message "report tx|reward" with subs:
  | did      | otherName | amount | why                | rewardAmount |*
  | refunded | Bea Two   | $100   | goods and services | $-5          |
  And with did
  | did      | amount | forCash |*
  | refunded | $100   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | to     | Bea Two   |
  And we notice "new refund|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $-10              |
  And balances:
  | id   | balance |*
  | ctty |    -985 |
  | .ZZA |     250 |
  | .ZZB |     340 |
  | .ZZC |     145 |

Scenario: A cashier asks to charge another member, with insufficient balance
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $300 for "goods": "food" at %now
  Then we return error "short from" with subs:
  | otherName | short |*
  | Bea Two   | $50   |

Scenario: A cashier asks to refund another member, with insufficient balance
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $-300 for "goods": "food" at %now
  Then we return error "short to" with subs:
  | short |*
  | $50   |

Scenario: A cashier asks to pay self
  When agent ":ZZA" asks device "devC" to charge ".ZZC" $300 for "goods": "food" at %now
  Then we return error "shoulda been login"

Scenario: Device gives no member id
  When agent ":ZZA" asks device "devC" to charge "" $300 for "goods": "food" at %now
  Then we return error "missing member"
  
Scenario: Device gives bad account id
  When agent ":ZZA" asks device "devC" to charge %whatever $300 for "goods": "food" at %now
  Then we return error "bad customer"

Scenario: Device gives no amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $"" for "goods": "food" at %now
  Then we return error "bad amount"
  
Scenario: Device gives bad amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $%whatever for "goods": "food" at %now
  Then we return error "bad amount"
  
Scenario: Device gives too big an amount
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $10,000,000 for "goods": "food" at %now
  Then we return error "amount too big"

Scenario: Device gives no purpose for goods and services
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $100 for "goods": "" at %now
  Then we return error "missing description"

Scenario: Seller agent lacks permission to buy
  When agent ":ZZB" asks device "devC" to charge ".ZZB" $-100 for "goods": "refund" at %now
  Then we return error "no perm" with subs:
  | what    |*
  | refunds |

Scenario: Seller agent lacks permission to scan and sell
  When agent ":ZZD" asks device "devC" to charge ".ZZA" $100 for "goods": "food" at %now
  Then we return error "no perm" with subs:
  | what  |*
  | sales |
  
Scenario: Buyer agent lacks permission to buy
  When agent ":ZZA" asks device "devC" to charge ":ZZE" $100 for "goods": "food" at %now
  Then we return error "other no perm" with subs:
  | otherName | what      |*
  | Eve Five  | purchases |

Scenario: Seller tries to charge the customer twice
  Given agent ":ZZA" asks device "devC" to charge ".ZZB" $100 for "goods": "food" at "%now-1min"
  When agent ":ZZA" asks device "devC" to charge ".ZZB" $100 for "goods": "food" at %now
  Then we return error "duplicate transaction" with subs:
  | op      |*
  | charged |
