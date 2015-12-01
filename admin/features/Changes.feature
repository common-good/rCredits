Feature: Changes
AS a community administrator
I WANT to review the significant changes to an rCredits account
SO I can provide informed customer service

AS an overall administrator
I WANT to review the significant changes to an rCredits account
SO I can request changes to software, that will enhance the experience of rCredits members

Setup:
  Given members:
  | id   | fullName | address | city | state | postalCode | email | flags        | minimum | achMin | share |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | 01000      | a@    | ok,bona      |     100 |     10 |    20 |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | 02000      | b@    | ok,bona,debt |     200 |     20 |    50 |
  | .ZZC | Cor Pub  | 3 C St. | Cton | CA    | 03000      | c@    | ok,co,bona   |     300 |     30 |    50 |
  | .ZZD | Dee Four | 4 D St. | Dton | DE    | 04000      | b@    | ok,admin     |     400 |     40 |    50 |

Scenario: A member changes some settings
  Given member ".ZZA" completes form "settings/preferences" with values:
  | minimum | achMin | smsNotices | notices | statements | debtOk | secretBal | share |*
  |     100 |     11 |          0 |       1 |          0 |      1 |         0 |    25 |
  When member ".ZZD" visits page "sadmin/changes/NEW.ZZA"
  Then we show "Account Changes for Abe One" with:
  | Date | Field   | Old Value      | New Value                  |
  | %dmy | achMin  | 10             | 11                         |
  | %dmy | share   | 20             | 25                         |
  | %dmy | flags   | member ok bona | member ok bona weekly debt |
