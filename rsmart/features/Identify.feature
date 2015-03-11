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
  | id   | fullName   | email | city  | state | cc  | cc2  | flags      |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 | ok,bona    |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 | ok,bona    |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      | ok,co,bona |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 | ok,bona    |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 | ok,bona,secret |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      | ok,co,bona |
  | .ZZG | Gil Seven  | g@    | Gtown | GA    | ccG |      |            |
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
  | id   | main | agent | permission | rCard |*
  | :ZZA | .ZZC | .ZZA  | buy        | yes   |
  | :ZZB | .ZZC | .ZZB  | scan       | yes   |
  | :ZZD | .ZZC | .ZZD  | read       |       |
  | :ZZE | .ZZF | .ZZE  | buy        |       |
  And transactions: 
  | created   | type   | amount | from | to   | purpose |*
  | %today-6m | signup | 250    | ctty | .ZZA | signup  |
  | %today-6m | signup | 250    | ctty | .ZZB | signup  |
  | %today-6m | signup | 250    | ctty | .ZZC | signup  |
  | %today-6m | signup | 250    | ctty | .ZZD | signup  |
  | %today-6m | signup | 250    | ctty | .ZZE | signup  |

Scenario: a cashier signs in
  When agent "" asks device "devC" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other |     | NEW.ZZC | Corner Pub | %now |

Scenario: Device has no identifier yet
  When agent "" asks device "" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | device | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other |     | ?      | NEW.ZZC | Corner Pub | %now |

Scenario: Device should have an identifier
  When agent ":ZZA" asks device "" to identify "ZZB-ccB2"
  Then we return error "missing device"

Scenario: a cashier signs in, signing another cashier out
  When agent ":ZZA" asks device "devC" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other |     | NEW.ZZC | Corner Pub | %now |

Scenario: a manager signs in
  When agent "" asks device "devC" to identify "ZZA-ccA2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | default | company    | time |*
  | 1  | Abe One | 1     | this,that,other | refund,r4usd | NEW.ZZC | Corner Pub | %now |

Scenario: a cashier scans a customer card
  When agent ":ZZB" asks device "devC" to identify "ZZD.ccD"
  Then we respond with:
  | ok | name     | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | Dtown, DE |         | 0     | 250     | 250     |

Scenario: the default cashier scans a customer card
  When agent ".ZZC" asks device "devC" to identify "ZZD.ccD"
  Then we respond with:
  | ok | name     | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | Dtown, DE |         | 0     | 250     | 250     |
  
Scenario: an unauthorized cashier scans in
  When agent "" asks device "devC" to identify "ZZD-ccD2"
  Then we return error "agent cannot scan"

Scenario: a cashier scans a customer card before signing in
  When agent "" asks device "devC" to identify "ZZD.ccD"
  Then we return error "scan in first"

Scenario: a cashier asks us to identify the cashier's own card
  When agent ":ZZB" asks device "devC" to identify "ZZB-ccB2" 
  Then we return error "already scanned in"
  
Scenario: a cashier scans a customer card whose balance is secret
  When agent ":ZZB" asks device "devC" to identify "ZZE.ccE"
  Then we respond with:
  | ok | name     | place     | company | logon | balance | rewards |*
  | 1  | Eve Five | Etown, IL |         | 0     | *250    | 250     |

Scenario: a cashier scans a company customer card
  When agent ":ZZB" asks device "devC" to identify "ZZE-ccE2"
  Then we respond with:
  | ok | name     | place     | company | logon |*
  | 1  | Eve Five | Ftown, FL | Far Co  | 0     |

Scenario: Device asks for a picture to go with the QR
  Given member ".ZZB" has picture "picture1"
  When agent ":ZZA" asks device "devC" for a picture of member ".ZZB" with card code "ccB"
  Then we respond with picture "picture1"

Scenario: Device asks for a picture but there isn't one
  Given member ".ZZB" has no picture
  When agent ":ZZA" asks device "devC" for a picture of member ".ZZB" with card code "ccB"
  Then we respond with picture "no photo"

Scenario: Device asks for a picture with the wrong card code
  Given member ".ZZB" has picture "picture1"
  When agent ":ZZA" asks device "devC" for a picture of member ".ZZB" with card code %random
  Then we respond with picture "bad customer"
  
Scenario: A non-yet-active member card is scanned
  When agent ":ZZB" asks device "devC" to identify "ZZG.ccG"
  Then we return error "member inactive" with subs:
  | name      |*
  | Gil Seven |
  
Scenario: A member makes a purchase for the first time
  Given member ".ZZD" has no photo ID recorded
  When agent ".ZZC" asks device "devC" to identify "ZZD.ccD"
  Then we respond with:
  | ok | name     | place     | company | logon | balance | rewards |*
  | 1  | Dee Four | Dtown, DE |         | -1    | 250     | 250     |