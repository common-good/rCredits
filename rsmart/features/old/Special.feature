Feature: Transact
AS a company agent
I WANT to transfer rCredits to or from another member
SO my company can sell stuff and give refunds.

# cc means cardCode
# while testing, bonus is artificially set at twice the rebate amount

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags      | *
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |     10 | ok,bona    |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |     10 | ok,bona    |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,co,bona |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 |     10 | ok,bona    |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 |     10 | ok,bona,secret_bal |
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
  | id   | main | agent | permission | draw |*
  | :ZZA | .ZZC | .ZZA  | buy        |    0 |
  | :ZZB | .ZZC | .ZZB  | scan       |    0 |
  | :ZZD | .ZZC | .ZZD  | read       |    0 |
  | :ZZE | .ZZF | .ZZE  | sell       |    1 |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  | 1   | %today-6m | signup |    250 | ctty | .ZZA | signup  |
  | 2   | %today-6m | signup |    250 | ctty | .ZZB | signup  |
  | 3   | %today-6m | signup |    250 | ctty | .ZZC | signup  |
  | 4   | %today-6m | grant  |  71.94 | ctty | .ZZE | stuff   |
  | 5   | %today-6m | grant  | 398.65 | ctty | .ZZF | stuff   |
  Then balances:
  | id   | balance |*
  | ctty |   -1000 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |
  | .ZZE |   71.94 |
  | .ZZF |  398.65 |

#Variants: with/without an agent
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent  |

Scenario: A cashier asks to charge someone who draws
  Given balances:
  | id    | r      | floor   | rewards |*
  | .ZZE  |  71.94 | -65.22  | 174.64  |
  | .ZZF  | 398.65 | -226.48 | 419.77  |
  When agent ":ZZA" asks device "devC" to charge ".ZZE" $167.43 for "goods": "groceries" at %now
  Then we respond ok txid 6 created %now balance 160 rewards 260
  And with message "report transaction" with subs:
  | did     | otherName | amount | rewardType | rewardAmount |*
  | charged | Bea Two   | $100   | reward     | $10          |
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
  | ctty |    -770 |
  | .ZZA |     250 |
  | .ZZB |     160 |
  | .ZZC |     360 |
