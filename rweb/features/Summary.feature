Feature: Summary
AS a member
I WANT to see an overview of an account
SO I know where it stands.

Setup:
  Given members:
  | id      | fullName  | address | city  | state   | postalCode | country | floor | flags                   |
  | NEW.ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | -100  | dft,ok,personal         |
  | NEW.ZZB | Bea Two    | POB 2   | Btown | Utah   | 02000      | US      | -200  | dft,ok,personal,company |
  | NEW.ZZC | Corner Pub | POB 3   | Ctown | Cher   |            | France  | -300  | dft,ok,company          |

  And relations:
  | id      | main    | agent   | permission |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy        |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA | read       |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy        |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell       |
  And transactions: 
  | tx_id    | created   | type         | state       | amount | from      | to      | purpose      | taking |
  | NEW:AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 000000 |
  | NEW:AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 000000 |
  | NEW:AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 000000 |
  | NEW:AAAE | %today-2m | %TX_TRANSFER | %TX_DONE    |     10 | NEW.ZZB   | NEW.ZZA | cash E       | 000000 |
  | NEW:AAAF | %today-3w | %TX_TRANSFER | %TX_DONE    |     20 | NEW.ZZC   | NEW.ZZA | usd F        | 000000 |
  | NEW:AAAG | %today-3d | %TX_TRANSFER | %TX_DONE    |     40 | NEW.ZZA   | NEW.ZZB | whatever43   | 000000 |
  | NEW:AAAH | %today-3d | %TX_REBATE   | %TX_DONE    |      2 | community | NEW.ZZA | rebate on #4 | 000000 |
  | NEW:AAAI | %today-3d | %TX_BONUS    | %TX_DONE    |      4 | community | NEW.ZZB | bonus on #3  | 000000 |
  | NEW:AAAJ | %today-2d | %TX_TRANSFER | %TX_DONE    |      5 | NEW.ZZB   | NEW.ZZC | cash J       | 000000 |
  | NEW:AAAK | %today-1d | %TX_TRANSFER | %TX_DONE    |     80 | NEW.ZZA   | NEW.ZZC | whatever54   | 000000 |
  | NEW:AAAL | %today-1d | %TX_REBATE   | %TX_DONE    |      4 | community | NEW.ZZA | rebate on #5 | 000000 |
  | NEW:AAAM | %today-1d | %TX_BONUS    | %TX_DONE    |      8 | community | NEW.ZZC | bonus on #4  | 000000 |
  Then balances:
  | id        | balance |
  | community |    -768 |
  | NEW.ZZA   |     166 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     323 |

Variants: with/without an agent
  | "NEW.ZZA" | "NEW.ZZC" | # member pro se     |
  | "NEW:ZZA" | "NEW:ZZC" | # agent for account |

Scenario: A member clicks on the summary tab
  When member "NEW.ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | Name             | Address                | Account ID         |  Balance | Rewards | Floor |
  | Abe One (abeone) | POB 1, Atown, AK 01000 | NEW.ZZA (personal) | $166     | $256    | $-100 |

Scenario: A foreign rTrader clicks on the summary tab
  When member "NEW.ZZC" visits page "summary"
  Then we show "Account Summary" with:
  | Name                   | Address                    | Account ID        | Balance | Rewards |  Floor |
  | Corner Pub (cornerpub) | POB 3, Ctown, Cher, FRANCE | NEW.ZZC (company) | $323    | $258    |  $-300 |

Scenario: Member's account is not active
  Given member "NEW.ZZA" account is not active
  When member "NEW.ZZA" visits page "summary"
  Then we say "status": "take a step"