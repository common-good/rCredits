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
  | id   | fullName   | email | city  | state | cc  | cc2  | flags               |
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 | dft,ok,person,bona  |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 | dft,ok,person,bona  |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      | dft,ok,company,bona |
  | .ZZD | Dee Four   | d@    | Dtown | DE    | ccD | ccD2 | dft,ok,person,bona  |
  | .ZZE | Eve Five   | e@    | Etown | IL    | ccE | ccE2 | dft,ok,person,bona,secret_bal |
  | .ZZF | Far Co     | f@    | Ftown | FL    | ccF |      | dft,ok,company,bona |
  | .ZZG | Gil Seven  | g@    | Gtown | GA    | ccG |      | dft |
  And devices:
  | id   | code |
  | .ZZC | devC |
  And selling:
  | id   | selling         |
  | .ZZC | this,that,other |
  And company flags:
  | id   | flags            |
  | .ZZC | refund,sell cash |
  And relations:
  | id   | main | agent | permission |
  | :ZZA | .ZZC | .ZZA  | buy        |
  | :ZZB | .ZZC | .ZZB  | scan       |
  | :ZZD | .ZZC | .ZZD  | read       |
  | :ZZE | .ZZF | .ZZE  | buy        |
  And transactions: 
  | created   | type   | amount | from | to   | purpose |
  | %today-6m | signup | 250    | ctty | .ZZA | signup  |
  | %today-6m | signup | 250    | ctty | .ZZB | signup  |
  | %today-6m | signup | 250    | ctty | .ZZC | signup  |
  | %today-6m | signup | 250    | ctty | .ZZD | signup  |
  | %today-6m | signup | 250    | ctty | .ZZE | signup  |

Scenario: a cashier signs in
  When agent "" asks device "devC" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | default | company    |
  | 1  | Bea Two | 1     | this,that,other |     | NEW.ZZC | Corner Pub |

Scenario: a cashier signs in as required
  Given company flags:
  | id   | flags                            |
  | .ZZC | refund,sell cash,require cashier |
  When agent "" asks device "devC" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | default | company    |
  | 1  | Bea Two | 1     | this,that,other |     |         | Corner Pub |
  
Scenario: Device has no identifier yet
  When agent "" asks device "" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | device | default | company    |
  | 1  | Bea Two | 1     | this,that,other |     | ?      | NEW.ZZC | Corner Pub |

Scenario: Device should have an identifier
  When agent ":ZZA" asks device "" to identify "ZZB-ccB2"
  Then we return error "missing device"
 
Scenario: a cashier signs in, signing another cashier out
  When agent ":ZZA" asks device "devC" to identify "ZZB-ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can | default | company    |
  | 1  | Bea Two | 1     | this,that,other |     | NEW.ZZC | Corner Pub |

Scenario: a manager signs in
  When agent "" asks device "devC" to identify "ZZA-ccA2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can              | default | company    |
  | 1  | Abe One | 1     | this,that,other | refund,sell cash | NEW.ZZC | Corner Pub |

Scenario: a cashier scans a customer card
  When agent ":ZZB" asks device "devC" to identify "ZZD.ccD"
  Then we respond with:
  | ok | name     | place     | company | logon |
  | 1  | Dee Four | Dtown, DE |         | 0     |
  And with balance
  | name     | balance | spendable | cashable |
  | Dee Four | $250    |           | $0       |

Scenario: the default cashier scans a customer card
  When agent ".ZZC" asks device "devC" to identify "ZZD.ccD"
  Then we respond with:
  | ok | name     | place     | company | logon |
  | 1  | Dee Four | Dtown, DE |         | 0     |
  And with balance
  | name     | balance | spendable | cashable |
  | Dee Four | $250    |           | $0       |
  
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
  | ok | name     | place     | company | logon |
  | 1  | Eve Five | Etown, IL |         | 0     |
  And with balance ""

Scenario: a cashier scans a company customer card
  When agent ":ZZB" asks device "devC" to identify "ZZE-ccE2"
  Then we respond with:
  | ok | name     | place     | company | logon |
  | 1  | Eve Five | Ftown, FL | Far Co  | 0     |
  And with balance
  | name                 | balance | spendable | cashable |
  | Eve Five, for Far Co | $0      |           | $0       |

Scenario: Device asks for a picture to go with the QR
  Given member ".ZZB" has picture "picture1"
  When agent ":ZZA" asks device "devC" for a picture of member ".ZZB"
  Then we respond with picture "picture1"

Scenario: Device asks for a picture but there isn't one
  Given member ".ZZB" has no picture
  When agent ":ZZA" asks device "devC" for a picture of member ".ZZB"
  Then we respond with picture "no photo"

Scenario: A non-yet-active member card is scanned
  When agent ":ZZB" asks device "devC" to identify "ZZG.ccG"
  Then we return error "member inactive" with subs:
  | name      |
  | Gil Seven |