Feature: Boxes (Devices / Cell Phones)
AS a member
I WANT to manage the list of devices used with my account
SO I can request transactions via SMS and/or track which devices were used for which transactions.

Setup:
  Given members:
  | id   | fullName   | flags  |
  | .ZZA | Abe One    | dft,ok |

Scenario: A member visits the devices page with no devices yet
  When member ".ZZA" visits page "account/boxes"
  Then we show "Devices" with:
  |_# | Type | Device name | Last used |
  | 1 | Web  | Browser #1  | %dmy      |

Scenario: A member has all kinds of devices
  Given devices:
  | id   | channel | code         | boxnum | boxName   | access    |
  | .ZZA | app     | A3           |      3 |           | %today-4w |
  | .ZZA | sms     | +12002002000 |      2 |           | %today-2d |
  | .ZZA | sms     | +14004004000 |      4 | whatEver  | %today-3w |
  When member ".ZZA" visits page "account/boxes"
  Then we show "Devices" with:
  |_# | Type | Device name     | Last used |
  | 2 | SMS  | +1 200.200.2000 | %dmy-2d   |
  | 3 | App  | POS Device #3   | %dmy-4w   |
  | 4 | SMS  | whatEver        | %dmy-3w   |
  | 5 | Web  | Browser #5      | %dmy      |
Skip
Scenario: A member adds a cell phone
  When member ".ZZA" confirms form "account/boxes" with values:
  | new            |
  | (413) 772-1000 |
  Then we show "Verify" with:
  | We sent a verification code | +1 413.772.1000 |
  | Code: | Verify |

Scenario: A member verifies the cell phone
  Given the expected nonce is %whatever
  When member ".ZZA" confirms form "account/boxes" with values:
  Then we show "Devices" with:
  |_# | Type | Device name     | Last used |
  | 2 | SMS  | +1 413.772.1000 | %dmy      |
  | 5 | Web  | Browser #5      | %dmy      |
  And we say "status": "info saved"
