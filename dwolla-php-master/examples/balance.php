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
 *   Fetch account balance for the 
 *   account associated with the provided
 *   OAuth token
 **/
$balance = $Dwolla->balance();
if(!$balance) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { echo $balance; } // Print user's balance