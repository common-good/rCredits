Feature: Fees may apply
	A fee should be applied, given certain parameters  - linked to fee.js in js/models/


	Scenario: fee of Cash
		Given: That a fee of "given amount" amount is applied
		When: A user charges a "given amount" of cash
		Then: This is the "given amount" that should be charged minus the fee

	Scenario: fee of Percentage
		Given: That a fee of "Percentage" is applied
		When: A user charges a "given amount" of cash
		Then: This is the "given amount" that should be charged minus this "given amount" times one hundredth  the Percentage

	Scenario: fee applied when the amount to be changed is zero
		Given: That the amount entered for the transaction is zero
		When: A user charges a zero value of cash
		Then: This amount should be zero
		
	Scenario: fee applied when the amount to be changed is less then zero
		Given: That the amount entered for the transaction is "less then zero"
		When: A user charges a value of cash "less then zero"
		Then: This amount should be zero