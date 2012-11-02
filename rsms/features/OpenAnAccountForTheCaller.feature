Feature: Open an account for the caller
  AS a newbie
  I WANT to try rCredits
  SO I can decide whether I want to participate.
  
Scenario: A total newbie calls
  Given phone %number1 has no account
  When phone %number1 says %whatever
  Then phone %number1 has an account
  And we say to phone %number1 "what's your name?" 
  # To set up your rCredits account, we need your full name and email address. What's your name?
  
Scenario: A nameless newbie calls
  Given phone %number1 has an account
  And phone %number1 has no name
  And phone %number1 is not waiting to "setup name"
  When phone %number1 says %random
  Then we say to phone %number1 "what's your name?" 
  And phone %number1 is waiting to "setup name"

Scenario: The newbie gives us his or her name
  Given phone %number1 is waiting to "setup name"
  When phone %number1 says "Jo Smith"
  Then phone %number1 account name is "Jo Smith"
  And we say to phone %number1 "what's your email?" with subs:
  | full_name | quid       |
  | Jo Smith  | %last_quid |
  And phone %number1 is waiting to "setup email"
  # Welcome to rCredits, Jo Smith. Last question: What is your email address?

Scenario: The newbie gives us an unlikely name
  Given phone %number1 is waiting to "setup name"
  When phone %number1 says %random
  Then we say to phone %number1 "what's your name really?" 
  And phone %number1 is waiting to "setup name"
  #To set up your rCredits account, we need your full name and email address. What's your name?

Scenario: The newbie gives us his or her email address
  Given phone %number1 is waiting to "setup email"
  When phone %number1 says " zot@example.com "
  Then phone %number1 email is "zot@example.com"
  And phone %number1 has r$250
  And we say to phone %number1 "your account is ready" with subs:
  | balance |
  | $250    |
  And phone %number1 is waiting to ""
  # Thank you! Your new balance is $250.

Scenario: The newbie gives a bad email address
  Given phone %number1 is waiting to "setup email"
  When phone %number1 says %random
  Then we say to phone %number1 "what's your email really?" 
  And phone %number1 is waiting to "setup email"
  # Please type carefully. What is your email address?
  