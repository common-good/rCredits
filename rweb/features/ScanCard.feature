Feature: Scan Card
AS a member
I WANT to use my rCard to make payments
SO I can easily buy stuff through the rCredits system.
AND
AS a member company
I WANT to scan customer rCards
SO I can charge them easily through the rCredits system.
AND
AS an agent of a member company
I WANT to scan in using my company rCard
SO I don't have to type my account ID or remember my password.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | email         | flags                |
  | .ZZA | Abe One    | POB 1   | Atown | Alaska | 01000      | US      | a@example.com | dft,ok,personal,bona |
  | .ZZB | Bea Two    | POB 2   | Btown | Utah   | 02000      | US      | b@example.com | dft,ok,personal,bona |
  | .ZZC | Corner Pub | POB 3   | Ctown | Cher   |            | France  | c@example.com | dft,ok,company,bona  |
  And relations:
  | id   | main | agent | permission |
  | :ZZA | .ZZA | .ZZB  | buy        |
  | :ZZB | .ZZB | .ZZA  | read       |
  | :ZZC | .ZZC | .ZZB  | buy        |
  | :ZZD | .ZZC | .ZZA  | sell       |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  And usd:
  | id   | usd  |
  | ctty | 1000 |
  | .ZZA |  100 |
  | .ZZB |  200 |
  | .ZZC |  300 |
  Then balances:
  | id   | r    |
  | ctty | -750 |
  | .ZZA |  250 |
  | .ZZB |  250 |
  | .ZZC |  250 |

Scenario: A member uses an rCard to pay
  Given member ".ZZB" card code is "WhAt3v3r"
  When member ":ZZD" visits page "I/NEW.ZZB-WhAt3v3r"
  Then we show "Bea Two" with:
  | Location  |
  | Btown, UT |
