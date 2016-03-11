<?php
ini_set('display_errors',1);
error_reporting(E_ALL);
define('ISDEMO', true);

define("FORREAL",(substr(PHP_OS,0,3) != 'WIN'));

$METHOD = 'POST'; // normally POST
$GETparmsok = ($METHOD == 'GET'); // don't allow GET unless debugging
$now = formatdate();
define('GRADELTRS', 'EDCBA');
define('VETOGRADE', -2);
define('BLANKGRADE', -1); // means voter did nothing with this option (so don't count it)
define('BPCT2INT', 10); // what to multiply budget pct field by, to get an integer (how many decimal places in the percentage)
define('FUNDED', 'funded');
define('DAY_SECS', 86400);
$thisDOMAIN = 'commongoodvoting.com';

list($db_host,$db_user,$db_password,$db_name)=
  (FORREAL? array("localhost","cgvote_system","Mt~AG2e;129F","cgvote_main"): // host was localhost:3306 db was compract_com
            array("localhost","root","","compract"));

function input($varnm, $format='', $default='', $extra='', $choicearray='') {
// OR input($varnm, 'submit', $label, $action)
// OR input($varnm, 'image', $img, $action)
// OR input($varnm, $format, $defaultselection, $extra, $choicearray) for M and R
//
// name	name of input field
// format	data type (SNCP2DVEBZ) -- caps means required
//			S = string
//      T = textarea
//			A = person's name (can't have weird characters, because it might be used in emails)
//			M = multivalue (<select>)
//      R = single or multivalue (radio buttons)
//			N = number (commas optional)
//			C = currency (dollar sign optional)
//			P = phone number
//			2 = retype previous field
//			D = date
//			V = credit card (mastercard or visa)
//			E = email address
//			B = boolean (yes/no)
//			Z = zipcode (US or Canadian postal code OR "NA")
//			[]= key/value array of SELECT options
// default	default value for field
// extra	extra code for input field (style, onclick, etc.)
// size	field size (UNUSED)
// ermsg	RETURNED initialized to '', if not set
// RETURN	html, if only one arg
// RETURN	(IMPLICIT) html as though only one argument, in &input_name

  global $inputs,$i_formats,$i_values,$i_htmls,$i_fmts,$TOP,$dbfields;
  global $ermsg; if(!isset($ermsg)) $ermsg = '';
  $ret = "input_$varnm"; global $$ret;

  if(!$format) {$$ret = $i_htmls[$varnm]; return $$ret;}
  $FMT = strtoupper($format);

	$gottenvalue = trim(parm($varnm, ($FMT == 'A'))); // gotta do this in any case, so that variable gets set

  $VARNM = strtoupper($varnm);
  $img = $FMT == 'IMAGE' ? " src='$default' border='0' alt='$VARNM'" : '';
  $issubmit = (($FMT == 'SUBMIT') or $img);

	if(!$issubmit) if(isset($dbfields[$varnm]) or isset($_POST[$varnm]) or isset($_GET[$varnm])) $default = htmlize($gottenvalue);

	if(($FMT=='C') or ($FMT=='N')) if($gottenvalue) {
		$default = str_replace(',', '', $default); // ignore commas in input numbers
		global $$varnm; $$varnm = $default;
	}
	$multi = strpos(' MR', $FMT); // true if select or radio
  if($FMT == 'M') $value = makeopts($choicearray, $default);
	if($FMT == 'R') $value = makeradio($varnm, $choicearray, $default, $extra);

  $type = $FMT == 'B' ? " type='checkbox'" :
          ($issubmit ? " type='$format'" : '');

	$valuecode = (($default == '') or $multi) ?
               '' : 
               ($FMT == 'B' ? ($default ? ' CHECKED' : '') : " value='$default'");

//de(compact(bug('name FMT default valuecode gottenvalue format')));
	//$size = strpos(' BM',$FMT) ? '' : ($FMT == 'T' ? " cols='$size'" : " size='$size'");
  if($extra) {
//    if($issubmit) $extra = "onclick=\"this.form.action='$extra.php';\"";
    $extra = " $extra";
  }

  if(!isset($inputs)) {
    $inputs = $i_formats = $i_values = $i_htmls = array();
    $i_fmts = '';
  }
  $inputs[] = $varnm;
  $i_formats[$varnm] = $format;
  $i_values[$varnm]  = $default;

  $i_fmts          .= ($issubmit ? '' : $format); // submit never gets used for SQL
  $i_htmls[$varnm] = 
		$FMT == 'M' ? "<select name='$varnm' id='input_$varnm' $extra>$value</select>" : (
		$FMT == 'R' ? $value : (
		$FMT == 'T' ? "<textarea name='$varnm' id='input_$varnm' $extra>$default</textarea>" : 
		"<input name='$varnm' id='input_$varnm' $valuecode$type$img$extra />"));
  $$ret = $i_htmls[$varnm]; 
//	echo "ret=$ret val=".$$ret."<br>\n";

	return $$ret;

}

function inputlinec($left='&nbsp;', $right='') {
	$left = str_replace(' ', '&nbsp;', $left) . ':';
	return inputline($left, $right);
}

function inputline($left='&nbsp;', $right='') {

  $lefttd = "td colspan='2'"; $lefttdx = 'td';

  if($right) {
    if(strpos($right,'~')) list($right,$rightextra) = split('~',$right); else $rightextra = '';
    $right = "<td valign='top' $rightextra>$right</td>";
    $lefttd = 'th'; $lefttdx = 'th';
  }

  if(strpos($left,'~')) list($left,$leftextra) = split('~',$left); else $leftextra = '';
  $left = "<$lefttd>$left</$lefttdx>";
  return "<tr>$left$right</tr>\n";
}

function inputerrors() {
// RETURN	"do not process form yet"
// ermsg	the message, formatted

  global $inputs,$i_values,$i_formats,$ermsg0,$ermsg;

  $ermsg = '';

  foreach($inputs as $fnm) {
    $v = $i_values[$fnm];
    $fmt = strtoupper($i_formats[$fnm]);
    $required = ($fmt == $i_formats[$fnm]);
    $num = preg_replace('/[^\d\.]/','',$v);
    $digs = str_replace('.','',$num);
    $v1 = substr($digs,0,1);
    if($required and ($v == '')) {
      error($fnm, gt('may not be left blank.'));
      continue;
    }
    if(($fmt == 'C' or $fmt == 'N') and $num != '' and !is_numeric($num)) {
			$nocommas = is_numeric(str_replace(',', '', $num)) ? ' (no commas allowed)' : '';
			error($fnm, gt("must be a number$nocommas."));
		}
    if(($fmt == 'P') and ($digs != '') and (strlen($digs) < 10)) error($fnm, gt('must have at least 10 digits.'));

    if($fmt=='2' and $v != $i_values[$fnm . 2]) error($fnm, gt('must be the same both times.'));
    if($fmt=='D' and strtotime($v) <= 0) error($fnm, gt('must be a valid date.'));
    if($fmt=='V' and ($v1 < '4' or $v1 > '5' or strlen($digs) != 16)) error($fnm, gt('must be a 16-digit MasterCard or Visa.'));
    if($fmt=='E' and ($v != '') and !emailok($v)) error($fnm, gt('must be a valid email address.'));
    if($fmt=='Z' and !preg_match('/^\d{5}(-\d{4})?$/',$v) and !preg_match('/^\pL\d\pL ?\d\pL\d$/',$v) and (strtoupper($v)!=gt('OTHER'))) error($fnm, gt('must be a USA+zipcode+(DDDDD+or+DDDDD-DDDD), Canadian+postal+code+(ADA+DAD), or the word "OTHER".'));
  }

  $ermsg = ermsg($ermsg0);
  return $ermsg;
}

function ermsg($ermsg, $slap=true) {
// slap	say "Please correct..."
	
  global $linkTOP;

  $slap = $slap ? '<BIG> <B>'.gt('Please correct the following error(s) and try again:').'</b></big><BR><BR>' : '';


  return !$ermsg ? '' : "<HR>
<div id='error' style='float:left; padding:10px;'>
<IMG src='$linkTOP/images/alert.gif'>$slap$ermsg</div><div style='clear:left;'><HR></div>\n";
}

function error($fnm, $msg) {
  global $ermsg0;
	$ermsg0 .= '<BIG><B>' . strtoupper($fnm) . "</b></big> $msg<BR>\n";
}

function is_associative($a) { return array_keys($a) != range(0, count($a) - 1); }

function makeopts($opts,$dft='') {
	$assoc = is_associative($opts);
  $ans = '';
  foreach($opts as $val => $opt) {
    if(!$assoc) $val = $opt;
    $selected = $val == $dft ? ' SELECTED' : '';
    $ans .= "<option$selected value='$val'>$opt</option>\n";
  }
  return $ans;
}

function makeradio($varnm, $opts, $dft='', $extra='') {
	if(!is_array($opts)) $opts = array($opts); // allow one button at a time
	$assoc = is_associative($opts);
	$ans = '';
	foreach($opts as $val => $opt) {
    if(!$assoc) $val = $opt;
    $checked = $val == $dft ? ' CHECKED' : '';
		$ans .= "<input type='radio' name='$varnm' id='input_{$varnm}_$val' value='$val'$checked $extra />";
		if($assoc and (count($opts) > 1)) $ans .= "&nbsp;$opt &nbsp; &nbsp; "; // total control if one at a time
  }
  return "$ans";
}		

function gt($s) {return $s;}

function setdbfield($fnm, $value) {
	global $dbfields, $parmset, $parmset0, $METHOD;

	if(!isset($dbfields)) {
		$dbfields = array();
//		$parmset = isset($METHOD) ? "_$METHOD" : '_POST';
//		$parmset0 = $$parmset; // remember original passed values	
	}
	$dbfields[$fnm] = isset($value) ? $value : ''; // isset check necessary for missing (null) database field
//	unset($$parmset[$fnm]); // ignore passed value (is this necessary?)
}

function ignore_dbfields() {
	global $dbfields, $parmset, $parmset0;
	unset($dbfields); // start with submitted data (don't start over with db record)
	if(isset($parmset)) $$parmset = $parmset0; // put submitted values back
}

function parm($s, $zap=false) {
// s		parameter name
// zap	ignore (delete) troublesome characters (\" " < > ,)
// mailHDRStoSELF, HACKMSG, $sysEMAIL
// RETURN	urldecoded parameter value
// $$s	RETURNED IMPLICIT (same as RETURN value)
// $$s0	RETURNED IMPLICIT (same, before urldecoding)
//
//NOTE: use !empty($_GET) to see if anything got passed that way

	global $orgEMAIL, $adminEMAIL, $orgDOMAIN, $dbfields, $GETparmsok;

  $s0 = $s.'0'; 
  global $$s, $$s0; // remember original get var (eg orginal email is $email0)

	if(!isset($GETparmsok)) $GETparmsok = true;
	$$s0 = isset($dbfields[$s]) ? $dbfields[$s] : (isset($_POST[$s])? $_POST[$s] : (($GETparmsok and isset($_GET[$s]))? trim($_GET[$s]) : ''));
	
	$$s = $$s0; // do not urldecode
  if($zap) {
    $$s = str_replace('\\"', '"', $$s);
    $$s = strtr($$s, '",<>', '* []');
  }
  if(!strpos(' notes.sqllines.comments,inviteeemail', $s)) if(strpos($$s, "\n")) {  // don't continue, if we're getting hacked
    $ip = $_SERVER['REMOTE_ADDR'];
    selfmail("hack attempt", "IP=$ip\n$s=".$$s0); 
    echo HACKMSG;
    exit();
  }

  return $$s;
}

function htmlize($s) {
  return htmlspecialchars(stripslashes($s), ENT_QUOTES);
}

function emailok($email,$extras='') {
// extras	subject and other inserted header values to test for spam

if(strpos($extras,"\r") or strpos($extras,"\n")) return false; // eols disallowed in subject and header inserts

  return ereg(
  '^[-!#$%&\'*+\\./0-9=?A-Z^_`a-z{|}~]+'.
  '@'.
  '[-!#$%&\'*+\\/0-9=?A-Z^_`a-z{|}~]+\.'.
  '[-!#$%&\'*+\\./0-9=?A-Z^_`a-z{|}~]+$',
  $email);
}

function doSQL($sql, $nolist=false) {
//  sql	the SQL query text
//	nolist	"don't add to $sql_list"
//  RETURN	the result of the SQL (dies if bad query)

//  IMPLICIT RETURNS: 
//  sql_row_count   number of rows selected or affected
//  sql_record_id   id of inserted or changed record
//  sql_list and sql_record_ids only if they exist as arrays

  global $sql_result, $db, $sql_row_count, $sql_list, $sql_ids, $sql_record_id;

  getdb();

  $sql_result = mysql_query($sql) or die("Bad query: \"$sql\"");

  if(substr($sql,0,7)=='SELECT ') {
    $sql_row_count = mysql_num_rows($sql_result);
  } else {	// data-altering
    $sql_row_count = mysql_affected_rows();
    $sql_record_id = mysql_insert_id();

		if(!$nolist) {
			if(!is_array($sql_list)) $sql_list = $sql_ids = array();
			$sql_list[]=$sql;
			$sql_ids[]=$sql_record_id;
		}
  }
  //echo "sql=$sql sql_row_count = $sql_row_count";
  return $sql_result;
}

function nextrow($sqlID = '') {	
// sqlID    which recordset (defaults to $sql_result)
// RETURN	array containing the next row in $sqlresult
//
// typical use:
//   while($rs = nextrow($sqlID)) {...$a = $rs[fnm];... }

  global $sql_result; 

  if(!$sqlID) $sqlID = $sql_result;
  return mysql_fetch_array($sqlID);	// PHP3 lacks mysql_fetch_assoc
}

function freeSQL(&$sqlID) {
  mysql_free_result($sqlID);
  unset($sqlID);
}

function dbLookup($what, $tnm, $crit='') {
// what     field or expression to lookup
// tnm      table name
// crit     criteria, if any
// RETURN   the value of the field in the first such record found ('' if not found)
//          (would unset be better?)

  if($crit) $crit = 'WHERE ' . $crit;

  $sqlID = doSQL("SELECT $what FROM $tnm $crit");
  $rs = nextrow($sqlID);
  freeSQL($sqlID);
  if(!$rs) return '';
  return $rs[0];
}

function dbCount($tnm, $crit='') {
// tnm       table name
// crit      criteria, if any
// RETURN    number of records in tnm, matching crit

  return dbLookup('Count(*)', $tnm, $crit);
}
function dbMax($what, $tnm, $crit='') {
// what     field or expression to lookup max of
// tnm      table name
// crit     criteria, if any
// RETURN	maximum value in that field

  return dbLookup("Max($what)", $tnm, $crit);
}
function dbMin($what, $tnm, $crit='') {
  return dbLookup("Min($what)", $tnm, $crit);
}

function getdb() {
  global $db_host,$db_user,$db_password,$db_name,$db_conn;

  if($db_conn) return; // already connected
//pr("db_host=$db_host db_user=$db_user db_password=$db_password db_name=$db_name");
  $db_conn = mysql_connect($db_host,$db_user,$db_password) or die("Can't connect to MySQL Server '$db_host'.");
  mysql_select_db($db_name,$db_conn) or die("Can't open database '$db_name'.");
}

function dbRow($fnms, $tnm, $crit) {
// extract(dbRow(...));
	$sqlID = doSQL("SELECT $fnms FROM $tnm WHERE $crit");
	$rs = nextrow($sqlID);
	freeSQL($sqlID);
	return $rs;
}

function simpleA($a) {
// RETURN an array omitting the table name in the field names of the row (eg value of field "tablename_id" becomes $result['id'])
//	if(!$tnm) $a = dbRow($a, $tnm, $crit);
	return array_combine(preg_replace('/^.*?_/', '', array_keys($a)), array_values($a));
}

function formatdate($dt='') {
	$fmt = 'Y-m-d H:i:s';
	return $dt ? date($fmt, $dt) : date($fmt);
}

function vv($s) { // return the value of the variable with name $s
	global $$s;
	return $$s;
}

function hidi($a, $value='') { // create list of hidden input fields
// hidi(compact('varname')); or
// hidi('varname', value);
	global $hiddeninputs;	if(!isset($hiddeninputs)) $hiddeninputs = '';

	if(is_array($a)) {$value = current($a); $a = key($a);}
	$hiddeninputs .= "<input type='hidden' name='$a' value='$value' />\n"; // id='$a' 
}

function de($a, $tag='') {
// de(compact(bug('varname1 varname2...')));
// outputs the value of each varname in the current scope
	if($tag) echo "$tag: ";
	if(is_array($a)) {
		foreach($a as $key => $value) echo "$key='$value' ";
	} else echo $a;
	echo "<br>\n";
}

function bug($s) {return split(' ', $s);}

function showpage($bodymiddle, $extraheaders1='', $onload='', $extraheaders2='') {
	global $TOP;

	if($onload) $onload = " onload=\"$onload\"";

	echo <<<EOF
<html>
<head>
<title>Common Good Democracy</title>
$extraheaders1
<link rel='stylesheet' type='text/css' href='$TOP/inc/styles.css' />
<!--[if IE]><link rel='stylesheet' type='text/css' href='$TOP/inc/ie.css' /><![endif]-->
$extraheaders2
</head>
<body$onload>
<div id='bodywrap'>
<h1><span style='color:darkblue;'>Common Good</span> <span style='color:darkgreen;'>Democracy&trade;</span></h1>
$bodymiddle
<div id='testoutput'></div testoutput>
</div bodywrap>
</body>
</html>
EOF;
}

function letterGrade($value) {
// value
// RETURN    letter grade representation of the value, eg "C+" (0.42)

	if($value <= -5 or $value >= 5.5) die("bad grade: \"$value\"");
  $integer = round($value);
  $fraction = $value - $integer;
  $grade = substr(GRADELTRS, $integer, 1);
  $sign = $fraction >= .167 ? '+' : ($fraction <= -.167 ? '-' : '');
	return $grade . $sign . ' (' . number_format($value, 2) . ')';
}

function formatQuestionDates($startdate, $enddate, $startplusdays=0) {
	$dt1 = strtotime($startdate) + $startplusdays*DAY_SECS; list($dt1y, $dt1m, $dt1d) = split('/', date('Y/F/j', $dt1));
	$dt2 = strtotime($enddate); list($dt2y, $dt2m, $dt2d) = split('/', date('Y/F/j', $dt2));
//echo "$dt1m $dt1d $dt1y $dt2m $dt2d $dt2y";
	if($dt1y == $dt2y) return ($dt1m == $dt2m) ? "$dt1m $dt1d-$dt2d, $dt1y" : "$dt1m $dt1d to $dt2m $dt2d, $dt2y";
	return "$dt1m $dt1d, $dt1y to $dt2m $dt2d, $dt2y";
}

function get_questions($community_id, $crit) {
	// note that if a ballot is kept open beyond the election end time, the following query will disallow it
//	if(!$crit) $crit = 'NOW() BETWEEN question_startdate AND question_enddate';
	$sqlID = doSQL("SELECT question_id, question_type FROM question WHERE question_community=$community_id AND $crit");

	$qtypes = $quids = array(); 
	while($rs = nextrow($sqlID)) {
		extract($rs);
		$quids[] = $question_id;
		$qtypes[$question_id] = $question_type;
	}
	freeSQL($sqlID);

	return array($quids, $qtypes);
}

function jscript($s) {
// s	javascript code, if it contains a semi-colon
//	else javascript source file name in inc/

	global $thisDOMAIN;

  if(!strpos($s, ';')) return "<script type='text/javascript' src='http://$thisDOMAIN/inc/$s'></script>\n";
  return "<script type='text/javascript'><!--
$s
//--></script>
";
}

function qid($qid) {
	return substr($qid + 10000, 1);
}

function get_community($ermsg, $fromhere=true) {
	global $thisDOMAIN;
	if(!isset($_SERVER['HTTP_REFERER'])) die('You cannot call this program directly.'); // probably a hacker
	$caller = $_SERVER['HTTP_REFERER'];
	$community_id = parm('community_id');

	if($fromhere) {
		if(strpos($caller, "http://$thisDOMAIN") === false) die('Bad call'); // probably a hacker
	} elseif(!dbLookup('community_id', 'community', "community_id='$community_id' AND community_site='$caller'")) die($ermsg);

	return $community_id;
}

// RETURN $community_id, $voter_id
function get_cv($gotcv) { 
  return [1, 1]; // debug
	global $now;

	if($gotcv) { // processing a ballot (submitted from this site)
		$community_id = get_community('Bad election ID.');
		$voter_id = parm('voter_id');
	} else { // getting request from an authorized site - expect community and voter
//		if(!$community_id) $community_id = $_GET['community']; // TESTING
		$community_id = get_community('Bad community ID.', false);
		$voter_nativeid = parm('voter_id');
//		if(!$voter) $voter = $_GET['voter']; // TESTING
		$voter_id = dbLookup('voter_id', 'voter', "voter_nativeid='$voter_nativeid' AND voter_community='$community_id'");
		if(!$voter_id) {
			doSQL("INSERT INTO voter (voter_nativeid, voter_community, voter_created) VALUES('$voter', '$community_id', '$now')");
			$voter_id = $sql_record_id; // let anyone vote; hackers will be weeded out later
		}
		if(!$voter_id) die('Bad voter ID.');
	}
	return array($community_id, $voter_id);
}

function goform($community_id, $voter_id, $action='NONE', $button='') {
	global $METHOD, $hiddeninputs;

	hidi(compact('community_id'));
	hidi(compact('voter_id'));
	hidi('question_id', ''); // this and dowhat and form action to be filled in by the showpage() javscript function
	hidi('dowhat', '');

	if($button) $button = "<input type='submit' value='$button' />";

	return <<<EOF
<form id='goform' method='$METHOD' action='$action'>
$hiddeninputs$button
</form>
EOF;
}

function cttydate($timestamp, $timezone='', $community_id='') { // return the local date/time for the community
	if(!$timezone) $timezone = dbLookup('community_timezone', 'community', "community_id='$community_id'");
	$timezone = ($timezone >= 0 ? '+' : '') . ($timezone + 0); // otherwise the minus is already there
//echo date('Y-m-d H:i:s', $timestamp) . " $timezone hours<br>\n";
	return strtotime(date('Y-m-d H:i:s', $timestamp) . " $timezone hours");
}
