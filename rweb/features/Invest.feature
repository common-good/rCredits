Feature: Transact
AS a member
I WANT to join the Investment Club and invest
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | id   | fullName | floor | flags                     |*
  | .ZZA | Abe One  |  -250 | ok,confirmed,debt,icadmin |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt,icadmin |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co           |

Scenario: A member joins the investment club
  When member ".ZZA" visits page "invest"
	Then we show "Join Your"

  When member ".ZZA" completes form "invest" with values:
  | signedBy |*
  | Abe One  |
  Then we show "Investment Club" with:
	| Club Value | $0 ($0 liquid) |
	| Your Share | $0 (0.00%) |
	| Invest |  |
