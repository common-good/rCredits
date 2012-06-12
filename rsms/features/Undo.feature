Feature: Undo
AS a player
I WANT to undo a transaction recently completed on my account
SO I can easily correct a mistake

Scenario: Undo the last transaction
  Given players:
  | @name      | @accountid| @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type   | @amount | @from    | @to      | @for         |
  | %today-4m | charge  | 11.11   | neabcdeb | neabcdea | cash         |
  | %today-3w | charge  | 22.22   | neabcdec | neabcdea | cash         |
  | %today-3d | payment | 33.33   | neabcdea | neabcdeb | cash         |
  | %today-2d | payment | 44.44   | neabcdeb | neabcdec | cash         |
  | %today-1d | payment | 55.55   | neabcdea | neabcdec | whatever5    |
  | %today-1d | rebate  | 2.78    | community| neabcdea | rebate on #4 |
  | %today-1d | bonus   | 5.56    | community| neabcdec | bonus on #3  |
  When phone %number1 says "undo"
  Then we say to phone %number1 "confirm undo" with subs:
  | @date     | @type   | @amount | @tofrom | @who       | @for      |
  | %today-1d | payment | 55.55   | to      | Corner Pub | whatever5 |
  # "Undo 01-02-2012 payment of 55.55 to Corner Pub?"

Scenario: Undo the last transaction with someone specific
  Given players:
  | @name      | @accountid| @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type   | @amount | @from    | @to      | @for         |
  | %today-4m | charge  | 11.11   | neabcdeb | neabcdea | cash         |
  | %today-3w | charge  | 22.22   | neabcdec | neabcdea | cash         |
  | %today-3d | payment | 33.33   | neabcdea | neabcdeb | whatever3    |
  | %today-3d | rebate  | 1.67    | community| neabcdea | rebate on #3 |
  | %today-3d | bonus   | 3.33    | community| neabcdeb | bonus on #2  |
  | %today-2d | payment | 44.44   | neabcdeb | neabcdec | cash         |
  | %today-1d | payment | 55.55   | neabcdea | neabcdec | cash         |
  When phone %number1 says "undo neabcdeb"
  Then we say to phone %number1 "confirm undo" with subs:
  | @date     | @type   | @amount | @tofrom | @who    | @for      |
  | %today-3d | payment | 33.33   | to      | Bea Two | whatever3 |
  # "Undo 05-17-2012 payment of 33.33 to Bea Two?"

Scenario: Undo the last transfer to me
  Given players:
  | @name      | @accountid| @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type   | @amount | @from    | @to      | @for         |
  | %today-4m | charge  | 11.11   | neabcdeb | neabcdea | cash         |
  | %today-3w | charge  | 22.22   | neabcdec | neabcdea | whatever2    |
  | %today-3w | rebate  | 1.11    | community| neabcdec | rebate on #1 |
  | %today-3w | bonus   | 2.22    | community| neabcdea | bonus on #2  |
  | %today-3d | payment | 33.33   | neabcdea | neabcdeb | cash         |
  | %today-2d | payment | 44.44   | neabcdeb | neabcdec | cash         |
  | %today-1d | payment | 55.55   | neabcdea | neabcdec | cash         |
  When phone %number1 says "undo from"
  Then we say to phone %number1 "confirm undo" with subs:
  | @date     | @type   | @amount | @tofrom | @who       | @for      |
  | %today-3w | charge  | 22.22   | from    | Corner Pub | whatever2 |
  # "Undo 01-03-2012 charge of 22.22 from Corner Pub?"

Scenario: Undo the last transfer from me to someone specific
  Given players:
  | @name      | @accountid| @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions: 
  | @date     | @type   | @amount | @from    | @to      | @for      |
  | %today-4m | charge  | 11.11   | neabcdeb | neabcdea | cash      |
  | %today-3w | charge  | 22.22   | neabcdec | neabcdea | cash      |
  | %today-3d | payment | 33.33   | neabcdea | neabcdeb | cash      |
  | %today-2d | payment | 44.44   | neabcdeb | neabcdec | cash      |
  | %today-1d | payment | 55.55   | neabcdea | neabcdec | cash      |
  When phone %number1 says "undo to neabcdeb"
  Then we say to phone %number1 "confirm undo" with subs:
  | @date     | @type   | @amount | @tofrom | @who    | @for     |
  | %today-3d | charge  | 33.33   | to      | Bea Two | cash     |
  # "Undo 01-03-2012 charge of 33.33 to Bea?"
  
Scenario: The caller confirms undoing a charge
  Given players:
  | @name      | @accountid| @cell    | @rebate |
  | Abe One    | neabcdea  | %number1 | 5       |
  | Bea Two    | neabcdeb  | %number2 | 5       |
  | Corner Pub | neabcdec  | %number3 | 5       |
  And transactions:
  | @date     | @type   | @amount | @from    | @to      | @for         |
  | %today-1d | payment | 100.00  | neabcdea | neabcdec | whatever     |
  | %today-1d | rebate  | 5.00    | community| neabcdea | rebate on #1 |
  | %today-1d | bonus   | 10.00   | community| neabcdec | bonus on #1  |
  When phone %number1 confirms "undo 1"
  Then the community has Pr$0
  And phone %number3 has Pr$0
  And transactions:
  | @date  | @type   | @amount | @from    | @to      | @for       |
  | %today | payment | -100    | neabcdea | neabcdec | cancels #1 |
  | %today | rebate  | -5      | community| neabcdea | cancels #2 |
  | %today | bonus   | -10     | community| neabcdec | cancels #3 |
  And we say to phone %number1 "report undo" with subs:
  | @old_id | @date     | @amount | @to_whom    | @from_whom | @balance | @transaction |
  | 1       | %today-1d | 100     | Corner Pub  | you        | 0        | 3            |
  # "CANCELED Transaction #1 (01-03-2012). $100 returned to Corner Pub from you (Transaction #3). Your new balance is $0."

