Feature: Contact Information
AS a member
I WANT to update my name, phone, postal address, or physical address
SO I can complete my registration and/or make sure I can be contacted by system administrators as needed.
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | physical | email         | phone |
  | .ZZA | Abe One    |         | Atown | Alaska | 99100      | US      |          | a@ |       |

Scenario: A member visits the contact info page
  When member ".ZZA" visits page "account/contact"
  Then we show "Contact Information" with:
  | Your Name |
  | Abe One   |

Scenario: A member updates contact info
  When member ".ZZA" confirms form "account/contact" with values:
  | fullName | phone        | country | postalCode | state | city    | address | physical  |
  | Abe One  | 413-253-0001 | US      | 01002      | MA    | Amherst | POB 1   | 2 Elm St. |
  Then members:
  | id   | fullName   | address | city    | state | postalCode | country | physical  | phone       |
  | .ZZA | Abe One    | POB 1   | Amherst | MA    | 01002      | US      | 2 Elm St. | 14132530001 |
  And we say "status": "info saved"
  
Scenario: A member gives a bad phone
  When member ".ZZA" confirms form "account/contact" with values:
  | fullName | phone   | country | postalCode | state | city    | address | physical  |
  | Abe One  | %random | US      | 01002      | MA    | Amherst | POB 1   | 2 Elm St. |
  Then we say "error": "bad phone"
