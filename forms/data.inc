<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Present a choice of data queries
 */
function formData($form, &$sta, $args = '', $isco = FALSE) {
  $isco = ($isco == 'co');
  require_once queryFile($isco);
  extract(just('qName', $args));
  
  if ($qName0 = urldecode(@$qName) and $sql = @$queries[$qName0] and $qName = w\queryPermitted($qName0, $sql, $subtext)) {
    $title = $qName;
    $subtext = $subtext ?: '&nbsp;';
    $report = item(w\showRecords(db\q($sql)->fetchAll()));
    $form = compact(ray('title subtext report'));
  } else {
    $title = $isco ? t('Company Reports') : t('Community and Money Data');
    $subtext = item(t('Click the query of your choice. If you need information not provided here, <%a1>request it</a>!', ['_a1' => u\emailTag(CGF_EMAIL, t('data request'))]), '&nbsp;');
    $isco = hidFld($isco ?: 0);
    $form = compact(ray('title subtext isco'));
    
    foreach ($queries as $k0 => $v) {
      if ($k = w\queryPermitted($k0)) $form[$k0] = submit($k, 'default', 'xs');
    }
  }
  return cgform($form);
}

function formData_submit($form, &$sta) {
  extract(just('isco', $sta['input']));
  require_once queryFile($isco);
  $op = w\opText($sta);
  $d = substr(op($sta), 0, 1);
  if (strpos('0123456789', $d) !== FALSE) $op = $d . $op; // leading digit, if any
  $url = $isco ? '/company-reports' : '/community/data';
  w\go("$url/qName=" . urlencode($op));
}

function queryFile($isco) {return R_ROOT . ($isco ? '/rweb/queries-co.inc' : '/rweb/queries.inc');}