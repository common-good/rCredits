Feature: A user signs up for rCredits
AS a newbie
I WANT to open an rCredits account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
Setup:
  Given members:
  | id   | fullName | acctType    | flags      | created  |*
  | .ZZZ | Zeta Zot | personal    | ok,bona    | 99654321 |
  And member is logged out

Scenario: A newbie visits the registration page with no invite
  Given invitation to email "a@" from member ".ZZZ" is ""
  When member "?" visits page "signup"
  Then we show "Open a Personal rCredits Account" with:
  |_errorPhrase         |
  | you must be invited |

Scenario: A newbie visits the registration page with bad invite
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" visits page "signup/code=WhAtEvEr"
  Then we show "Open a Personal rCredits Account" with:
  |_errorPhrase         |
  | you must be invited |

Scenario: A newbie visits the registration page with expired invite
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And invitation "c0D3" was sent on "%today-5w"
  When member "?" visits page "signup/code=c0D3"
  Then we show "Open a Personal rCredits Account"
# lateness no longer makes a big difference
#  And we say "error": "expired invite" with subs:
#  | a | inviterName |*
#  | ? | Zeta Zot    |

Scenario: A newbie visits the registration page with a used invite
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 | .ZZZ   |
  When member "?" visits page "signup/code=c0D3"
  Then we show "Open a Personal rCredits Account"
  And we say "error": "used invite"

Scenario: A newbie registers in Western Massachusetts
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags     | floor | address | postalAddr                | tenure | owns | helper |*
  | .AAC | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam | confirmed | 0     |    1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ  |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAC | %R_SITE_URL | %name |
  And member ".AAC" one-time password is set
  #And we show "Empty"

Scenario: A newbie registers with an unconfirmed icard invitation
  Given member ".ZZZ" email invitation code is "BRFWWVZCH3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=BRFWWVZCH3E" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags | floor | address | postalAddr              | tenure | owns | iCode | helper |*
  | .AAC | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | 0     |   1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 |     5 | .ZZZ   |
  And we say "status": "your account is ready"
  And we say "status": "must be confirmed" with subs:
  | inviterName |*
  | Zeta Zot    |
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAC | %R_SITE_URL | %name |
  And member ".AAC" one-time password is set
  And we message "confirm invite" to member ".ZZZ" with subs:
  | fullName | a1 |*
  | Abe One  | ?  |
  #And we show "Empty"
  
Scenario: A newbie registers with an unconfirmed self-invitation
# unconfirmed by email is very similar except "B" gets added to the code and the iCode is %IBY_EMAIL (1)
  Given member ".ZZZ" email invitation code is "BRFWWVZCH3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=BRFWWVZCH3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags | floor | address | postalAddr              | tenure | owns | iCode      | helper |*
  | .AAC | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | 0     |   1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 | %IBY_SELF | .ZZZ   |
  And we say "status": "your account is ready"
  And we say "status": "must be confirmed" with subs:
  | inviterName |*
  | Zeta Zot    |
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAC | %R_SITE_URL | %name |
  And member ".AAC" one-time password is set
  And we message "confirm invite" to member ".ZZZ" with subs:
  | fullName | a1 |*
  | Abe One  | ?  |
  #And we show "Empty"
  
Scenario: A newbie registers with a different legal name
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | legalName | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One   | Abey One | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | postalCode | country | state | city   | flags     | name    | helper |*
  | .AAC | Abey One | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam | confirmed | abeyone | .ZZZ   |

Scenario: A newbie registers elsewhere
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | postalCode | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 212-253-0000 | US      | 10001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A St. | New York | NY    | 1 A St., New York, NY 10001 |     18 |    1 | .ZZZ   |
  Then members:
  | id      | fullName | email | phone     | postalCode | country | state | city     | flags     | helper |*
  | NYA.AAA | Abe One  | a@ | +12122530000 | 10001      | US      | NY    | New York | confirmed | .ZZZ   |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NYA.AAA | %R_SITE_URL | %name |
  And member "NYA.AAA" one-time password is set

Scenario: A newbie registers with no case
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType     | address | city    | state | postalAddr                | tenure | owns | helper |*
  | abe one  | a@ | 413-253-0000 | 01002      | 111-22-3333 | 1/2/1990 | %R_PERSONAL  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | email | phone     | postalCode | state | city    | flags     | floor | postalAddr | helper |*
  | .AAC | Abe One  | a@ | +14132530000 | 01002      | MA    | Amherst | confirmed | 0     | 1 A ST., Amherst, MA 01001 | .ZZZ   |

Scenario: A member registers bad email
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | postalCode | acctType    |*
  | Abe One  | %whatever | 01001      | %R_PERSONAL |
  Then we say "error": "bad email"

Scenario: A member registers bad name
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone     | postalCode | federalId   | dob      | acctType     | tenure |*
  | � %random | a@ | 413-253-0000 | 01001-3829 | 111-22-3333 | 1/2/1990 | %R_PERSONAL  |     18 |
  Then we say "error": "illegal char" with subs:
  | field    |*
  | fullName |

Scenario: A member registers bad zip
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | phone | postalCode | federalId   | dob      | acctType    |*
  | Abe One  | a@ | 413-253-0001 | %random    | 111-22-3333 | 1/2/1990 | %R_PERSONAL |
  Then we say "error": "bad zip"
  
Scenario: A member registers again
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  Given members:
  | id   | fullName  | phone  | email | city  | state |*
  | .ZZA | Abe One    | +20001 | a@   | Atown | AK    |
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone      | postalCode | federalId   | dob      | acctType    |*
  | Bea Two  | a@ | 413-253-0002 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL |
  Then we say "error": "duplicate email|forgot password" with subs:
  | who     | emailTagged            | a                                       |*
  | Abe One | a+whatever@example.com | a href=account/password/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

Scenario: A member registers with an existing company
  Given members:
  | id   | fullName | email | postalCode | phone        | city     | flags        |*
  | .AAD | Aacme Co | myco@ | 01330      | +14136280000 | Ashfield | co,confirmed |
  And invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhon  | copts                    | address | city    | state | postalAddr                 | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 | isOwner=>1,contractor=>1 | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | email | postalCode | state | city    | flags     | helper |*
  | .AAC | Abe One  | a@    | 01002      | MA    | Amherst | confirmed | .ZZZ   |
  And relations:
  | id   | main | agent | permission | employee | isOwner | draw |*
  | :AAA | .AAD | .AAC  |            |        0 |       1 |    0 |

Scenario: A member registers with an unknown company
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType    | company  | companyPhon  | copts          | address | city    | state | postalAddr                 | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-9876 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 | employeeOk=>1  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | legalName | email | postalCode | phone        | city    | flags     | helper |*
  | .AAC | Abe One   | a@    | 01002      | +14132539876 | Amherst | confirmed | .ZZZ   |
  And no relation:
  | main | agent |*
  | .AAD | .AAC  |
  And signup company info for account ".AAC" is remembered

Scenario: A member registers with a company with no relation
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone        | postalCode | federalId  | dob  | acctType    | company  | companyPhon  | companyOptions | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | (413)628-0000 |               |     18 |    1 | .ZZZ   |
  Then we say "error": "what relation"
Skip (requirement relaxed)
Scenario: A member registers with a missing company
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | postalCode | federalId   | dob      | acctType | company  | companyPhon | companyOptions | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-0002 | 01001 | 111-22-3333 | 1/2/1990 | %R_PERSONAL |       | (413)628-0000 | isOwner=>1     |     18 |    1 | .ZZZ   |
  Then we say "error": "missing field" with subs:
  | field   |*
  | company |
Skip (requirement relaxed)
Scenario: A member registers with a missing company phone
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co |             | isOwner=>1     |     18 |    1 | .ZZZ   |
  Then we say "error": "missing field" with subs:
  | field   |*
  | companyPhon |
Resume
Scenario: A member registers with a bad company
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | 2sp  ces | (413)628-0000 | isOwner=>1   |     18 |    1 | .ZZZ   |
  Then we say "error": "multiple spaces" with subs:
  | field   |*
  | Company |

Scenario: A member registers with a bad company phone
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email  | phone    | postalCode | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %R_PERSONAL | Aacme Co | %random      | isOwner=>1    |     18 |    1 | .ZZZ   |
  Then we say "error": "bad company phone" with subs: ""

Scenario: A newbie registers from elsewhere
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "%name"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone       | postalCode | federalId   | dob      | acctType    | address | city | state | postalAddr                   | tenure | owns | helper |*
  | Abe One  | a@ | (333) 253-0000 | 03768-2345 | 111-22-3333 | 1/2/1990 | %R_PERSONAL | 1 A ST. | Lyme | NH    | 1 A ST., Lyme, NH 03768-2345 |     18 |    1 | .ZZZ   |
 Then members:
  | id      | fullName | email | phone     | postalCode | state | city | flags     | helper |*
  | NEN.AAA | Abe One  | a@ | +13332530000 | 03768-2345 | NH    | Lyme | confirmed | .ZZZ   |
  And we say "status": "your account is ready"
  And we email "welcome" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEN.AAA | %R_SITE_URL | %name |
  # And we show "Empty"
