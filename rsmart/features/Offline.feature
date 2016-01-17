Feature: Offline
AS a company agent
I WANT to accept transactions offline
SO my company can sell stuff, give refunds, and trade USD for rCredits even when no internet is available

and I WANT those transactions to be reconciled when an internet connection becomes available again
So my company's online account records are not incorrect for long.

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags                | *
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |     10 | ok,confirmed,bona    |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |     10 | ok,confirmed,bona    |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,confirmed,co,bona |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 |     10 | ok,confirmed,bona    |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 |     10 | ok,confirmed,bona,secret |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      |      5 | ok,confirmed,co,bona |
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
  | id   |       r |*
  | ctty |   -1000 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |
  | .ZZF |     250 |

Scenario: A cashier charged someone offline
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour" force 1
  Then we respond ok txid 5 created "%now-1hour" balance 160 rewards 260
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $10               |
  And balances:
  | id   |       r |*
  | ctty |   -1015 |
  | .ZZA |     250 |
  | .ZZB |     160 |
  | .ZZC |     355 |

Scenario: A cashier charged someone offline and they have insufficient balance
  Given transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  | 5   | %today  | transfer |    200 | .ZZB | .ZZA | cash    |
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour" force 1
  Then we respond ok txid 6 created "%now-1hour" balance -40 rewards 260
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $10               |
  And balances:
  | id   |       r |*
  | ctty |   -1015 |
  | .ZZA |     450 |
  | .ZZB |     -40 |
  | .ZZC |     355 |

Scenario: A cashier charged someone offline but it actually went through
  Given agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour"
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour" force 1
  Then we respond ok txid 5 created "%now-1hour" balance 160 rewards 260
  #And we notice nothing
  And balances:
  | id   |       r |*
  | ctty |   -1015 |
  | .ZZA |     250 |
  | .ZZB |     160 |
  | .ZZC |     355 |

Scenario: A cashier declined to charge someone offline and it didn't go through
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour" force -1
  Then we respond ok txid 0 created "" balance 250 rewards 250
  #And we notice nothing
  And balances:
  | id   |       r |*
  | ctty |   -1000 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through
  Given agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour"
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $100 for "goods": "food" at "%now-1hour" force -1
  Then we respond ok txid 8 created %now balance 250 rewards 250
  And with undo "5"
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | food         | reward          | $10               |
  And we notice "new refund|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | reverses #2  | reward          | $-10              |
  And balances:
  | id   |       r |*
  | ctty |   -1000 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through, but customer is broke
  Given agent ":ZZA" asks device "devC" to charge ".ZZB-ccB" $-100 for "goods": "refund" at "%now-1hour"
  And transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  | 8   | %today  | transfer |    300 | .ZZB | .ZZA | cash    |
  When reconciling ":ZZA" on "devC" charging ".ZZB-ccB" $-100 for "goods": "refund" at "%now-1hour" force -1
  Then we respond ok txid 9 created %now balance -50 rewards 250
  And with undo "5"
  And we notice "new refund|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | refund       | reward          | $-10              |
  And we notice "new charge|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Corner Pub | $100   | reverses #2  | reward          | $10               |
  And balances:
  | id   |       r |*
  | ctty |   -1000 |
  | .ZZA |     550 |
  | .ZZB |     -50 |
  | .ZZC |     250 |
