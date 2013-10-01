#!/usr/local/bin/php -q 
<?php

define('SYS_EMAIL', 'info@rc4.me');

$s = stream_get_contents(fopen('php://stdin', 'r'));
$link = $response = $coding = '';
list ($zot, $who, $subject, $text) = parseHeader($s);

if (preg_match('/Content-Transfer-Encoding: base64(.*?)----boundary/ms', @$s, $matches)) {
  $text = base64_decode($matches[1]);
  if ($parsed = parseHeader($text) and $parsed[0]) list ($zot, $who, $subject, $text) = $parsed;
  $coding .= ', base64';
}

if ($text) {
  if (strpos($text, '=3DW') and $text2 = quoted_printable_decode($text)) {
    $text = $text2;
    $coding .= ', quoted_printable';
  }
  if (preg_match('~(https://www.dwolla.com/.*?)["<\s]~ms', $text, $matches)) {
    $coding .= ', linky';
    $link = $matches[1];
    if (strpos($subject, 'TEST') !== FALSE) $link = str_replace('/www.', '/uat.', $link);
    if (stripos($subject . $text, 'Verify your e') !== FALSE and strpos($link, '/register/verify?')) $response = file_get_contents($link);
  }
}

$s = <<<EOF
link: $link<br>
coding: $coding<br>
subject: $subject<br>
to: $who<br>
decoded: $text<br>
original: $s<br>
response: $response
EOF;
// response here is temporary?

//echo $s;
htmlmail('wspademan@gmail.com', "rC4: $subject (to $who)", $s);

/**
 * Return the subject, recipient, and subsequent text
 * @param string $msg: the email body
 * @return [ignorethis, who, subject, text]
 */
function parseHeader($msg) {
  if (preg_match('~$\\s*To: (.*?>).*Subject: (.*?)$(.*)~ms', $msg, $matches)) return $matches;
  if (preg_match('~$\\s*Subject: (.*?)$.*To: (.*?)$(.*)~ms', $msg, $matches)) return array('', $matches[2], $matches[1], $matches[3]);
  return array('', '', '', '');
}

function htmlmail($to, $subject, $htmlmsg) {
  require_once('class.html2text.php');

  $boundary = 'Msg_Boundary--';
  $htmltype = 'text/html; charset="iso-8859-1"';
  $plaintype = str_replace('html', 'plain', $htmltype);

  if(stripos($htmlmsg, '<html') === FALSE) $htmlmsg = "<html>$htmlmsg</html>";
  $h2t = new html2text($htmlmsg);
  $plain = $h2t->get_text(); 

  while(stripos($plain, $boundary) + stripos($htmlmsg, $boundary) > 0) $boundary .= rand(); // prevent malicious hacking

  $msg = <<<EOF
If you see this message, you may need a newer email program.
--$boundary
Content-Type: $plaintype
Content-Transfer-Encoding: 7bit
Content-Disposition: inline

$plain
--$boundary
Mime-Version: 1.0
Content-Type: $htmltype
Content-Transfer-Encoding: 7bit
Content-Disposition: inline

$htmlmsg
--$boundary--
EOF;

  $hdrs = array(
    'From' => 'rC4 <' . SYS_EMAIL . '>',
    'Return-Path' => SYS_EMAIL,
    'Errors-To' => SYS_EMAIL,
    'Content-Type' => "multipart/alternative; boundary=\"$boundary\"",
  );
	
  $smhdrs = '';
  foreach($hdrs as $key => $value) $smhdrs .= "$key: $value\n";
  $smhdrs = substr($smhdrs, 0, strlen($smhdrs)-strlen("\n")); // omit final EOL

  mail($to, $subject, $msg, $smhdrs, '-f' . SYS_EMAIL);
}
