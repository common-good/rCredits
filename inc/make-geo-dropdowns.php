<?php
$js = 'var cs = new Array();'; // countries
$js .= "\n\nvar ss = new Array();"; // states
$cq = \db_query('SELECT * FROM r_countries');
while ($crow = $cq->fetchAssoc()) {
  $cid = $crow['id']; $cname = $crow['name'];
  $js .= "\n\ncs[$cid] = '$cname';";
  $js .= "\n\nss[$cid] = new Array();";
  $sq = \db_query('SELECT * FROM r_states WHERE country_id=:cid', array(':cid' => $cid));
  while ($srow = $sq->fetchAssoc()) {
    $sid = $srow['id']; $sname = $srow['name'];
    $js .= "\nss[$cid][$sid] = '$sname';";
  }
}

$js .= <<<EOF
\n
function print_country(country_id, dft_country, dft_state){
  var options = document.getElementById(country_id);
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in cs) {
    options.options[i] = new Option(cs[x],cs[x]);
    if (dft_country == cs[x]) options.selectedIndex = i;
    i++;
  }
  if (dft_country == "") {
    options.selectedIndex = 1228; // 1228 is US
    dft_state = 1020; // 1020 is MA
  }
  print_state('edit-state', options.selectedIndex, dft_state);
}

function print_state(state_id, ci, dft_state){
  var options = document.getElementById(state_id);
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in ss) {
    options.options[i] = new Option(ss[ci][x],ss[ci][x]);
    if (dft_state == ss[x]) options.selectedIndex = i;
    i++;
  }
}
EOF;

echo $js;
file_put_contents(__DIR__ . '/zot.js', $js);