Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | id   | fullName   | floor | acctType      | flags                        |
  | .ZZA | Abe One    | -100  | %R_PERSONAL   | dft,ok,person,bona         |
  | .ZZB | Bea Two    | -200  | %R_PERSONAL   | dft,ok,person,company,bona |
  | .ZZC | Corner Pub | -300  | %R_COMMERCIAL | dft,ok,company,bona          |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd:
  | id   | usd   |
  | ctty | 10000 |
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  And transactions: 
  | xid   | created   | type     | state   | amount | r   | from | to   | purpose | taking |
  | .AAAB | %today-7m | signup   | done    |    250 | 250 | ctty | .ZZA | signup  | 0      |
  | .AAAC | %today-6m | signup   | done    |    250 | 250 | ctty | .ZZB | signup  | 0      |
  | .AAAD | %today-6m | signup   | done    |    250 | 250 | ctty | .ZZC | signup  | 0      |
  | .AAAE | %today-5m | transfer | done    |     10 |  10 | .ZZB | .ZZA | cash E  | 0      |
  | .AAAF | %today-4m | transfer | done    |    100 |  20 | .ZZC | .ZZA | usd F   | 1      |
  | .AAAG | %today-3m | transfer | done    |    240 |  40 | .ZZA | .ZZB | what G  | 0      |
  | .AAAH | %today-3m | rebate   | done    |      2 |   2 | ctty | .ZZA | rebate  | 0      |
  | .AAAI | %today-3m | bonus    | done    |      4 |   4 | ctty | .ZZB | bonus   | 0      |
  | .AAAJ | %today-3w | transfer | pending |    100 | 100 | .ZZA | .ZZB | pie N   | 1      |
  | .AAAK | %today-3w | rebate   | pending |      5 |   5 | ctty | .ZZA | rebate  | 0      |
  | .AAAL | %today-3w | bonus    | pending |     10 |  10 | ctty | .ZZB | bonus   | 0      |
  | .AAAM | %today-2w | transfer | pending |    100 | 100 | .ZZC | .ZZA | labor M | 0      |
  | .AAAN | %today-2w | rebate   | pending |      5 |   5 | ctty | .ZZC | rebate  | 0      |
  | .AAAO | %today-2w | bonus    | pending |     10 |  10 | ctty | .ZZA | bonus   | 0      |
  | .AAAP | %today-2w | transfer | done    |     50 |   5 | .ZZB | .ZZC | cash P  | 0      |
  | .AAAQ | %today-1w | transfer | done    |    120 |  80 | .ZZA | .ZZC | this Q  | 1      |
  | .AAAR | %today-1w | rebate   | done    |      4 |   4 | ctty | .ZZA | rebate  | 0      |
  | .AAAS | %today-1w | bonus    | done    |      8 |   8 | ctty | .ZZC | bonus   | 0      |
  | .AAAT | %today-6d | transfer | pending |    100 | 100 | .ZZA | .ZZB | cash T  | 0      |
  | .AAAU | %today-6d | transfer | pending |    100 | 100 | .ZZB | .ZZA | cash U  | 1      |
  | .AAAV | %today-6d | transfer | done    |    100 |   0 | .ZZA | .ZZB | cash V  | 0      |
  Then balances:
  | id   | r    | usd   | rewards |
  | ctty | -768 | 10000 |       0 |
  | .ZZA |  166 |   740 |     256 |
  | .ZZB |  279 |  2255 |     254 |
  | .ZZC |  323 |  3005 |     258 |
  When cron runs ""
  Then balances:
  | id   | r       | usd      | rewards    |
  | ctty | -769.25 | 10000.00 |       0.00 |
  | .ZZA |  166.25 |   739.75 |     256.25 |
  | .ZZB |  279.50 |  2254.50 |     254.50 |
  | .ZZC |  323.50 |  3004.50 |     258.50 |  
  
Scenario: A member clicks NO
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
  And balances:
  | id   | r   |
  | .ZZA | 500 |
  # otherwise test dies for lack of Dwolla accounts
  When member ".ZZA" visits page "transactions/period=5"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose | Rewards |
  | 12  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X       | cash CL | --      |
  # expand this to have rewards, for a better test
  When member ".ZZA" visits page "transactions/period=5&do=no&xid=100"
  Then we show "tx summary|confirm tx action" with subs:
  | amount | otherName  | otherDid | purpose | created   | txAction                     |
  | $100   | Corner Pub | gave     | cash CL | %today-5d | REVERSE this disputed charge |
  
Scenario: A member confirms NO
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | disputed |    101 | .ZZC | .ZZA | cash CL  | 1      |
  And balances:
  | id   | r   |
  | .ZZA | 500 |
  When member ".ZZA" confirms form "transactions/period=5&do=no&xid=100" with values: ""
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | r%  | Status   | Buttons | Purpose               | Rewards |
  | 13  | %dm    | Corner Pub | 101.00   | --     | 0.0 | %chk     | X       | reverses #12              | --  |
  | 12  | %dm-5d | Corner Pub | --       | 101.00 | 100 | disputed |         | (reversed by #13) cash CL | --  |

Scenario: A member looks at transactions for the past year
# Same result as above, but with tid#11 added
  Given transactions: 
  | xid | created   | type     | state   | amount | r   | from | to   | purpose | taking |
  | 100 | %today-3d | transfer | done    |      0 | 100 | .ZZC | .ZZA | virtual | 0      |
  # plus rebate and bonus transactions
  When member ".ZZA" visits page "transactions/period=365&options=%RUSD_BOTH%STATES_BOTH%_N%_N%_N%_XCH%_VPAY"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | From Bank | From You | To You | Rewards | End Balance |
  | %dmy-12m   | %dmy     | $0.00         | 0.00      | 460.25   | 110.00 | 266.25  | - $84.00    |
  |            |          | PENDING       | 0.00      | 200.00   | 200.00 | 15.00   | + $15.00    |
  And we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | r%   | Status  | Buttons | Purpose    | Rewards |
  | 12  | %dm-3d | Corner Pub | 100.00   | 100.00 | --   | %chk    |         | virtual    | 10.00   |
  | 11  | %dm    | %ctty      |   0.25   | --     | --   | %chk    |         | Dwolla fee | 0.25    |
  | 10  | %dm-6d | Bea Two    | 100.00   | --     | 0.0  | %chk    | X       | cash V     | --      |
  | 9   | %dm-6d | Bea Two    | --       | 100.00 | 100  | pending | X       | cash U     | --      |
  | 8   | %dm-6d | Bea Two    | 100.00   | --     | 100  | pending | X       | cash T     | --      |
  | 7   | %dm-1w | Corner Pub | 120.00   | --     | 66.7 | %chk    | X       | this Q     | 4.00    |
  | 6   | %dm-2w | Corner Pub | --       | 100.00 | 100  | ok?     | OK X    | labor M    | 10.00   |
  | 5   | %dm-3w | Bea Two    | 100.00   | --     | 100  | ok?     | OK X    | pie N      | 5.00    |
  | 4   | %dm-3m | Bea Two    | 240.00   | --     | 16.7 | %chk    | X       | what G     | 2.00    |
  | 3   | %dm-4m | Corner Pub | --       | 100.00 | 20.0 | %chk    | X       | usd F      | --      |
  | 2   | %dm-5m | Bea Two    | --       | 10.00  | 100  | %chk    | X       | cash E     | --      |
  | 1   | %dm-7m | %ctty      | --       | --     | 100  | %chk    |         | signup     | 250.00  |
  And we show "Transaction History" without:
  | Purpose |
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "transactions/period=15&options=%RUSD_BOTH%STATES_BOTH%_N%_N%_N%_XCH%_VPAY"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | From You | To You | Rewards | End Balance |
  | %dmy-15d   | %dmy     | $122.00       | 220.25   | 0.00   | 4.25    | - $94.00    |
  |            |          | PENDING       | 200.00   | 200.00 | 15.00   | + $15.00   |
  And we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | r%   | Status  | Buttons | Purpose    | Rewards |
  | 11  | %dm    | %ctty      |   0.25   | --     | --   | %chk    |         | Dwolla fee | 0.25    |
  | 10  | %dm-6d | Bea Two    | 100.00   | --     | 0.0  | %chk    | X       | cash V     | --      |
  | 9   | %dm-6d | Bea Two    | --       | 100.00 | 100  | pending | X       | cash U     | --      |
  | 8   | %dm-6d | Bea Two    | 100.00   | --     | 100  | pending | X       | cash T     | --      |
  | 7   | %dm-1w | Corner Pub | 120.00   | --     | 66.7 | %chk    | X       | this Q     | 4.00    |
  | 6   | %dm-2w | Corner Pub | --       | 100.00 | 100  | ok?     | OK X    | labor M    | 10.00   |
  And we show "Transaction History" without:
  | Purpose  |
  | pie N    |
  | whatever |
  | usd F    |
  | cash E   |
  | signup   |
  | rebate   |
  | bonus    |

Scenario: Transactions with other states show up properly
  Given transactions:
  | xid   | created   | type     | state    | amount | from | to   | purpose  | taking |
  | .AACA | %today-5d | transfer | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
  | .AACB | %today-5d | rebate   | denied   |      5 | ctty | .ZZC | rebate   | 0      |
  | .AACD | %today-5d | bonus    | denied   |     10 | ctty | .ZZA | bonus    | 0      |
  | .AACE | %today-5d | transfer | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
  | .AACF | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  | .AACG | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  | .AACH | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  | .AACI | %today-5d | transfer | deleted  |    200 | .ZZA | .ZZC | never    | 1      |
  | .AACL | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
  Then balances:
  | id   | balance |
  | ctty |    -780 |
  | .ZZA |     190 |
  | .ZZB |     279 |
  | .ZZC |     311 |
  When member ".ZZA" visits page "transactions/period=5&options=%RUSD_BOTH%STATES_BOTH%_N%_N%_N%_XCH%_VPAY"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose    | Rewards |
  | 15  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X       | cash CL    | --      |
  | 14  | %dm-5d | Corner Pub | 80.00    | --     | disputed | OK      | this CF    | 4.00    |
  | 12  | %dm-5d | Corner Pub | --       | 100.00 | denied   | X       | labor CA   | 10.00   |
  | 11  | %dm    | %ctty      |   0.25   | --     | %chk     |         | Dwolla fee | 0.25    |
  # 13 is missing because ZZA denied it
  And we show "Transaction History" without:
  | Purpose |
  | cash CE |
  | never   |
  | rebate  |
  | bonus   |
  When member ".ZZC" visits page "transactions/period=5&options=%RUSD_BOTH%STATES_BOTH%_N%_N%_N%_XCH%_VPAY"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose    | Rewards |
  | 12  | %dm-5d | Abe One    | 100.00   | --     | disputed | OK      | cash CL    | --      |
  | 10  | %dm-5d | Abe One    | --       | 80.00  | disputed | X       | this CF    | 8.00    |
  | 9   | %dm-5d | Abe One    | --       | 5.00   | denied   | X       | cash CE    | --      |
  | 7   | %dm    | %ctty      |   0.25   | --     | %chk     |         | Dwolla fee | 0.25    |
  | 6   | %dm    | %ctty      |   0.25   | --     | %chk     |         | Dwolla fee | 0.25    |
  And we show "Transaction History" without:
  | Purpose |
  | labor CA|
  | never   |
  | rebate  |
  | bonus   |
  
Scenario: A member clicks OK
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | pending |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   | pending |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    | pending |      8 | ctty | .ZZC | bonus    | 0      |
  When member ".ZZA" visits page "transactions/period=5"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status | Buttons | Purpose | Rewards |
  | 12  | %dm-5d | Corner Pub | 80.00    | --     | ok?    | OK      | this CF | 4.00    |
  When member ".ZZA" visits page "transactions/period=5&do=ok&xid=100"
  Then we show "tx summary|confirm tx action" with subs:
  | amount | otherName  | otherDid | purpose | created   | txAction            |
  | $80    | Corner Pub | charged  | this CF | %today-5d | APPROVE this charge |

Scenario: A member confirms OK
  Given transactions:
  | xid | created   | type     | state   | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | pending |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   | pending |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    | pending |      8 | ctty | .ZZC | bonus    | 0      |
  When member ".ZZA" confirms form "transactions/period=5&do=ok&xid=100" with values: ""
  Then we say "status": "report transaction" with subs:
  | did    | otherName  | amount | tid | rewardType | rewardAmount |
  | paid   | Corner Pub | $80    | 13  | rebate     | $4           |
  And we notice "new payment|reward other" to member ".ZZC" with subs:
  | created | fullName   | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |
  | %today  | Corner Pub | Abe One   | $80    | this CF      | bonus           | $8                |
  And we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status | Buttons | Purpose | Rewards |
  | 13  | %dm    | Corner Pub | 80.00    | --     | %chk   | X       | this CF | 4.00    |
  
Scenario: A member confirms OK for a disputed transaction
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  | 101 | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  | 102 | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  When member ".ZZA" confirms form "transactions/period=5&do=ok&xid=100" with values: ""
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status | Buttons | Purpose | Rewards |
  | 13  | %dm-5d | Corner Pub | 80.00    | --     | %chk   | X       | this CF | 4.00    |
  
Scenario: A member clicks NO
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
  And balances:
  | id   | r   |
  | .ZZA | 500 |
  # otherwise test dies for lack of Dwolla accounts
  When member ".ZZA" visits page "transactions/period=5"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose | Rewards |
  | 12  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X       | cash CL | --      |
  # expand this to have rewards, for a better test
  When member ".ZZA" visits page "transactions/period=5&do=no&xid=100"
  Then we show "tx summary|confirm tx action" with subs:
  | amount | otherName  | otherDid | purpose | created   | txAction                     |
  | $100   | Corner Pub | gave     | cash CL | %today-5d | REVERSE this disputed charge |
  
Scenario: A member confirms NO
  Given transactions:
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  | 100 | %today-5d | transfer | disputed |    101 | .ZZC | .ZZA | cash CL  | 1      |
  And balances:
  | id   | r   |
  | .ZZA | 500 |
  When member ".ZZA" confirms form "transactions/period=5&do=no&xid=100" with values: ""
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | r%  | Status   | Buttons | Purpose               | Rewards |
  | 13  | %dm    | Corner Pub | 101.00   | --     | 0.0 | %chk     | X       | reverses #12              | --  |
  | 12  | %dm-5d | Corner Pub | --       | 101.00 | 100 | disputed |         | (reversed by #13) cash CL | --  |
