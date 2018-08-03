Feature: Pilot
AS a group of pioneering members
I WANT the first few transactions of the pilot phase to go as expected
SO I will have confidence to continue using the rCredits system.

# (1) Snow's starts with 0
# (2) Assuming that William lends Snow's $100 US directly through Dwolla (and adjust the caches accordingly) ?
Setup:
  Then members:
  | id   | fullName            | flags                             |
  | .AAA | William Spademan    | dft,member,ok,personal,bona,bank  |
  | .AAB | Common Good Finance | dft,member,ok,company,charge,bona |
  Given members:
  | id   | fullName   | email | flags                         |
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
  | id   | r | usd | rewards | minimum | share | committed |
  | .AAA | 0 | 200 |       0 |     200 |    50 |         0 |
  | .AAB | 0 |   0 |       0 |       0 |     0 |         0 |
  | .AAF | 0 |   0 |       0 |     200 |     1 |         0 |
  | .AAG | 0 | 200 |       0 |     200 |    50 |         0 |
  | .AAH | 0 | 100 |       0 |     100 |    10 |         0 |
  | .AAI | 0 |   0 |       0 |      50 |    50 |         0 |
  | .AAJ | 0 |  20 |       0 |      20 |    10 |         0 |
  | .AAK | 0 |   0 |       0 |      50 |    25 |         0 |
  | .AAL | 0 |   5 |       0 |       0 |     0 |         0 |
  | .AAM | 0 | 100 |       0 |     100 |    50 |         0 |
  | .AAO | 0 | 200 |       0 |     200 |   100 |         0 |
  | .AAQ | 0 |   0 |       0 |      10 |    25 |         0 |
  | .AAR | 0 |   0 |       0 |     200 |   100 |         0 |
  | .AAS | 0 |   0 |       0 |     200 |     2 |         0 |
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
  When transactions:
  | xid | created   | type      | state | amount | r    | from | to   | purpose      | taking |
  |   1 | %today-8d | signup    | done  |     20 |   20 | ctty | .AAA | signup bonus | 0      |
  Then balances:
  | id   | r  | usd | rewards | minimum | maximum | share | committed |
  | .AAA | 20 | 200 |      20 |     200 |      -1 |    50 |         0 |
  
Scenario: all
  When member ".AAS" charges member ".AAA" $40 for "groceries"
  Then transactions:
  | xid | created | type      | state | amount | r    | from | to   | purpose      | taking |
  |   2 | %today  | transfer  | done  |     40 |   20 | .AAA | .AAS | groceries    |      1 |
  |   3 | %today  | rebate    | done  |      1 |    1 | ctty | .AAA | rebate on #2 |      0 |
  |   4 | %today  | bonus     | done  |      2 |    2 | ctty | .AAS | bonus on #1  |      0 |
  And balances:
  | id   | r  | usd | rewards | minimum | maximum | share | committed |
  | .AAA |  1 | 180 |      21 |     200 |      -1 |    50 |      0.50 |
  # You are HERE

  When cron runs "ALL"

  # recache
  Then transactions: 
  | xid | created   | type      | state | amount | r    | from | to   | purpose      | taking |
  |   5 | %today    | signup    | done  |     20 |   20 | ctty | .AAG | signup bonus | 0      |
  |   6 | %today    | signup    | done  |     20 |   20 | ctty | .AAH | signup bonus | 0      |
  |   7 | %today    | signup    | done  |     20 |   20 | ctty | .AAJ | signup bonus | 0      |
  |   8 | %today    | signup    | done  |     20 |   20 | ctty | .AAL | signup bonus | 0      |
  |   9 | %today    | signup    | done  |     20 |   20 | ctty | .AAM | signup bonus | 0      |
  |  10 | %today    | signup    | done  |     20 |   20 | ctty | .AAO | signup bonus | 0      |
  |  11 | %today    | inflation | done  |   0.02 | 0.02 | ctty | .AAA | inflation adjustment | 0 |

  # gifts
  And transactions: 
  | xid | created   | type      | state | amount | r    | from | to   | purpose      | taking |
  |  12 | %today    | transfer  | done  |     50 | 1.02 | .AAA | .AAB | contribution | 0      |
  |  13 | %today    | rebate    | done  |   2.50 | 2.50 | ctty | .AAA | rebate on #4 | 0      |
  |  14 | %today    | bonus     | done  |      5 |    5 | ctty | .AAB | bonus on #1  | 0      |
  |  15 | %today    | transfer  | done  |      5 |    5 | .AAG | .AAB | contribution | 0      |
  |  16 | %today    | rebate    | done  |   0.25 | 0.25 | ctty | .AAG | rebate on #2 | 0      |
  |  17 | %today    | bonus     | done  |   0.50 | 0.50 | ctty | .AAB | bonus on #2  | 0      |
  |  18 | %today    | transfer  | done  |     50 |   20 | .AAH | .AAB | contribution | 0      |
  |  19 | %today    | rebate    | done  |   2.50 | 2.50 | ctty | .AAH | rebate on #2 | 0      |
  |  20 | %today    | bonus     | done  |      5 |    5 | ctty | .AAB | bonus on #3  | 0      |
  |  21 | %today    | transfer  | done  |     25 |   20 | .AAJ | .AAB | contribution | 0      |
  |  22 | %today    | rebate    | done  |   1.25 | 1.25 | ctty | .AAJ | rebate on #2 | 0      |
  |  23 | %today    | bonus     | done  |   2.50 | 2.50 | ctty | .AAB | bonus on #4  | 0      |
  |  24 | %today    | transfer  | done  |      1 |    1 | .AAL | .AAB | contribution | 0      |
  |  25 | %today    | rebate    | done  |   0.05 | 0.05 | ctty | .AAL | rebate on #2 | 0      |
  |  26 | %today    | bonus     | done  |   0.10 | 0.10 | ctty | .AAB | bonus on #5  | 0      |
  |  27 | %today    | transfer  | done  |    100 |   20 | .AAM | .AAB | contribution | 0      |
  |  28 | %today    | rebate    | done  |      5 |    5 | ctty | .AAM | rebate on #2 | 0      |
  |  29 | %today    | bonus     | done  |     10 |   10 | ctty | .AAB | bonus on #6  | 0      |
  |  30 | %today    | transfer  | done  |    100 |   20 | .AAO | .AAB | contribution | 0      |
  |  31 | %today    | rebate    | done  |      5 |    5 | ctty | .AAO | rebate on #2 | 0      |
  |  32 | %today    | bonus     | done  |     10 |   10 | ctty | .AAB | bonus on #7  | 0      |
  # then sharing rewards and fee reimbursements
  And usd transfers:
  | payer | amount  | payee |
  | .AAA  |   20.00 | .AAS  |
  | .AAA  |   48.98 | .AAB  |
  | .AAH  |   30.00 | .AAB  |
  | .AAJ  |    5.00 | .AAB  |
  | .AAM  |   80.00 | .AAB  |
  | .AAO  |   80.00 | .AAB  |
  | .AAA  |  -66.48 |    0  |
  | .AAF  | -200.00 |    0  |
  | .AAH  |  -27.50 |    0  |
  | .AAI  |  -50.00 |    0  |
  | .AAJ  |   -4.00 |    0  |
  | .AAK  |  -50.00 |    0  |
  | .AAM  |  -75.00 |    0  |
  | .AAO  |  -75.00 |    0  |
  | .AAQ  |  -10.00 |    0  |
  | .AAR  | -200.00 |    0  |
  | .AAS  | -158.25 |    0  |
  # $4 is the minimum!

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
