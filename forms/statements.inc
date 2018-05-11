<?php
namespace CG\Web;
use CG as r;

/**
 * List the available account statement dates, with links.
 */
function formStatements($form, $sta, $month = '') {
  if ($month) return r\go("history/statement/$month");
  
  $mo = date('F');
  $subtext = date('d') < 10 ? t('note|Statements are not final until the 10th of the next month.', compact('mo')) : '';
  return cgform(monthChoices(t('Statements'), 'statements', $subtext));
}
