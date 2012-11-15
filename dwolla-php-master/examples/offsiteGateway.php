<?php
// Include the Dwolla REST Client
require '../lib/dwolla.php';

// Include any required keys
require '_keys.php';

// Instantiate a new Dwolla REST Client
$Dwolla = new DwollaRestClient($apiKey, $apiSecret, 'http://localhost:8888/offsiteGateway.php');
?>

<div>
<h2>Example 1: Simple checkout</h2>
<p>Create a new offsite gateway checkout session, with 1 test product, and a minimum of parameters</p>
<?php
/**
 * EXAMPLE 1: (simple example) 
 *   Create a new offsite gateway checkout
 *   session, with 1 test product, and
 *   a minimum of parameters
 **/
// Clears out any previous products
$Dwolla->startGatewaySession();

// Add first product; Price = $10, Qty = 1
$Dwolla->addGatewayProduct('Test 1', 10);

// Creates a checkout session, and return the URL
// Destination ID: 812-713-9234
$url = $Dwolla->getGatewayURL('812-713-9234');
if(!$url) { echo $Dwolla->getError(); } // Display any errors returned from Dwolla
else { echo "<p>Example 1 URL: <a href='{$url}'>{$url}</a></p>"; } // Forward the user to the offsite gateway
?>
</div>


<div>
<h2>Example 1: In depth checkout</h2>
<p>Create a new test-mode offsite gateway checkout session, with 2 test products, a discount, add shipping costs, tax, and order ID, and a memo/note</p>
<?php
/**
 * EXAMPLE 2: (in depth example) 
 *   Create a new offsite gateway checkout
 *   session, with 2 test products, a
 *   discount, add shipping costs, tax,
 *   and order ID, and a memo/note
 **/
// Set the server mode to test mode
$Dwolla->setMode('TEST');

// Clears out any previous products
$Dwolla->startGatewaySession();

// Add first product; Price = $10, Qty = 1
$Dwolla->addGatewayProduct('Test 1', 10, 1, 'Test product');

// Add second product; Price = $6, Qty = 2
$Dwolla->addGatewayProduct('Test 2', 6, 2, 'Another Test product');

// Creates a checkout session, and return the URL
// Destination ID: 812-713-9234
// Order ID: 10001
// Discount: $5
// Shipping: $0.99
// Tax: $1.87
// Memo: 'This is a great purchase'
// Callback: 'http://requestb.in/1fy628r1' (you'll need to create your own bin at http://requestb.in/)
$url = $Dwolla->getGatewayURL('812-713-9234', '10001', 24.85, 0.99, 1.87, 'This is a great purchase', 'http://requestb.in/1fy628r1');
if(!$url) { echo $Dwolla->getError(); } // Display any errors returned from Dwolla
else { echo "<p>Example 2 URL: <a href='{$url}'>{$url}</a></p>"; } // Forward the user to the offsite gateway
?>
</div>


<div>
<h2>Example 3: Verifying an offsite gateway signature</h2>
<p>Verify the signature returned from Dwolla's offsite gateway redirect</p>
<?php
/**
 * EXAMPLE 3: (verifying an offsite gateway signature) 
 *   Verify the signature returned from
 *   Dwolla's offsite gateway redirect
 **/
// Grab Dwolla's proposed signature
$signature = $_GET['signature'];

// Grab Dwolla's checkout ID
$checkoutId = $_GET['checkoutId'];

// Grab the reported total transaction amount
$amount = $_GET['amount'];

// Verify the proposed signature
$didVerify = $Dwolla->verifyGatewaySignature($signature, $checkoutId, $amount);

if($didVerify) { 
	echo "<p>Dwolla's signature verified successfully. You should go ahead and process the order.</p>";
} else {
	echo "<p>Dwolla's signature failed to verify. You shouldn't process the order before some manual verification.</p>";
}
?>
</div>
