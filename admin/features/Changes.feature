Feature: Changes
AS a community administrator
I WANT to review the significant changes to an rCredits account
SO I can provide informed customer service

AS an overall administrator
I WANT to review the significant changes to an rCredits account
SO I can request changes to software, that will enhance the experience of rCredits members

Setup:
  Given members:
  | id   | fullName | address | city | state | flags        | minimum | achMin | savings | saveWeekly | share |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | ok,bona      |     100 |     10 |       0 |          0 |    20 |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | ok,bona,debt |     200 |     20 |       0 |          0 |    50 |
  | .ZZC | Cor Pub  | 3 C St. | Cton | CA    | ok,co,bona   |     300 |     30 |       0 |          0 |    50 |
  | .ZZD | Dee Four | 4 D St. | Dton | DE    | ok,admin     |     400 |     40 |       0 |          0 |    50 |

Scenario: A member changes some settings
  Given member ".ZZA" completes form "settings/preferences" with values:
#  | minimum | achMin | savings | smsNotices | notices | statements | debtOk | secretBal | share |*
#  |     100 |     11 |       0 |          0 |       1 |          0 |      1 |         0 |    25 |
  | minimum | achMin | savings | saveWeekly | smsNotices | notices | statements | secretBal | share |*
  |     100 |     11 |       0 |          0 |          0 |  weekly | electronic |         0 |    25 |
  When member ".ZZD" visits page "sadmin/changes/NEW.ZZA"
  Then we show "Account Changes for Abe One" with:
  | Date | Field   | Old Value      | New Value                  |
  | %dmy | achMin  | 10             | 11                         |
  | %dmy | share   | 20             | 25                         |
#  | %dmy | flags   | member ok bona | member ok bona weekly debt |
  | %dmy | flags   | member ok bona | member ok bona weekly      |
