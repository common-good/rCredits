<?php

/**
 * @file
 * Report (to an admin) bounced messages emailed from the server.
 * Copy this to the web root.
 */
 
$pw = 'i{RyS)hf]c@6';
$domain = $_SERVER['HTTP_HOST'];
$bounceBox = "bounce@$domain";
$adminEmail = "info@$domain"; // forward this from cPanel, if necessary

if (!$f = imap_open('{localhost/novalidate-cert}INBOX', $bounceBox, $pw)) die('Cannot open mailbox.');

for ($i = 1; $i <= imap_num_msg($f); $i++) {
  $header = imap_header($f, $i);
  $headers = print_r($header, 1);
  $body = imap_body($f, $i);
  preg_match('/[<\\s](\\S+@\\S+)[>\\s]/', $body, $m);
  $email = @$m[1];
  $permanently = stripos($body, 'permanent') ? ' -- *permanently*' : '';
  $msg = <<<EOF
We sent an email from $domain to this email address, but it bounced$permanently:

  $email
  
---------------------------------

Here's the whole bounce message (followed by headers array):

$body

HEADERS: $headers
EOF;

  mail($adminEmail, 'BOUNCE', $msg, "From:$bounceBox");
  imap_delete($f, $i);
}

imap_expunge($f);
imap_close($f);
