Feature: Transaction Service
	Handle the various types of transactions possible

	Scenario: Should create a Transaction given a transaction response
	  Given: A TRANSACTION_RESPONSE_OK, it Should create a Transaction 
		When: A TRANSACTION_RESPONSE_OK is parsed from transactionService
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"

	Scenario: Should charge and return a Transaction Object
	  Given: A Transaction object is returned when something is charged 
		When: A transactionService is <charged>
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"
		Then: Expect "transaction.description" to be "description"
		Then: Expect "transaction.amount" to be <charged>
		Then: Expect "transaction.goods" to be <goods>
		
		|charged|goods  |
		| 0.12  | 1     |
		| 1.15  | 1     |
		| 0     | 1     |
		| 5     | 1     |
    | 1     | 1     |
		| asd   | 1     |
		| 1     | 0.12  |
		| 1     | 1.15  |
		| 1     | 0     |
		| 1     | 5     |
		| 1     | asd   |
		| 0.12  | 0.12  |
		| 1.15  | 0.12  |
		| 0     | 0.12  |
		| 5     | 0.12  |
    | 1     | 0.12  |
		| asd   | 0.12  |
		| 0.12  | 1.15  |
		| 1.15  | 1.15  |
		| 0     | 1.15  |
		| 5     | 1.15  |
    | 1     | 1.15  |
		| asd   | 1.15  |
		| 0.12  | asd   |
		| 1.15  | asd   |
		| 0     | asd   |
		| 5     | asd   |
    | 1     | asd   |
		| asd   | asd   |
		| 0.12  | 0     |
		| 1.15  | 0     |
		| 0     | 0     |
		| 5     | 0     |
    | 1     | 0     |
		| asd   | 0     |
		| 0.12  | 5     |
		| 1.15  | 5     |
		| 0     | 5     |
		| 5     | 5     |
    | 1     | 5     |
		| asd   | 5     |

	Scenario: Should charge and update customer reward and balance
	  Given: A Transaction object is returned when something is charged
		When: A TRANSACTION_RESPONSE_OK is parsed from transactionService
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"

	Scenario: Should create a Transaction given a transaction response
	  Given: A TRANSACTION_RESPONSE, it Should create a Transaction 
		When: A TRANSACTION_RESPONSE_OK is parsed from transactionService
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"

	Scenario: Should create a Transaction given a transaction response
	  Given: A TRANSACTION_RESPONSE, it Should create a Transaction 
		When: A TRANSACTION_RESPONSE_OK is parsed from transactionService
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"

	Scenario: Should create a Transaction given a transaction response
	  Given: A TRANSACTION_RESPONSE, it Should create a Transaction 
		When: A TRANSACTION_RESPONSE_OK is parsed from transactionService
		Then: Expect "transaction.getId" to be "TRANSACTION_RESPONSE_OK.txid"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.created"
		Then: Expect "transaction.created" to be "TRANSACTION_RESPONSE_OK.did"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.undo"
		Then: Expect "transaction.undo" to be "TRANSACTION_RESPONSE_OK.message"