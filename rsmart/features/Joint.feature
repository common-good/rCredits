Feature: Joint
AS a member with a joint account
I WANT to draw on the sum of the balances in the two accounts
SO I can make purchases as a financial unit with my account partner.

Setup:
  Given members:
  | id   | fullName   | email | cc  | cc2  | floor | flags                | jid  |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -200 | ok,confirmed,debt    | .ZZB |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -200 | ok,confirmed,debt    | .ZZA |
  | .ZZC | Corner Pub | c@    | ccC |      |  -200 | ok,confirmed,co,debt |    0 |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed         |    0 |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |     0 | ok,confirmed,secret  |    0 |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,confirmed,co      |    0 |
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
  | main | agent | permission | rCard |*
  | .ZZC | .ZZA  | buy        |       |
  | .ZZC | .ZZB  | scan       |       |
  | .ZZC | .ZZD  | read       |       |
  | .ZZC | .ZZE  | sell       | yes   |
  | .ZZA | .ZZB  | joint      |       |
  | .ZZB | .ZZA  | joint      |       |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  | 1   | %today-6m | signup |    200 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup |    200 | ctty | .ZZB | signup  |
  | 3   | %today-6m | signup |    200 | ctty | .ZZC | signup  |
  | 4   | %today-6m | grant  |    200 | ctty | .ZZF | stuff   |
  Then balances:
  | id   | balance |*
  | ctty |    -200 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  | .ZZF |     200 |
  
Scenario: A cashier asks to charge someone
  When agent "C:E" asks device "devC" to charge ".ZZB,ccB" $400 for "goods": "food" at %now
  Then we respond ok txid 5 created %now balance -400 rewards 440 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $400   | goods |
  And with did
  | did     | amount | forCash |*
  | charged | $400   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $400   | from   | Bea Two   |
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $400   | food         |
  And balances:
  | id   | balance |*
  | ctty |    -200 |
  | .ZZA |       0 |
  | .ZZB |    -400 |
  | .ZZC |     400 |
  | .ZZF |     200 |
  