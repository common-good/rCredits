<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once __DIR__ . '/invoiceupload.inc';
 
/**
 * Upload payroll spreadsheet to create partial payments in rCredits.
 */
function formPayroll($form, &$sta) {
  global $base_url;
  global $mya;
  
  $title = t('Payroll Upload');
  $instructions = csvUploadInstructions(t('pay'), t('labor: <startdate> to <enddate>'));

  $subtext = t(<<<EOF
  <ul id="instructions">
  <li><b>Ask your participating employees</b> how many %RCREDITS they can receive each pay period, without having to cash them out (how many can they easily spend in two weeks, for example). Or pay them entirely in %RCREDITS (your choice*).</li>
  <li><b>In your accounting software</b>, create an after-tax payroll deduction called "%PROJECT" (you need do this only once). Set the %PROJECT deduction for each employee to the amount of %RCREDITS they asked to be paid. Use that same amount for the employee's account line in the spreadsheet you will create, as described below. Run payroll, to pay the US Dollar part of their wages, calculate taxes, etc.</li>
%instructions
  </ul>
  <p><b>* NOTE:</b> You cannot require an employee to participate in %PROJECT. But if they ARE a %PROJECT member, they have agreed to accept %RCREDITS (without limit) as payment for their goods and services.</p>
EOF
  , '_a _instructions', atag('/settings/relations'), $instructions);

  $file = fileFld();
  if ($end = $mya->payrollEnd) { // get date(s) from previous payroll
    $start = strtotime('tomorrow', $end); // start the day after previous period ends
    $end = $start + ($end - $mya->payrollStart); // same period length as last time
  } else {
    $end = $fri = strtotime('next Friday', strtotime('-3 days'));
    $start = $fri - 13 * DAY_SECS + HOUR_SECS; // + HOUR_SECS is in case of daylight time
  }
  $startDate = textFld(t('Starting:'), [t('Pay period start')], required(u\fmtDate($start, TRUE)));
  $endDate = textFld(t('Ending:'), [t('Pay period end')], required(u\fmtDate($end, TRUE)));
  $submit = submit('Upload and Pay');

  return cgform(compact(ray('title subtext startDate endDate file submit')));
}

function formPayroll_validate($form, &$sta) {
  extract(just('startDate endDate', $sta['input']));
  list ($min, $max) = [strtotime('-2 months'), strtotime('+7 days')];
  foreach (['startDate', 'endDate'] as $k) if ($err = u\badDate($$k, $min, $max)) return sayFieldErr($err, $k);
  return formInvoiceUpload_validate($form, $sta);
}

//define('R_NOREWARD', '<noreward>'); // "no rewards" flag in description

function formPayroll_submit($form, &$sta) {
  global $startDate, $endDate;
  extract(just('startDate endDate', $sta['input']));

  $inv = FALSE;
  $did = t('paid');
  $map = ray('<startdate> <enddate>', $startDate, $endDate);
  $dftDesc = strtr(t('labor: <startdate> to <enddate>'), $map);
  $onSuccess = function ($total) {
    global $startDate, $endDate, $mya;
    $mya->update('payrollStart payrollEnd', u\s2t($startDate), u\s2t($endDate));
  };
  return formInvoiceUpload_submit($form, $sta, compact(ray('inv dftDesc did map onSuccess')));
}
