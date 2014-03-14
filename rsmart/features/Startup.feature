Feature: Start up
AS a member
I WANT to run the rCredits POS app on my device
SO I can use it to charge customers through the rCredits system.

Setup:
  Given members:
  | id   | fullName   | phone  | email | cc  | cc2  | flags      |
  | .ZZA | Abe One    | +20001 | a@    | ccA | ccA2 | ok,bona    |
  | .ZZB | Bea Two    | +20002 | b@    | ccB |      | ok,bona    |
  | .ZZC | Corner Pub | +20003 | c@    | ccC |      | ok,co,bona |
  | .ZZF | For Co     | +20006 | f@    | ccF |      | co         |
  And devices:
  | id   | code |
  | .ZZC | devC |
  And relations:
  | id   | main | agent | permission |
  | :ZZA | .ZZC | .ZZA  | buy        |
  | :ZZB | .ZZC | .ZZB  | scan       |
  | :ZZE | .ZZF | .ZZA  | scan       |

Scenario: Device requests a bad op
  When agent ":ZZA" asks device "devC" for op %random with: ""
  Then we return error "bad op"

Scenario: Device should have an identifier
  When agent ":ZZA" asks device "" for op "charge" with:
  | member | code |
  | .ZZB   | ccB  |
  Then we return error "missing device"
  
Scenario: Device gives a bad code
  When agent ":ZZA" asks device %random for op "identify" with:
  | member | code |
  | .ZZB   | ccB  |
  Then we return error "unknown device"

Scenario: An Agent for an inactive company tries an op
  When agent ":ZZE" asks device "devC" for op "charge" with: ""
  Then we return error "company inactive"