Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | id   | fullName   | flags                 | email         |
  | .ZZA | Abe One    | dft,person,ok         | a@ |
  | .ZZB | Bea Two    | dft,person,company,ok | b@ |
  | .ZZC | Corner Pub | dft,company,ok        | c@ |

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
Skip
Scenario: a member gets some weekly notices
  Given notices:
  | id   | created | sent | message    |
  | .ZZA | %today  |    0 | You rock.  |
  | .ZZA | %today  |    0 | You stone. |
  When cron runs "notices"
  Then we email "notices" to member "a@" with subs:
  | fullName | shortName | unit | range           | yestertime      | region | messages             | balance | rewards | return | ourEmail |
  | Abe One  | abeone    | day  | %fancyYesterday | %fancyYesterday | new    | %md: You rock.,%md: You stone. | $0 | $0 | 5.0 | %R_REGION_EMAIL |
  And notices:
  | id   | created | sent   | message    |
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |