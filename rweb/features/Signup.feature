Feature: A user signs up for rCredits
AS a newbie
I WANT to open an rCredits account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | id   | fullName | acctType    | flags      | created  |*
  | .ZZZ | Zeta Zot | personal    | ok         | 99654321 |
  And member is logged out

Skip because this error should happen after saying what zipcode
Scenario: A newbie visits the registration page with no invite
  Given community "invites" is "on"
  And invitation to email "a@" from member ".ZZZ" is ""
  When member "?" visits page "signup"
  Then we show "Open a Personal %PROJECT Account" with:
  |~errorPhrase         |
  | you must be invited |

Scenario: A newbie visits the registration page with bad invite
  Given community "invites" is "on"
  And invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" visits page "signup/code=WhAtEvEr"
  Then we show "Open a Personal %PROJECT Account" with:
  |~errorPhrase         |
  | you must be invited |
Resume

#Scenario: A newbie visits the invitation acceptance page with no invite
#  Given community "invites" is "on"
#  When member "?" confirms form "accept/self" with values:
#  | friend | zip |*
#  | self   | 01001      |
#  Then we say "error": "invitation required" with subs:
#  | a1 |*
#  | a href=''%PROMO_URL/signup'' |

Scenario: A newbie visits the registration page with expired invite
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And invitation "c0D3" was sent on "%today-5w"
  When member "?" visits page "signup/code=c0D3"
  Then we show "Open a Personal %PROJECT Account"
# lateness no longer makes a big difference
#  And we say "error": "expired invite" with subs:
#  | a | inviterName |*
#  | ? | Zeta Zot    |

Scenario: A newbie visits the registration page with a used invite
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | zip | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 | .ZZZ   |
  When member "?" visits page "signup/code=c0D3"
  Then we show "Open a Personal %PROJECT Account"
#  And we say "error": "used invite"

Scenario: A newbie registers in Western Massachusetts
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | country | zip   | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01002 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | zip | country | state | city   | flags     | floor | address | postalAddr                | tenure | owns | helper |*
  | .AAA | Abe One  | Abe One   | a@ | +14132530000 | 01002 | US      | MA    | Agawam | confirmed | 0     |    1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ  |
  And we email "verify" to member "a@" with subs:
  | fullName | name   | quid    | site      | code      |*
  | Abe One  | abeone | NEW.AAA | %BASE_URL | WHATEVER |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  #And we show "Empty"

Scenario: A newbie registers with an unconfirmed icard invitation
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=SCMZDDI26QF" with values:
  | fullName | email | phone     | country | zip | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ |
  Then members:
  | id   | fullName | legalName | email | phone     | zip | country | state | city   | flags | floor | address | postalAddr              | tenure | owns | iCode | helper |*
  | .AAA | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | 0     |   1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 |     5 | .ZZZ   |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
# LATER  And we say "status": "must be confirmed" with subs:
#  | inviterName |*
#  | Zeta Zot    |
  And we email "verify" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAA | %BASE_URL | WHATEVER |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we message "confirm invite" to member ".ZZZ" with subs:
  | fullName | a1 |*
  | Abe One  | ?  |
  #And we show "Empty"
  
Scenario: A newbie registers with an unconfirmed self-invitation
  Given member ".ZZZ" email invitation code is "VDYQGBRQO7A"
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=VDYQGBRQO7A" with values:
  | fullName | email | phone     | country | zip | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St. Agawam MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | zip | country | state | city   | flags | floor | address | postalAddr              | tenure | owns | iCode      | helper |*
  | .AAA | Abe One  | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam |       | 0     |   1 A St. | 1 A St. Agawam MA 01001 |     18 |    1 | %IBY_SELF | .ZZZ   |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
# LATER  And we say "status": "must be confirmed" with subs:
#  | inviterName |*
#  | Zeta Zot    |
  And we email "verify" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEW.AAA | %BASE_URL | WHATEVER |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we message "confirm invite" to member ".ZZZ" with subs:
  | fullName | a1 |*
  | Abe One  | ?  |
  #And we show "Empty"
  
Scenario: A newbie registers with a different legal name
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | legalName | fullName | email | phone     | country | zip | federalId   | dob      | acctType     | address | city       | state | postalAddr                | tenure | owns | helper |*
  | Abe One   | Abey One | a@ | 413-253-0000 | US      | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A St. | Agawam | MA    | 1 A St., Agawam, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | legalName | email | phone     | zip | country | state | city   | flags     | name    | helper |*
  | .AAA | Abey One | Abe One   | a@ | +14132530000 | 01001      | US      | MA    | Agawam | confirmed | abeyone | .ZZZ   |

Scenario: A newbie registers from elsewhere
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone       | zip | federalId   | dob      | acctType    | address | city | state | postalAddr                   | tenure | owns | helper |*
  | Abe One  | a@ | (333) 253-0000 | 03768-2345 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | 1 A ST. | Lyme | NH    | 1 A ST., Lyme, NH 03768-2345 |     18 |    1 | .ZZZ   |
 Then members:
  | id      | fullName | email | phone     | zip | state | city | flags     | helper |*
  | NEN.AAA | Abe One  | a@ | +13332530000 | 03768-2345 | NH    | Lyme | confirmed | .ZZZ   |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And we email "verify" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEN.AAA | %BASE_URL | WHATEVER |
  # And we show "Empty"

Scenario: A newbie registers with no case
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone     | zip | federalId   | dob      | acctType     | address | city    | state | postalAddr                | tenure | owns | helper |*
  | abe one  | a@ | 413-253-0000 | 01002      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | email | phone     | zip | state | city    | flags     | floor | postalAddr | helper |*
  | .AAA | Abe One  | a@ | +14132530000 | 01002      | MA    | Amherst | confirmed | 0     | 1 A ST., Amherst, MA 01001 | .ZZZ   |

Scenario: A member registers bad email
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | zip | acctType    |*
  | Abe One  | %whatever | 01001      | %CO_PERSONAL |
  Then we say "error": "bad email"

Scenario: A member registers bad name
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName  | email | phone     | zip | federalId   | dob      | acctType     | tenure |*
  | ™ %random | a@ | 413-253-0000 | 01001-3829 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL  |     18 |
  Then we say "error": "illegal char" with subs:
  | field    |*
  | fullName |

Scenario: A member registers bad zip
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email     | phone | zip | federalId   | dob      | acctType    |*
  | Abe One  | a@ | 413-253-0001 | %random    | 111-22-3333 | 1/2/1990 | %CO_PERSONAL |
  Then we say "error": "bad zip"
  
Scenario: A member registers again
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  Given members:
  | id   | fullName  | phone  | email | city  | state |*
  | .ZZA | Abe One    | +20001 | a@   | Atown | AK    |
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone      | zip | federalId   | dob      | acctType    |*
  | Bea Two  | a@ | 413-253-0002 | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL |
  Then we say "error": "duplicate email|forgot password" with subs:
  | who     | a                                          |*
  | Abe One | a href="settings/password/a%40example.com" |
#  Then we say "error": "duplicate email|forgot password" with subs:
#  | who     | emailTagged            | a                                       |*
#  | Abe One | a+whatever@example.com | a href=settings/password/a%40example.com |
#  And member is logged out
# That email is taken. Click here to get a new password.

Scenario: A member registers with an existing company
  Given members:
  | id   | fullName | email | zip | phone        | city     | flags        |*
  | .AAD | AAAme Co | myco@ | 01330      | +14136280000 | Ashfield | co,confirmed |
  And invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | zip | federalId   | dob      | acctType    | company  | companyPhon  | companyOptions           | address | city    | state | postalAddr                 | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | AAAme Co | (413)628-0000 | owner=>1,contractor=>1 | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | fullName | email | zip | state | city    | flags     | helper |*
  | .AAA | Abe One  | a@    | 01002      | MA    | Amherst | confirmed | .ZZZ   |
  And relations:
  | id   | main | agent | permission | employee | owner | draw |*
  | .AAA | .AAD | .AAA  |            |        0 |       1 |    0 |

Scenario: A member registers with an unknown company
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone   | zip | federalId   | dob      | acctType    | company  | companyPhon  | companyOptions | address | city    | state | postalAddr                 | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-9876 | 01002 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | AAAme Co | (413)628-0000 | employee=>1  | 1 A ST. | amherst | MA    | 1 A ST., Amherst, MA 01001 |     18 |    1 | .ZZZ   |
  Then members:
  | id   | legalName | email | zip | phone        | city    | flags     | helper |*
  | .AAA | Abe One   | a@    | 01002      | +14132539876 | Amherst | confirmed | .ZZZ   |
  And no relation:
  | main | agent |*
  | .AAD | .AAA  |
  And signup company info for account ".AAA" is remembered

Scenario: A member registers with a company with no relation
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone        | zip | federalId  | dob  | acctType    | company  | companyPhon  | companyOptions | tenure | owns | helper |*
  | Abe One  | a@    | 413-253-0002 | 01002 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | AAAme Co | (413)628-0000 |               |     18 |    1 | .ZZZ   |
  Then we say "error": "what relation"

Scenario: A member registers with a bad company phone
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email  | phone    | zip | federalId   | dob      | acctType    | company  | companyPhon | companyOptions | tenure | owns | helper |*
  | Abe One  | a@ | 413-253-9876 | 01001      | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | AAAme Co | %random      | owner=>1    |     18 |    1 | .ZZZ   |
  Then we say "error": "bad company phone" with subs: ""
