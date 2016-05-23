Feature: Payment Type
	depending on the type of transaction, a fee may or may not be applied


	Scenario: Should apply a fee
		Given: That a the method of payment is a "Credit card"
		When: A user charges a "given amount" of to their "Credit card"
		Then: A fee of a certain "Percentage" will be taken out of the "given amount"
