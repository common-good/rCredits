Feature: Joint
AS a member with a joint account
I WANT to draw on the sum of the balances in the two accounts
SO I can make purchases as a financial unit with my account partner.

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags                    | jid  |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |     10 | ok,confirmed             | .ZZB |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |     10 | ok,confirmed             | .ZZA |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,confirmed,co          |      |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 |     10 | ok,confirmed             |      |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 |     10 | ok,confirmed,secret |      |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      |      5 | ok,confirmed,co          |      |
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
  | id   | balance | rewards |*
  | ctty |    -200 |       0 |
  | .ZZA |       0 |     200 |
  | .ZZB |       0 |     200 |
  | .ZZC |       0 |     200 |
  | .ZZF |     200 |       0 |
  
Scenario: A cashier asks to charge someone
  When agent "C:E" asks device "devC" to charge ".ZZB,ccB" $400 for "goods": "food" at %now
  Then we respond ok txid 5 created %now balance -400 rewards 440 saying:
  | did     | otherName | amount | why   | reward |*
  | charged | Bea Two   | $400   | goods | $40    |
  And with did
  | did     | amount | forCash |*
  | charged | $400   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $400   | from   | Bea Two   |
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $400   | food         | reward          | $40               |
  And balances:
  | id   | balance | rewards |*
  | ctty |    -200 |       0 |
  | .ZZA |       0 |     200 |
  | .ZZB |    -400 |     240 |
  | .ZZC |     400 |     220 |
  | .ZZF |     200 |       0 |
  