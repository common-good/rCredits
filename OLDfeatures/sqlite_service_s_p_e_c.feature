Feature: SQLLite Service
	Creates fake member data that is stored in the the sql lite service 


	Scenario: Should create Members table
		Given: the injection of "member data"
		When: fake "member data" is created
		Then:this "member data" should be stored in a SQLLite "Members" table
