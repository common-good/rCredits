Feature: Identify
AS a cashier
I WANT to scan myself in or validate a customer's rCard
SO I can charge customers on behalf of my company.

# A is a manager
# B is an ordinary cashier
# D is an individual customer
# E is a company agent customer

Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | flags      | floor |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 | ok         |     0 |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 | ok         |     0 |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      | ok,co      |     0 |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 | ok         |  -444 |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 | ok,secret  |  -555 |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      | ok,co      |     0 |
  | .ZZG | Gil Seven  | g@    | Gtown | GA    | ccG |      |            |     0 |
  And devices:
  | id   | code |*
  | .ZZC | devC |
  And selling:
  | id   | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | id   | flags        |*
  | .ZZC | refund,r4usd |
  And relations:
  | main | agent | num | permission | rCard |*
  | .ZZC | .ZZA  |   1 | buy        | yes   |
  | .ZZC | .ZZB  |   2 | scan       | yes   |
  | .ZZC | .ZZD  |   3 | read       |       |
  | .ZZF | .ZZE  |   1 | buy        |       |
  And transactions: 
  | created   | type   | amount | from | to   | purpose |*
  | %today-6m | signup | 250    | ctty | .ZZA | signup  |
  | %today-6m | signup | 250    | ctty | .ZZB | signup  |
  | %today-6m | signup | 250    | ctty | .ZZC | signup  |
  | %today-6m | signup | 250    | ctty | .ZZD | signup  |
  | %today-6m | signup | 250    | ctty | .ZZE | signup  |

Scenario: a cashier signs in
  When agent "" asks device "devC" to identify "C:B,ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | bad | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other | refund,r4usd |     | NEWZZC  | Corner Pub | %now |

Scenario: Device has no identifier yet
  When agent "" asks device "" to identify "C:B,ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | bad | device | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other | refund,r4usd |     | ?      | NEWZZC  | Corner Pub | %now |

Scenario: Device should have an identifier
  When agent "C:A" asks device "" to identify "C:B,ccB2"
  Then we return error "missing device"

Scenario: a cashier signs in, signing another cashier out
  When agent "C:A" asks device "devC" to identify "C:B,ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | bad | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other | refund,r4usd |     | NEWZZC  | Corner Pub | %now |

Scenario: a manager signs in
  When agent "" asks device "devC" to identify "C:A,ccA2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | bad | default | company    | time |*
  | 1  | Abe One | 1     | this,that,other | refund,r4usd |     | NEWZZC  | Corner Pub | %now |

Scenario: a cashier scans a customer card
  When agent "C:B" asks device "devC" to identify ".ZZD,ccD"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | this,that,other | refund,r4usd |     | Dtown, DE |         | 0     | 0       | 444     |

Scenario: the default cashier scans a customer card
  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | this,that,other | refund,r4usd |     | Dtown, DE |         | 0     | 0       | 444     |

Scenario: a customer scans their own card for self-service
  Given members have:
  | id   | pin  |*
  | .ZZD | 4444 |
  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD" with PIN "4444"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | this,that,other | refund,r4usd |     | Dtown, DE |         | 0     | 0       | 444     |

Scenario: a customer scans their own card for self-service with wrong PIN
  Given members have:
  | id   | pin  |*
  | .ZZD | 4444 |
  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD" with PIN "1234"
  Then we return error "bad pin"

Scenario: the default cashier scans a de-activated card
  When we change member ".ZZD" cardCode
  Then bad codes ".ZZD,ccD"
  // member reported lost card, we just changed cardCode, now the member (or someone) tries to use the card with app online:
  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD"
  Then we return error "bad member"
  
Scenario: an unauthorized cashier scans in
  When agent "" asks device "devC" to identify "C:D,ccD2"
  Then we return error "agent cannot scan"

Scenario: a cashier scans a customer card before signing in
  When agent "" asks device "devC" to identify ".ZZD,ccD"
  Then we return error "scan in first"

Scenario: a cashier asks us to identify the cashier's own card
  When agent "C:B" asks device "devC" to identify "C:B,ccB2" 
  Then we return error "already scanned in"
  
Scenario: a cashier scans a customer card whose balance is secret
  When agent "C:B" asks device "devC" to identify ".ZZE,ccE"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
  | 1  | Eve Five | this,that,other | refund,r4usd |     | Etown, IL |         | 0     | *0      | 555     |

Scenario: a cashier scans a company customer card
  When agent "C:B" asks device "devC" to identify "F:E,ccE2"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon |*
  | 1  | Eve Five | this,that,other | refund,r4usd |     | Ftown, FL | Far Co  | 0     |

Scenario: Device asks for a picture to go with the QR
  Given member ".ZZB" has picture "picture1"
  When agent "C:A" asks device "devC" for a picture of member ".ZZB" with card code "ccB"
  Then we respond with picture "picture1"

Scenario: Device asks for a picture but there isn't one
  Given member ".ZZB" has no picture
  When agent "C:A" asks device "devC" for a picture of member ".ZZB" with card code "ccB"
  Then we respond with picture "no photo"

Scenario: Device asks for a picture with the wrong card code
  Given member ".ZZB" has picture "picture1"
  When agent "C:A" asks device "devC" for a picture of member ".ZZB" with card code "garbage#@!"
  Then we respond with picture "bad member"
  
Scenario: A non-yet-active member card is scanned
  When agent "C:B" asks device "devC" to identify ".ZZG,ccG"
  Then we return error "member inactive" with subs:
  | name      |*
  | Gil Seven |
  
Scenario: A member makes a purchase for the first time
  Given member ".ZZD" has no photo ID recorded
  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD"
  Then we respond with:
  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | this,that,other | refund,r4usd |     | Dtown, DE |         | -1    | 0       | 444     |

# disabled because "fast" bit is no longer used  
#Scenario: A member makes a purchase for the first time from an exempt company
#  Given member ".ZZD" has no photo ID recorded
#  And company flags:
#  | id   | flags             |*
#  | .ZZC | refund,r4usd,fast |
#  When agent ".ZZC" asks device "devC" to identify ".ZZD,ccD"
#  Then we respond with:
#  | ok | name     | descriptions    | can          | bad | place     | company | logon | balance | rewards |*
#  | 1  | Dee Four | this,that,other | refund,r4usd |     | Dtown, DE |         | 0     | 0       | 250     |