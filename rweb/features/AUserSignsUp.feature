Feature: A user signs up for rCredits
AS a newbie
I WANT to get access to the rCredits Participants section
SO I can start pretending

Scenario: User completes the signup form
Given user @email1 has no account
When user @email1 signs up
Then user @email1 gets a confirmation email

Scenario: User confirms email ownership
Given user @email1 has an account
And user @email1 has not confirmed email ownership
When user @email1 confirms email ownership
Then we say ‘signup confirmed’
# Congratulations, your rCredits account is ready. Here’s what to do next.
