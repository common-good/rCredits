<?php
$s = '';
for ($i=0; $i < 16; $i++) {
$s .= chr(rand(1,255));
}
echo $s;
echo "<br>";
echo $s = base64_encode($s);
echo "<br>";
echo strlen($s)  ;
echo "<br>";
echo strlen(base64_decode(substr($s, 0, 22)))  ;