Feature: Summary
AS a member
I WANT to see an overview of an account
SO I know where it stands.

# use weeks, not months, for consistent test statistics

Setup:
  Given members:
  | id   | fullName   | postalAddr                   | floor | flags      | rebate |*
  | .ZZA | Abe One    | 1 A St., Atown, AK 01000     | -100  | ok,bona    |      5 |
  | .ZZB | Bea Two    | 2 B St., Btown, UT 02000     | -200  | ok,bona    |     10 |
  | .ZZC | Corner Pub | 3 C St., Ctown, Cher, FRANCE | -300  | ok,co,bona |     10 |
  And members have:
  | id   | created   | share |*
  | ctty | %today-9w |     0 |
  | .ZZA | %today-7w |    10 |
  | .ZZB | %today-6w |    20 |
  | .ZZC | %today-6w |    30 |
  And usd transfers:
  | payer | amount | completed |*
  | .ZZA  | -100   | %today-7w |
  | .ZZB  | -200   | %today-6w |
  | .ZZC  | -300   | %today-6w |
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
  | id   | balance | rewards |*
  | .ZZA |      10 |     256 |
  | .ZZB |     225 |     254 |
  | .ZZC |     365 |     258 |
  Given cron runs "acctStats"

Scenario: A member clicks the summary tab
  When member ".ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name          | Abe One (abeone) |
  | ~Address      | 1 A St., Atown, AK 01000 |
  | ID            | ZZA (personal account) |
  | Balance       | $10 |
#  | Savings       | $256 |
  | ~rewards      | $256 |
#  | Credit limit  | $100 |
#  | Committed     | $0.60 |
# (including savings in balance)  | Your return   | 20.6% |
#  | Your return   | 72.1% |
#  | ~ever         | 544.1% |
#  or 541.4% (depends on daylight time?) or 280.9%?!
  | Social return | $27 |
  | ~ever         | $27 |

Scenario: An agent clicks the summary tab without permission to manage
  When member "A:B" visits page "summary"
  Then we show "Account Summary" with:
  | Name | Abe One (abeone)   |
  | ID   | NEWZZA (personal account) |
  And without:
  | Balance | Rewards | Floor  |

Scenario: A company agent clicks the summary tab
  When member "C:A" visits page "summary"
  Then we show "Account Summary" with:
  | Name         | Corner Pub (cornerpub) |
  | ~Address     | 3 C St., Ctown, Cher, FRANCE |
  | ID           | ZZC (company account) |

Scenario: Member's account is not active
  Given members have:
  | id   | flags |*
  | .ZZA |       |
  When member ".ZZA" visits page "summary"
  Then we show "Verify Your Email Address"