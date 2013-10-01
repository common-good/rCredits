Feature: Pool
AS members of a community
WE WANT to pool our unused US Dollars
SO we can use them for something as a community, while losing nothing as individual account holders

Setup:
  Given members:
  | id   | fullName   | email | flags              |
  | .ZZA | Abe One    | a@    | dft,ok,person,bona |
  | .ZZB | Bea Two    | b@    | dft,ok,person,bona |
  | .ZZC | Corner Pub | c@    | dft,ok,company     |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   1 | %today-6m | signup   |     25 | ctty | .ZZC | signup  |
  And balances:
  | id   | r   | usd |
  | ctty | -25 |   0 |
  | .ZZA |   0 |  15 |
  | .ZZB |   0 |  10 |
  | .ZZC |  25 |  80 |

Scenario: Normal pooling happens
  When cron runs "pool"
  Then balances:
  | id   | r   | usd |
  | ctty | -40 |   0 |
  | .ZZA |   5 |  10 |
  | .ZZB |   0 |  10 |
  | .ZZC |  35 |  70 |
