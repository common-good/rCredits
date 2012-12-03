Feature: Info transaction pre-launch
AS a player
I WANT to simulate a transfer of rCredits from my account to someone else's or vice versa
SO I will get a rebate or bonus that I can spend once I am an active participant

Scenario: A notyet asks to pay a member id
  Given players:
  | @uid | @name      | @memberid | @cell    | @type  | @rebate | @postal_code |
  | 1    | Abe One    | neabcdea  | %number1 | player | 5       | 01301        |
  | 2    | Bea Two    | neabcdeb  | %number2 | active | 5       | 01302        |
  | 3    | Corner Pub | neabcdec  | %number3 | active | 5       | 01301        |
  When phone %number1 says "123.45 to neabcdeb for groceries"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type   | @amount | @who   | @whatfor           |
  | Pay     | 123.45 | Bea Two | goods and services |
  # "INFO Pay Bea Two $123.45 for goods and services? Type MANGO to confirm."

# Scenario: An active participant asks to pay a notyet
# (by memberid, phone number, email, name)

Scenario: The caller asks to pay a player by name
# or charge (or info pay or charge). Nickname matches the full name of a local business, organization, or individual account
  Given players:
  | @uid | @name      | @memberid | @cell    | @type  | @rebate | @postal_code |
  | 1    | Abe One    | neabcdea  | %number1 | player | 5       | 01301        |
  | 2    | Bea Two    | neabcdeb  | %number2 | active | 5       | 01302        |
  | 3    | Corner Pub | neabcdec  | %number3 | active | 5       | 01302        |
# And phone %number1 chosen nicknames do not include "cornerpub"
  When phone %number1 says "123.45 to .cornerpub for groceries"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type   | @amount | @who       | @whatfor           |
  | Pay     | 123.45  | Corner Pub | goods and services |
  # "INFO Pay Corner Pub $123.45 for goods and services? Type MANGO to confirm."

Scenario: The caller asks to charge a member id
  Given players:
  | @uid | @name      | @memberid | @cell    | @type  | @rebate | @postal_code |
  | 1    | Abe One    | neabcdea  | %number1 | player | 5       | 01301        |
  | 2    | Bea Two    | neabcdeb  | %number2 | active | 5       | 01302        |
  | 3    | Corner Pub | neabcdec  | %number3 | active | 5       | 01302        |
  When phone %number1 says "123.45 from neabcdec for labor"
  Then we say to phone %number1 "confirm transaction" with subs:
  | @type   | @amount | @who       | @whatfor           |
  | Charge  | 123.45  | Corner Pub | goods and services |
  # "INFO Charge Corner Pub $123.45 for goods and services? Type MANGO to confirm."

Scenario: The caller confirms an informational payment
  Given players:
  | @uid | @name      | @memberid | @cell    | @type  | @rebate | @postal_code |
  | 1    | Abe One    | neabcdea  | %number1 | player | 5       | 01301        |
  | 2    | Bea Two    | neabcdeb  | %number2 | active | 5       | 01302        |
  | 3    | Corner Pub | neabcdec  | %number3 | active | 5       | 01302        |
  | 4    | Community  | neab0000  |          | active | 0       | 013          |
  And transactions: 
  | @transaction_id | @date     | @type    | @amount | @from | @to | @whatfor     |
  | 1               | %today-1d | ipayment | 100.00  | 1     | 3   | whatever     |
  | 2               | %today-1d | irebate  | 5.00    | 4     | 1   | rebate on #1 |
  | 3               | %today-1d | ibonus   | 10.00   | 4     | 3   | bonus on #1  |
  When phone %number1 confirms "123.45 to neabcdec for groceries"
  Then the community has r$-33.52
  And phone %number3 has r$22.35
  And transactions:
  | @transaction_id | @date  | @type    | @amount | @from | @to | @whatfor     |
  | 4               | %today | ipayment | 123.45  | 1     | 3   | whatever     |
  | 5               | %today | irebate  | 6.17    | 4     | 1   | rebate on #4 |
  | 6               | %today | ibonus   | 12.35   | 4     | 3   | bonus on #4  |
  And we say to phone %number1 "report transaction" with subs:
  | @type   | @tofrom | @amount | @who       | @whatfor           | @rewardType | @rewardAmount | @balance | @transaction | moreless | unavailable |
  | Payment | to      | 123.45  | Corner Pub | goods and services | rebate       | 6.17           | 11.17    | 3            | less     | 11.17       |
  # "INFO Payment: $123.45 to Corner Pub (rebate: $6.17). Your new balance is $11.17, including $11.17 not yet available. (If this were an ACTUAL Payment, your balance would be $123.45 less.) Transaction #3"
Scenario: The caller confirms an informational charge
  Given players:
  | @uid | @name      | @memberid | @cell    | @type  | @rebate | @postal_code |
  | 1    | Abe One    | neabcdea  | %number1 | player | 5       | 01301        |
  | 2    | Bea Two    | neabcdeb  | %number2 | active | 5       | 01302        |
  | 3    | Corner Pub | neabcdec  | %number3 | active | 5       | 01302        |
  | 4    | Community  | neab0000  |          | active | 0       | 013          |
  And transactions: 
  | @transaction_id | @date     | @type    | @amount | @from | @to | @whatfor     |
  | 1               | %today-1d | ipayment | 100.00  | 1     | 3   | whatever     |
  | 2               | %today-1d | irebate  | 5.00    | 4     | 1   | rebate on #1 |
  | 3               | %today-1d | ibonus   | 100.00  | 4     | 3   | bonus on #1  |
  When phone %number1 confirms "123.45 from neabcdec for labor"
  Then the community has r$-33.52
  And phone %number3 has r$16.17
  And transactions:
  | @transaction_id | @date  | @type    | @amount | @from | @to | @whatfor     |
  | 4               | %today | icharge  | 123.45  | 1     | 3   | whatever     |
  | 5               | %today | irebate  | 6.17    | 4     | 3   | rebate on #4 |
  | 6               | %today | ibonus   | 12.35   | 4     | 1   | bonus on #4  |
  And we say to phone %number1 "report transaction" with subs:
  | @type   | @tofrom | @amount | @who       | @whatfor | @rewardType | @rewardAmount | @balance | @transaction | more | unavailable |
  | Charge  | from    | 123.45  | Corner Pub | labor    | bonus        | 12.35          | 17.35    | 2            | more | 17.35       |
  # "INFO Charge: $123.45 from Corner Pub (bonus: $12.35. Your new balance is $17.35, including $17.35 not yet available. (If this were an ACTUAL Payment, your balance would be $123.45 more.) Transaction #2"
