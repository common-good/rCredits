Feature: Risk
AS a Common Good Community Administrator or Member or Compliance Officer at a partnering financial institution
I WANT to know what financial or regulatory risks each account, transaction, and ACH pose
SO I can handle those risks appropriately

Setup:
  Given members:
  | id   | fullName   | email | address | postalCode | flags   | risks         | tenure | moves |*
  | .ZZA | Abe One    | a@    | 1 A St. |      01001 | ok,bona |               | 21     | 0     |
  | .ZZB | Bea Two    | b@    | 2 A St. |      01001 | ok,bona | rents         | 13     | 1     |
  | .ZZC | Corner Pub | c@    | 3 C St. |      01003 | ok,co   | cashCo        | 18     |       |
  | .ZZD | Dee Four   | d@    | 3 C St. |      01003 | ok,bank |               | 15     | 0     |
  | .ZZE | Eve Five   | e@    | 5 A St. |      01001 | ok      | shady         | 0      | 0     |
  | .ZZF | Flo Six    | f@    | 6 A St. |      01001 | ok      | photoMismatch | 12     | 0     |
  | .ZZG | Guy Seven  | g@    | 7 A St. |      01001 | ok      | addrMismatch  | 11     | 5     |
  | .ZZH | Hal Eight  | h@    | 8 A St. |      01001 | ok      | ssnMismatch   | 100    | 10    |
  | .ZZI | Ida Nine   | i@    | 9 A St. |      01001 | ok      | fishy         | 3      | 20    |
  And member field values:
  | id   | field      | value |*
  | .ZZA | community  |    -2 |
  | .ZZB | mediaConx  |    12 |
  | .ZZD | postalAddr | Box 3 |
  | .ZZE | postalAddr | Other |
  And invites:
  | inviter | invitee | email |*
  | .ZZA    | .ZZD    | d2@   |
  | .ZZA    |    0    | e@    |
  | .ZZG    | .ZZH    | h2@   |
  | .ZZG    | .ZZI    | i@    |
  And relations:
  | main | agent | permission | employee | isOwner | draw |*
  | .ZZC | .ZZA  | scan       |        1 |       0 |    0 |
  | .ZZC | .ZZB  |            |          |         |      |
  | .ZZC | .ZZD  |            |          |         |      |
  And gifts:
  | id   | amount | often | share |*
  | .ZZA |     10 |     1 |    10 |
  | .ZZB |      5 |     1 |    20 |
  | .ZZC |      1 |     1 |     1 |
  | .ZZD |      1 |     Q |     5 |
  | .ZZE |      1 |     M |     8 |
  | .ZZF |      1 |     1 |    50 |
  And proxies:
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZD  |        1 |
  | .ZZB   | .ZZA  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZB  |        2 |
  When cron runs "trust"
  Then members:
  | id   | trust |*
  | .ZZA |  8.57 |
  | .ZZB |  8.57 |
  | .ZZC |     0 |
  | .ZZD |  8.57 |
  | .ZZE |     0 |
  | .ZZF |     0 |
  | .ZZG |     0 |
  | .ZZH |     0 |
  | .ZZI |     0 |

Scenario: We calculate account risks
  When cron runs "acctRisk"
  Then members:
  | id   | risk  | risks |*
  | .ZZA |  8.57 | geography,moves,trusted |
  | .ZZB |  8.57 | rents,trusted,socialConx |
  | .ZZC |     0 | cashCo,homeCo,miser |
  | .ZZD |  8.57 | trusted,banks |
  | .ZZE |     0 | new,shady |
  | .ZZF |     0 | photoMismatch |
  | .ZZG |     0 | new,moves,addrMismatch |
  | .ZZH |     0 | moves,ssnMismatch |
  | .ZZI |     0 | new,moves |
  
#geography:4,cashCo:2,new:2,moves:1,rents:10,trusted:-4,socialConx:-3,badConx:3,homeCo:20,shady:3,banks:-3,miser:5,photoMismatch:1,addrMismatch:5,ssnMismatch:1,poBox:5,activity:1,fishy:2