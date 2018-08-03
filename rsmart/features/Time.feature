Feature: Set Device Time
AS an rPOS device
I WANT to synchronize my time with the server
SO I don't inadvertently record bogus data, potentially causing all manner of havoc.

Summary:
  The device ask for the time and we give it.
  
Setup:
  Given members:
  | id   | fullName   | email | city  | state | cc  | cc2  | rebate | flags      |*
  | .ZZA | Abe One    | a@    | Atown | AK    | ccA | ccA2 |      5 | ok         |
  | .ZZB | Bea Two    | b@    | Btown | UT    | ccB | ccB2 |      5 | ok         |
  | .ZZC | Corner Pub | c@    | Ctown | CA    | ccC |      |      5 | ok,co      |
  And devices:
  | id   | code |*
  | .ZZC | devC |
  And selling:
  | id   | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | id   | flags        |*
  | .ZZC | refund,r4usd |
  And relations:
  | main | agent | num | permission | rCard |*
  | .ZZC | .ZZA  |   1 | scan       |       |
  | .ZZC | .ZZB  |   2 | scan       | yes   |

Scenario: The device asks for the time
  When agent ".ZZC" on device "devC" asks for the time
  Then we respond with:
  | ok | time |*
  | 1  | %now |

Scenario: a cashier signs in
  When agent "" asks device "devC" to identify "C:B,ccB2"
  Then we respond with:
  | ok | name    | logon | descriptions    | can          | default | company    | time |*
  | 1  | Bea Two | 1     | this,that,other | refund,r4usd | NEWZZC  | Corner Pub | %now |
