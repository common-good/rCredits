Feature: Summary
AS a member
I WANT to see an overview of an account
SO I know where it stands.

# use weeks, not months, for consistent test statistics

Setup:
  Given members:
  | id   | fullName   | postalAddr                   | floor | flags      | rebate |*
  | .ZZA | Abe One    | 1 A St., Atown, AK 01000     | -100  | ok         |      5 |
  | .ZZB | Bea Two    | 2 B St., Btown, UT 02000     | -200  | ok,roundup |     10 |
  | .ZZC | Corner Pub | 3 C St., Ctown, Cher, FRANCE | -300  | ok,co      |     10 |
  And members have:
  | id   | created   | share |*
  | ctty | %today-9w |     0 |
  | .ZZA | %today-7w |    10 |
  | .ZZB | %today-6w |    20 |
  | .ZZC | %today-6w |    30 |
  And usd transfers:
  | payee | amount | completed |*
  | .ZZA  |  100   | %today-7w |
  | .ZZB  |  200   | %today-6w |
  | .ZZC  |  300   | %today-6w |
  And relations:
  | main | agent | num | permission |*
  | .ZZA | .ZZB  |   1 | buy        |
  | .ZZB | .ZZA  |   1 | read       |
  | .ZZC | .ZZB  |   1 | buy        |
  | .ZZC | .ZZA  |   2 | sell       |
  And transactions: 
  | xid | created   | type     | amount | payerReward | payeeReward | from | to   | purpose      |*
  |   1 | %today-7w | signup   |      0 |           0 |         250 | ctty | .ZZA | signup       |
  |   2 | %today-6w | signup   |      0 |           0 |         250 | ctty | .ZZB | signup       |
  |   3 | %today-6w | signup   |      0 |           0 |         250 | ctty | .ZZC | signup       |
  |   4 | %today-5w | transfer |     10 |           0 |           0 | .ZZB | .ZZA | cash E       |
  |   5 | %today-4w | transfer |     20 |           0 |           0 | .ZZC | .ZZA | usd F        |
  |   6 | %today-3w | transfer |     40 |           2 |           4 | .ZZA | .ZZB | whatever43   |
  |   7 | %today-2d | transfer |      5 |           0 |           0 | .ZZB | .ZZC | cash J       |
  |   8 | %today-1d | transfer |     80 |           4 |           8 | .ZZA | .ZZC | whatever54   |
  Then balances:
  | id   | balance |*
  | .ZZA |      10 |
  | .ZZB |     225 |
  | .ZZC |     365 |
  Given cron runs "acctStats"

Scenario: A member clicks the summary tab
  When member ".ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | ID            | ZZA (personal account) |
  | Name          | Abe One (abeone) |
  | Postal Addr   | 1 A St., Atown, AK 01000 |
  | Balance       | $10 |
#  | Savings       | $256 |
#  | ~rewards      | $256 |
#  | Credit limit  | $100 |
#  | Committed     | $0.60 |
# (including savings in balance)  | Your return   | 20.6% |
#  | Your return   | 72.1% |
#  | ~ever         | 544.1% |
#  or 541.4% (depends on daylight time?) or 280.9%?!
#  | Social return | $27 |
#  | including     | $0 |
  
Scenario: A member clicks the summary tab with roundups
  Given transactions:
  | xid | created | type     | amount | payerReward | payeeReward | from | to   | purpose |*
  |   9 | %today  | transfer |  80.02 |           4 |           8 | .ZZB | .ZZC | goodies |
  When member ".ZZB" visits page "summary"
  Then balances:
  | id   | balance |*
  | .ZZB |  144.98 |
  And we show "Account Summary" with:
  | Name          | Bea Two (beatwo) |
  | Balance       | $144 |

Scenario: An agent clicks the summary tab without permission to manage
  When member "A:B" visits page "summary"
  Then we show "Account Summary" with:
  | ID   | NEWZZA (personal account) |
  | Name | Abe One (abeone)   |
  And without:
  | Make This a Joint |

Scenario: A company agent clicks the summary tab
  When member "C:A" visits page "summary"
  Then we show "Account Summary" with:
  | ID           | ZZC (company account) |
  | Name         | Corner Pub (cornerpub) |
  | Postal Addr  | 3 C St., Ctown, Cher, FRANCE |
  
Scenario: Member's account is not active
  Given members have:
  | id   | flags |*
  | .ZZA |       |
  When member ".ZZA" visits page "summary"
  Then we show "Verify Your Email Address"