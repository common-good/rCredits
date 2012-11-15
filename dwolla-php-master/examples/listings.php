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
 **/
$listings = $Dwolla->listings('01-11-2012');
if(!$listings) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { print_r($listings); } // Print listings
