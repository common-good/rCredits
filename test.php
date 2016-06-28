<?php
$text = <<<EOF
/**
 * security code is (ARG)
 *
 * in: TEST ParseQRCode WeScanAValidPersonalCard
 *     TEST ParseQRCode WeScanAValidCompanyCard
 */
this.securityCodeIs = function(\$arg1) {


/**
 * relations: (ARG)
 *
 * in: 
 */
function relations(\$relations)

EOF;

  $pattern = ''
  . '^/\\*\\*$\\s'
  . '^ \\* ([^\*]*?)$\\s'
  . '^ \\*$\\s'
  . '^ \\* in: ((.*?)$\\s'
  . '^ \\*/$\\s'
  . '^function (.*?)\()';  
  
  $pattern = ''
  . '^/\\*\\*\\s?$\\s^'
;  // yes

  $pattern = ''
  . '^/\\*\\*\\s?$\\s'
  . '^ \\* ([^\*]*?)\\s?$\\s'
  . '^ \\*\\s?$\\s'
  . '^ \\* in: ((.*?)\\s?$\\s'
  . '^ \\*/\\s?$\\s'
  . '^this\.(.*?) )';  
  
preg_match_all("~$pattern~ms", $text, $matches, PREG_SET_ORDER);
if (!$matches) die('nope');
exit('yes');