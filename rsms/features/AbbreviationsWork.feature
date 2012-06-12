Feature: Abbreviations work
AS a caller
I WANT to type as little as possible
SO I can get stuff done fast

Scenario: A caller abbreviates command words
  Given phone %number1 is a player
  When phone %number1 says "h g"
  Then we say to phone %number1 "help for get"

Scenario: A caller abbreviates command words
  Given phone %number1 is a player
  When phone %number1 says "12 f abcdef f apples"
  # later "b me 12 f abcdef f apples"
  Then we say to phone %number1 "Don't abbreviate" with subs:
  | @word |
  | for   |
  # You must not abbreviate "for"
