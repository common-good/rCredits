<?php
namespace rCredits\SMS;
use \rCredits as r;
/**
 * @file
 * rCredits SMS Call class
 *
 * This class includes functions that directly reference the call's parameters (message, number, etc).
 * All other rSMS functions go in rsms.module or rsms.inc instead.
 */
 
class call {
  var $message;
  var $number;
  var $caller;
  var $command;
  var $parms;
  var $confirmed;

  /**
   * Instantiate a call.
   */
  function __construct($number, $message) {
    $this->number = $number;
    $this->message = trim($message);
    $this->confirmed = FALSE;
    r\r_log('SMSin', $message, $number);
  }

  /**
   * Handle the incoming SMS message appropriately,
   * including ALWAYS return a reply, to send to the caller.
   *
   * @return string
   *   what to reply to the caller
   */
  function handle_call() {
      try {
        if($error_message = $this->validate_caller()) return $error_message;
        $this->parse();
      }
      catch (\Exception $e) {return $e->getMessage();}
      return $this->rdo();
  }
  
  /**
   * Look up the calling number in the database to see who's calling.
   *
   * If the caller does not have a record in our database, create one.
   * If the record is not yet adequate, ask for more information.
   * The caller is not validated unless we have a full_name and email address for him or her.
   * @returns string: error message to send to the caller (null if success).
   * @throws \Exception
   *   Message is the error message to send the caller.
   */
  function validate_caller() {
    if (!($this->caller = \sms_user_get_uid($this->number))) $this->caller = create_account(array('number' => $this->number));
    if (!$this->caller) die('missing uid');

    $account = \user_load($this->caller);
    $todo = todo($this->number);
    if (is_object($todo)) $todo = $todo->todo;
    if (r\is_temp_name($account->name)) {
      if($todo != 'setup name') {
        todo($this->number, 'setup name', '');
        return tt("what's your name?");
      }
      $full_name = setup_name($account, $this->message);
      todo($this->number, 'setup email', '');
      $id = $account->credit_id;
      return tt("what's your email?", compact('id', 'full_name'));
    } elseif ($todo == 'setup email' and !$account->mail) {
      setup_email($account, $this->message);
      todo($this->number, '', '');
      $balance = r\format_amount(r\give_signup_incentive($this->caller));
      return tt('your account is ready', compact('balance'));
    } elseif (!\user_access('transact via sms', $account)) { // user does not have permission to use the rCredits SMS interface
      // send the user an email here?
      // throw new \Exception(''); // (due to misbehavior?), so don't waste a call?
      return tt('no SMS permission'); // be polite 
    }
    return ''; // success
  }

  /**
   * Decide what syntax best matches the caller's message.
   * Identify the command and its parameters
   *
   * @throws
   *   error message to reply to the caller if message cannot be completely parsed
   *
   * @todo
   *   "who" identifiers chosen in the "name" command cannot be me, r, u, us, or usd.
   */
  function parse() {
    $todo = todo($this->number);
    todo($this->number, '', ''); // todo and nonce are only good once

    $message = trim($this->message);
    $commands = $GLOBALS['SMS commands'];
    $intent = ''; // the caller's intended major command (for helpme, in case they get the syntax wrong)
    foreach (array_keys($GLOBALS['SMS commands']) as $command) {
      $syntaxes = is_array($commands[$command]) ? $commands[$command] : array($commands[$command]);
      foreach($syntaxes as $syntax) {
        $pattern = str_replace(' ', ' +', trim($syntax)); // extra spaces are fine
        $pattern = strtr($pattern, $GLOBALS['SMS command tokens']);
        foreach ($GLOBALS['SMS reserved words'] as $word) {
          if (!in_array($word, $GLOBALS['SMS gateway words'])) $pattern = str_replace($word, abbreviator($word), $pattern);
        }
        
        // isolate major command, if possible, potentially for more accurate help
        $intent = intent($intent, $command, $pattern, $message);
        $pattern = "/^$pattern$/i";
        if (preg_match($pattern, $message, $matches)) {
          array_shift($matches);
          preg_match_all("/%[^ \(\)%]*/", $syntax, $keys);
          $keys = str_replace('%', '', $keys[0]);
          if (count($keys)) {
            if (count($matches) < count($keys)) $matches[] = ''; // make the optional final argument null, if omitted
            $this->parms = array_combine($keys, $matches);
          } else $this->parms = array();
          if ($command == 'more') $this->parms['todo'] = $todo->todo;
          $this->command = $command;
          return $this;
        }
      }
    }

    $unparseable = !$todo->nonce or strpos($message, ' '); // no nonce or not intended to be a nonce
    if ($unparseable) throw new \Exception(tt('syntax') . ' ' . $this->do_helpme($intent));
    if (strtoupper($message) != $todo->nonce) throw new \Exception(tt('wrong nonce'));

    foreach (unserialize(stripslashes($todo->todo)) as $key => $value) $this->$key = $value;
    $this->confirmed = TRUE;
    return $this;
  }

  /**
   * Ask for confirmation for the command.
   * @parm string $type: what operation to confirm
   * @parm array $info: associative array of parameters for the confirmation message
   * @return string: the confirmation request
   */
  function confirm($type, $info) {
    $info['nonce'] = todo($this->number, serialize(get_object_vars($this)));
    return tt("confirm $type|please confirm", $info);
  }
  
  /**
   * Call a function to handle the specific command
   * and return an appropriate reply.
   *
   * @return
   *   the appropriate SMS reply
   */
  function rdo() {
    $function = 'do_' . $this->command;
    return $this->$function();
  }

  /**
   * Handle a user's request to receive no SMS messages from us (already handled by the gateway).
   * Only two gateway commands get passed through to us: start and stop (help does not get passed through).
   */
  function do_stop() {
    \user_role_revoke_permissions(DRUPAL_AUTHENTICATED_RID, array('receive sms'));
  }

  /**
   * Handle a user's request to (again) accept SMS messages from us (already handled by the gateway).
   */
  function do_start() {
    \user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, array('receive sms'));
  }

  /**
   * Give the caller a helpful message
   *
   * @param string $feature (optional): what the caller wants help on
   * @return: the appropriate SMS reply
   * @todo
   *   1. Possibly customize the message for the caller (write a function for that).
   *      For example, using a local phone number and local businesses in the examples.
   */
  function do_helpme($feature = '') {
    if (!$feature) {
      $feature = isset($this->parms['feature']) ? $this->parms['feature'] : '';
      if (!$feature) $feature = 'helpme';
    }

    foreach(array_keys($GLOBALS['SMS commands']) as $try) {
      if (r\abbreviates($feature, $try)) {
        // we might want to customize the messages here (see @todo above)
        todo($this->number, "help {$try}2", '');
        return tt("help $try");
      }
    }
    return tt('not a feature|help helpme', compact('feature'));
  }

  /**
   * Give additional help
   * @parm (implicit) string $this->parms['todo']: index to the additional help wanted
   */
  function do_more() {
    $todo = $this->parms['todo'];
    if (isset($GLOBALS['SMS messages'][$todo])) {
      todo($this->number, $todo . '2', ''); // queue up tertiary help
      return tt($todo);
    }
    return tt('help helpme');
  }
    
  /**
   * Give the caller useful information.
   */
  function do_information() {
//    todo($this->number, 'account info2', ''); // queue up more info (not used yet)
    return tt('account info', r\credit_info($this->caller)->fancy);
  }

  /**
   * Help the caller exchange rCredits for US Dollars or vice versa.
   */
  function do_get() {
    $dbtx = db_transaction();
    extract($this->parms, EXTR_PREFIX_ALL, 'my');
    extract((array) r\credit_info($this->caller), EXTR_PREFIX_ALL, 'info');
    $info_fancy['amount'] = r\format_amount($my_amount);
    if ($my_rusd == 'r') {
      r\demand($this->caller, $my_amount);
      return tt('your demand', $info_fancy);
    }

    // usd
    if ($my_amount > $info_cashable) return tt("can't cash incentives", $info_fancy);
    $type = 'payment';
    $amount = $my_amount;
    $payer = $creator = $this->caller;
    $payee = variable_get('rcredits_community_uid');
    $for = t('cash');
    $info = compact('type', 'amount', 'payer', 'payee', 'for', 'creator');
    r\transact($info);
    $info_fancy['balance'] = r\format_amount($info_balance - $my_amount);
    if ($this->confirmed) return tt('get usd', $info_fancy);
    
    $dbtx->rollback();
    return $this->confirm('get usd', $info_fancy);
  }

  /**
   * Undo a recent transaction by creating its opposite (using a negative amount).
   * If the last transaction is pending, we will simply change its state to
   * cancelled.
   * SELECT (*) FROM mcapi_transactions WHERE () ORDER BY xid DESC LIMIT 1
   * WHERE payer=me / payee=me / payer=me OR payee=me
   * AND payer=other OR payee=other
   * AND type='payment'
   */
  function do_undo() {
    extract($this->parms, EXTR_PREFIX_ALL, 'my');	// who and/or tofrom

    if (isset($my_tofrom)) {
      $my_tofrom = lc($my_tofrom);
      if ($my_tofrom === 'to')
	$where = "payer={$this->caller}";
	else $where = "payee={$this->caller}";
    } else $where = "(payee={$this->caller} OR payer={$this->caller})";

    if (isset($my_who)) {
      $other_uid = identify($my_who, $create_info);
      if (!$other_uid) return tt('unknown user', $this->parms);
      if (is_array($other_uid)) return tt('ambiguous other', $this->parms);
      if ($other_uid == $this->caller) return tt('no self-undo');
      $where .= " AND ";
      if (!isset($my_tofrom))
	$where .= "(payee=$other_uid OR payer=$other_uid)";
      elseif ($my_tofrom === 'to') $where .= "payee=$other_uid";
      else $where .= "payer=$other_uid";
    }

    $sql = "SELECT * FROM mcapi_transactions WHERE ($where AND type='payment')"
      . " ORDER BY xid DESC LIMIT 1";
    $result = db_query($sql)->fetchAssoc();
    if (! $result) return tt('undo no match');

    $result['date'] = strftime('%m/%d/%Y',$result['created']);
    if (! $this->confirmed) {
      $this->parms['xid'] = $result['xid'];
      return $this->confirm ('undo', $result);
    } elseif ($result['xid'] != $my_xid) {
      return tt('undo moving target');
    // !!! Here we would test for pending transactions
    } else {
      $rewards = unserialize($result['data']);
      list ($message, $args, $confirm) = r\transfer(FALSE, $result['payer'],
	$result['payee'], -$result['amount'], 'undo '.$result['payer_for'],
	TRUE, $rewards);
    }
    return tt('report undo', $result);
  }

  /**
   * Give funds to the caller from someone else.
   */
  function do_charge() {
    return $this->do_transfer(TRUE);
  }

  /**
   * Give the caller's funds to someone else.
   */
  function do_payment() {
    return $this->do_transfer(FALSE);
  }

  /**
   * Create a transaction for the caller.
   * @todo: cast everything to integers
   * @todo: move all this logic to rcredits.inc (pass relevant info, respond according to returned status)
   *   rcredits.inc might return the status as a message index!
   */
  function do_transfer($isfrom) {
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, and what

    $other_uid = identify($my_who, $create_info);
    if (!$other_uid and !$create_info) return tt('unknown user', $this->parms);
    if(is_array($other_uid)) return tt('ambiguous other', $this->parms);
    if ($other_uid == $this->caller) return tt('no self-trading');
    
    if (!$other_uid) $other_uid = create_account($create_info);
    $other_account = \user_load($other_uid);
    $other = $other_account->full_name;
    if (!\user_access('transact', $other_account)) return tt('non trader', compact('other'));

    list ($message, $args, $confirm) = r\transfer($isfrom, $this->caller, $other_uid, $my_amount, $my_what, $this->confirmed);
    return $confirm ? $this->confirm($message, $args) : tt($message, $args);
  }
  
} // end of SMS call class definition
