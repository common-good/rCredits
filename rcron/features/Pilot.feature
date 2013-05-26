Feature: Pilot
AS a group of pioneering members
I WANT the first few transactions of the pilot phase to go as expected
SO I will have confidence to continue using the rCredits system.

# (1) Snow's starts with 0
# (2) Assuming that William lends Snow's $100 US directly through Dwolla (and adjust the caches accordingly)
Setup:
  Then members:
  | id   | fullName            | flags                      |
  | .AAA | William Spademan    | dft,ok,personal,bona       |
  | .AAB | Common Good Finance | dft,ok,company,charge,bona |
  Given members:
  | id   | fullName   | email | flags                         |
  | .AAD | John Root  | d@ex  | dft,ok,personal,to_bank       |
  | .AAE | Janet H    | e@ex  | dft,ok,personal,to_bank       |
  | .AAF | John Eich  | f@ex  | dft,ok,personal,to_bank       |
  | .AAG | Alden B    | g@ex  | dft,ok,personal,to_bank       |
  | .AAH | Becca King | h@ex  | dft,ok,personal,to_bank       |
  | .AAI | Barbara F  | i@ex  | dft,ok,personal,to_bank       |
  | .AAJ | Julia E    | j@ex  | dft,ok,personal,to_bank       |
  | .AAK | Gary S     | k@ex  | dft,ok,personal,to_bank       |
  | .AAL | Diane M    | l@ex  | dft,ok,personal,to_bank       |
  | .AAM | Peter L    | m@ex  | dft,ok,personal               |
  | .AAO | Nancy H    | o@ex  | dft,ok,personal,to_bank       |
  | .AAQ | Snow's     | q@ex  | dft,ok,company,charge         |
  | .AAR | Pint       | r@ex  | dft,ok,company,charge,to_bank |
  | .AAS | GFM        | s@ex  | dft,ok,company,charge,to_bank |
  # not testing flags SMS and SECRET
  And balances:
  | id   | r | usd | rewards | minimum | maximum | share | committed |
  | .AAA | 0 | 200 |       0 |     200 |       0 |    50 |         0 |
  | .AAB | 0 |   0 |       0 |       0 |       0 |     0 |         0 |
  | .AAD | 0 | .20 |       0 |     200 |       0 |    50 |         0 |
  | .AAE | 0 |   0 |       0 |      10 |      50 |    10 |         0 |
  | .AAF | 0 |   0 |       0 |     200 |       0 |     1 |         0 |
  | .AAG | 0 |   0 |       0 |     200 |       0 |    50 |         0 |
  | .AAH | 0 | 100 |       0 |     100 |       0 |    10 |         0 |
  | .AAI | 0 |   0 |       0 |      50 |     100 |    50 |         0 |
  | .AAJ | 0 |  20 |       0 |      20 |      50 |    10 |         0 |
  | .AAK | 0 |   0 |       0 |      50 |     100 |    25 |         0 |
  | .AAL | 0 |   0 |       0 |       0 |      20 |     0 |         0 |
  | .AAM | 0 | 100 |       0 |     100 |       0 |    50 |         0 |
  | .AAO | 0 | 200 |       0 |     200 |     200 |   100 |         0 |
  | .AAQ | 0 |   0 |       0 |      10 |       0 |    25 |         0 |
  | .AAR | 0 |   0 |       0 |     200 |       0 |   100 |         0 |
  | .AAS | 0 |   0 |       0 |     200 |    1000 |     2 |         0 |
  And relations:
  | id   | main | agent | draw | amount | employerOk | employeeOk | isOwner | permission |
  | .AAB | .AAB | .AAA  |    0 |   1000 |          0 |          0 |       0 | manage     |
  | .AAD | .AAQ | .AAK  |    1 |    100 |          1 |          0 |       1 | manage     |
  | .AAE | .AAK | .AAQ  |    1 |      0 |          0 |          0 |       0 |            |
  | .AAF | .AAQ | .AAI  |    1 |    100 |          1 |          0 |       1 | manage     |
  | .AAI | .AAR | .AAG  |    0 |   3000 |          1 |          0 |       1 | manage     |
  | .AAJ | .AAR | .AAQ  |    0 |    500 |          0 |          0 |       0 |            |
  | .AAK | .AAS | .AAF  |    0 |    100 |          1 |          1 |       0 | manage     |
  | .AAL | .AAS | .AAL  |    0 |     10 |          1 |          0 |       0 | sell       |
  | .AAM | .AAS | .AAJ  |    0 |     10 |          1 |          0 |       0 | manage     |
  And gifts:
  | id   | amount | often | giftDate   |
  | .AAA |     50 |     1 |          0 |
  | .AAD |     50 |     1 |          0 |
  | .AAE |     25 |     1 |          0 |
  | .AAF |     50 |     1 |          0 |
  | .AAG |      5 |     1 |          0 |
  | .AAH |     50 |     1 |          0 |
  | .AAI |     10 |     1 |          0 |
  | .AAJ |     25 |     1 |          0 |
  | .AAK |    100 |     1 |          0 |
  | .AAL |      1 |     M | 1367937541 |
  | .AAM |    100 |     1 |          0 |
  | .AAO |    100 |     1 |          0 |
  | .AAQ |     50 |     1 |          0 |
  | .AAR |      5 |     1 |          0 |
  | .AAS |     50 |     1 |          0 |

Scenario: cron runs for the first time
  When cron runs "ALL"
  Then transactions: 
  | xid | created | type      | state | amount | r    | from | to   | purpose      | taking |
  | 1   | %today  | signup    | done  |     20 |   20 | ctty | .AAD | signup       | 0      |
  | 2   | %today  | signup    | done  |     20 |   20 | ctty | .AAH | signup       | 0      |
  | 3   | %today  | signup    | done  |     20 |   20 | ctty | .AAJ | signup       | 0      |
  | 4   | %today  | signup    | done  |     20 |   20 | ctty | .AAM | signup       | 0      |
  | 5   | %today  | signup    | done  |     20 |   20 | ctty | .AAO | signup       | 0      |
  | 6   | %today  | transfer  | done  |     50 |    0 | .AAA | .AAB | contribution | 1      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 1      |
  | 8   | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAB | bonus  on #1 | 1      |
  | 6   | %today  | transfer  | done  |     50 |   20 | .AAH | .AAB | contribution | 1      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 1      |
  | 8   | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAB | bonus  on #1 | 1      |
  | 6   | %today  | transfer  | done  |     50 |    0 | .AAA | .AAB | contribution | 1      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 1      |
  | 8   | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAB | bonus  on #1 | 1      |
  | 6   | %today  | transfer  | done  |     50 |    0 | .AAA | .AAB | contribution | 1      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 1      |
  | 8   | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAB | bonus  on #1 | 1      |
  | 6   | %today  | transfer  | done  |     50 |    0 | .AAA | .AAB | contribution | 1      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 1      |
  | 8   | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAB | bonus  on #1 | 1      |
7	2	ctty	William Spademan	2.50	2.50	rebate on #1	same		
8	3	ctty	Common Good Finance	5.00	5.00	bonus on #1	same		
9	1	Becca King	Common Good Finance	50.00	20.00	contribution	same	2	2
10	2	ctty	Becca King	2.50	2.50	rebate on #2	same		
11	3	ctty	Common Good Finance	5.00	5.00	bonus on #2	same		
12	1	Julia E	Common Good Finance	25.00	20.00	contribution	same	2	3
13	2	ctty	Julia E	1.25	1.25	rebate on #2	same		
14	3	ctty	Common Good Finance	2.50	2.50	bonus on #3	same		
15	1	Peter L	Common Good Finance	100.00	20.00	contribution	same	2	4
16	2	ctty	Peter L	5.00	5.00	rebate on #2	same		
17	3	ctty	Common Good Finance	10.00	10.00	bonus on #4	same		
18	1	Nancy H	Common Good Finance	100.00	20.00	contribution	same	2	5
19	2	ctty	Nancy H	5.00	5.00	rebate on #2	same		
20	3	ctty	Common Good Finance	10.00	10.00	bonus on #5	same		
  
#  | .AAAE | %today-5m | transfer  | done    |     10 |  10 | .ZZB | .ZZA | cash E  | 0      |
#  | .AAAF | %today-4m | transfer  | done    |    100 |  20 | .ZZC | .ZZA | usd F   | 1      |
#  Then balances:
#  | id   | r    | usd      | rewards |
#  | ctty | -775 | 10000.00 |       0 |
#  | .ZZA |  169 |   741.75 |     257 |
#  | .ZZB |  284 |  2256.50 |     256 |
#  | .ZZC |  322 |  3002.50 |     261 |
#  
#Scenario: cron calculates the totals
#  When cron runs "totals"
#  Then totals:
#  | r   | floor | rewards | usd     | minimum | maximum | excess | signup | rebate | bonus | inflation | grant | loan | fine | maxRebate | balance | demand |
#  | 775 |   -10 |     774 | 6000.75 |    3005 |     513 |    350 | 750    |      6 |    12 |         6 |     4 |    5 |   6 |          4 |    -775 |   2394 |
#
