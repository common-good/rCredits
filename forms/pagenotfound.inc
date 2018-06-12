<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle page not found error (404).
 */
function formPageNotFound($form, &$sta, $arg = '') {
  $page = $_SERVER['REQUEST_URI'];

  $page = isDEV ? basename($page) : substr($page, 1); // ignore leading slash
//  if ($page == 'timedout') return w\go(PROMO_URL); // something weird went wrong (only on DEV?)
  header_remove(); // prevent 404 circle (doesn't help)
  
  if ($page[0] == '-') {
    include_once __DIR__ . '/../forms/signup.inc';
    return formSignup('', $sta, 'code=' . substr($page, 1)); // invitation!
  }
  return formEmpty(t('The page you requested does not exist.'));
}