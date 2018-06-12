<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Pay or deny an invoice, possibly by clicking a link without signing in (from rDo).
 * If no user is signed in, $args must include the do code (to prevent mischief)
 * @param mixed $args: array of paramaters (from rDo) or invoice number (if signed in):
 *   int nvid: invoice number
 *   bool toMe: is the invoice for me to pay (or deny)? (FALSE if it is FROM me)
 *   string ret: URL to return to
 *   bool webpay: <this is confirmation after clicking a "Pay with Common Good" button>
 */
function formHandleInvoice($form, &$sta, $args = '') {
  global $mya;
  extract(just('nvid ret code toMe webpay', $args));
  $ret = @$ret ? @u\deurlify($ret) : 'empty/1';
  
  if ($err = r\badUnpaidInv(@$nvid, $inv)) return w\go($ret, $err, 'ERR');

  extract(just('nvid amount payer payee purpose created data', $inv));
  if (@$code) {
    if (!$info = r\doCode($code)) return hack(t('fake doCode in handleInvoice'));
    if (@$mya and $payer != $mya->id) w\signout(FALSE); // make sure we're signed out of any possibly different acct
  } elseif (!$mya) {
    return hack(t('attempt to handle invoice not signed in'));
  } else if (!in($mya->id, [$payer, $payee]) or ($mya->id == $payer xor $toMe)) return hack(t('bad person in invoice payment'));

  $subs = ray('amount what', u\fmtAmt($amount), $purpose);
  
  if (@$toMe) {
    $a = r\acct($payer);
    $title = t('Confirm Payment');
    $subs += ray('who', r\acct($payee)->fullName);

    if ($a->hasBank) {
      $_warning = t('<p><b>IMPORTANT:</b> To avoid insufficient funds charges, be sure there are enough funds in your bank account to cover any automatic transfer that may result from approving payment of this invoice (and future invoices, if you check the "Auto" box).</p>');
      if (!@$webpay) $always = boxFld('always', t('Auto:'), t('Pay all invoices from %who automatically.', $subs), 0);
    } elseif ($a->balance <= 0) {
      $_warning = t('You cannot approve this invoice until you have money in your account. Sign in at %CG_DOMAIN and click "Bank" on the menu to transfer funds from (or to) your bank account.');
      $disabled = w\disabled();
    } else $_warning = '';

    $subtext = t('<p>Pay %amount to %who for %what.</p>%warning', $subs + compact('_warning'));
    $link = '&nbsp; &nbsp;' . w\lnk('', t('dispute'), ray('id', 'dispute-it'));
    $pay = submit(t('Pay Now'), '', '', w\suffix($link) + (@$disabled ?: []));
    
    $whyNot = textFld(t('Reason for dispute:'));
    $deny = submit(t('Dispute'));
    $denySet = fieldSet('denySet', compact('whyNot', 'deny'), BARE);
    $toMe = hidFld(TRUE);
  } else { // confirming delete by originator
    $title = t('Delete?');
    $subs += ray('who', r\acct($payer)->fullName);
    $subtext = t('Delete invoice charging %who %amount for %what?', $subs);
    $delete = submit(t('Delete'));
    $cancel = submit(t('Cancel'));
    $delSet = fieldSet('delSet', compact('delete', 'cancel'), BARE); // separate it from subtext
  }
  
  $hid = w\hidFlds(compact('ret') + $inv);
  $form['#validate'] = array('CG\\Web\\formHandleInvoice_validate');
  $form['#submit'] = array('CG\\Web\\formHandleInvoice_submit');
  w\jsx('dispute');
  
  return cgform(compact(ray('title subtext always pay dispute denySet delSet toMe hid')) + $form);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(just('whyNot toMe', $sta['input']));
  $deny = (op($sta) == 'deny');
  if (@$toMe and $deny and !$whyNot) return say('say why not', 'whyNot');
}

function formHandleInvoice_submit($form, &$sta) {
  $hidFlds = 'ret nvid amount payer payee purpose created data';
  extract(just('hid whyNot toMe always ' . $hidFlds, $sta['input'])); // hidFlds here is for testing
  extract(just($hidFlds, w\hidFlds($hid)));

  list ($fromA, $toA) = [r\acct($payer), r\acct($payee)];
  $payeeName = $toA->fullName;
  $subs = ray('amount payerName payeeName created purpose reason', u\fmtAmt($amount), $fromA->fullName, $payeeName, u\fmtDate($created), $purpose, @$whyNot);

  $op = op($sta);
  $ok = TRUE; // messages default to status, rather than error

  if ($op == 'pay' and !@$whyNot) {
    $short = round($amount - $fromA->avail(), 2);
    if ($fromA->ok and $short <= 0) {
      $ok = be\payInvoice($nvid, $msg);
    } else {
      db\update('r_invoices', ray('nvid status', $nvid, TX_APPROVED), 'nvid');
      if ($fromA->ok) {
        $msg = $fromA->hasBank ? 'expect a transfer' : 'when funded';
        $msg = t('short invoice', ray('short payeeName', u\fmtAmt($short), $payeeName)) . "|$msg";
      } else $msg = t('finish signup|when funded');
    }
    
    if (@$always) {
      r\acct($payee, $payer)->setRel('customer autopay', TRUE, TRUE);
      $msg .= '|' . t('auto pay on', compact('payeeName'));
    }
  } elseif ($op == 'delete') {
    db\del('r_invoices', $nvid);
    r\notify($payer, 'invoice withdrawn', $subs);
    $msg = 'invoice delete done';
  } elseif ($op == 'deny' or @$whyNot) {
    $data = serialize(compact('whyNot') + (@$data ? unserialize($data) : []));
//    db\q('UPDATE r_invoices SET status=:TX_DENIED, data=:data WHERE nvid=:nvid', compact('nvid', 'data'));
    db\update('r_invoices', ray('status data nvid', TX_DENIED, $data, $nvid), 'nvid');
    r\notify($payee, 'invoice denied', $subs);
    $msg = 'denial done';
  } else $msg = ''; // default: canceled

//  if ($toMe and !r\acct()) return w\go('do/code=' . ($msg ?: 'nothing done', $ok ? 'status' : 'error');
  return w\go($ret, $msg ?: 'nothing done', $ok ? '' : 'ERR');
}