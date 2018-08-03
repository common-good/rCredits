Feature: Offline
AS a company agent
I WANT to accept transactions offline
SO my company can sell stuff, give refunds, and trade USD for rCredits even when no internet is available

and I WANT those transactions to be reconciled when an internet connection becomes available again
SO my company's online account records are not incorrect for long.

Setup:
  Given members:
  | id   | fullName   | email | cc  | cc2  | floor | flags                | *
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
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  | 1   | %today-6m | signup |    250 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup |    250 | ctty | .ZZB | signup  |
  | 3   | %today-6m | signup |    250 | ctty | .ZZC | signup  |
  | 4   | %today-6m | grant  |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  | .ZZF |     250 |

Scenario: A cashier charged someone offline
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100 rewards 260 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $100   | goods |
# NOPE  And with proof of agent "C:A" amount 100.00 created "%now-1h" member ".ZZB" code "ccB"
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     100 |

Scenario: A cashier charged someone offline and they have insufficient balance
  Given transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  | 5   | %today  | transfer |    200 | .ZZB | .ZZA | cash    |
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 6 created "%now-1h" balance -300 rewards 240
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |     200 |
  | .ZZB |    -300 |
  | .ZZC |     100 |

Scenario: A cashier charged someone offline but it actually went through
  Given agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at "%now-1h"
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100 rewards 260
  #And we notice nothing
  And balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     100 |

Scenario: A cashier declined to charge someone offline and it didn't go through
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force -1
  Then we respond ok txid 0 created "" balance 0 rewards 250
  #And we notice nothing
  And balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through
  Given agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at "%now-1h"
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force -1
  Then we respond ok txid 6 created %now balance 0 rewards 250
  And with undo "5"
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $10               |
  And we notice "new refund" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose       | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food (reverses #2)  | reward          | $-10              |
  And balances:
  | id   | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through, but customer is broke
  Given transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  | 5   | %today  | grant    |    500 | ctty | .ZZC | growth  |
  And agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-100 for "goods": "refund" at "%now-1h"
  And transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  | 7   | %today  | transfer |    300 | .ZZB | .ZZA | cash    |
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $-100 for "goods": "refund" at "%now-1h" force -1
  Then we respond ok txid 8 created %now balance -300 rewards 250
  And with undo "6"
  And we notice "new refund" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | refund       |
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose         |*
  | %today  | Bea Two  | Corner Pub | $100   | refund (reverses #2)  |
  And balances:
  | id   | balance |*
  | ctty |    -750 |
  | .ZZA |     300 |
  | .ZZB |    -300 |
  | .ZZC |     500 |

Scenario: Device sends correct old proof for legit tx after member loses card, with app offline
  Given members have:
  | id   | cardCode |*
  | .ZZB | ccB2     |
  // member just changed cardCode
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100 rewards 260 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $100   | goods |

Scenario: Device sends correct old proof for legit tx after member loses card, with app online
  Given members have:
  | id   | cardCode |*
  | .ZZB | ccB2     |
  // member reported lost card, we just changed cardCode, now the member (or someone) tries to use the card with app online:
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 0
  Then we return error "bad proof"


Scenario: Device sends correct old proof for legit tx after member loses card, with tx date after the change
  Given members have:
  | id   | cardCode |*
  | .ZZB | ccB2     |
  // member reported lost card, we just changed cardCode, now the member (or someone) tries to use the card with app online:
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now+1h" force 1
  Then we return error "bad proof"
