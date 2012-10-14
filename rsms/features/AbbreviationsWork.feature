Feature: Abbreviations work
AS a caller
I WANT to type as little as possible
SO I can get stuff done fast

Scenario: A caller abbreviates command words
  Given phone %number1 is a member
  When phone %number1 says "h g"
  Then we say to phone %number1 "help get"

Scenario: A caller abbreviates with wrong syntax
  Given phone %number1 is a member
  When phone %number1 says "12 f"
  # later maybe "b me 12 f abcdef f apples"
  Then we say to phone %number1 "syntax|help charge"
