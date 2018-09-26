Feature: Transact
AS a member
I WANT to join the Investment Club and invest
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | id   | fullName | floor | flags                     | state |*
  | .ZZA | Abe One  |  -250 | ok,confirmed,debt,icadmin |    MA |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt,icadmin |    MA |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co         	|    MA |
	| .ZZI | In Club  |     0 | ok,confirmed,co,icadmin   |    MA |
	# for now, all MA accounts have the same iclub

Scenario: A member joins the investment club
  When member ".ZZA" visits page "invest"
	Then we show "Join Your"

  When member ".ZZA" completes form "invest" with values:
  | signedBy |*
  | Abe One  |
  And member ".ZZA" visits page "invest"
  Then we show "Investment Club" with:
	| Club Value | $0 ($0 liquid) |
	| Your Share | $0 (0.00%) |
	| Invest |  |
