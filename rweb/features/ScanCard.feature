Feature: Scan Card
AS a non-member
I WANT to scan someone's rCard
SO I can find out about rCredits.
AND
AS a member company
I WANT to have someone scan my Company Agent rCard
SO I can show them my company's rCredits web page.

Setup:
  Given members:
  | id   | fullName   | flags      | cc  | cc2  | address | city | phone        |*
  | .ZZA | Abe One    | ok         | ccA | ccA2 | 1 A St. | Aton | +12000000001 |
  | .ZZB | Bea Two    | ok         | ccB | ccB2 | 2 B St. | Bton | +12000000002 |
  | .ZZC | Corner Pub | ok,co      |     |      | 3 C St. | Cton | +12000000003 |
  And relations:
  | id   | main | agent | permission |*
  | .ZZB | .ZZC | .ZZB  | read       |

Scenario: Someone scans a member card
  When member "?" visits page "I/NEWZZB"
  Then we redirect to "%PROMO_URL/?region=NEW"

Scenario: Someone scans a company agent card
  When member "?" visits page "I/NEWZZC"
  Then we show "Corner Pub" with:
#  |~Address  |~phone        | ~Button |*
#  | Cton     | 200.000.0003 | Pay     |
  |~Address  |~phone        |
  | Cton     | 200 000 0003 |