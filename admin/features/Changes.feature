Feature: Changes
AS a community administrator
I WANT to review the significant changes to an rCredits account
SO I can provide informed customer service

AS an overall administrator
I WANT to review the significant changes to an rCredits account
SO I can request changes to software, that will enhance the experience of rCredits members

Setup:
  Given members:
  | id   | fullName | address | city | state | flags    | minimum | achMin | saveWeekly | crumbs |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | ok,ided  |     100 |     10 |          0 |   0.02 |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | ok,debt  |     200 |     20 |          0 |   0.05 |
  | .ZZC | Cor Pub  | 3 C St. | Cton | CA    | ok,co    |     300 |     30 |          0 |   0.05 |
  | .ZZD | Dee Four | 4 D St. | Dton | DE    | ok,admin |     400 |     40 |          0 |   0.05 |

Scenario: A member changes some settings
  Given member ".ZZA" completes form "settings/preferences" with values:
#  | minimum | achMin | savingsAdd | smsNotices | notices | statements | debtOk | secretBal | share |*
#  |     100 |     11 |          0 |          0 |       1 |          0 |      1 |         0 |    25 |
  | roundup | notices | statements | secretBal | crumbs |*
  |       0 |  weekly |      paper |         0 |      1 |
  And member ".ZZA" completes form "settings/connect" with values:
  | connect | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  |       1 |     211870281 |         123 |          123 |       0 |       1 |    100 |     11 |          0 |
  When member ".ZZD" visits page "sadmin/changes/NEWZZA"
  Then we show "Account Changes for Abe One" with:
  | Date   | Field       | Old Value            | New Value                   | Changed by |
  | %dmy | flags       | ok ided              | ok ided weekly              | abeone     |
  | %dmy | flags       | ok ided weekly       | ok ided weekly paper        | abeone     |
  | %dmy | crumbs      |                 0.02 |                        0.01 | abeone     |
  | %dmy | flags       | ok ided weekly paper | ok ided refill weekly paper | abeone     |
  | %dmy | achMin      |                   10 |                          11 | abeone     |
  | %dmy | bankAccount |                      | USkk211870281123            | abeone     |
#  | %dmy | flags   | member ok bona | member ok bona weekly debt |
