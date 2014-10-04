Feature: Risk
AS a Common Good Community Administrator or Member or Compliance Officer at a partnering financial institution
I WANT to know what financial or regulatory risks each account, transaction, and ACH pose
SO I can handle those risks appropriately

Setup:
  Given members:
  | id   | fullName   | email | address | postalCode | flags   | risks    | tenure | moves | share |*
  | .ZZA | Abe One    | a@    | 1 A St. |      01001 | ok,bona | adminOk  | 21     | 0     |    10 |
  | .ZZB | Bea Two    | b@    | 2 A St. |      01001 | ok,bona | rents    | 13     | 1     |    20 |
  | .ZZC | Corner Pub | c@    | 3 C St. |      01003 | ok,co   | cashCo   | 18     |       |     1 |
  | .ZZD | Dee Four   | d@    | 3 C St. |      01003 | ok,bank |          | 15     | 0     |     5 |
  | .ZZE | Eve Five   | e@    | 5 A St. |      01001 | ok,bona | shady    | 1      | 0     |     8 |
  | .ZZF | Flo Six    | f@    | 6 A St. |      01001 | ok,bona | photoOff | 12     | 0     |    50 |
  | .ZZG | Guy Seven  | g@    | 7 A St. |      01001 | ok      | addrOff  | 11     | 5     |    25 |
  | .ZZH | Hal Eight  | h@    | 8 A St. |      01001 | ok,bona | ssnOff   | 100    | 10    |    25 |
  | .ZZI | Ida Nine   | i@    | 9 A St. |      01001 | ok      | fishy    | 3      | 20    |    25 |
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
  | main | agent | permission | employee | isOwner | draw |*
  | .ZZC | .ZZA  | scan       |        1 |       0 |    0 |
  | .ZZC | .ZZB  |            |          |         |      |
  | .ZZC | .ZZD  |            |          |         |      |
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
  |   5 | %today-1m | transfer |   1100 | .ZZA | .ZZC | USD in  | %TX_POS |
  |   6 | %today-3w | transfer |    240 | .ZZA | .ZZB | what G  | %TX_POS |
  |   7 | %today-3w | rebate   |     12 | ctty | .ZZA | rebate  | %TX_POS |
  |   8 | %today-3w | bonus    |     24 | ctty | .ZZB | bonus   | %TX_POS |
  |   9 | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P  | %TX_POS |
  |  10 | %today-1w | transfer |    120 | .ZZA | .ZZH | offline | %TX_POS |
  |  11 | %today-1w | rebate   |      6 | ctty | .ZZA | rebate  | %TX_POS |
  |  12 | %today-1w | bonus    |     12 | ctty | .ZZH | bonus   | %TX_POS |
  |  13 | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V  | %TX_WEB |
  |  14 | %today-1d | transfer |    120 | .ZZA | .ZZC | undoneBy:17 | %TX_POS |
  |  17 | %today-1d | transfer |   -120 | .ZZA | .ZZC | undoes:14 | %TX_POS |
  |  20 | %today-1d | transfer |     40 | .ZZC | .ZZE | labor   | %TX_WEB |
  |  23 | %today-1d | transfer |     10 | .ZZF | .ZZE | cash    | %TX_WEB |
  |  24 | %today-1d | transfer |     11 | .ZZF | .ZZE | cash    | %TX_WEB |  
  And usd transfers:
  | txid | payer | payee | amount | completed |*
  |    1 | .ZZA  |     0 |   -400 | %today-2m |  
  |    2 | .ZZB  |     0 |   -100 | %today-2m |  
  |    3 | .ZZC  |     0 |   -300 | %today-2m |  
  |    4 | .ZZE  |     0 |   -200 | %today    |  
  |    5 | .ZZF  |     0 |    600 | %today    |  
  |    6 | .ZZC  |     0 |    500 | %today    |
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
  | .ZZC |     0 |
  | .ZZD |  8.57 |
  | .ZZE |     0 |
  | .ZZF |     0 |
  | .ZZG |     0 |
  | .ZZH |     0 |
  | .ZZI |     0 |

Scenario: We calculate risks
  When cron runs "acctRisk"
  Then members:
  | id   | risk    | risks |*
  | .ZZA | -124.96 | adminOk,trusted,geography,badConx,moreIn,moreOut,big7Week |
  | .ZZB | -431.17 | trusted,socialConx,moves,rents,moreIn |
  | .ZZC |  391.67 | cashCo,homeCo,miser,bigDay,bigWeek,big7Week |
  | .ZZD | -441.83 | trusted,hasBank |
  | .ZZE |  403.33 | new,shady,poBox,moreOut |
  | .ZZF |  225.00 | photoOff,bigDay |
  | .ZZG |  647.27 | new,moves,badConx,addrOff |
  | .ZZH | 1103.00 | moves,ssnOff |
  | .ZZI | 2150.00 | new,moves,fishy |
# Do not specify exact risk because minor tweaks in the calculations cause major changes
  When cron runs "txRisk"
  Then transactions:
  | xid | risks |*
  |   1 | |
  |   2 | |
  |   3 | toSuspect |
  |   4 | exchange,p2p |
  |   5 | cashIn,inhouse,toSuspect,biggestFrom,biggestTo |
  |   6 | p2p,biggestTo |
  |   7 |   |
  |   8 |   |
  |   9 | cashOut,toSuspect,biggestFrom |
  |  10 | p2p,toSuspect,biggestTo,offline,firstOffline |
  |  13 | exchange,p2p,absent,invoiceless,bigFrom,bigTo |
  |  14 | inhouse,toSuspect,oftenFrom,oftenTo |
  |  17 | fromSuspect,biggestFrom,biggestTo,origins |
  |  20 | b2p,fromSuspect,toSuspect,absent,invoiceless,biggestTo,origins |
  |  23 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,bigFrom,bigTo,suspectOut |
  |  24 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,biggestFrom,suspectOut |
  When cron runs "achRisk"
  Then usd transfers:
  | txid | risks |*
  |    1 |  |
  |    2 |  |
  |    3 | toSuspect |
  |    4 | toSuspect |
  |    5 | toBank,suspectOut |
  |    6 | toBank,origins,suspectOut |
