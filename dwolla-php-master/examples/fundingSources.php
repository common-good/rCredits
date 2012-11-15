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
 *   Fetch all funding sources for the
 *   account associated with the provided
 *   OAuth token
 **/
$fundingSources = $Dwolla->fundingSources();
if(!$fundingSources) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { print_r($fundingSources); } // Print funding sources

/**
 * EXAMPLE 2: 
 *   Fetch detailed information for the
 *   funding source with a specific ID
 **/
$fundingSourceId = 'pJRq4tK38fiAeQ8xo2iH9Q==';
$fundingSource = $Dwolla->fundingSource($fundingSourceId);
if(!$fundingSource) { echo "Error: {$Dwolla->getError()} \n"; } // Check for errors
else { print_r($fundingSource); } // Print funding sources