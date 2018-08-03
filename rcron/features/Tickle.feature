Feature: Tickle
AS a member
I WANT to be reminded of things to do, when it's time,
SO I don't get forgotten and miss out on stuff.

Setup:
  Given members:
  | id   | fullName | email | flags   | access    | floor |*
  | .ZZA | Abe One  | a@    |         | %today-1d |     0 |
  | .ZZB | Bea Two  | b@    |         | %today-2d |     0 |
  | .ZZD | Dee Four | d@    |         | %today-9d |     0 |
  | .ZZE | Eve Five | e@    | ok      | %today-3m |     0 |
  | .ZZF | Flo Six  | f@    | ok      | %today-3m |     0 |

Scenario: A newbie has not taken the first step
  Given invites:
  | email | inviter | code   | invited    | invitee |*
  | d@    | .ZZE    | codeD1 | %today-11d | .ZZD    |
  And member ".ZZD" has done step "signup"
  When cron runs "tickle"
  Then we notice "do step one|sign in|daily messages" to member ".ZZD"
  And we notice "invitee slow" to member ".ZZE" with subs:
  | fullName | elapsed | step   |*
  | Dee Four |       9 | verify |
  
Scenario: A newbie has taken some steps but not all
  Given member ".ZZA" has done step "signup verify sign donate proxies prefs photo"
  When cron runs "tickle"
  Then we notice "take another step|sign in|daily messages" to member ".ZZA"

#Scenario: A newbie is on the verify step
#  Given member ".ZZA" has done step "sign contact donate proxies prefs photo connect"
#  And member ".ZZB" has done step "sign contact donate proxies prefs photo connect"
#  And member ".ZZD" has done step "sign contact donate proxies prefs photo connect"
#  When cron runs "tickle"
#  Then we notice "call bank|sign in" to member ".ZZB" with subs:
#  | when                       |*
#  | tomorrow (how about 10am?) |
#  And we notice "call bank|sign in" to member ".ZZD" with subs:
#  | when                      |*
#  | today between 9am and 4pm |
#  And we notice "gift sent" to member ".ZZA" with subs:
#  | amount | rewardAmount |*
#  |    $10 |        $0.50 |

Scenario: A nonmember has not accepted the invitation
  Given invites:
  | email           | inviter | code   | invited   |*
  | zot@example.com | .ZZF    | codeF1 | %today-8d |
  When cron runs "tickle"
  Then we email "nonmember" to member "zot@example.com" with subs:
  | inviterName | code   | site                      | nudge        | noFrame |*
  | Flo Six     | codeF1 | http://localhost/rMembers | reminder one | 1       |
  # site should be %BASE_URL, but compiler doesn't know stuff defined in boot.inc (just defs.inc)
  And we notice "invite languishing" to member ".ZZF" with subs:
  | email           | elapsed |*
  | zot@example.com |       8 |

Scenario: A nonmember has not accepted the invitation from a not-yet-active member
  Given invites:
  | email           | inviter | code   | invited   |*
  | zot@example.com | .ZZA    | codeA1 | %today-8d |
  When cron runs "tickle"
  Then we do not email "zot@example.com"
#  Then we email "nonmember" to member "zot@example.com" with subs:
#  | inviterName | code   | site                      | nudge        | noFrame |*
#  | Abe One     | codeA1 | http://localhost/rMembers | reminder one | 1       |
  And we do not notice to member ".ZZA"

Scenario: A nonmember has accepted the invitation
  Given invites:
  | email           | inviter | code   | invited   | invitee |*
  | zot@example.com | .ZZA    | codeA1 | %today-8d | .ZZB    |
  When cron runs "tickle"
  Then we do not email "nonmember" to member "b@example.com"
  
Scenario: A nonmember has accepted an invitation from someone else instead
  Given invites:
  | email         | inviter | code   | invited   | invitee |*
  | b@example.com | .ZZA    | codeA1 | %today-8d | 0       |
  | b@example.com | .ZZD    | codeA1 | %today-5d | .ZZB    |
  When cron runs "tickle"
  Then we do not email "nonmember" to member "b@example.com"
Skip
Scenario: A member gets a credit line
# This fails if run on a day of the month that the previous month doesn't have (for example on 10/31)
  Given transactions:
  | created   | type     | amount | from | to   | rebate | bonus | purpose |*
  | %today-1m | transfer |    300 | .ZZE | .ZZF |    500 |     0 | gift    |
  Then balances:
  | id   | rewards |*
  | .ZZE |     500 |
  When cron runs "tickle"
  Then members have:
  | id   | floor |*
  | .ZZE |   -50 |
Resume

#  And we notice "new floor|no floor effect" to member ".ZZE" with subs:
#  | limit |*
#  |  $50 |
# (feature temporarily disabled)

# We use rewards rather than floor, to measure credit-worthiness
#Scenario: A member gets a bigger credit line after several months
# This fails if run on a day of the month that the previous month doesn't have (for example on 10/31)
#  Given transactions:
#  | created   | type     | amount | from | to   | rebate | bonus | purpose |*
#  | %today-6m | transfer |    300 | .ZZE | .ZZF |   5000 |     0 | gift    |
#  | %today-5m | transfer |   1500 | .ZZE | .ZZF |      0 |     0 | gift    |
#  Then balances:
#  | id   | rewards |*
#  | .ZZE |    5000 |
#  When cron runs "tickle"
#  Then members have:
#  | id   | floor |*
#  | .ZZE |  -300 |
#  And we notice "new floor|no floor effect" to member ".ZZE" with subs:
#  | limit |*
#  | $300 |
# (feature temporarily disabled)

# only works if not the first of the month
#Scenario: A member gets no new credit line because it's the wrong day
#  Given usd transfers:
#  | payee | amount | created   |*
#  | .ZZE  | 500   | %today-6w |
#  And transactions:
#  | created   | type     | amount | from | to   | purpose |*
#  | %today-5w | transfer |    300 | .ZZE | .ZZF | gift    |
#  When cron runs "tickle"
#  Then members have:
#  | id   | floor |*
#  | .ZZE |     0 |

Scenario: A member gets no new credit line because the change would be minimal
  Given balances:
  | id   | rewards |*
  | .ZZE | 500 |
  And members have:
  | id   | floor |*
  | .ZZE |    49 |  
  And transactions:
  | created   | type     | amount | from | to   | purpose |*
  | %today-5w | transfer |    300 | .ZZE | .ZZF | gift    |
  When cron runs "tickle"
  Then members have:
  | id   | floor |*
  | .ZZE |    49 |
  