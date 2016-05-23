Feature: Barcode Scanner Service
	Handle barcode scanning


	Scenario: Should Scan a code
		Given: A user would like authentication
		When: A valid "QR code" is entered
		Then: The bacode_service.scanSuccess function should return a "result" from the server

	Scenario: Scan is NOT a QR CODE
		Given: A user would like authentication
		When: A NOT valid "QR code" is entered
		Then: The bacode_service.scanSuccess function should return "rejectFn('scanQRCode')"
		And: The user should need to try the scan again                   #is there/should there be a limit to how many attepts are possible?

	Scenario: Scan Was Cancelled
		Given: A user would like authentication
		When: The user cancels the transaction
		Then: The bacode_service.scanSuccess_ function should return "rejectFn('scanQRCode')" 
		And: The user should need to try the scan again or be able to quit the app                   

	Scenario: Scan Failed
		Given: A user would like authentication
		When: For some reason the scan fails
		Then: The bacode_service.scanFail_ function should return the "scanError"
		And: The user should need to try the scan again