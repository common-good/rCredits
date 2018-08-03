Feature: Start up
AS a member
I WANT to run the rCredits POS app on my device
SO I can use it to charge customers through the rCredits system.

Setup:
  Given members:
  | id   | fullName   | phone  | email | cc  | cc2  | flags      |*
  | .ZZA | Abe One    | +20001 | a@    | ccA | ccA2 | ok         |
  | .ZZB | Bea Two    | +20002 | b@    | ccB |      | ok         |
  | .ZZC | Corner Pub | +20003 | c@    | ccC |      | ok,co      |
  | .ZZF | For Co     | +20006 | f@    | ccF |      | co         |
  And devices:
  | id   | code |*
  | .ZZC | devC |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | scan       |

Scenario: Device requests a bad op
  When agent "C:A" asks device "devC" for op %random with: ""
  Then we return error "bad op"

Scenario: Device should have an identifier
  When agent "C:A" asks device "" for op "charge" with:
  | member | code |*
  | .ZZB   | ccB  |
  Then we return error "missing device"
  
Scenario: Device gives a bad code
  When agent "C:A" asks device %random for op "time" with: ""
  Then we return error "unknown device"

Scenario: An Agent for an inactive company tries an op
  When agent "F:A" asks device "devC" for op "charge" with: ""
  Then we return error "company inactive"