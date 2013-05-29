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
  | .AAD | John Root  | d@ex  | dft,ok,personal               |
  | .AAE | Janet H    | e@ex  | dft,ok,personal               |
  | .AAF | John Eich  | f@ex  | dft,ok,personal               |
  | .AAG | Alden B    | g@ex  | dft,ok,personal               |
  | .AAH | Becca King | h@ex  | dft,ok,personal               |
  | .AAI | Barbara F  | i@ex  | dft,ok,personal               |
  | .AAJ | Julia E    | j@ex  | dft,ok,personal               |
  | .AAK | Gary S     | k@ex  | dft,ok,personal               |
  | .AAL | Diane M    | l@ex  | dft,ok,personal               |
  | .AAM | Peter L    | m@ex  | dft,ok,personal               |
  | .AAO | Nancy H    | o@ex  | dft,ok,personal               |
  | .AAQ | Snow's     | q@ex  | dft,ok,company,charge         |
  | .AAR | Pint       | r@ex  | dft,ok,company,charge         |
  | .AAS | GFM        | s@ex  | dft,ok,company,charge         |
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
  | 6   | %today  | transfer  | done  |     50 |    0 | .AAA | .AAB | contribution | 0      |
  | 7   | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #1 | 0      |
  | 8   | %today  | bonus     | done  |   5.00 | 5.00 | ctty | .AAB | bonus on #1  | 0      |
  | 9   | %today  | transfer  | done  |  50.00 |   20 | .AAH | .AAB | contribution | 0      |
  | 10  | %today  | rebate    | done  |   2.50 | 2.50 | ctty | .AAH | rebate on #2 | 0      |
  | 11  | %today  | bonus     | done  |   5.00 | 5.00 | ctty | .AAB | bonus on #2  | 0      |
  | 12  | %today  | transfer  | done  |  25.00 |   20 | .AAJ | .AAB | contribution | 0      |
  | 13  | %today  | rebate    | done  |   1.25 | 1.25 | ctty | .AAJ | rebate on #2 | 0      |
  | 14  | %today  | bonus     | done  |   2.50 | 2.50 | ctty | .AAB | bonus on #3  | 0      |
  | 15  | %today  | transfer  | done  | 100.00 |   20 | .AAM | .AAB | contribution | 0      |
  | 16  | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAM | rebate on #2 | 0      |
  | 17  | %today  | bonus     | done  |     10 |   10 | ctty | .AAB | bonus on #4  | 0      |
  | 18  | %today  | transfer  | done  | 100.00 |   20 | .AAO | .AAB | contribution | 0      |
  | 19  | %today  | rebate    | done  |   5.00 | 5.00 | ctty | .AAO | rebate on #2 | 0      |
  | 20  | %today  | bonus     | done  |     10 |   10 | ctty | .AAB | bonus on #5  | 0      |
  And bank transfers:
  | payer | amount  |
  | .AAA  |  -47.50 |
  | .AAD  | -179.80 |
  | .AAH  |  -47.50 |
  | .AAE  |  -10.00 |
  | .AAF  | -200.00 |
  | .AAG  | -200.00 |
  | .AAI  |  -50.00 |
  | .AAK  |  -50.00 |
  | .AAM  |  -85.00 |
  | .AAO  |  -85.00 |
  | .AAQ  |  -10.00 |
  | .AAR  | -200.00 |
  | .AAS  | -200.00 |

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
