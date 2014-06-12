Feature: Edit Transaction
AS a member
I WANT to change the details of a payment to or from me
SO I can make it right.

Setup:
  Given members:
  | id   | fullName   | email | flags      |*
  | .ZZA | Abe One    | a@    | ok,bona    |
  | .ZZB | Bea Two    | b@    | ok,bona    |
  | .ZZC | Corner Pub | c@    | ok,co,bona |
  And relations:
  | id   | main | agent | permission |*
  | :ZZA | .ZZC | .ZZA  | sell       |
  | :ZZB | .ZZC | .ZZB  | buy        |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose      |*
  |   1 | %today-6m | signup   |    250 | ctty | .ZZA | signup       |
  |   2 | %today-6m | signup   |    250 | ctty | .ZZB | signup       |
  |   3 | %today-6m | signup   |    250 | ctty | .ZZC | signup       |
  |   4 | %today    | transfer |     20 | .ZZA | .ZZB | stuff        |
  |   5 | %today    | rebate   |      1 | ctty | .ZZA | rebate on #2 |
  |   6 | %today    | bonus    |      2 | ctty | .ZZB | bonus on #2  |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -753 |         |
  | .ZZA |     231 |     251 |
  | .ZZB |     272 |     252 |
  | .ZZC |     250 |     250 |

Scenario: A buyer increases a payment amount
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods | purpose |*
  |     40 |     1 | stuff   |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -756 |         |
  | .ZZA |     212 |     252 |
  | .ZZB |     294 |     254 |
  | .ZZC |     250 |     250 |
  And we say "status": "info saved"
  And we notice "tx edited|new tx amount" to member ".ZZA" with subs:
  | tid | who     | amount |*
  | 2   | you     | $40    |
  And we notice "tx edited|new tx amount" to member ".ZZB" with subs:
  | tid | who     | amount |*
  | 2   | Abe One | $40    |

Scenario: A buyer changes the goods status
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods | purpose |*
  |     20 |     0 | stuff   |
  Then balances:
  | id   | balance | rewards |*
  | ctty |    -750 |         |
  | .ZZA |     230 |     250 |
  | .ZZB |     270 |     250 |
  | .ZZC |     250 |     250 |
  And we say "status": "info saved"
  And we notice "tx edited|new tx goods" to member ".ZZA" with subs:
  | tid | who     | what          |*
  | 2   | you     | cash/loan/etc |
  And we notice "tx edited|new tx goods" to member ".ZZB" with subs:
  | tid | who     | what          |*
  | 2   | Abe One | cash/loan/etc |

Scenario: A buyer tries to decrease a payment amount
  When member ".ZZA" edits transaction "4" with values:
  | amount | goods | purpose |*
  |     10 |     1 | stuff   |
  Then we say "error": "illegal amount change" with subs:
  | amount | action   | a |*
  | $20    | decrease | ? |
