<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('OFX_TX_FLDS', 'trnType dtPosted trnAmt fitId name memo');

/**
 * Upload a bank statement to reconcile the community fund.
 */
function formReconcileCttyFund($form, &$sta) {
  $title = t('Reconcile Community Fund');
	$subtext = t('Upload a Community Fund bank statement file (in OFX format).');
  $file = fileFld();
  $submit = t('Upload and Reconcile');

  return cgform(compact(ray('title subtext file submit')));
}

function formReconcileCttyFund_validate($form, &$sta) {
  if (!$filename = w\uploadFile($err)) return w\say($err, 'file');
	$s = file_get_contents($filename);

	if (!$acct = ofxData('bankId acctId', $s)) return w\say(t('That is not a valid OFX file.'));
	if (!$dts = ofxData('dtStart dtEnd', $s)) return w\say(t('That is not a valid OFX file.'));
  if (!$txs = ofxData(OFX_TX_FLDS, $s, TRUE)) return w\say(t('No transactions found.'));
  $sta['input'] += $acct + $dts + ['txs' => serialize($txs)];
}

function formReconcileCttyFund_submit($form, &$sta) {
  global $mya;

	extract(just('bankId acctId dtStart dtEnd txs', $sta['input']));
	$txKs = ray(OFX_TX_FLDS);
	$sql = 'SELECT IF(amount<0,-deposit,deposit) AS dt,SUM(amount) AS amt FROM r_usd where amount<>0 and deposit<>0 GROUP BY deposit,amount<0';
	$res = db\q($sql)->fetchAllKeyed();
	debug($res);
	
	foreach ($txs = unserialize($txs) as $tx) {
		extract(array_combine($txKs, $tx)); 
		$btxid = bankTxId($fitId);
		debug(compact($txKs)); break;
	}
	
}

/**
 * Extricate the specified parameters from the given OFX file contents.
 * @param string $ks: list of parameter names (case-insensitive)
 * @param string $s: contents of an OFX file
 * @return matches found
 */
function ofxData($ks, $s, $multi = FALSE) {
	foreach ($ks = ray($ks) as $k) $pat[] = "<$k>([^\\r\\n]+)";
	$pat = join('\s*', $pat);
	if ($multi) {
	  if (!preg_match_all("/$pat/ism", $s, $m, PREG_SET_ORDER)) return FALSE;
		foreach ($m as $i => $one) $m[$i] = array_slice($m[$i], 1);
		return $m;
	} else return preg_match("/$pat/ism", $s, $m) ? array_combine($ks, array_slice($m, 1)) : FALSE;
}

function bankTxId($btxid) {
	list ($txid, $dt, $acct) = [substr($btxid, isDEV ? -1 : -7), substr($btxid, -15, 8), substr($btxid, 0, strlen($btxid) - 7 - 8)];
	return 0 + (substr($acct, -5, isDEV ? 4 : 5) . floor(strtotime($dt) / DAY_SECS) . (isDEV ? substr($txid, -1) : $txid)); // (intval fails here)
}