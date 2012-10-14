Feature: Get help
AS a caller
I WANT helpful information
SO I can use the rCredits SMS interface effectively

Scenario: A caller wants help with a specific command
  Given phone %number1 is a member
  When phone %number1 says "help get"
  Then we say to phone %number1 "help get"  

Scenario: A caller wants to know what's available
  Given phone %number1 is a member
  When phone %number1 says "help"
  Then we say to phone %number1 "help helpme"

Scenario: A caller types an unrecognized command
  Given phone %number1 is a member
  And the expected nonce for phone %number1 is ""
  When phone %number1 says %whatever
  Then we say to phone %number1 "syntax|help helpme"

Scenario: Caller gives the wrong nonce
  Given the expected nonce for phone %number1 is "mango"
  When phone %number1 says "anything but a legal command"
  Then we say to phone %number1 "wrong nonce"
  # That is not the expected confirmation response. Start over and try again.
  
Scenario: A caller types the wrong number of arguments
  Given phone %number1 is a member
  When phone %number1 says "get r 100 with too many arguments"
  Then we say to phone %number1 "syntax|help get"
  
Scenario: Caller types a wrong argument
  Given phone %number1 is a member
  When phone %number1 says "get zot"
  Then we say to phone %number1 "syntax|help get"

Scenario: Amount is too big
# 6 digits or more?
  Given phone %number1 is a member
  When phone %number1 says "get r 100000"
  Then we say to phone %number1 "amount too big"
  # Transactions larger than $99,999 are not permitted at this time.

Scenario: Account does not exist
  Given phone %number1 is a member
  And ".ZZZ" is not an account id
  When phone %number1 says "100 to .ZZZ"
  Then we say to phone %number1 "not an account id|help to" with subs:
  | id   |
  | .ZZZ |
