Feature: A user signs up for rCredits
AS a newbie
I WANT to get access to the rCredits Participants section
SO I can start pretending
# Note that "member" in the scenarios below means new member (newbie).

Scenario: A newbie registers
  When a member completes form "register" with values:
  | full_name | email         | country       | postal_code | state         | city    | account_type |
  | Abe One   | a@example.com | United States | 01001       | Massachusetts | Amherst | personal     |
  Then members:
  | id      | full_name | email         | country       | postal_code | state         | city    | account_type |
  | NEW.ZZA | Abe One   | a@example.com | United States | 01001       | Massachusetts | Amherst | personal     |
  And we say "status": "your account is ready" with subs:
  | quid    | balance |
  | NEW.ZZA | $250    |
  And we show "dashboard" with subs:
  | quid    |
  | NEW.ZZA |
  And we email "welcome" to member "a@example.com" with subs:
  | full_name | name   | quid    |
  | Abe One   | abeone | NEW.ZZA |
#Formatting and links are ignored

Scenario: A member registers again
  Given members:
  | id      | full_name  | phone  | email         | city  | state  | country       | 
  | NEW.ZZA | Abe One    | +20001 | a@example.com | Atown | Alaska | United States |
  When a member completes form "register" with values:
  | full_name | email         | country       | postal_code | state         | city    | account_type |
  | Abe One   | a@example.com | United States | 01001       | Massachusetts | Amherst | personal     |
  Then we say "error": "already a member" with subs:
  | quid    |
  | NEW.ZZA |
  And we show "login"
