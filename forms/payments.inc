<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once __DIR__ . '/invoiceupload.inc';

/**
 * Upload a spreadsheet of payments.
 */
function formPayments($form, &$sta) {
  global $base_url;
  global $mya;

  $title = t('Payments Upload');
  $instructions = csvUploadInstructions(t('pay'), t('payment'));
  $subtext = "<ul id=\"instructions\">$instructions</ul>";

  $file = fileFld();
  $submit = t('Upload and Pay');

  return cgform(compact(ray('title subtext file submit')));
}

function formPayments_validate($form, &$sta) {
  return formInvoiceUpload_validate($form, $sta);
}

function formPayments_submit($form, &$sta) {
  $args = ray('inv dftDesc did map onSuccess', FALSE, t('payment'), t('paid'), [], NULL);
  return formInvoiceUpload_submit($form, $sta, $args);
}
 