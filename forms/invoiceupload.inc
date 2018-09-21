<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Upload invoices spreadsheet to create invoices.
 * The validate and submit functions are also called by PaymentsUpload and Payroll.
 */
function formInvoiceUpload($form, &$sta) {
  global $mya;

  $title = t('Upload Invoices');
  $selling = explode("\n", $mya->selling);
  $instructions = csvUploadInstructions(t('invoice'), $selling[0]);
  $subtext = "<ul id=\"instructions\">$instructions</ul>";
  $file = fileFld();
  $submit = t('Upload and Invoice');

  return cgform(compact(ray('title subtext file submit')));
}

function formInvoiceUpload_validate($form, &$sta) {
  if (!$filename = w\uploadFile($err)) return w\say($err, 'file');
  $sta['rows'] = w\uploadRows($filename);
}

function formInvoiceUpload_submit($form, &$sta, $args = NULL) {
  global $mya, $iuErrs;

  $fields = 'inv dftDesc did map onSuccess';
  $selling = explode("\n", $mya->selling);
  u\setDft($args, ray($fields, TRUE, $selling[0], t('invoiced'), [], NULL));
  extract(just($fields, $args));
  
  $total = 0;

  if ($rows = @$sta['rows']) foreach ($rows as $i => $row) {
    if ($i == 0 and !strpos(@$row[0], '@')) {say(t('The header row (line 1) will be ignored.')); continue;}

    $e = ray('did line row', $did, $i + 1, join(' | ', $row)); // params for error message(s)
    if (count($row) < 3) return say('csv error', $e + ray('msg', t('Incomplete row')), 'file');
    @list ($account, $email, $amt, $desc) = $row; // desc is optional
    if (!$account or u\starts(strtoupper($account), t('TOTAL'))) break; // should never happen unless user has a total line
    if ($err = u\badAmount($amt, '>=0')) {iuErr($err, $e); continue;}
    if (!$amt) {iuErr(t('The amount for this account is zero so it will not be %did', $e), $e); continue;}
	  if (!$account or !$a = r\acct($account)) {iuErr(t('Bad account ID'), $e); continue;} // not 'bad account'
    if (!$a->ok) {iuErr(t('This account is not yet activated'), $e); continue;}
    $a->setBit(B_CONFIRMED); // count this as equivalent to an invitation to the person
    if (!$email = trim(strtolower($email)) or $a->email != $email) { // not the right email
      if ($a->phone != u\fmtPhone($email, '+n')) { // not the right phone number
        if (strtolower(r\acct($mya->id, $a->id)->relCode) != $email) {iuErr(t('Wrong email, phone, or account ID'), $e); continue;} // not the right customer ID
      }
    }
    $txs[] = [$a->id, $email, $amt, $desc ? strtr($desc, $map) : $dftDesc]; // someday we wlll allow paying and invoicing non-members by email and name
    $total += $amt;
  }      

	if (@$iuErrs) return say(join('<br>', $iuErrs), 'file');
	
  if (!@$txs) return say('nothing to do', 'file');
  
  if (!$inv) {
    if ($shortfall = $mya->shortfall(FOR_GOODS, $total)) return say('short to', ray('short', u\fmtAmt($shortfall)), 'file');
  }  
  
  foreach ($txs as $one) {
    list ($uid, $email, $amt, $desc) = $one;
    $a2 = r\acct($uid);
    list ($msg, $subs) = $inv 
    ? $mya->invoice($a2, $amt, $desc)
    : $mya->pay($a2, $amt, $desc);
    if (!@$subs['success']) $err = TRUE;
    if ($inv) {
      $msg = str_replace('|balance unchanged', '', $msg); // otherwise this gets tiresome, repeated over and over
      if (!$err and !$a2->ok) $msg .= '|' . t('<b class="loud">NOTE: </b>%otherName will need to finish opening a %PROJECT account.');
    }
    say($msg, $subs);
  }
  
  if (!@$err) {
    if (is_callable($onSuccess)) $onSuccess($total);
    return w\go('/history', t('txs upload done', 'did total count', $did, u\fmtAmt($total), count($txs)));
  }
}

function csvUploadInstructions($do, $dftDesc) {
  return t(<<<EOF
  <li><b>Create a 4-column spreadsheet</b> with or without headers, with this information in each row, in this order (any additional columns will be ignored):
    <ol>
    <li>The %PROJECT account ID to %do.</li>
    <li>The account's email address or phone number, or your identifier for them.</li>
    <li>The amount to %do that account.</li>
    <li>A short description of what the transaction is for (like "groceries" or "inv #7"). If you omit this, the description will be "%dftDesc".</li>
    </ol>
  </li>
  <li><b>Save</b> the spreadsheet as a CSV file.</li>
  <li><b>Upload</b> the CSV file here, to automatically %do that amount of %RCREDITS to each listed account.</li>
EOF
  , compact(ray('do dftDesc')));
}

function iuErr($msg, $e) {global $iuErrs;	$iuErrs[] = t('csv error', $e + compact('msg'));}
