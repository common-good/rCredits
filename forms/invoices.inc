<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;

include __DIR__ . '/../cg-txs.inc';

/**
 * List Invoices
 */
function formInvoices($form, &$sta, $toFrom, $args = '') {
  w\downloadCheck('history/invoices-' . $toFrom);

  $toMe = ($toFrom == 'to');
  extract(just('period starting ending download downloaded states nvid do', $args)); 

  $choices = array(t('open'), t('paid'), t('denied'));
  extract(just('dateRange url starting ending states', dateRange($args, "history/invoices-$toFrom", $choices, '0,1,2')));
/*
  if (@$do == 'ok') {$ok = be\payInvoice($nvid, $msg); say($msg, $ok ? '' : 'ERR');}
  if (@$do == 'no') return w\go("handle-invoice/nvid=$nvid&toMe=$toMe&ret=" . u\urlify($url));
*/
  
  $list = item(invoiceList($toMe, $starting, $ending, $states, $url, @$download)); // must precede starting & ending

  $title = t('Invoices ') . ($toMe ? t('TO You') : t('FROM You'));
  $subtext =  $toMe ? t('pay or deny an invoice') : t('edit or delete an invoice');
  $subtext = t('Click on a row to %subtext (unpaid invoices are listed first).', compact('subtext'));
  $toFrom = hidFld($toFrom);

  //simple advanced
  $form = compact('title', 'subtext') + $dateRange + compact(ray('list toFrom'));
  if (@$downloaded) js('download', compact('url'));
  
  w\jsx('invoices', ray('args', "&toMe=$toMe&ret=" . u\urlify($url)));
  return cgform($form);
}

function formInvoices_validate($form, &$sta) {w\dateRangeValidate($sta, FALSE);}

function formInvoices_submit($form, &$sta) {
  dateRangeRedirect('history/invoices-' . $sta['input']['toFrom'], $sta);
}

/**
 * Prepare the list of invoices
 * @return string table-based HTML listing the relevant invoices
 */
function invoiceList($toMe, $starting, $ending, $states, $url, $download) {

  $headHelp = t('Invoice number,Date invoice was created,Who sent you the invoice,What is the the invoice for,Total invoice amount,Click a symbol to approve or deny an invoice');
  $classes = ray('nvid date name purpose amount buttons');
  $headers = t('Inv#,Date,Name,Purpose,Amount,Status');
  $headHelp = array_combine($classes, ray($headHelp));

  $header = array_combine($classes, ray($headers)); // make splices and additions easy and clear
  $downloadHeader = $header;
  foreach ($header as $key => $value) $header[$key] = w\popHelp($value, $headHelp[$key]);

  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  if (@$download) {
    $csv = x\setupDownload($starting, $ending);
    u\csv($downloadHeader);
  } else $csv = FALSE;

  $rows = [];
  list ($yu, $me) = u\order($toMe, 'payee', 'payer');
  $where = ":uid=i.$me AND i.created BETWEEN :starting AND :ending";
  /*
  This is broken anyway
  if (strpos($states, '0') === FALSE) $where .= ' AND i.status<>:TX_PENDING';
  if (strpos($states, '1') === FALSE) $where .= ' AND i.status<=0';
  if (strpos($states, '2') === FALSE) $where .= ' AND i.status<>:TX_DENIED';
  */
  $sql = <<<EOF
    SELECT nvid, i.created, u.fullName AS other, i.purpose, i.amount, i.status, i.data, t.{$me}Tid AS tid
    FROM r_invoices i 
    LEFT JOIN users u ON u.uid=i.$yu
    LEFT JOIN r_txs t ON t.xid=i.status
    WHERE $where
    ORDER BY (i.status>2), nvid DESC
EOF;
//    ORDER BY (i.status=:TX_PENDING OR i.status=:TX_APPROVED) DESC, nvid DESC

  global $mya;
  $q = db\q($sql, ray('uid starting ending', $mya->id, $starting, $ending));
///   debug(compact('sql','starting','ending', 'mya'));
  while (count($rows) < $maxRows and $tx = $q->fetchAssoc()) {
    extract($tx);
    if (!@$download) $amount = number_format($amount, 2);
    $row = array($nvid, u\fmtDate($created), $other, $purpose, $amount);
    
    $do = @"$url&nvid=$nvid&do";

    if ($status > 1) {
//      $button1 = $button2 = $button3 = '';
      $status = t('paid') . " (Tx#$tid)";
    } else {
//      $button1 = $toMe ? button(t(' PAY '), "$do=ok", t('Pay this invoice')) : '';
//      $button2 = (!$toMe ?: $status == TX_PENDING) ? button(' X ', "$do=no", $toMe ? t('Deny this invoice') : t('Delete this invoice')) : '';
//    $button3 = (!$toMe and $status == TX_PENDING) ? "<input type='image' title='Edit this invoice' src='$rUrl/images/icons/edit.png' />" : '';
//      $button3 = '';
      $statuses = ray(TX_STATUSES);
      $data = unserialize($data);
      $status = ucfirst($statuses[$status0 = $status]);
//      $status = $status == TX_DENIED ? w\doLnk('reason', $statuses[$status], '', ray('title', t('Why: ') . @$data['whyNot'])) : ucfirst($statuses[$status]);
      $status = $status0 == TX_DENIED ? tags('a', w\popHelp($status, t('Reason: ') . @$data['whyNot'])) : $status;
    }

//    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$status</div></div>";
    $buttons = "<div class='txs-buttons'>$status</div>";
    
    if (!@$download) $row[] = $buttons;
    $rows[] = $row;
  }
  if ($tx and $q->fetchAssoc()) say('transaction list truncated'); // there was more than we could show

  if (@$download) {
    foreach ($rows as $row) u\csv($row);
    if (u\test()) return; else exit();
  }

  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = t('no invoices');
  return theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));
}
