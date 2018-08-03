Feature: Checks and Deposits
AS an administrator
I WANT to print checks from members to rCredits
SO we can accept their US Dollars in exchange for rCredits

AND I WANT to print checks from rCredits to members
So we can accommodate members' requests to cash out

AND I WANT to display deposit statements and individual checks for current and past deposits
SO I can make deposits easily and review past deposits as necessary

Setup:
  Given members:
  | id   | fullName | floor | flags            | postalAddr          | phone | bankAccount     |*
  | .ZZA | Abe One  | -500  | ok,admin    | 1 A, Aton, MA 01001 |     1 | USkk21187028101 |
  | .ZZB | Bea Two  | -500  | ok,co            | 2 B, Bton, MA 01002 |     2 | USkk21187028102 |
  | .ZZC | Cor Pub  |    0  | ok,co            | 3 C, Cton, MA 01003 |     3 | USkk21187028103 |
  
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   1 | %today-2m | signup   |    100 | ctty | .ZZA | signup  |
  |   2 | %today-2m | signup   |    100 | ctty | .ZZB | signup  |
  |   3 | %today-2m | signup   |    100 | ctty | .ZZC | signup  |
  
Scenario: admin prints checks
  Given usd transfers:
  | txid | payee | amount | created   | deposit   | completed |*
  | 5001 | .ZZA  |    100 | %today-3w | %today-2w | %today-3w |
  | 5002 | .ZZA  |    400 | %today-2w |         0 | %today    |
  | 5003 | .ZZB  |    100 | %today-1d |         0 | %today    |  
  | 5004 | .ZZC  |    300 | %today    |         0 |         0 |
  When member ".ZZA" visits page "sadmin/deposits"
  Then we show "Bank Transfers" with:
  | New IN | 3 |
# UNUSED  | Include   | 1 checks from %dm-2w |
  When member ".ZZA" visits page "sadmin/checks/way=IN&date=0&mark=1"
  Then we show pdf with:
  |~name    |~postalAddr          |~phone        |~transit      |~acct |~txid |~date |~amt   |~amount |~bank |*
  | Abe One | 1 A, Aton, MA 01001 | 413 772 0001 | 53-7028/2118 |   01 | 5002 | %dmy | $ 400 | Four Hundred and NO/100 | Greenfield Co-op Bank |
  | Bea Two | 2 B, Bton, MA 01002 | 413 772 0002 | 53-7028/2118 |   02 | 5003 | %dmy | $ 100 | One Hundred and NO/100 | Greenfield Co-op Bank |
  | Cor Pub | 3 C, Cton, MA 01003 | 413 772 0003 | 53-7028/2118 |   03 | 5004 | %dmy | $ 300 | Three Hundred and NO/100 | Greenfield Co-op Bank |  
  And usd transfers:
  | txid | deposit   |*
  | 5002 | %today    |
  | 5003 | %today    |
  | 5004 | %today    |
  And balances:
  | id   | balance |*
  | .ZZA |     500 |
  | .ZZB |     100 |
  | .ZZC |       0 |
