Feature: Contact Information
AS a member
I WANT to update my name, phone, postal address, or physical address
SO I can complete my registration and/or make sure I can be contacted by system administrators as needed.
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | id   | fullName | address | city  | state | zip   | country | postalAddr       | email | phone       | tenure | flags  |*
  | .ZZA | Abe One  | 1 A St. | Atown | AL    | 99100 | US      | 1 A, T, AK 23456 | a@    | 14132001000 |     19 | member |

Scenario: A member visits the contact info page
  When member ".ZZA" visits page "settings/contact"
  Then we show "Contact Information" with:
  | Your Name | Abe One   |
  | Email | a@ |
  | Phone | +1 413 200 1000 |
// fails because it's JS  | Country | United States |
  | Mailing Address | |
  | Street Address | 1 A St. |
  | City | Atown |
// fails because it's JS  | State | Alaska |
  | Postal Code | 99100 |
  | Mailing Address | |
  | Street Address | 1 A |
  | City | T |
// fails because it's JS  | State | Arkansas |
  | Postal Code | 23456 |

Scenario: A member updates contact info
  When member ".ZZA" confirms form "settings/contact" with values:
  | fullName | phone        | country | zip | state | city    | address   | postalAddr | email | tenure |*
  | Abe One  | 413-253-0001 | US      | 01002      | MA    | Amherst | 2 Elm St. | PO Box 1   | a@    |     18 |
  Then members:
  | id   | fullName   | address   | city    | state | zip | country | postalAddr | phone       | email |*
  | .ZZA | Abe One    | 2 Elm St. | Amherst | MA    | 01002      | US      | PO Box 1   | 14132530001 | a@    |
  And we say "status": "info saved"
  
Scenario: A member gives a bad phone
  When member ".ZZA" confirms form "settings/contact" with values:
  | fullName | phone   | country | zip | state | city    | address   | postalAddr | email | tenure |*
  | Abe One  | %random | US      | 01002      | MA    | Amherst | 2 Elm St. | PO Box 1   | a@    |     18 |
  Then we say "error": "bad phone"

Scenario: A member gives a bad email
  When member ".ZZA" confirms form "settings/contact" with values:
  | fullName | phone        | country | zip | state | city    | address   | postalAddr | email   |*
  | Abe One  | 413-253-0002 | US      | 01002      | MA    | Amherst | 2 Elm St. | PO Box 1   | %random |
  Then we say "error": "bad email"
  
Scenario: A member updates to a different state
  When member ".ZZA" confirms form "settings/contact" with values:
  | fullName | phone        | country | zip | state | city    | address   | postalAddr | email | tenure |*
  | Abe One  | 413-253-0001 | US      | 01002      | MI    | Amherst | 2 Elm St. | PO Box 1   | a@    |     18 |
  Then members:
  | id   | fullName   | address   | city    | state | zip | country | postalAddr | phone       | email |*
  | .ZZA | Abe One    | 2 Elm St. | Amherst | MI    | 01002      | US      | PO Box 1   | 14132530001 | a@    |
  And we say "status": "info saved"
  
Scenario: A member updates to a different name
  When member ".ZZA" confirms form "settings/contact" with values:
  | fullName  | phone        | country | zip | state | city    | address   | postalAddr | email | tenure |*
  | Abe Other | 413-253-0001 | US      | 01002      | MA    | Amherst | 2 Elm St. | PO Box 1   | a@    |     18 |
  Then members:
  | id   | fullName  | legalName | address   | city    | state | zip | country | postalAddr |*
  | .ZZA | Abe Other | Abe One   | 2 Elm St. | Amherst | MA    | 01002      | US      | PO Box 1   |
  And we say "status": "info saved"