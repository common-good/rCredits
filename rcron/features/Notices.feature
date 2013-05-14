Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | id   | fullName   | flags                   | email         |
  | .ZZA | Abe One    | dft,personal,company,ok | a@example.com |
  | .ZZB | Bea Two    | dft,personal,company,ok | b@example.com |
  | .ZZC | Corner Pub | dft,personal,company,ok | c@example.com |

Scenario: a member gets some notices
  Given notices:
  | id   | created | sent | message    |
  | .ZZA | %today  |    0 | You rock.  |
  | .ZZA | %today  |    0 | You stone. |
  When cron runs "notices"
  Then we email "notices" to member "a@example.com" with subs:
  | fullName | shortName | yesterday       | region | messages             |
  | Abe One  | abeone    | %fancyYesterday | new    | You rock.,You stone. |
  And notices:
  | id   | created | sent   | message    |
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |
  