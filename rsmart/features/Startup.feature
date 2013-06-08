Feature: Start up
As a member
I WANT to run the rCredits mobile app on my device
SO I can use it to buy and sell with rCredits.

Setup:
  Given members:
  | id      | fullName  | phone  | email         |
  | NEW.ZZA | Abe One    | +20001 | a@example.com |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  And relations:
  | id      | main    | agent   | permissions  |
  | NEW.ZZA | NEW.ZZA | NEW.ZZB |              |

Scenario: Member logs in successfully to initialize device
  Given member "NEW.ZZA" password is %whatever1
  When member initializes the device as member "NEW.ZZA" with password %whatever1
  Then we respond with:
  | success | message    | code       | my_id   | account_name | allow_change_account | allow_change_agent | require_agent | show_buttons |
  | 1       | first time | (varies) | NEW.ZZA | Abe One      | 0                    | 1                  | 0             | 3             |
#op="first_time"
#update_link (URL of updated app or null if no update is available)
#allow_change_account=TRUE or FALSE
#allow_change_agent=TRUE or FALSE
#require_agent=TRUE or FALSE
#show_buttons=0, 1, 2, or 3

Scenario: Member initializes with an ill-formed id
  When member initializes the device as member %random with password %whatever
  Then we respond with:
  | success | message |
  | 0       | bad id  |

Scenario: Device owner is not a member
  When member initializes the device as member "NEW.ZZZ" with password %whatever1
  Then we respond with:
  | success | message        |
  | 0       | unknown member |
  
Scenario: Member types the wrong password
  When member initializes the device as member "NEW.ZZA" with password %random
  Then we respond with:
  | success | message   |
  | 0       | bad login |

Scenario: Member reruns the app
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When the app starts up as code %whatever1
  Then we respond with:
  | success | message    | my_id   | account_name | allow_change_account | allow_change_agent | require_agent | show_buttons |
  | 1       |            | NEW.ZZA | Abe One      | 0                    | 1                  | 0             | 3             |
#op=”startup”
#update_link (URL of updated app or null if no update is available)
#allow_change_account=TRUE or FALSE
#allow_change_agent=TRUE or FALSE
#require_agent=TRUE or FALSE

Scenario: Device requests a bad op
  When the app requests op %random as member "NEW.ZZA" and code "codeA"
  Then we respond with:
  | success | message    |
  | 0       | unknown op |

Scenario: Device gives no code
  When the app starts up as code ""
  Then we respond with:
  | success | message       |
  | 0       | no code given |
  
Scenario: Device gives a bad code
  When the app starts up as code %random
  Then we respond with:
  | success | message        |
  | 0       | unknown device |
