<?php
// localhost/cgMembers/rcredits/test1099.php?t1 tests forms1099b-FY2015(1).bin1 (?1 -> .bin)
// The "t" means it's a test file

$num = $_SERVER['QUERY_STRING'];
if (substr($num, 0, 1) == 't') {$num = substr($num, 1); $test = 1;}
if ($num) $num = " ($num)";
$year = date('Y') - 1;
$flnm = "c:\\Documents\\Downloads\\forms1099b-FY$year$num.bin" . @$test;
$s = file_get_contents($flnm);
$s = explode("\n", $s);

checkTyp($rec = getRec($s), 'T');
echo $flnm . ":<br>\nThis is " . (substr($rec, 27, 1) == 'T' ? 'a <b style="color:orange;">TEST</b> file.' : 'NOT a test file.') . "<br>\n";
checkTyp(getRec($s), 'A');

$tot = 0;

while(typ($rec = getRec($s)) == 'B') {
  $amt = amt($rec);
  $tot += $amt;
  //echo "amt=$amt tot=$tot<br>\n";
}

checkTyp($rec , 'C');
$amt = amt($rec, FALSE);
if ($amt != $tot) err("bad C total amt=$amt tot=$tot");

checkTyp($rec = getRec($s), 'K');
$amt = amt($rec, FALSE);
if ($amt != $tot) err("bad K total amt=$amt tot=$tot");

echo 'File looks <b style="color:lightgreen;">GOOD</b>! Total = $' . number_format($tot/100, 2);

function checkTyp($rec , $typ) {if (typ($rec) != $typ) err('missing ' . $typ);}

function typ($rec) {return substr($rec, 0, 1);}

function err($err) {echo "<b style=\"color:red;\">ERROR</b>: $err<br>\n"; exit();}

function getRec($s) {
  global $reci;
  $reci = isset($reci) ? $reci + 1 : 1;
  $rec = $s[$reci-1];
  if (substr($rec, 500, 8) != $reci) err("bad seq num at $reci");
  return $rec;
}

function amt($rec, $B = TRUE) {return substr($rec, $B ? 126 : 123, $B ? 12 : 18);}