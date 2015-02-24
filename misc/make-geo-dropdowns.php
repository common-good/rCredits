<?php
//include 'sites\all\modules\rcredits\inc\make-geo-dropdowns.php';

function geofix($s) {return str_replace("'", "\\'", $s);}

$js = 'var cs = new Array();'; // countries
$js .= "\n\nvar ss = new Array();"; // states
$js .= "\n\nvar STs = new Array();"; // states
$STs = '';
$cq = \db_query('SELECT * FROM r_countries');
while ($crow = $cq->fetchAssoc()) {
  $cid = $crow['id']; $cname = geofix($crow['name']);
  $js .= "\n\ncs[$cid] = '$cname';";
  $js .= "\n\nss[$cid] = new Array();";
  $sq = \db_query('SELECT * FROM r_states WHERE country_id=:cid', array(':cid' => $cid));
  while ($srow = $sq->fetchAssoc()) {
    $sid = $srow['id']; $sname = geofix($srow['name']);
    $js .= "\nss[$cid][$sid] = '$sname';";
    if ($cname == 'United States') $STs .= "\nSTs[$sid] = '" . $srow['abbreviation'] . "';";
  }
}

$js .= <<<EOF
\n
$STs

function print_country(dft_country, dft_state){
  if (dft_country == "") {
    dft_country = 1228; // 1228 is US
    dft_state = 1020; // 1020 is MA
  }
  var options = document.getElementById('edit-country');
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in cs) {
    options.options[i] = new Option(cs[x], x);
    if (dft_country == x) options.selectedIndex = i;
    i++;
  }
  print_state(options[options.selectedIndex].value, dft_state);
}

function print_state(ci, dft_state){
  var options = document.getElementById('edit-state');
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in ss[ci]) {
    options.options[i] = new Option(ss[ci][x], x);
    if (dft_state == x) options.selectedIndex = i;
    i++;
  }
}

EOF;

echo $js;
file_put_contents(__DIR__ . '/zot.js', $js);