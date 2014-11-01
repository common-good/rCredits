Feature: Get help
AS a caller
I WANT helpful information
SO I can use the rCredits SMS interface effectively

Scenario: A caller wants help with a specific command
  Given phone %number1 is a member
  When phone %number1 says "help payment"
  Then we say to phone %number1 "help payment"  

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
  Given phone %number1 is a member
  And the expected nonce for phone %number1 is "mango"
  When phone %number1 says "anySingleWordOtherThanALegalCommand"
  Then we say to phone %number1 "wrong nonce"
  # That is not the expected confirmation response. Start over and try again.
  
Scenario: A caller types the wrong number of arguments
  Given phone %number1 is a member
  When phone %number1 says "info r 100 with too many arguments"
  Then we say to phone %number1 "syntax|help information"
  
Scenario: Caller types a wrong argument
  Given phone %number1 is a member
  When phone %number1 says "info zot"
  Then we say to phone %number1 "syntax|help information"

Scenario: Amount is too big
# 6 digits or more?
  Given phone %number1 is a member
  And phone %number2 is a member
  When phone %number1 says "pay 1%R_MAX_AMOUNT to %number2"
  Then we say to phone %number1 "amount too big"
  # Transactions larger than $99,999 are not permitted at this time.

Scenario: Account does not exist
  Given phone %number1 is a member
  And ".ZZZ" is not an account id
  When phone %number1 says "100 to .ZZZ"
  Then we say to phone %number1 "unknown member" with subs:
  | who  |*
  | .ZZZ |
