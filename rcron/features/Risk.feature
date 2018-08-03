Feature: Risk
AS a Common Good Community Administrator or Member or Compliance Officer at a partnering financial institution
I WANT to know what financial or regulatory risks each account, transaction, and ACH pose
SO I can handle those risks appropriately

Setup:
  Given members:
  | id   | fullName   | rebate | address | zip   | flags   | risks    | tenure | moves | share |*
  | .ZZA | Abe One    |      5 | 1 A St. | 01001 | ok      | adminOk  | 21     | 0     |    10 |
  | .ZZB | Bea Two    |     10 | 2 A St. | 01001 | ok      | rents    | 43     | 1     |    20 |
  | .ZZC | Corner Pub |     10 | 3 C St. | 01003 | ok,co   | cashCo   | 18     |       |     1 |
  | .ZZD | Dee Four   |     10 | 3 C St. | 01003 | ok      | hasBank  | 25     | 0     |     5 |
  | .ZZE | Eve Five   |     10 | 5 A St. | 01001 | ok      | shady    | 1      | 0     |     8 |
  | .ZZF | Flo Six    |     10 | 6 A St. | 01001 | ok      | photoOff | 32     | 0     |    50 |
  | .ZZG | Guy Seven  |     10 | 7 A St. | 01001 | ok      | addrOff  | 11     | 5     |    25 |
  | .ZZH | Hal Eight  |     10 | 8 A St. | 01001 | ok      | ssnOff   | 100    | 10    |    25 |
  | .ZZI | Ida Nine   |     10 | 9 A St. | 01001 | ok      | fishy    | 3      | 20    |    25 |
  And invites:
  | inviter | invitee | email |*
  | .ZZA    | .ZZD    | d2@   |
  | .ZZA    |    0    | e@    |
  | .ZZG    | .ZZH    | h2@   |
  | .ZZG    | .ZZI    | i@    |
  And proxies:
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZD  |        1 |
  | .ZZB   | .ZZA  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZB  |        2 |
  And relations:
  | main | agent | permission | employee | owner | draw |*
  | .ZZC | .ZZA  | scan       |        1 |     0 |    0 |
  | .ZZC | .ZZB  |            |          |       |      |
  | .ZZC | .ZZD  |            |          |       |      |
  And gifts:
  | id   | amount | often | share |*
  | .ZZA |     10 |     1 |     0 |
  | .ZZB |      5 |     1 |     0 |
  | .ZZC |      1 |     1 |     0 |
  | .ZZD |      1 |     Q |     0 |
  | .ZZE |      1 |     M |     0 |
  | .ZZF |      1 |     1 |     0 |
  | .ZZG |      5 |     1 |     0 |
  | .ZZH |      5 |     1 |     0 |
  | .ZZI |      5 |     1 |     0 |
# share is irrelevant here as long as it is non-negative
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose | channel |*
  |   1 | %today-7m | signup   |    250 | ctty | .ZZA | signup  | %TX_SYS |
  |   2 | %today-6m | signup   |    250 | ctty | .ZZB | signup  | %TX_SYS |
  |   3 | %today-6m | signup   |    250 | ctty | .ZZE | signup  | %TX_SYS |
  |   4 | %today-5m | transfer |     10 | .ZZB | .ZZA | cash E  | %TX_POS |
  |   5 | %today-1m | transfer |   1100 | .ZZA | .ZZC | USD in (cash) | %TX_POS |
  # (cash) is required else a transaction fee transaction is created
  |   6 | %today-3w | transfer |    240 | .ZZA | .ZZB | what G  | %TX_POS |

  |   7 | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P  | %TX_POS |
  |   8 | %today-1w | transfer |    120 | .ZZA | .ZZH | offline | %TX_POS |

  |   9 | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V  | %TX_WEB |
  |  10 | %today-1d | transfer |    120 | .ZZA | .ZZC | undoneBy:17 | %TX_POS |
  |  11 | %today-1d | transfer |   -120 | .ZZA | .ZZC | undoes:14 | %TX_POS |
  |  12 | %today-1d | transfer |     40 | .ZZC | .ZZE | labor   | %TX_WEB |
  |  13 | %today-1d | transfer |     10 | .ZZF | .ZZE | cash    | %TX_WEB |
  |  14 | %today-1d | transfer |     11 | .ZZF | .ZZE | cash    | %TX_WEB |  
  And usd transfers:
  | txid | payee | amount | completed |*
  |    1 | .ZZA  |    400 | %today-2m |  
  |    2 | .ZZB  |    100 | %today-2m |  
  |    3 | .ZZC  |    300 | %today-2m |  
  |    4 | .ZZE  |    200 | %today    |  
  |    5 | .ZZF  |   -600 | %today    |  
  |    6 | .ZZC  |   -500 | %today    |
  And member field values:
  | id   | field      | value |*
  | .ZZA | community  |    -2 |
  | .ZZB | mediaConx  |    12 |
  | .ZZE | postalAddr | Box 5 |
# don't set community to -2 until after transactions  
  When cron runs "trust"
  Then members:
  | id   | trust |*
  | .ZZA |  8.57 |
  | .ZZB |  8.57 |
  | .ZZD |  8.57 |
  | .ZZE |     1 |
  | .ZZF |     1 |
  | .ZZG |     1 |
  | .ZZH |     1 |
  | .ZZI |     1 |

Scenario: We calculate risks
  When cron runs "acctRisk"
  Then members:
  | id   | risks |*
  | .ZZA | adminOk,trusted,geography,badConx,moreOut,big7Week |
  | .ZZB | trusted,socialConx,moves,rents,moreIn,moreOut |
  | .ZZC | cashCo,homeCo,miser,bigDay |
  | .ZZD | trusted,hasBank |
  | .ZZE | new,shady,poBox,moreIn |
  | .ZZF | miser,photoOff,bigDay,bigWeek |
  | .ZZG | new,moves,badConx,addrOff |
  | .ZZH | moves,ssnOff |
  | .ZZI | new,moves,fishy |
# Do not specify exact risk because minor tweaks in the calculations cause major changes
  When cron runs "txRisk"
  Then transactions:
  | xid | risks |*
  |   1 | |
  |   2 | |
  |   3 | |
  |   4 | exchange,p2p |
  |   5 | cashIn,inhouse,toSuspect,biggestFrom,biggestTo |
  |   6 | p2p,biggestTo |
  |   7 | cashOut,toSuspect,biggestFrom |
  |   8 | p2p,toSuspect,biggestTo,offline,firstOffline |
  |   9 | exchange,p2p,absent,invoiceless,bigFrom,bigTo |
  |  10 | inhouse,toSuspect,oftenFrom,oftenTo |
  |  11 | fromSuspect,biggestFrom,biggestTo,origins |
  |  12 | b2p,fromSuspect,toSuspect,absent,invoiceless,biggestTo,origins |
  |  13 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,bigFrom,bigTo,suspectOut |
  |  14 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,biggestFrom,suspectOut |
  When cron runs "achRisk"
  Then usd transfers:
  | txid | payee | risks |*
  |    1 |  .ZZA | |
  |    2 |  .ZZB | |
  |    3 |  .ZZC | |
  |    4 |  .ZZE | toSuspect |
  |    5 |  .ZZF | toBank,suspectOut |
  |    6 |  .ZZC | toBank,origins,suspectOut |
