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
 *   Fetch last 10 contacts from the 
 *   account associated with the provided
 *   OAuth token
 **/
$contacts = $Dwolla->contacts('Ben');
if(!$contacts) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { print_r($contacts); } // Print contacts

/**
 * EXAMPLE 2: 
 *   Search through the contacts of the
 *   account associated with the provided
 *   OAuth token
 **/
$contacts = $Dwolla->contacts('Ben');
if(!$contacts) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { print_r($contacts); } // Print contacts