Feature: Tickle
AS a member
I WANT to be reminded of things to do, when it's time,
SO I don't get forgotten and miss out on stuff.

Setup:
  Given members:
  | id   | fullName | email | flags | access    |
  | .ZZA | Abe One  | a@    |       | %today-1d |
  | .ZZB | Bea Two  | b@    |       | %today-2d |
  | .ZZD | Dee Four | d@    |       | %today-3d |
Scenario: A newbie has not taken the first step
  When cron runs "tickle"
  Then we notice "do step one|sign in" to member ".ZZA"
  
Scenario: A newbie has taken some steps but not all
  Given member ".ZZA" has done step "sign contact dw donate proxies prefs photo"
  When cron runs "tickle"
  Then we notice "take another step|sign in" to member ".ZZA"
  
Scenario: A newbie is on the verify step
  Given member ".ZZA" has done step "sign contact dw donate proxies prefs photo connect"
  And member ".ZZB" has done step "sign contact dw donate proxies prefs photo connect"
  And member ".ZZD" has done step "sign contact dw donate proxies prefs photo connect"
  When cron runs "tickle"
  Then we notice "call bank|sign in" to member ".ZZB" with subs:
  | when                       |
  | tomorrow (how about 10am?) |
  And we notice "call bank|sign in" to member ".ZZD" with subs:
  | when                      |
  | between 9am and 4pm today |

#  And we notice "gift sent" to member ".ZZA" with subs:
#  | amount | rewardAmount |
#  |    $10 |        $0.50 |
