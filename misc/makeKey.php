<?php
/**
 * @file
 * Create a private key (for GPG asymmetric encryption).
 */

$vs = explode(' ', 'configPath alg bits flnm maxlen submit');

foreach ($vs as $k) $$k = @$_POST[$k];

if (@$submit) {
	
	$config = array(
		"config" => $configPath,
		"digest_alg" => $alg, // eg sha256
		"private_key_bits" => 0 + $bits, // eg 4096
		"private_key_type" => OPENSSL_KEYTYPE_RSA,
	);
	
  $res = openssl_pkey_new($config);
/**/  if (!$res) die('FAIL new');
  openssl_pkey_export($res, $privKey, NULL, $config);

/**/	echo $privKey . "<br><br>len=" . strlen($privKey);
//	file_put_contents("$flnm-private.key", $privKey);
///	print_r(openssl_pkey_get_details($res));
//	$pubKey = openssl_pkey_get_details($res)['key'];
//	file_put_contents("$flnm-public.key", $pubKey);
	exit();
}


/**/ echo <<<EOF
<form method="post"><table>
  <tr><th>Algorithm: </th><td><input name="configPath" value="C:/xampp/apache/bin/openssl.cnf"></td></tr>
  <tr><th>Algorithm: </th><td><input name="alg" value="sha512"></td></tr>
  <tr><th>Bits: </th><td><input name="bits" value="2048"></td></tr>
  <!--tr><th>Filename: </th><td><input name="flnm"></td></tr-->
  <tr><th></th><td><input name="submit" type="submit" value="submit"></td></tr>
</table></form>
EOF;
	