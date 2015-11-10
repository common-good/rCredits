Feature: Joint
AS a member with a joint account
I WANT to draw on the sum of the balances in the two accounts
SO I can make purchases as a financial unit with my account partner.

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags                    | jid  |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |     10 | ok,confirmed,bona        | .ZZB |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |     10 | ok,confirmed,bona        | .ZZA |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,confirmed,co,bona     |      |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 |     10 | ok,confirmed,bona        |      |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 |     10 | ok,confirmed,bona,secret |      |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      |      5 | ok,confirmed,co,bona     |      |
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
  | :ZZE | .ZZC | .ZZE  | sell       |
  | :ZZF | .ZZA | .ZZB  | joint      |
  | :ZZG | .ZZB | .ZZA  | joint      |
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
  
Scenario: A cashier asks to charge someone
  When agent ":ZZE" asks device "devC" to charge ".ZZB-ccB" $400 for "goods": "food" at %now
  Then we respond ok txid 5 created %now balance 140 rewards 540
  And with message "report tx|reward" with subs:
  | did     | otherName | amount | why                | rewardAmount |*
  | charged | Bea Two   | $400   | goods and services | $20          |
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
  | id   | balance |*
  | ctty |   -1060 |
  | .ZZA |     100 |
  | .ZZB |      40 |
  | .ZZC |     670 |
  | .ZZF |     250 |
  