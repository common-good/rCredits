David:
- added line to settings to disable poorman's cron
- use hook_cron? timeouts

to do:
- employee ID cards are US mailed to employee.
- employer´s ok is authoritative (employee's ok is merely request to employer)
- track 3 buys and one sale before rTrader (in membership form)
- use mdash for virtual transactions r%
- non-automatic virtual payments are <<pending>> payments, unseen by recipient (don't implement yet)
- bounce processing (separate?)
- check execution time compared to PHP timeout on daily cron and warn admin

CRON
update / check balances (often -- 15 minutes?)
other security / scam checks (often)
completion of (deliberate) pending virtual payments (daily, but not implemented yet)
automatic payments (just virtual payments for now) (daily)
redistribution (selling of unwanted rCredits) (daily)
interest (monthly, calculating daily bals from history)
statements (daily, weekly, monthly -- at user's option)
1099s (annual)
(just auto pay and redist happen in a chunk)
