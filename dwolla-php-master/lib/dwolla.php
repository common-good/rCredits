<?php

/**
 * Dwolla REST API Library for PHP
 *
 * MIT LICENSE
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @package   Dwolla
 * @author    Michael Schonfeld <michael@dwolla.com>
 * @copyright Copyright (c) 2012 Dwolla Inc. (http://www.dwolla.com)
 * @license   http://opensource.org/licenses/MIT MIT
 * @version   1.2.6
 * @link      http://www.dwolla.com
 */

if (!function_exists('curl_init')) { 
    throw new Exception("Dwolla's API Client Library requires the CURL PHP extension.");
}

if (!function_exists('json_decode')) {
    throw new Exception("Dwolla's API Client Library requires the JSON PHP extension.");
}

/**
 * Dwolla REST API Library for PHP
 *
 * @package   Dwolla
 * @author    Michael Schonfeld <michael@dwolla.com>
 * @copyright Copyright (c) 2012 Dwolla Inc. (http://www.dwolla.com)
 * @license   http://opensource.org/licenses/MIT MIT
 */
class DwollaRestClient
{

    /**
     * @var string Dwolla API key
     */
    private $apiKey;

    /**
     * @var string Dwolla API key
     */
    private $apiSecret;

    /**
     * @var string oauth token
     */
    private $oauthToken;

    /**
     * @var array oauth authentication scopes
     */
    private $permissions;

    /**
     *
     * @var string URL to return the user to after the authentication request
     */
    private $redirectUri;

    /**
     * @var string Transaction mode. Can be 'live' or 'test'
     */
    private $mode;

    /**
     * @var array Off-Site Gateway order items
     */
    private $gatewaySession;

    /**
     * @var string error messages returned from Dwolla
     */
    private $errorMessage = false;

    const API_SERVER = "https://www.dwolla.com/oauth/rest/";

    /**
     * Sets the initial state of the client
     * 
     * @param string $apiKey
     * @param string $apiSecret
     * @param string $redirectUri
     * @param array $permissions
     * @param string $mode 
     * @throws InvalidArgumentException
     */
    public function __construct($apiKey = false, $apiSecret = false, $redirectUri = false, $permissions = array("send", "transactions", "balance", "request", "contacts", "accountinfofull", "funding"), $mode = 'live')
    {
        $this->apiKey = $apiKey;
        $this->apiSecret = $apiSecret;
        $this->redirectUri = $redirectUri;
        $this->permissions = $permissions;
        $this->apiServerUrl = self::API_SERVER;
        $this->setMode($mode);
    }

    /**
     * Get oauth authenitcation URL
     * 
     * @return string URL
     */
    public function getAuthUrl()
    {
        $params = array(
            'client_id' => $this->apiKey,
            'response_type' => 'code',
            'scope' => implode('|', $this->permissions)
        );

        // Only append a redirectURI if one was explicitly specified
        if ($this->redirectUri) {
            $params['redirect_uri'] = $this->redirectUri;
        }

        $url = 'https://www.dwolla.com/oauth/v2/authenticate?' . http_build_query($params);

        return $url;
    }

    /**
     * Request oauth token from Dwolla
     * 
     * @param string $code Temporary code returned from Dwolla
     * @return string oauth token
     */
    public function requestToken($code)
    {
        if (!$code) {
            return $this->setError('Please pass an oauth code.');
        }

        $params = array(
            'client_id' => $this->apiKey,
            'client_secret' => $this->apiSecret,
            'redirect_uri' => $this->redirectUri,
            'grant_type' => 'authorization_code',
            'code' => $code
        );
        $url = 'https://www.dwolla.com/oauth/v2/token?' . http_build_query($params);
        $response = $this->curl($url, 'GET');

        if (isset($response['error'])) {
            $this->errorMessage = $response['error_description'];
            return false;
        }

        return $response['access_token'];
    }

    /**
     * Grabs the account information for the
     * authenticated user
     *
     * @return array Authenticated user's account information
     */
    public function me()
    {
        $response = $this->get('users');
        return $this->parse($response);
    }

    /**
     * Grabs the basic account information for
     * the provided Dwolla account Id
     * 
     * @param string $userId Dwolla Account Id
     * @return array Basic account information 
     */
    public function getUser($userId)
    {
        $params = array(
            'client_id' => $this->apiKey,
            'client_secret' => $this->apiSecret
        );

        $response = $this->get("users/{$userId}", $params);
        $user = $this->parse($response);

        return $user;
    }

    /**
     * Register new Dwolla user
     *  
     * @param string $email
     * @param string $password
     * @param int $pin
     * @param string $firstName
     * @param string $lastName
     * @param string $address
     * @param string $city
     * @param string $state
     * @param int $zip
     * @param string $phone
     * @param string $dateOfBirth
     * @param bool $acceptTerms
     * @param string $address2
     * @param string $type Dwolla account type
     * @param string $organization
     * @param string $ein
     * @return array New user information 
     */
    public function register($email, $password, $pin, $firstName, $lastName, $address, $city, $state, $zip, $phone, $dateOfBirth, $acceptTerms, $address2 = '', $type = 'Personal', $organization = '', $ein = ''
    )
    {
        $params = array(
            'client_id' => $this->apiKey,
            'client_secret' => $this->apiSecret,
            'email' => $email,
            'password' => $password,
            'pin' => $pin,
            'firstName' => $firstName,
            'lastName' => $lastName,
            'address' => $address,
            'address2' => $address2,
            'city' => $city,
            'state' => $state,
            'zip' => $zip,
            'phone' => $phone,
            'dateOfBirth' => $dateOfBirth,
            'type' => $type,
            'organization' => $organization,
            'ein' => $ein,
            'acceptTerms' => $acceptTerms
        );
        $response = $this->post('register/', $params, false); // false = don't include oAuth token

        $user = $this->parse($response);

        return $user;
    }

    /**
     * Search contacts
     * 
     * @param string $search Search term(s)
     * @param array $types Types of contacts (Dwolla, Facebook . . .)
     * @param int $limit Number of contacts to retrieve between 1 and 200. 
     * @return array
     */
    public function contacts($search = false, $types = array('Dwolla'), $limit = 10)
    {
        $params = array(
            'search' => $search,
            'types' => implode(',', $types),
            'limit' => $limit
        );
        $response = $this->get('contacts', $params);

        $contacts = $this->parse($response);

        return $contacts;
    }

    /**
     * Use this method to retrieve nearby Dwolla spots within the range of the 
     * provided latitude and longitude. 
     * 
     * Half of the limit are returned as spots with closest proximity. The other 
     * half of the spots are returned as random spots within the range.
     * 
     * @todo implement
     * 
     * @param float $latitude
     * @param float $longitude
     * @param int $range Range to search in miles
     * @param int $limit Limit search to this number results
     * @return array Search results 
     */
    public function nearbyContacts($latitude, $longitude, $range = 10, $limit = 10)
    {
        // TODO implement
        return null;
    }

    /**
     * Retrieve a list of verified funding sources for the user associated 
     * with the authorized access token.
     *
     * @return array Funding Sources
     */
    public function fundingSources()
    {
        $response = $this->get('fundingsources');
        return $this->parse($response);
    }

    /**
     * Retrieve a verified funding source by identifier for the user associated 
     * with the authorized access token.
     *
     * @param string Funding Source ID
     * @return array Funding Source Details
     */
    public function fundingSource($fundingSourceId)
    {
        $response = $this->get("fundingsources?fundingid={$fundingSourceId}");
        return $this->parse($response);
    }

    /**
     * Retrieve the account balance for the user with the given authorized 
     * access token. 
     * 
     * @return float Balance in USD 
     */
    public function balance()
    {
        $response = $this->get('balance');
        return $this->parse($response);
    }

    /**
     * Send funds to a destination user from the user associated with the 
     * authorized access token.
     * 
     * @param int $pin
     * @param string $destinationId Dwolla identifier, Facebook identifier, Twitter identifier, phone number, or email address
     * @param float $amount
     * @param string $destinationType Type of destination user. Can be Dwolla, Facebook, Twitter, Email, or Phone. Defaults to Dwolla.
     * @param string $notes Note to attach to the transaction. Limited to 250 characters.
     * @param float $facilitatorAmount
     * @param bool $assumeCosts Will sending user assume the Dwolla fee?
     * @param string $fundsSource Funding source ID to use. Defaults to Dwolla balance.
     * @return string Transaction Id 
     */
    public function send($pin = false, $destinationId = false, $amount = false, $destinationType = 'Dwolla', $notes = '', $facilitatorAmount = 0, $assumeCosts = false, $fundsSource = 'balance'
    )
    {
        // Verify required paramteres
        if (!$pin) {
            return $this->setError('Please enter a PIN.');
        } else if (!$destinationId) {
            return $this->setError('Please enter a destination ID.');
        } else if (!$amount) {
            return $this->setError('Please enter a transaction amount.');
        }

        // Build request, and send it to Dwolla
        $params = array(
            'pin' => $pin,
            'destinationId' => $destinationId,
            'destinationType' => $destinationType,
            'amount' => $amount,
            'facilitatorAmount' => $facilitatorAmount,
            'assumeCosts' => $assumeCosts,
            'notes' => $notes,
            'fundsSource' => $fundsSource,
        );
        $response = $this->post('transactions/send', $params);

        // Parse Dwolla's response
        $transactionId = $this->parse($response);

        return $transactionId;
    }

    /**
     * Request funds from a source user, originating from the user associated 
     * with the authorized access token.
     * 
     * @param int $pin
     * @param string $sourceId
     * @param float $amount
     * @param string $sourceType
     * @param string $notes
     * @param float $facilitatorAmount
     * @return int Request Id 
     */
    public function request($pin = false, $sourceId = false, $amount = false, $sourceType = 'Dwolla', $notes = '', $facilitatorAmount = 0)
    {
        // Verify required paramteres
        if (!$pin) {
            return $this->setError('Please enter a PIN.');
        } else if (!$sourceId) {
            return $this->setError('Please enter a source ID.');
        } else if (!$amount) {
            return $this->setError('Please enter a transaction amount.');
        }

        // Build request, and send it to Dwolla
        $params = array(
            'pin' => $pin,
            'sourceId' => $sourceId,
            'sourceType' => $sourceType,
            'amount' => $amount,
            'facilitatorAmount' => $facilitatorAmount,
            'notes' => $notes
        );
        $response = $this->post('transactions/request', $params);

        // Parse Dwolla's response
        $transactionId = $this->parse($response);

        return $transactionId;
    }

    /**
     * Grab information for the given transaction ID
     *
     * @param int Transaction ID to which information is pulled
     * @return array Transaction information
     */
    public function transaction($transactionId)
    {
        // Verify required paramteres
        if (!$transactionId) {
            return $this->setError('Please enter a transaction ID.');
        }

        // Build request, and send it to Dwolla
        $response = $this->get("transactions/{$transactionId}");

        // Parse Dwolla's response
        $transaction = $this->parse($response);

        return $transaction;
    }

    /**
     * Retrieve a list of transactions for the user associated with the 
     * authorized access token.
     * 
     * @param string $sinceDate Earliest date and time for which to retrieve transactions. Defaults to 7 days prior to current date and time in UTC. Format: DD-MM-YYYY
     * @param array $types Types of transactions to retrieve.  Options are money_sent, money_received, deposit, withdrawal, and fee.
     * @param int $limit Number of transactions to retrieve between 1 and 200
     * @param int $skip Number of transactions to skip
     * @return array Transaction search results 
     */
    public function listings($sinceDate = false, $types = false, $limit = 10, $skip = 0)
    {
        $params = array(
            'limit' => $limit,
            'skip' => $skip
        );

        if($sinceDate) { $params['sinceDate'] = $sinceDate; }
        if($types) { $params['types'] = implode('|', $types); }

        // Build request, and send it to Dwolla
        $response = $this->get("transactions", $params);

        // Parse Dwolla's response
        $listings = $this->parse($response);

        return $listings;
    }

    /**
     * Retrieve transactions stats for the user associated with the authorized 
     * access token.
     * 
     * @param array $types Options are 'TransactionsCount' and 'TransactionsTotal'
     * @param string $startDate Starting date and time to for which to process transactions stats. Defaults to 0300 of the current day in UTC.
     * @param string $endDate Ending date and time to for which to process transactions stats. Defaults to current date and time in UTC.
     * @return array Transaction stats search results 
     */
    public function stats($types = array('TransactionsCount', 'TransactionsTotal'), $startDate = null, $endDate = null)
    {
        $params = array(
            'types' => implode(',', $types),
            'startDate' => $startDate,
            'endDate' => $endDate
        );

        // Build request, and send it to Dwolla
        $response = $this->get("transactions/stats", $params);

        // Parse Dwolla's response
        $stats = $this->parse($response);

        return $stats;
    }

    /**
     * Creates an empty Off-Site Gateway Order Items array 
     */
    public function startGatewaySession()
    {
        $this->gatewaySession = array();
    }

    /**
     * Adds new order item to gateway session
     * 
     * @param string $name
     * @param float $price Item price in USD
     * @param int $quantity Number of items
     * @param string $description Item description
     */
    public function addGatewayProduct($name, $price, $quantity = 1, $description = '')
    {
        $product = array(
            'Name' => $name,
            'Description' => $description,
            'Price' => $price,
            'Quantity' => $quantity
        );

        $this->gatewaySession[] = $product;
    }

    /**
     * Creates and executes Server-to-Server checkout request
     * @link http://developers.dwolla.com/dev/docs/gateway#server-to-server
     * 
     * @param string $destinationId
     * @param string $orderId
     * @param float $discount
     * @param float $shipping
     * @param float $tax
     * @param string $notes
     * @param string $callback
     * @param boolean $allowFundingSources
     * @return string Checkout URL 
     */
    public function getGatewayURL($destinationId, $orderId = null, $discount = 0, $shipping = 0, $tax = 0, $notes = '', $callback = null, $allowFundingSources = TRUE)
    {
        // TODO add validation? Throw exception if malformed?
        $destinationId = $this->parseDwollaID($destinationId);

        // Normalize optional parameters
        if (!$shipping) {
            $shipping = 0;
        } else {
            $shipping = floatval($shipping);
        }
        if (!$tax) {
            $tax = 0;
        } else {
            $tax = floatval($tax);
        }
        if (!$discount) {
            $discount = 0;
        } else {
            $discount = abs(floatval($discount));
        }
        if (!$notes) {
            $notes = '';
        }

        // Calcualte subtotal
        $subtotal = 0;

        foreach ($this->gatewaySession as $product) {
            $subtotal += floatval($product['Price']) * floatval($product['Quantity']);
        }

        // Calculate grand total
        $total = round($subtotal - $discount + $shipping + $tax, 2);

        // Create request body
        $request = array(
            'Key' => $this->apiKey,
            'Secret' => $this->apiSecret,
            'Test' => ($this->mode == 'test') ? 'true' : 'false',
            'AllowFundingSources' => $allowFundingSources ? 'true' : 'false',
            'PurchaseOrder' => array(
                'DestinationId' => $destinationId,
                'OrderItems' => $this->gatewaySession,
                'Discount' => -$discount,
                'Shipping' => $shipping,
                'Tax' => $tax,
                'Total' => $total,
                'Notes' => $notes
            )
        );

        // Append optional parameters
        if ($this->redirectUri) {
            $request['Redirect'] = $this->redirectUri;
        }

        if ($callback) {
            $request['Callback'] = $callback;
        }

        if ($orderId) {
            $request['OrderId'] = $orderId;
        }

        // Send off the request
        $response = $this->curl('https://www.dwolla.com/payment/request', 'POST', $request);

        if ($response['Result'] != 'Success') {
            $this->errorMessage = $response['Message'];
            return false;
        }

        return 'https://www.dwolla.com/payment/checkout/' . $response['CheckoutId'];
    }

    /**
     * Verify a signature that came back
     * with an offsite gateway redirect
     *
     * @param {string} Proposed signature; (required)
     * @param {string} Dwolla's checkout ID; (required)
     * @param {string} Dwolla's reported total amount; (required)
     *
     * @return {boolean} Whether or not the signature is valid
     */

    /**
     * Verifiy the signature returned from Offsite-Gateway Redirect
     * 
     * @param string $signature
     * @param string $checkoutId
     * @param float $amount
     * @return bool Is signature valid? 
     */
    public function verifyGatewaySignature($signature = false, $checkoutId = false, $amount = false)
    {
        // Verify required paramteres
        if (!$signature) {
            return $this->setError('Please pass a proposed signature.');
        }
        if (!$checkoutId) {
            return $this->setError('Please pass a checkout ID.');
        }
        if (!$amount) {
            return $this->setError('Please pass a total transaction amount.');
        }

        // Normalize parameters
        $amount = floatval($amount);

        // Calculate an HMAC-SHA1 hexadecimal hash
        // of the checkoutId and amount ampersand separated
        // using the consumer secret of the application
        // as the hash key.
        //
        // @doc: http://developers.dwolla.com/dev/docs/gateway
        $hash = hash_hmac("sha1", "{$checkoutId}&{$amount}", $this->apiSecret);

        if($hash !== $signature) {
          return $this->setError('Dwolla signature verification failed.');
        }

        return TRUE;
    }

    /**
     * @return string|bool Error message or false if error message does not exist
     */
    public function getError()
    {
        if (!$this->errorMessage) {
            return false;
        }

        $error = $this->errorMessage;
        $this->errorMessage = false;

        return $error;
    }

    /**
     * Returns properly formatted Dwolla Id
     * 
     * @param string|int $id
     * @return string Properly formatted Dwolla Id 
     */
    public function parseDwollaID($id)
    {
        $id = preg_replace("/[^0-9]/", "", $id);
        $id = preg_replace("/([0-9]{3})([0-9]{3})([0-9]{4})/", "$1-$2-$3", $id);

        return $id;
    }

    /**
     * @param string $message Error message
     */
    protected function setError($message)
    {
        $this->errorMessage = $message;
    }

    /**
     * Parse API response
     * 
     * @param array $response
     * @return array
     */
    protected function parse($response)
    {
        if (!$response['Success']) {
            $this->errorMessage = $response['Message'];

            // Exception for /register method
            if ($response['Response']) {
                $this->errorMessage .= " :: " . json_encode($response['Response']);
            }

            return false;
        }

        return $response['Response'];
    }

    /**
     * Executes POST request against API
     * 
     * @param string $request
     * @param array $params
     * @param bool $includeToken Include oauth token in request?
     * @return array|null 
     */
    protected function post($request, $params = false, $includeToken = true)
    {
        $url = $this->apiServerUrl . $request . ($includeToken ? "?oauth_token=" . urlencode($this->oauthToken) : "");

        $rawData = $this->curl($url, 'POST', $params);

        return $rawData;
    }

    /**
     * Executes GET requests against API
     * 
     * @param string $request
     * @param array $params
     * @return array|null Array of results or null if json_decode fails in curl()
     */
    protected function get($request, $params = array())
    {
        $params['oauth_token'] = $this->oauthToken;

        $delimiter = (strpos($request, '?') === false) ? '?' : '&';
        $url = $this->apiServerUrl . $request . $delimiter . http_build_query($params);

        $rawData = $this->curl($url, 'GET');

        return $rawData;
    }

    /**
     * Execute curl request
     * 
     * @param string $url URL to send requests
     * @param string $method HTTP method
     * @param array $params request params
     * @return array|null Returns array of results or null if json_decode fails
     */
    protected function curl($url, $method = 'GET', $params = array())
    {
        // Encode POST data
        $data = json_encode($params);

        // Set request headers
        $headers = array('Accept: application/json', 'Content-Type: application/json;charset=UTF-8');
        if ($method == 'POST') {
            $headers[] = 'Content-Length: ' . strlen($data);
        }

        // Set up our CURL request
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
        curl_setopt($ch, CURLOPT_TIMEOUT, 5);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_HEADER, false);
        curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);

        // Windows require this certificate
        $ca = dirname(__FILE__);
        curl_setopt($ch, CURLOPT_CAINFO, $ca); // Set the location of the CA-bundle
        curl_setopt($ch, CURLOPT_CAINFO, $ca . '/cacert.pem'); // Set the location of the CA-bundle
        // Initiate request
        $rawData = curl_exec($ch);

        // If HTTP response wasn't 200,
        // log it as an error!
        $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        if ($code !== 200) {
            return array(
                'Success' => false,
                'Message' => "Request failed. Server responded with: {$code}"
            );
        }

        // All done with CURL
        curl_close($ch);

        // Otherwise, assume we got some
        // sort of a response
        return json_decode($rawData, true);
    }

    /**
     * @param string $token oauth token
     */
    public function setToken($token)
    {
        $this->oauthToken = $token;
    }

    /**
     * @return string oauth token
     */
    public function getToken()
    {
        return $this->oauthToken;
    }

    /**
     * Sets client mode.  Appropriate values are 'live' and 'test'
     * 
     * @param string $mode
     * @throws InvalidArgumentException
     * @return void
     */
    public function setMode($mode = 'live')
    {
        $mode = strtolower($mode);
        
        if ($mode != 'live' && $mode != 'test') {
            throw new InvalidArgumentException('Appropriate mode values are live or test');
        }

        $this->mode = $mode;
    }

    /**
     * @return string Client mode
     */
    public function getMode()
    {
        return $this->mode;
    }

}