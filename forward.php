#!/usr/local/bin/php -q 
<?php
$s = stream_get_contents(fopen('php://stdin', 'r'));
preg_match('/Content-Transfer-Encoding: base64\s*([^=]*)=/ms', @$s, $matches);
$msg = @$matches[1] ? base64_decode($matches[1]) : @$s;

mail('orders@compract.com', 'indirect test', 'this is a test: ' . @$s);
mail('wspademan@gmail.com', 'test', $msg);