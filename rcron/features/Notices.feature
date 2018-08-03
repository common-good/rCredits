Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | id   | fullName   | flags     | email |*
  | .ZZA | Abe One    | ok        | a@    |
  | .ZZB | Bea Two    | member,ok,weekly | b@    |
  | .ZZC | Corner Pub | co,ok     | c@    |
  And community email for member ".ZZA" is "%whatever@rCredits.org"

Scenario: a member gets some notices
  Given notices:
  | id   | created | sent | message    |*
  | .ZZA | %today  |    0 | You rock.  |
  | .ZZA | %today  |    0 | You stone. |
  When cron runs "notices"
#  Given variable "balance" is "balance notice" with subs:
#  | balance | savings | rewards |*
#  | $0      | $0      | $0      |
  Then we email "notices" to member "a@" with subs:
  | fullName | shortName | unit | range   | yestertime | region | messages                  | balance  | savings | ourEmail      |*
  | Abe One  | abeone    | day  | %dmy-1d | %dmy-1d    | new    | *You rock.<br>*You stone. | $0       | $0      | %whatever@rCredits.org |
  And notices:
  | id   | created | sent   | message    |*
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |

Scenario: a member gets some weekly notices
  Given notices:
  | id   | created | sent | message    |*
  | .ZZB | %today  |    0 | You rock.  |
  | .ZZB | %today  |    0 | You stone. |
  And it's time for "weekly"
  When cron runs "notices"
#  Given variable "balance" is "balance notice" with subs:
#  | balance | savings | rewards |*
#  | $0      | $0      | $0      |
  Then we email "notices" to member "b@" with subs:
  | fullName | shortName | unit | range               | yestertime | region | messages                            | balance  | savings | ourEmail      |*
  | Bea Two  | beatwo    | week | the week of %dmy-1w | %dmy-1w    | new    | %md<x>You rock.<br>%md<x>You stone. | $0       | $0      | %whatever@rCredits.org |
#  | Bea Two  | beatwo    | week | the week of %dmy-1w | %dmy-1w    | new    | %md<x>You rock.<br>%md<x>You stone. | @balance | 5.0    | %whatever@rCredits.org |
  And notices:
  | id   | created | sent   | message    |*
  | .ZZB | %today  | %today | You rock.  |
  | .ZZB | %today  | %today | You stone. |