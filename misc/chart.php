<?php

/**
 * @file
 * Display a chart (normally in an iframe), using data from a Common Good Regional Server.
 * for example at http://localhost/cgMembers/rcredits/chart.php?ctty=0&chart=growth
 * NOTE: this script resides on cg4.us, not on the regional server.
 */

$title = 'Charts | Common Good Western Massachusetts';
///   echo "<h1>$title</h1>";
//exit();

//$site = 'https://new.commongood.earth';
//$site = 'https://ws.rcredits.org';
//$site = 'http://localhost/cgMembers';

foreach (ray('ctty chart site selectable') as $k) $$k = urlencode(@$_GET[$k]); // encode to sanitize output
if (!$site0 = @$site) {
  $site = 'https://new.commongood.earth';
} else if (@$site == 'dev') {
  $site = 'http://localhost/cgMembers';
} else $site = 'https://' . $site;

$data = file_get_contents("$site/community/chart-data/ctty=$ctty&chart=$chart");
/**/ if ($site0 != 'dev' and strlen($data) < 200) die($data);
$version = time();

if ($selectable) {
  $cttys = file_get_contents("$site/community/list");
  $cttys = (array) json_decode($cttys);
  $cttys = opts($cttys, $ctty);

  $charts = [
    'funds' => 'Dollar Pool',
    'growth' => 'Growth',
    'banking' => 'Exchanges for Dollars',
    'volume' => 'Transaction Volume',
    'velocity' => 'Circulation Velocity',
  ];

  $help = str_replace(' ', '-', strtolower($charts[$chart]));
  $help = "<div id=\"help-line\"><a href=\"$site/help/$help/qid=$ctty\">More information</a></div>";

  $charts = opts($charts, $chart);

  $controls = <<<X
    <div>
    <select id="ctty" class="form-control">$cttys</select><br>
    <select id="chart" class="form-control">$charts</select>
    </div>
X;
} else $controls = $help = '';

/**/ echo <<<EOF
 
<!DOCTYPE html>
<html lang="en" dir="ltr">

<head profile="http://www.w3.org/1999/xhtml/vocab">
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <!-- The above 3 meta tags *must* come first in the head -->

  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

  <base target="_parent">
  <link rel="apple-touch-icon" sizes="180x180" href="$site/rcredits/images/favicons/apple-touch-icon.png">
  <link rel="icon" type="image/png" href="$site/rcredits/images/favicons/favicon-32x32.png" sizes="32x32">
  <link rel="icon" type="image/png" href="$site/rcredits/images/favicons/favicon-16x16.png" sizes="16x16">
  <link rel="manifest" href="$site/rcredits/images/favicons/manifest.json">
  <link rel="mask-icon" href="$site/rcredits/images/favicons/safari-pinned-tab.svg" color="#5bbad5">
  <link rel="shortcut icon" href="$site/rcredits/images/favicons/favicon.ico">
  <meta name="msapplication-config" content="$site/rcredits/images/favicons/browserconfig.xml">
  <meta name="theme-color" content="#ffffff">
  <meta name="MobileOptimized" content="width" />
  <meta name="HandheldFriendly" content="true" />
  <meta name="apple-mobile-web-app-capable" content="no"><!-- (not yet) -->
  <meta http-equiv="cleartype" content="on" />
  <title>$title</title>
  <meta name="description" content="">
  <meta name="author" content="William Spademan -- for Society to Benefit Everyone, Inc.">
  <link rel="stylesheet" href="$site/rcredits/css/x/bootstrap.min.css?1522254546" />
  
<style>
  .form-control {width:auto; margin-left:30px;}
  #chart {font-size:200%; font-weight:bold; height:200%;}
  #chart, #chart .selected {color:darkgreen;}
  .onechart {margin-left:-50px;}
  #help-line {margin-left:190px;}
</style>
</head>

<body>
$controls
<div id="{$chart}Chart" class="onechart"></div>
<div id="chart-data"><!--$data--></div>
$help

<script src="$site/rcredits/js/x/jquery-3.3.1.min.js"></script>
<script id="script-goo-jsapi" src="https://www.google.com/jsapi"></script>
<script id="script-parse-query" src="$site/rcredits/js/parse-query.js?v=$version"></script>
<script id="script-charts" src="$site/rcredits/js/charts.js?selectable=$selectable&ctty=$ctty&chart=$chart&site=$site0&v=$version"></script>

</body>
</html>
EOF;

exit();

function ray($a) {return explode(' ', $a);}

function opts($ray, $dft = '') {
  $res = '';
  foreach ($ray as $k => $v) {
    $sel = $k == $dft ? ' SELECTED' : '';
    $res .= "<option value=\"$k\"$sel>$v</option>\n";
  }
  return $res;
}