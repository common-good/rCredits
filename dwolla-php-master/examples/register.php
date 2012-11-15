<?php
// Include the Dwolla REST Client
require '../lib/dwolla.php';

// Include any required keys
require '_keys.php';

// Instantiate a new Dwolla REST Client
$Dwolla = new DwollaRestClient($apiKey, $apiSecret, 'http://developers.dwolla.com/some_redirect_uri');

/**
 * EXAMPLE 1: 
 *   Register a new Dwolla user
 **/
$email = 'michael+phplibtest@dwolla.com';
$password = '0neGre4tP4ss';
$pin = '1234';
$firstName = 'Michael';
$lastName = 'Schonfeld';
$address = '902 Broadway Ave';
$address2 = 'Fl 4';
$city = 'New York';
$state = 'NY';
$zip = '10010';
$phone = '8182670931';
$dateOfBirth = '08-01-1987';
$acceptTerms = TRUE;
$type = 'Personal';
$organization = FALSE;
$ein = FALSE;

$user = $Dwolla->register($email, $password, $pin, $firstName, $lastName, $address, $city, $state, $zip, $phone, $dateOfBirth, $acceptTerms, $address2, $type, $organization, $ein);
$errorMsg="Error: {$Dwolla->getError()}";
if(!$user) { echo $errorMsg; } // Check for errors
else {print_r($user);} // Show registration response
