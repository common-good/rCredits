<?php
// Include the Dwolla REST Client
require '../lib/dwolla.php';

// Include any required keys
require '_keys.php';

// OAuth parameters
$redirectUri = 'http://localhost:8888/oauth.php'; // Point back to this file/URL
$permissions = array("Send", "Transactions", "Balance", "Request", "Contacts", "AccountInfoFull", "Funding");

// Instantiate a new Dwolla REST Client
$Dwolla = new DwollaRestClient($apiKey, $apiSecret, $redirectUri, $permissions);


/**
 * STEP 1: 
 *   Create an authentication URL
 *   that the user will be redirected to
 **/


if(!isset($_GET['code']) && !isset($_GET['error'])) {
	$authUrl = $Dwolla->getAuthUrl();
	header("Location: {$authUrl}");
}

/**
 * STEP 2:
 *   Exchange the temporary code given
 *   to us in the querystring, for
 *   a never-expiring OAuth access token
 **/
if(isset($_GET['error'])) {
	echo "There was an error. Dwolla said: {$_GET['error_description']}";
}

else if(isset($_GET['code'])) {
	$code = $_GET['code'];

	$token = $Dwolla->requestToken($code);
	if(!$token) { $Dwolla->getError(); } // Check for errors
	else {
		session_start();
		$_SESSION['token'] = $token;
		echo "Your access token is: {$token}";
	} // Print the access token
}
