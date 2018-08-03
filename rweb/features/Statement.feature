Feature: Transactions
AS a member
I WANT a prinatable report of my transactions for the month
SO I have a formal record of them.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags | created    | rebate |*
  | .ZZA | Abe One    | -100  | personal    | ok    | %today-15m |      5 |
  | .ZZB | Bea Two    | -200  | personal    | ok,co | %today-15m |     10 |
  | .ZZC | Corner Pub | -300  | corporation | ok,co | %today-15m |     10 |
  And members have:
  | id   | fullName |*
  | ctty | ZZrCred  |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount | tid | created   | completed |*
  | 1001 |  .ZZA |   1000 |   1 | %today-3m | %today-3m |
  | 1002 |  .ZZB |   2000 |   1 | %today-3m | %today-3m |
  | 1003 |  .ZZC |   3000 |   1 | %today-3m | %today-3m |
  | 1004 |  .ZZA |     11 |   2 | %lastm+5d | %lastm+5d |
  | 1005 |  .ZZA |    -22 |   4 | %lastm+8d | %lastm+8d |
  | 1006 |  .ZZA |    -33 |   3 | %lastm+9d | %lastm+9d |
  Then balances:
  | id   | balance |*
  | .ZZA |     956 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given transactions: 
  | xid| created   | type     | amount | from | to   | purpose | taking |*
  | 1  | %lastm+1d | signup   |    250 | ctty | .ZZA | signup  | 0      |
  | 2  | %lastm+2d | signup   |    250 | ctty | .ZZB | signup  | 0      |
  | 3  | %lastm+2d | signup   |    250 | ctty | .ZZC | signup  | 0      |
  | 4  | %lastm+3d | transfer |     10 | .ZZB | .ZZA | cash E  | 0      |
  | 5  | %lastm+4d | transfer |   1100 | .ZZC | .ZZA | usd F   | 1      |
  | 6  | %lastm+5d | transfer |    240 | .ZZA | .ZZB | what G  | 0      |

  | 9  | %lastm+6d | transfer |     50 | .ZZB | .ZZC | cash P  | 0      |
  | 10 | %lastm+7d | transfer |    120 | .ZZA | .ZZC | this Q  | 1      |

  | 13 | %lastm+8d | transfer |    100 | .ZZA | .ZZB | cash V  | 0      |
  Then balances:
  | id   | balance |*
  | .ZZA |    1606 |
  | .ZZB |    2280 |
  | .ZZC |    2070 |

Scenario: A member looks at a statement for previous month
  When member ".ZZA" views statement for %lastmy
  Then we show "ZZA" with:
  | Starting | From Bank | Paid   | Received | Ending   |
  | 1,000.00 | -44.00    | 460.00 | 1,110.00 | 1,606.00 |
#   |     0.00 |           |        |        | 268.00  |   268.00 |
  And with:
  | Tx   | Date       | Name       | Purpose  | Amount  |
#  | 1    | %lastmd+1d | ZZrCred    | signup  |     0.00 |
  | 2    | %lastmd+3d | Bea Two    | cash E  |    10.00 |
  | 3    | %lastmd+4d | Corner Pub | usd F   | 1,100.00 |
  | 4    | %lastmd+5d | Bea Two    | what G  |  -240.00 |
  | 1004 | %lastmd+5d | --         | from bank |  11.00 |
  | 5    | %lastmd+7d | Corner Pub | this Q  |  -120.00 |
  | 6    | %lastmd+8d | Bea Two    | cash V  |  -100.00 |
  | 1005 | %lastmd+8d | --         | to bank |   -22.00 |
  | 1006 | %lastmd+9d | --         | to bank |   -33.00 |
  And without:
  | rebate  |
  | bonus   |
