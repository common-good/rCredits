<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * List the people honored or remembered with contributions to Common Good.
 */
function formHonors($form, &$sta) {
  $title = t('Honored and Remembered');
  $subtext = t('Listed here are the people and principles honored and remembered with a <%a>contribution</a> to %PROJECT.', '_a', atag('/community/donate'));
  $oftens = ray(OFTENS);
  $q = db\q("SELECT DISTINCT honor,honored FROM r_honors ORDER BY honor,honored");
  $list = '';
  $type = 'none';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($honor != $type) {
      $list .= '<div class="honor-type">' . (trim(u\honorDesc($honor)) ?: t('other')) . "</div>\n";
      $type = $honor;
    }
    $list .= "<div class=\"well\">$honored</div>\n";
  }
  return cgform(compact(ray('title subtext list')));
}