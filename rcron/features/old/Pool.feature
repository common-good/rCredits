Feature: Pool
AS members of a community
WE WANT to pool our unused US Dollars
SO we can use them for something as a community, while losing nothing as individual account holders

Setup:
  Given members:
  | id   | fullName   | email | flags      |*
  | .ZZA | Abe One    | a@    | ok,dw,bona |
  | .ZZB | Bea Two    | b@    | ok,dw,bona |
  | .ZZC | Corner Pub | c@    | ok,dw,co   |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   1 | %today-6m | signup   |     25 | ctty | .ZZC | signup  |
  And balances:
  | id   | r   | usd |*
  | ctty | -25 |   0 |
  | .ZZA |   0 |  25 |
  | .ZZB |   0 |  10 |
  | .ZZC |  25 |  80 |
  And members have:
  | id   | usdAccount |*
  | ctty | cttyAcct   |
# cttyAcct is apparently ignored (look into that)

# (no longer leaving anything unpooled)
# Scenario: Normal pooling happens
#   When cron runs "pool"
#   Then balances:
#   | id   | r   | usd |*
#   | ctty | -45 |  20 |
#   | .ZZA |  10 |  15 |
#   | .ZZB |   0 |  10 |
#   | .ZZC |  35 |  70 |

Scenario: Normal pooling happens
  When cron runs "pool"
  Then balances:
  | id   | r   | usd |*
  | ctty | -55 |  30 |
  | .ZZA |  10 |  15 |
  | .ZZB |  10 |   0 |
  | .ZZC |  35 |  70 |
  