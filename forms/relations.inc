<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

//define('R_REL_EMPLOYER_COL', 2); // number of amount/employer column
define('R_REL_OWNER_COL', 3); // number of family/owner column

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function formRelations($form, &$sta, $args = '') {
  global $base_url;
  global $mya;
  $myid = $mya->id;
  extract(just('customers', $args)); // linked from an email

  //  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers = be\accountName($myid);
  $header = ray('The other account,Draw,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', ray('person draw employee owner permission link'));
  if ($co = $mya->co) {
    $header[R_REL_OWNER_COL] = 'Owns 10%+';
//    if (!$mya->ok) $mya->stepDone('relations');
  }

  if ($mya->hasCustomers) $showCustomers = @$customers
  ? item(w\btn("/settings/relations", t('Hide Customers')))
  : item(w\btn('/relations/customers=1', t('Show Customers')));
  
  $where = @$customers ? '1' : '(NOT isCustomer OR s.permission OR s.employee OR s.isOwner OR s.draw)';
  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, IF(:myid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.employee, s.isOwner, s.draw
    FROM ((r_relations s LEFT JOIN users u ON u.uid=s.main)
      LEFT JOIN users v ON v.uid=s.other)
    WHERE s.main=:myid AND $where
    ORDER BY fullName, IF(:myid=s.main, s.other, s.main), (:myid=s.other)
EOF;
  $result = db\q($sql, compact('myid'));
  $rows = $raws0 = $raws = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original ("0") raw values from table
    $a = r\acct($other);
    $otherCo = $a->co;
    $otherPerson = !$otherCo;

    $idLink = R_NONE; // default to no link
    if ($mya->co and !$otherCo and $permission >= r\perm(B_SCAN) and $permission != r\perm(B_JOINT)) {
      $text = str_replace(' ', '&nbsp;', $permission >= r\perm(B_BUY) ? t('request %PROJECT Card') : t('request Cashier Card'));
      $idLink = "<a href=\"$base_url/request-employee-rcard/$other\">$text</a>";
    }

    $ii = count($rows) . "-$reid"; // "row-reid"
    $drawn = toggle('draw', $draw, $ii);
//    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employee2);
    $isEmployee = !$otherCo ? toggle('employee', $employee, $ii) : R_NONE;
    $Owner = toggle('isOwner', $isOwner, $ii);
    $permissions = $GLOBALS['share permissions'];
// NO    if (!$mya->can(B_JOINED)) $permissions[$permission = B_JOINT] .= '?'; // pending agreement
    if ($co or $otherCo or !$mya->ok or !$a->ok) unset($permissions[r\perm(B_JOINT)]); // no joint accounts with a company or nonmember
    $perm = $otherPerson ? dropdown('permission', $permission, $permissions, $ii) : R_NONE;
    $amountValue = ''; // was amountField('amount', number_format($amount,0), $ii);

    $thisRow = array($fullName, $drawn, $isEmployee, $Owner, $perm, $idLink);
    $thisRaw = compact(ray('draw employee isOwner permission'));
//      if ($co) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    
    $rows[] = $thisRow; // what to show on screen
    $raws[] = $thisRaw; // original conceptual ("raw") values for what's show on screen
  }
  $sta['rawValues'] = compact('raws0', 'raws');
  
  $aExact = "a href=\"$base_url/help/payment-exchanges\"";
  $_aInvite = atag('/community/invite');
  $aAgent = "a href=\"$base_url/agent\"";
  $help = t('Connect individual %PROJECT participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. If they don\'t have an account yet, <%aInvite>invite them</a>! Each permission includes all the permissions above it.', compact(ray('aExact _aInvite aAgent')));
  //If they wish NOT to open a real account, create an <@aAgent>rPOS Sign-in Account</a> for them.

  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$acctIdentifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = t('no relations');
  $list = theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));

  $title = t('Relations');
  $list = fld('item', '', '', array('markup' => $list));
//    'newPerson' => textFld(t('Add Someone:'), t('name, id, email, or phone')), // autocomplete causes problems
  $newPerson = textFld(t('Add Someone:'), [t('Name, id, email, or phone')]);
  $go = submit(t('Save Changes'));
  $form = compact(ray('title subtext showCustomers list newPerson go'));

  if (!$mya->member and !$mya->stepDone['relations']) $form += w\setupFoot();
  w\whoFldSubmit('newperson', t('Create a new relation with %name?'));
//  w\jsx('toggle');

  return cgform($form);
}

function formRelations_validate($form, &$sta) {
//  if (confirming_v($sta)) return;
  global $mya;
  if ($newPerson = @$sta['input']['newPerson']) {
    $a = whois($newPerson, 'newPerson', $sta['input'], 'no self-relation');
    $id = $a ? $a->id : '';
    if ($id and r\relation(1, $mya->id, $id)) return say('already related', 'newPerson');
    $sta['input']['newPerson'] = $id; // save for submission handling
  }
  $managers = 0; // for company accounts there needs to be at least one manager
  foreach ($sta['input'] as $key => $value) {
    if (!strpos($key, '-')) continue;
    list ($keya, $rawnum, $reid) = explode('-', $key);
/*    if ($keya == 'amount') {
      $amount = $value;
      if ($err = u\badAmount($amount)) return say($err, compact('amount'), 'amount');
      if (($sta['input'][$key] = round($amount, 0)) != $amount) say('amount rounded', compact('amount'));
    } else
    */
    if ($keya == 'permission') {
      if ($value >= r\perm(B_MANAGE)) {
        $managers++;
      } else {
        $other = r\relation('other', $reid);
        if ($other == $mya->id) return say('need a manager', 'permission');
      }
    }
  }
//  if ($mya->co and !$managers and !$mya->cAdmin) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
//  if (confirming_s($sta)) return;
  global $mya;

  $vs = $sta['input'];
  if ($other = @$vs['newPerson']) { // adding a new relation
    $mya->newRelation(compact('other'));
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }
  $changes = [];
  extract(just('raws0 raws', $sta['rawValues']));
  $joinPerm = r\perm(B_JOINT);

  $DBTX = \db_transaction();

  // handle all fields even when they don't appear (because they're false)
  foreach (array_keys($raws0) as $i => $reid) {
    foreach (ray('permission draw employee isOwner') as $fld) {
      $k = "$fld-$i-$reid"; // rawnum
      $v = 0 + ($fld == 'permission' ? @$vs[$k] : (bool) @$vs[$k]);
      $vX = 0 + $raws[$i][$fld];

      if ($v != $vX) { // changed
        if ($fld == 'permission' and ($v == $joinPerm or $vX == $joinPerm)) {
          if ($v == $joinPerm) {
            if (@$gotJoin) {say('too many joins', 'err'); continue;} else $gotJoin = TRUE;
          }
          $msg = r\setJoin($reid, @$gotJoin); // handle joining or unjoining accounts
          say($msg);
        }
        $changes[$reid][$fld] = $v;
        $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
      }
    }
  }

  if (!empty($changes)) say(be\updateRelations($changes));
  unset($DBTX);
  
  return w\goNextStep('relations');
}
