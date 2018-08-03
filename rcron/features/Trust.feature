Feature: Trust
AS a Common Good Community
I WANT to know how much each member is trusted by the others
SO I can choose the most trusted members as candidates for Trustee
AND identify disjoint circles of trust that may flag potential security risks.

Setup:
  Given members:
  | id   | fullName   | email | flags   |*
  | .ZZA | Abe One    | a@    | ok      |
  | .ZZB | Bea Two    | b@    | ok      |
  | .ZZC | Corner Pub | c@    | ok,co   |
  | .ZZD | Dee Four   | d@    | ok      |
  | .ZZE | Eve Five   | e@    | ok      |
  | .ZZF | Flo Six    | f@    | ok      |
  | .ZZG | Guy Seven  | g@    | ok      |
  | .ZZH | Hal Eight  | h@    | ok      |
  | .ZZI | Ida Nine   | i@    | ok      |
  And proxies:
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZA  |        1 |
  | .ZZB   | .ZZE  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZF  |        2 |
  | .ZZE   | .ZZD  |        1 |
  | .ZZE   | .ZZB  |        2 |
  | .ZZF   | .ZZD  |        1 |
  | .ZZF   | .ZZE  |        2 |
  | .ZZG   | .ZZH  |        1 |
  | .ZZG   | .ZZI  |        2 |
  | .ZZH   | .ZZI  |        1 |
  | .ZZH   | .ZZG  |        2 |
  | .ZZI   | .ZZG  |        1 |
  | .ZZI   | .ZZF  |        2 |
  
Scenario: we calculate trust
  When cron runs "trust"
  Then members:
  | id   | trust |*
  | .ZZA | 12.98 |
  | .ZZB |  9.47 |
  | .ZZC |     0 |
  | .ZZD | 12.42 |
  | .ZZE |  5.72 |
  | .ZZF |  6.79 |
  | .ZZG |  7.81 |
  | .ZZH |  5.90 |
  | .ZZI |  7.47 |
