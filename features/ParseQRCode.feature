Feature: Parse QR Code
AS a cashier
I WANT the customer's QR code to be interpreted correctly
SO we know who we're dealing with.

Setup:
  
Scenario: We scan a valid old personal card.
  When we scan QR "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw"
  Then account is personal
  And account ID is "NEWABB"
  And security code is "ZzhWMCq0zcBowqw"
  
Scenario: We scan a valid old company card.
  When we scan QR "HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G"
  Then account is company
  And account ID is "NEWAAB-A"
  And security code is "WeHlioM5JZv1O9G"
  
Scenario: We scan a valid personal card.
  When we scan QR "HTTP://6VM.RC4.ME/GORZzhWMCq0zcBowqw"
  Then account is personal
  And account ID is "NEWABB"
  And security code is "ZzhWMCq0zcBowqw"
  
Scenario: We scan a valid company card.
  When we scan QR "HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G"
  Then account is company
  And account ID is "NEWAAB-A"
  And security code is "WeHlioM5JZv1O9G"
