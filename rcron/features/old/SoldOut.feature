Feature: Sold Out
AS a member
I WANT to receive a suggestion for how to change my settings, after unnecessarily selling rCredits
SO I can maximize my rebates

Setup:
  Given members:
  | id   | fullName   | floor | acctType      | flags                      | rebate | minimum |
  | .ZZA | Abe One    | -100  | %R_PERSONAL   | dft,ok,person,bona         |      4 |     100 |
  | .ZZB | Bea Two    | -200  | %R_PERSONAL   | dft,ok,person,company,bona |      5 |     100 |
  And transactions: 
  | xid | created   | type     | state | amount | r   | from | to   | purpose | goods |
  |   1 | %today-2m | signup   | done  |    500 | 100 | ctty | .ZZA | signup  |     0 |
  |   2 | %today-2m | signup   | done  |    200 | 100 | ctty | .ZZB | signup  |     0 |
  |   4 | %today-3d | transfer | done  |      0 |  12 | .ZZA | ctty | exch    |     0 |

Scenario: Member sold some rCredits this week
  Given next DO code is "WhAtEvEr"
  When cron runs "everyWeek"
  Then we notice "you sold out" to member ".ZZA" with subs:
  | total | rebates | suggested               | a1                                         |
  | $12   | $0.48   | $%(100*%R_SUGGEST_BUMP) | a href=''%BASE_URL/do/id=1&code=WhAtEvEr'' |
