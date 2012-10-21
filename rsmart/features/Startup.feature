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

Scenario: Member logs in successfully to initialize device
  Given member NEW.ZZA password is %whatever1
  When member initializes the device as member "new.zza" with password %whatever1
  Then we respond with:
  | success | owner_id | message    | code     |
  | 1       | NEW.ZZA  | first time | whatever |
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
  When member initializes the device as member "new.zzz" with password %whatever1
  Then we respond with:
  | success | message      |
  | 0       | unknown user |
  
Scenario: Member types the wrong password
  When member initializes the device as member "new.zza" with password %whatever
  Then we respond with:
  | success | message   |
  | 0       | bad login |

Scenario: Member reruns the app
  Given member "NEW.ZZA" has initialized a device whose code is %whatever1
  When the app starts up as member "NEW.ZZA" and code %whatever1
  Then we respond with:
  | success | message |
  | TRUE    |         |
#op=”startup”
#update_link (URL of updated app or null if no update is available)
#allow_change_account=TRUE or FALSE
#allow_change_agent=TRUE or FALSE
#require_agent=TRUE or FALSE
