Feature: Seller Model
	Set and get aspects of the current seller


	Scenario: Should set Name
		Given: A need to get the "name" of the current seller and match it with the "seller on record"
		When: A request is given for a "name"
		Then: This "name" should match the "seller on record"

	Scenario: Should have localStorage device
		Given: That there is a need to reference localStorage
		When: A request is given for the name of the "current localStorage device" 
		Then: The name of the "current localStorage device" should match the "name that has been given"

	Scenario: Should not have Device ID
		Given: That there is assumed to be no Divice ID
		When: There is a query to localStorage for the Divice ID of a "certain seller"
		Then: This "certain seller" should have a "Divice ID" ''

	Scenario: Should have custom Device ID
		Given: That a seller has a "Custom Device ID"
		When: There is a query for the "Divice ID on file"
		Then: The "Custom Device ID" should match the  "Divice ID on file"

	Scenario: Not valid device ID
		Given: The user must have a valid "ID"
		And: That an exception should be thrown when setting invalid Device ID
		When: A non-valid device "ID" is given
		Then: the isValidDeviceId function should be false
		And: An Exception should be thrown
