#!/usr/local/bin/php -q 
<?php
$s = stream_get_contents(fopen('php://stdin', 'r'));
preg_match('/Content-Transfer-Encoding: base64\s*([^=]*)=/ms', $s, $matches);
$msg = base64_decode($matches[1]);

mail('wspademan@gmail.com', 'test', $msg);