Feature: Undo Transaction
AS a managing agent for an rCredits member company
I WANT to undo the last transaction completed on the POS device I am using
SO I can easily correct a mistake made by another company agent or by me

Summary:
  An agent asks to undo a charge
  An agent asks to undo a refund
  An agent asks to undo a cash in payment
  An agent asks to undo a cash out charge
  An agent asks to undo a charge, with insufficient balance  
  An agent asks to undo a refund, with insufficient balance
  
Setup:
  Given members:
  | id   | fullName   | email | cc  | cc2  | floor | flags                |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt    |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt    |
  | .ZZC | Corner Pub | c@    | ccC |      |  -250 | ok,confirmed,co,debt |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed         |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |     0 | ok,confirmed,secret  |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,confirmed,co      |
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
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | scan       |
  | .ZZC | .ZZB  |   2 | refund     |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | taking |*
  | 1   | %today-7m | signup   |    250 | ctty | .ZZA | signup       |      0 |
  | 2   | %today-6m | signup   |    250 | ctty | .ZZB | signup       |      0 |
  | 3   | %today-6m | signup   |    250 | ctty | .ZZC | signup       |      0 |
  Then balances:
  | id   | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

#Variants: with/without an agent
#  | ".ZZA" asks device "devC" | ".ZZC" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # member to member (pro se) |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member           |
#  | ".ZZA" asks device "devC" | ".ZZC" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # member to agent           |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent            |

Scenario: An agent asks to undo a charge
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d | transfer |     80 | .ZZA | .ZZC | whatever     | %FOR_GOODS |      1 |
  When agent "C:B" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description | created   |*
  | .ZZA   | ccA  | 80.00  |     1 | whatever    | %today-1d |
#  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new refund" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose           |*
  | %today  | Corner Pub | $80    | whatever (reverses #2)  |

Scenario: An agent asks to undo a charge when balance is secret
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | taking |*
  | 4   | %today-6m | signup   |    250 | ctty | .ZZE | signup       |      0 |
  | 5   | %today-1d | transfer |     80 | .ZZE | .ZZC | whatever     |      1 |
  When agent "C:B" asks device "devC" to undo transaction 5 code "ccE"
  Then we respond ok txid 6 created %now balance "*0" rewards 250 saying:
  | solution | did      | otherName | amount | why   | reward |*
  | reversed | refunded | Eve Five  | $80    | goods | $-4    |
  And with did ""
  And with undo "5"
  And we notice "new refund" to member ".ZZE" with subs:
  | created | otherName  | amount | payerPurpose           |*
  | %today  | Corner Pub | $80    | whatever (reverses #2)  |

Scenario: An agent asks to undo a refund
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | taking |*
  | 4   | %today-1d | transfer |    -80 | .ZZA | .ZZC | refund       |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did        | otherName | amount | why   | reward |*
  | reversed | re-charged | Abe One   | $80    | goods | $4     |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose         |*
  | %today  | Corner Pub | $80    | refund (reverses #2)  |

Scenario: An agent asks to undo a cash-out charge
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose  | goods      | taking |*
  | 4   | %today-1d | transfer |     80 | .ZZA | .ZZC | cash out | %FOR_USD |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did      | otherName | amount | why |*
  | reversed | credited | Abe One   | $80    | usd |
  And with did ""
  And with undo "4"
  And we notice "new payment linked" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payeePurpose           | aPayLink |*
  | %today  | Abe One  | Corner Pub | $80    | cash out (reverses #2)  | ?        |

Scenario: An agent asks to undo a cash-in payment
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose | goods      | taking |*
  | 4   | %today-1d | transfer |    -80 | .ZZA | .ZZC | cash in | %FOR_USD |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did        | otherName | amount | why |*
  | reversed | re-charged | Abe One   | $80    | usd |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Abe One  | Corner Pub | $80    | cash in (reverses #2)  |

Scenario: An agent asks to undo a charge, with insufficient balance  
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d | transfer |     80 | .ZZA | .ZZC | whatever     | %FOR_GOODS |      1 |
  | 5   | %today    | transfer |    300 | .ZZC | .ZZB | cash out     | %FOR_USD   |      0 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 6 created %now balance 0 rewards 250 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new refund" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose           |*
  | %today  | Corner Pub | $80    | whatever (reverses #2)  |
  And balances:
  | id   | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |     300 |
  | .ZZC |    -300 |

Scenario: An agent asks to undo a refund, with insufficient balance  
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d | transfer |    -80 | .ZZA | .ZZC | refund       | %FOR_GOODS |      1 |
  | 5   | %today    | transfer |    300 | .ZZA | .ZZB | cash out     | %FOR_USD   |      0 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 6 created %now balance -300 rewards 250 saying:
  | solution | did        | otherName | amount | why   |*
  | reversed | re-charged | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose         |*
  | %today  | Corner Pub | $80    | refund (reverses #2)  |
  And balances:
  | id   | balance |*
  | ctty |       0 |
  | .ZZA |    -300 |
  | .ZZB |     300 |
  | .ZZC |       0 |

Scenario: An agent asks to undo a charge, without permission
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d | transfer |     80 | .ZZB | .ZZC | whatever     | %FOR_GOODS |      1 |
  When agent "C:A" asks device "devC" to undo transaction 4 code "ccB"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Bea Two   | $80    | goods |
#  Then we return error "no perm" with subs:
#  | what    |*
#  | refunds |
#  And we notice "bad forced tx|no perm" to member ".ZZC" with subs:
#  | what    | account | amount | created | by                  |*
#  | refunds | Bea Two | $80    | %dmy-1d | %(" agent Abe One") |

Scenario: An agent asks to undo a refund, without permission
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d | transfer |    -80 | .ZZB | .ZZC | refund       | %FOR_GOODS |      1 |
  When agent "C:D" asks device "devC" to undo transaction 4 code "ccB"
  Then we respond ok txid 5 created %now balance 0 rewards 250 saying:
  | solution | did        | otherName | amount | why   |*
  | reversed | re-charged | Bea Two   | $80    | goods |
#  Then we return error "no perm" with subs:
#  | what  |*
#  | sales |

Scenario: An agent asks to undo a non-existent transaction
#  When agent "C:A" asks device "devC" to undo transaction 99 code %whatever
  When agent "C:B" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description   | created   |*
  | .ZZA   | ccA  | 80.00  |     1 | neverhappened | %today-1d |
  Then we respond ok txid 0 created "" balance 0 rewards 250
  And with did ""
  And with undo ""

Scenario: A cashier reverses a transaction with insufficient funds
  Given transactions: 
  | xid | created   | type  | amount | from | to   | purpose |*
  | 4   | %today-1m | grant |    100 | ctty | .ZZC | jnsaqwa |
  And agent "C:B" asks device "devC" to charge ".ZZA,ccA" $-100 for "cash": "cash in" at "%now-1h" force 0
  Then transactions: 
  | xid | created    | type     | amount | from | to   | purpose |*
  | 5   | %now-1h | transfer |   -100 | .ZZA | C:B  | cash in |
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose |*
  | 6   | %today  | transfer |      1 | .ZZA | .ZZB | cash    |
  When agent "C:B" asks device "devC" to charge ".ZZA,ccA" $-100 for "cash": "cash in" at "%now-1h" force -1
  Then we respond ok txid 7 created %now balance -1 rewards 250
#  And with proof of agent "C:B" amount -100.00 created "%now-1h" member ".ZZA" code "ccA"
  And with undo "5"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payerPurpose          |*
  | %today  | Bea Two  | Corner Pub | $100   | cash in (reverses #2)  |
  And balances:
  | id   | balance |*
  | ctty |    -100 |
  | .ZZA |      -1 |
  | .ZZB |       1 |
  | .ZZC |     100 |
  