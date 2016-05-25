Feature: QRCode Parser
	Handle the parsing of QRCodes


	Scenario: Account type must be Company
		Given: A user is trying to scan to login to a "company account"
		When: A "QR Code" is scanned
		Then:A "company account" should be returned

	Scenario: Account type must be Personal
		Given: A user is trying to scan to login to a "Personal account"
		When: A "QR Code" is scanned
		Then:A "Personal account" should be returned

	Scenario: Parse Company Account ID
		Given: A user is has scaned to login to a "Personal account"
		When: A "QR Code" is scanned
		Then:A valid "account Id" should be returned               

	Scenario: Parse Personal Account ID
		Given: A user is has scaned to login to a "Personal account"
		When: A "QR Code" is scanned
		Then:A valid "account Id" should be returned
		
	Scenario: Parse Company Security Code
		Given: A user is has scaned to login to a "Company account"
		When: A "QR Code" is scanned
		Then:A valid "Security Code" should be returned
		
	Scenario: Parse Individual Security Code
		Given: A user is has scaned to login to a "Personal account"
		When: A "QR Code" is scanned
		Then:A valid "Security Code" should be returned