Feature: Exchange for cash
AS a player
I WANT to exchange rCredits for someone's cash or vice versa
SO I can buy the things I want, locally and elsewhere

Setup:
  Given members:
  | id   | full_name  | phone  |
  | .ZZA | Abe One    | +20001 |
  | .ZZB | Bea Two    | +20002 |
  | .ZZC | Corner Pub | +20003 |
  # later and elsewhere: name, email, country, minimum, community
  And transactions: 
  | created   | type       | amount | from      | to   | purpose |
  | %today-1d | %TX_SIGNUP | 250    | community | .ZZA | signup  |
  | %today-1d | %TX_SIGNUP | 250    | community | .ZZB | signup  |
  | %today-1d | %TX_SIGNUP | 250    | community | .ZZC | signup  |

Scenario: The caller confirms a trade of rCredits for cash
  When phone +20001 confirms "100 to .ZZC for cash"
  Then the community has r$-750
  And phone +20003 has r$350
  And phone +20001 has r$150
  And we say to phone +20001 "report exchange" with subs:
  | action | other_name | amount | balance | tid |
  | gave   | Corner Pub | $100   | $150    | 2   |
  # "You gave Corner Pub $100 cash/loan/etc. Your new balance is $150. Transaction #2"

Scenario: The caller confirms an implicit trade of rCredits for cash
  When phone +20001 confirms "100 to .ZZC"
  Then phone +20003 has r$350
  And phone +20001 has r$150

Scenario: The caller confirms a request to trade cash for rCredits
  When phone +20003 confirms "100 from .ZZA for cash"
  Then phone +20003 has r$250
  And phone +20001 has r$250
  And we say to phone +20003 "report exchange request" with subs:
  | action  | other_name | amount | tid |
  | charged | Abe One    | $100   | 2   |
  # "You requested $100 from Abe One for cash/loan/etc. Your balance is unchanged, pending approval. Invoice transaction #2"

Scenario: The caller confirms a unilateral trade of cash for rCredits
  Given phone +20003 can charge unilaterally
  When phone +20003 confirms "100 from .ZZA for cash"
  Then phone +20003 has r$350
  And phone +20001 has r$150
  And we say to phone +20003 "report exchange" with subs:
  | action  | other_name | amount | balance | tid |
  | charged | Abe One    | $100   | $350    | 2   |
  # "You charged Abe One $100 cash/loan/etc. Your new balance is $350. Transaction #2"

Scenario: The caller requests an implicit trade with insufficient balance
  When phone +20001 says "300 to .ZZC"
  Then we say to phone +20001 "short cash to" with subs:
  | short |
  | $50   |
  # "You are $50 short for that exchange."
  