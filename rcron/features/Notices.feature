Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | id   | fullName   | flags     | email |
  | .ZZA | Abe One    | ok        | a@    |
  | .ZZB | Bea Two    | member,ok,notice_weekly | b@    |
  | .ZZC | Corner Pub | co,ok     | c@    |

Scenario: a member gets some notices
  Given notices:
  | id   | created | sent | message    |
  | .ZZA | %today  |    0 | You rock.  |
  | .ZZA | %today  |    0 | You stone. |
  When cron runs "notices"
  Then we email "notices" to member "a@" with subs:
  | fullName | shortName | unit | range           | yestertime      | region | messages             | balance | rewards | return | ourEmail |
  | Abe One  | abeone    | day  | %fancyYesterday | %fancyYesterday | new    | You rock.,You stone. | $0 | $0 | 5.0 | %R_REGION_EMAIL |
  And notices:
  | id   | created | sent   | message    |
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |

Scenario: a member gets some weekly notices
  Given notices:
  | id   | created | sent | message    |
  | .ZZB | %today  |    0 | You rock.  |
  | .ZZB | %today  |    0 | You stone. |
  And it's time for "weekly"
  When cron runs "notices"
  Then we email "notices" to member "b@" with subs:
  | fullName | shortName | unit | range                       | yestertime        | region | messages                      | balance | rewards | return | ourEmail        |
  | Bea Two  | beatwo    | week | the week of %fancyYesterweek | %fancyYesterweek | new    | %md: You rock.,%md: You stone. | $0      | $0      | 5.0    | %R_REGION_EMAIL |
  And notices:
  | id   | created | sent   | message    |
  | .ZZB | %today  | %today | You rock.  |
  | .ZZB | %today  | %today | You stone. |