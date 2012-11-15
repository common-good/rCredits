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
$transactionId = $Dwolla->send($pin, '812-734-7288', 1.00);
if(!$transactionId) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { echo "Send transaction ID: {$transactionId} \n"; } // Print Transaction ID


/**
 * EXAMPLE 2: 
 *   Send money ($1.00) to an email address, with a note
 **/
$transactionId = $Dwolla->send($pin, 'michael@dwolla.com', 1.00, 'Email', 'Everyone loves getting money');
if(!$transactionId) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { echo "Send transaction ID: {$transactionId} \n"; } // Print Transaction ID