<?php
define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows');
define('API_URL', REAL ? 'http://devcore.rCredits.org/api' : 'http://localhost/devcore/api');

$my_id = 'NEW.AAC';
$op = 'first_time';
$password = '123';
//send(compact('op', 'password'));

$op = 'reset';
//send(compact('op'));

$code = REAL ? '$S$DYdUC1HmGS1e5tRmX1izygjbF2xWA/UAYWgROsbXdvbnK967xfe5/' : '$S$D4M40JWYSLvrCme.6CdBcA5x62wam6kMk1uki430FtIl6WZBVlfd/';

$op = 'startup';
//send(compact('op'));

$op = 'identify';
$account_id = 'NEW.AAE';
//$account_id = 'NEW:AAB';
//send(compact('op', 'account_id'));

$op = 'photo';
//send(compact('op', 'account_id'));

$op = 'transact';
$type = 'pay';
$amount = '50';
$goods = 1;
$purpose = 'whatever';
send(compact('op', 'account_id', 'type', 'amount', 'goods', 'purpose'));

$op = 'undo';
$tx_id = 'NEW:ABQZ';
$confirmed = 0;
//send(compact('op', 'account_id', 'tx_id', 'confirmed'));

$confirmed = 1;
//send(compact('op', 'account_id', 'tx_id', 'confirmed'));

$op = 'change';
$what = 'account';
$account_id = 'NEW.AAD';
//send(compact('op', 'account_id', 'what'));



function send($data) {
  $prep = prep($data);
  echo "Request: " . $prep['json'] . "<br><br>\n\n";
  
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, API_URL);
  curl_setopt($ch, CURLOPT_POST, 1); //post the data
  curl_setopt($ch, CURLOPT_POSTFIELDS, $prep);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); //enable RETURN_TRANSFER so curl_exec() returns result of the request
  $result = curl_exec($ch);
  curl_close($ch);
  echo "Response: $result";
  exit();
}

function prep($data) {
  global $my_id, $code;
  return array('json' => json_encode(compact($data['op'] == 'startup' ? '' : 'my_id', 'code') + $data));
}

/*
$host = "localhost";
$path = "/devcore/api";
$data = "{zot}";
$data = urlencode($data);
sendHttpRequest($host, $path, $data);

function sendHttpRequest($host, $path, $query, $port=80){
    header("POST $path HTTP/1.1\r\n" );
    header("Host: $host\r\n" );
    header("Content-type: application/x-www-form-urlencoded\r\n" );
    header("Content-length: " . strlen($query) . "\r\n" );
    header("Connection: close\r\n\r\n" );
    header($query);
}
*/