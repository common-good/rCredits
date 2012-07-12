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
  And we did not just ask phone %number1 for a full name
  When phone %number1 says %random
  Then we say to phone %number1 "what's your name?" 

Scenario: The newbie gives us his or her name
  Given we just asked phone %number1 for "name"
  When phone %number1 says "Jo Smith"
  Then phone %number1 account name is "Jo Smith"
  And we say to phone %number1 "what's your email?" with subs:
  | @name    |
  | Jo Smith |
  # Welcome to rCredits, Jo Smith. Last question: What is your email address?

Scenario: The newbie gives us an unlikely name
  Given we just asked phone %number1 for "name"
  When phone %number1 says %random
  Then we say to phone %number1 "what's your name really?" 
  #To set up your rCredits account, we need your full name and email address. What's your name?

Scenario: The newbie gives us his or her email address
  Given we just asked phone %number1 for "email"
  And the community has r$-1000
  When phone %number1 says "zot@email.com"
  Then phone %number1 email is "zot@email.com"
  And phone %number1 has r$250
  And the community has r$-1250
  And we say to phone %number1 "your account is ready" with subs:
  | @balance |
  | $250      |
  # Thank you! Your new balance is $250.

Scenario: The newbie gives a bad email address
  Given we just asked phone %number1 for "email"
  When phone %number1 says %random
  Then we say to phone %number1 "what's your email really?" 
  # Please type carefully. What is your email address?
  