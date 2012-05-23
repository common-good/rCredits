Feature: Get general help
AS a caller
I WANT helpful information
SO I can use the rCredits SMS interface effectively

Scenario: A caller wants help with a specific command
  Given phone %number1 is a player
  When phone %number1 says "help get"
  Then we say to phone %number1 "help for get"  

Scenario: A caller wants to know what's available
  Given phone %number1 is a player
  When phone %number1 says "help" with no argument
  Then we say to phone %number1 "help for help"

Scenario: A caller types an unrecognized command
  Given phone %number1 is a player
  When phone %number1 says "something %random"
  # or caller says "53 something %random"
  Then we say to phone %number1 "Syntax error...|help for help"

Scenario: A caller types the wrong number of arguments
  Given phone %number1 is a player
  When phone %number1 says "get r 100 with too many arguments"
  Then we say to %number1 "Syntax error...|help for get"
  
Scenario: A caller types a wrong argument
  Given phone %number1 is a player
  When phone %number1 says "get zot"
  Then we say to %number1 "Syntax error...|help for get"

Scenario: Amount is too big
# 6 digits or more?
  Given phone %number1 is a player
  When phone %number1 says "get r 100000"
  Then we say to phone %number1 "amount too big"
  # Transactions larger than $99,999 are not permitted at this time.

Scenario: Account does not exist
  Given phone %number1 is a player
  And "neabcdef" is not an account id
  When phone %number1 says "100 to neabcdef"
  Then we say to phone %number1 "not an account id|help for to"
  | @id |
  | neabcdef |
