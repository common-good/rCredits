Feature: Track commands
AS a player
I WANT the system to count how many times I use each SMS command
SO I can fulfill my membership requirements

AS a system administrator
I WANT the same thing
SO I can tell how much the various commands are used And can detect anomalies
NOTE: the various forms of each command should be tracked separately, especially "to" or "from" (1) without "for", (2) with "for cash" and (3) with "for" something else.

Scenario: A caller uses a command that does not require confirmation
  Given phone %number1 is a player
  And phone %number1 "info" command use count is 5
  And overall "info" command use count is 100
  When phone %number1 says "info"
  Then phone %number1 "info" command use count is 6
  And overall "info" command use count is 101

Scenario: A caller uses a command that DOES require confirmation
  Given phone %number1 is a player
  And phone %number1 "to" command use count is 5
  And phone %number1 "cash" command use count is 2
  And overall "to" command use count is 100
  And overall "cash" command use count is 30
  When phone %number1 confirms "100 to abcdef for ca"
  Then phone %number1 "to" command use count is 6
  And phone %number1 "cash" command use count is 3
  And overall "to" command use count is 101
  And overall "cash" command use count is 31
