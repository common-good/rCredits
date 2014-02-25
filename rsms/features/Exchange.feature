Feature: Exchange for cash
AS a player
I WANT to exchange rCredits for someone's cash or vice versa
SO I can buy the things I want, locally and elsewhere

Setup:
  Given members:
  | id   | fullName   | number | flags           |
  | .ZZA | Abe One    | +20001 | person,ok,bona  |
  | .ZZB | Bea Two    | +20002 | person,ok,bona  |
  | .ZZC | Corner Pub | +20003 | company,ok,bona |
  # later and elsewhere: name, email, country, minimum, community
  And transactions: 
  | type     | amount | from | to   | purpose |
  | signup   | 250    | ctty | .ZZA | signup  |
  | signup   | 250    | ctty | .ZZB | signup  |
  | signup   | 250    | ctty | .ZZC | signup  |
  | transfer | 100    | .ZZB | .ZZA | goodies |
  Then balances:
  | id   | r    |
  | ctty | -765 |
  | .ZZA |  360 |
  | .ZZB |  155 |
  | .ZZC |  250 |

Scenario: The caller confirms a trade of rCredits for cash
  When phone +20001 confirms "100 to .ZZC for cash"
  Then balances:
  | id   | r    |
  | ctty | -765 |
  | .ZZA |  260 |
  | .ZZB |  155 |
  | .ZZC |  350 |
  And we say to phone +20001 "report exchange" with subs:
  | did    | otherName  | amount | balance | tid |
  | paid   | Corner Pub | $100   | $260    | 3   |
  # "You gave Corner Pub $100 cash/loan/etc. Your new balance is $150. Transaction #2"

Scenario: The caller confirms an implicit trade of rCredits for cash
  When phone +20001 confirms "100 to .ZZC"
  Then phone +20003 has r$350
  And phone +20001 has r$260

Scenario: The caller asks to trade cash for rCredits
  When phone +20003 says "100 from .ZZA for cash"
  Then phone +20003 has r$250
  And phone +20001 has r$360
  And we say to phone +20003 "no charging"
  Skip
  And we say to phone +20003 "report exchange request" with subs:
  | did     | otherName | amount | tid |
  | charged | Abe One    | $100   | 2   |
  # "You requested $100 from Abe One for cash/loan/etc. Your balance is unchanged, pending approval. Invoice transaction #2"
  Resume

Skip
Scenario: The caller confirms a unilateral trade of cash for rCredits
  Given phone +20003 can charge unilaterally
  When phone +20003 confirms "100 from .ZZA for cash"
  Then phone +20003 has r$350
  And phone +20001 has r$150
  And we say to phone +20003 "report exchange" with subs:
  | did     | otherName | amount | balance | tid |
  | charged | Abe One    | $100   | $350    | 2   |
  # "You charged Abe One $100 cash/loan/etc. Your new balance is $350. Transaction #2"
Resume

Scenario: The caller requests an implicit trade with insufficient balance
  When phone +20001 says "150 to .ZZC"
  Then we say to phone +20001 "short cash to" with subs:
  | short |
  | $50   |
  # "You are $50 short for that exchange."
  