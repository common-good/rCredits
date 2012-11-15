<?php
// Include the Dwolla REST Client
require '../lib/dwolla.php';

// Include any required keys
require '_keys.php';

// Instantiate a new Dwolla REST Client
$Dwolla = new DwollaRestClient();

// Seed a previously generated access token
$Dwolla->setToken($token);


/**
 * EXAMPLE 1: 
 *   Send money ($1.00) to a Dwolla ID 
 **/
$transactionId = $Dwolla->request($pin, '812-734-7288', 1.00);
if(!$transactionId) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { echo "Request ID: {$transactionId} \n"; } // Print Transaction ID

