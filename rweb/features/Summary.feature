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
  | id   | main | agent | permission |*
  | :ZZA | .ZZA | .ZZB  | buy        |
  | :ZZB | .ZZB | .ZZA  | read       |
  | :ZZC | .ZZC | .ZZB  | buy        |
  | :ZZD | .ZZC | .ZZA  | sell       |
  And transactions: 
  | xid   | created   | type     | amount | from | to   | purpose      |*
  | .AAAB | %today-7w | signup   |    250 | ctty | .ZZA | signup       |
  | .AAAC | %today-6w | signup   |    250 | ctty | .ZZB | signup       |
  | .AAAD | %today-6w | signup   |    250 | ctty | .ZZC | signup       |
  | .AAAE | %today-5w | transfer |     10 | .ZZB | .ZZA | cash E       |
  | .AAAF | %today-4w | transfer |     20 | .ZZC | .ZZA | usd F        |
  | .AAAG | %today-3w | transfer |     40 | .ZZA | .ZZB | whatever43   |
  | .AAAH | %today-3w | rebate   |      2 | ctty | .ZZA | rebate on #4 |
  | .AAAI | %today-3w | bonus    |      4 | ctty | .ZZB | bonus on #3  |
  | .AAAJ | %today-2d | transfer |      5 | .ZZB | .ZZC | cash J       |
  | .AAAK | %today-1d | transfer |     80 | .ZZA | .ZZC | whatever54   |
  | .AAAL | %today-1d | rebate   |      4 | ctty | .ZZA | rebate on #5 |
  | .AAAM | %today-1d | bonus    |      8 | ctty | .ZZC | bonus on #4  |
  Then balances:
  | id   | r   |*
  | .ZZA | 266 |
  | .ZZB | 479 |
  | .ZZC | 623 |
  Given cron runs "acctStats"

Scenario: A member clicks the summary tab
  When member ".ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name          | Abe One (abeone) |
  | _Address      | 1 A St., Atown, AK 01000 |
  | ID            | .ZZA (personal account) |
  | Balance       | $266 |
  | Rewards       | $256 |
  | Committed     | $0.60 |
  | Your return   | 20.6% |
#  | _ever         | 544.1% | or 541.4% (depends on daylight time?) or 280.9%?!
  | Social return | $9 |
  | _ever         | $9 |
  | Credit limit  | $100 |

Scenario: An agent clicks the summary tab without permission to manage
  When member ":ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name | Abe One (abeone)   |
  | ID   | NEW.ZZA (personal account) |
  And without:
  | Balance | Rewards | Floor  |

Scenario: A company agent clicks the summary tab
  When member ":ZZD" visits page "summary"
  Then we show "Account Summary" with:
  | Name         | Corner Pub (cornerpub) |
  | _Address     | 3 C St., Ctown, Cher, FRANCE |
  | ID           | .ZZC (company account) |
  And without:
  | Balance      | $623 |

Scenario: Member's account is not active
  Given member ".ZZA" account is not active
  When member ".ZZA" visits page "summary"
  Then we say "status": "take a step"