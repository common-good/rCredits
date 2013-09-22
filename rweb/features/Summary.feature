Feature: Summary
AS a member
I WANT to see an overview of an account
SO I know where it stands.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | floor | flags                        |
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | -100  | dft,ok,person,bona         |
  | .ZZB | Bea Two    | 2 B St. | Btown | Utah   | 02000      | US      | -200  | dft,ok,person,company,bona |
  | .ZZC | Corner Pub | 3 C St. | Ctown | Cher   |            | France  | -300  | dft,ok,company,bona          |

  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And transactions: 
  | xid   | created   | type     | state | amount | from | to   | purpose      | taking |
  | .AAAB | %today-7m | signup   | done  |    250 | ctty | .ZZA | signup       | 000000 |
  | .AAAC | %today-6m | signup   | done  |    250 | ctty | .ZZB | signup       | 000000 |
  | .AAAD | %today-6m | signup   | done  |    250 | ctty | .ZZC | signup       | 000000 |
  | .AAAE | %today-2m | transfer | done  |     10 | .ZZB | .ZZA | cash E       | 000000 |
  | .AAAF | %today-3w | transfer | done  |     20 | .ZZC | .ZZA | usd F        | 000000 |
  | .AAAG | %today-3d | transfer | done  |     40 | .ZZA | .ZZB | whatever43   | 000000 |
  | .AAAH | %today-3d | rebate   | done  |      2 | ctty | .ZZA | rebate on #4 | 000000 |
  | .AAAI | %today-3d | bonus    | done  |      4 | ctty | .ZZB | bonus on #3  | 000000 |
  | .AAAJ | %today-2d | transfer | done  |      5 | .ZZB | .ZZC | cash J       | 000000 |
  | .AAAK | %today-1d | transfer | done  |     80 | .ZZA | .ZZC | whatever54   | 000000 |
  | .AAAL | %today-1d | rebate   | done  |      4 | ctty | .ZZA | rebate on #5 | 000000 |
  | .AAAM | %today-1d | bonus    | done  |      8 | ctty | .ZZC | bonus on #4  | 000000 |
  Then balances:
  | id   | balance |
  | ctty |    -768 |
  | .ZZA |     166 |
  | .ZZB |     279 |
  | .ZZC |     323 |

Scenario: A member clicks on the summary tab
  When member ".ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name         | Abe One (abeone) |
  | _Address     | 1 A St., Atown, AK 01000 |
  | ID           | .ZZA (personal) |
  | Balance      | $166 |
  | Rewards      | $256 |
  | Credit floor | $-100 |

Scenario: An agent clicks on the summary tab without permission to manage
  When member ":ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name | Abe One (abeone)   |
  | ID   | NEW.ZZA (personal) |
  And without:
  | Balance | Rewards | Floor  |

Scenario: A foreign rTrader clicks on the summary tab
  When member ".ZZC" visits page "summary"
  Then we show "Account Summary" with:
  | Name         | Corner Pub (cornerpub) |
  | _Address     | 3 C St., Ctown, Cher, FRANCE |
  | ID           | .ZZC (company)|
  | Balance      | $323 |
  | Rewards      | $258 |
  | Credit floor | $-300 |

Scenario: Member's account is not active
  Given member ".ZZA" account is not active
  When member ".ZZA" visits page "summary"
  Then we say "status": "take a step"