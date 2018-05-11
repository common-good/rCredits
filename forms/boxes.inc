<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show a list of devices used by this account. UNUSED AT PRESENT.
 * @todo: focus on code input
 */
function formBoxes($form, &$sta) {
  $myid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only deletion
  if (form_step($sta, $info) == 'prove') {
    $new = u\fmtPhone($sta['storage']['values']['new']);
    $form = array(
      'title' => item(t('Verify')),
      'subtext' => item(t('verify cell', 'number', $new)),
      'code' => textFld(t('Code:')),
      'new' => hidFld($new),
      'verify' => submit(t('Verify')),
    );
    return cgform($form);
  }

  $headers = array('#', 'Type', 'Device name', 'Last used');
  $types = ray(TX_CHANNELS);

  $sql = "SELECT channel, boxnum, code, boxName, access FROM r_boxes WHERE uid=:myid ORDER BY boxnum";
  $result = db\q($sql, compact('myid'));
  $list = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $type = $types[$channel += 0]; // make sure channel is int, not string
    $boxName = 
        @$boxName ? plain($boxName)
      : ($channel == TX_POS ? (t('POS Device #') . $boxnum)
      : ($channel == TX_WEB ? (t('Computer/Browser #') . $boxnum)
      : ($channel == TX_SMS ? u\fmtPhone($code)
      : 'ERROR')));
//    $list["x$boxid"] = array($boxid, $boxName, $restricted ? 'Yes' : 'No', u\fmtDate($access));
    $list["x$boxnum"] = array($boxnum, $type, $boxName, u\fmtDate($access, TRUE));
  }

  unset($sta['input']['new']); // don't show number just added
  $form = array(
    'title' => item(t('Devices (computers, phones, tablets, etc.)')),
    'subtext' => item(t('These are the devices used with this account.')),
    'boxes' => fld('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => t('no devices')), $list),
    'new' => textFld(t('Add a cell phone*: ')),
    'save' => submit(t('Save changes')),
//    'delete' => submit(t('Remove Selected')),
//    'postscript' => item(t('* You may have a cell phone listed on the <a href="settings/contact">Contact Information</a> page, or even as a POS device on this page, but to use it for text message transactions you must add its phone number here.')),
    'postscript' => item(t('* A device may appear on this list more than once if you use it in different ways (for texting, as a Point of Sale device, and/or as a web browser).')),
  );
  return cgform($form);
}

function formBoxes_validate($form, &$sta) {
  $myid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract(just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op == 'delete') {
    if (no_selection($boxes)) {
      if ($new) $op = 'add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(t('confirm delete cell'), $sta);
  }

  if ($op == 'save') {
    if (blank_field(compact('new'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($new, 'VALIDATE'), 'new');
  }
}

function formBoxes_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract(just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'save') {
    if ($info = previous_state($sta)) return say($info, 'new'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($new, t('verification code', compact('nonce')));
/**/ if (isDEV) debug("nonce is $nonce"); // (keep this) tell developer what code to type
    $info = compact('new');
    return form_step($sta, $info, 'prove');
  }

  if ($op == 'verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($new));
    $sta['redirect'] = '/settings/boxes';
  }
    
  if ($op == 'delete') {
// needs work    foreach ($boxes as $key => $one) if ($one[1]) say(be\deleteCell(substr($one[1], 1))); // ignore the leading 'x'
  }
}