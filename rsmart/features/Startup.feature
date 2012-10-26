Feature: Start up
As a member
I WANT to run the rCredits mobile app on my device
SO I can use it to buy and sell with rCredits.

# Terminology in these Features:
# "I": the device holder (not necessarily the owner)
# "the system": the rCredits system

Setup:
  Given members:
  | id      | full_name  | phone  | email         |
  | NEW.ZZA | Abe One    | +20001 | a@example.com |
  | NEW.ZZB | Bea Two    | +20002 | b@example.com |
  | NEW.ZZC | Corner Pub | +20003 | c@example.com |
  And devices:
  | id      | code  |
  | NEW.ZZA | codeA |
  And relations:
  | id      | main    | agent   | permissions  |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB |              |

Scenario: Member logs in successfully to initialize device
  Given member "NEW.ZZA" password is %whatever1
  When member initializes the device as member "NEW.ZZA" with password %whatever1
  Then we respond with:
  | success | owner_id | code       | message    |
  | 1       | NEW.ZZA  | (the code) | first time |
#op="first_time"
#update_link (URL of updated app or null if no update is available)
#allow_change_account=TRUE or FALSE
#allow_change_agent=TRUE or FALSE
#require_agent=TRUE or FALSE

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
  When the app starts up as member "NEW.ZZA" and code %whatever1
  Then we respond with:
  | success | message |
  | 1       |         |
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

Scenario: Device gives no member id
  When the app starts up as member "" and code "codeA"
  Then we respond with:
  | success | message       |
  | 0       | bad id format |
  
Scenario: Device gives bad member id
  When the app starts up as member %random and code "codeA"
  Then we respond with:
  | success | message       |
  | 0       | bad id format |
  
Scenario: Device gives no code
  When the app starts up as member "NEW.ZZA" and code ""
  Then we respond with:
  | success | message       |
  | 0       | no code given |
  
Scenario: Device gives a bad code
  When the app starts up as member "NEW.ZZA" and code %random
  Then we respond with:
  | success | message        |
  | 0       | unknown device |

Scenario: Agent does not have permission
  When the app starts up as member "NEW.ZZB" and code "codeA"
  Then we respond with:
  | success | message       |
  | 0       | no permission |
