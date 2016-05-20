Feature: logging in
	When a user clicks 'Scan to sign in,' a QR code scanning screen should appear

	Scenario: clicking the sign in button
		Given: The app has focus
		And: I am not logged in
		When: I click the 'Scan to sign in' button
		Then: The QR code should appear, filling the entire screen
