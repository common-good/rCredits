Feature: Transfer funds to or from a customer.
AS a company or individual
I WANT to scan a customer card and transfer funds from their account to my account or vice versa
SO we can account fairly for our business dealings.

#NOTES - I modified the gherkin code in two minor ways: 1) I got rid of the "< " before the back button calls because they are represented as CSS and not actual text; 2) I put "" around any numbers so that they represent the way the text is displayed and not just the numerical equivalent (0.00 instead of 0)

Setup:
  #Given members:
  #| id | fullName      | city     | state | balance | flags |
  #| C  | Corner Store  | Ashfield | MA    | 0       | ok,co |
  #| S  | Susan Shopper | Montague | MA    | 100     | ok    |
  #And company is "C"
  When show page "Home"
  Then show button "Scan Customer rCard"
  
Scenario: We identify and charge a customer
  When button "Scan Customer rCard" pressed
  Then show scanner
  
  When scanner sees QR "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw"
  Then show photo of member "NEWABB"
  And show text "Susan Shopper"
  And show text "Montague, MA"
  And show button "Charge"
  And show button "Refund"
  And show button "Trade USD"
  And show back button "Back"
  
  When button "Charge" pressed
  Then show number keypad
  And show amount "0.00"
  And show dropdown with "groceries" selected
  And show button "Charge"
  And show back button "Back"
  
  When button "3" pressed
  Then show amount "0.03"
  When button "00" pressed
  Then show amount "3.00"
  When button "Charge" pressed
  Then show "ok" message "Susan Shopper paid you $3.00" titled "Success!"
  When message button "ok" pressed
  Then show page "Home"
