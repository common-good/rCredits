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
  var $message; // the text message received
  var $number; // calling phone number
  var $uid; // record id of caller's record in user table
  var $xid; // record id of transaction, related to an operation awaiting confirmation
  var $command; // the major command operation (as parsed)
  var $parms; // parameters to the command (as parsed)
  var $confirmed; // whether the operation has been confirmed

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
    if (!($this->uid = \sms_user_get_uid($this->number))) $this->uid = create_account(array('number' => $this->number));
    if (!$this->uid) die('missing uid');

    $account = \user_load($this->uid);
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
      $balance = r\format_amount(r\give_signup_incentive($this->uid));
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
        $intent = intent($intent, $command, $pattern, $message); // isolate major command, if possible, potentially for more accurate help
        $pattern = "/^$pattern\$/i";
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
    return tt('account info', r\credit_info($this->uid)->fancy);
  }

  /**
   * Help the caller exchange rCredits for US Dollars or vice versa.
   */
  function do_get() {
    $dbtx = db_transaction();
    extract($this->parms, EXTR_PREFIX_ALL, 'my');
    extract((array) r\credit_info($this->uid), EXTR_PREFIX_ALL, 'info');
    $info_fancy['amount'] = r\format_amount($my_amount);
    if (strtolower($my_rusd) == 'r') {
      r\demand($this->uid, $my_amount);
      return tt('your demand', $info_fancy);
    }

    // usd
    if ($my_amount > $info_cashable) return tt("can't cash incentives", $info_fancy);
    $type = 'payment';
    $amount = $my_amount;
    $payer = $creator = $this->uid;
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
   * Undo, cancel, contest, or deny a recent transaction.
   * Syntax:
   *   undo (undo my last transaction) (see also "pay" with no arguments)
   *   undo who (undo my last transaction with who) (see also "pay who")
   *   undo to|from (undo my last transaction to anyone or from anyone
   *   undo to|from who (undo my last transaction to someone or from someone)
   * Situations:
   *   I paid someone: create an opposite transaction (using a negative amount)
   *   I charged someone unilaterally: create an opposite transaction
   *   I invoiced someone: change the transaction state to CANCELED
   *   Someone paid me: create an opposite transaction
   *   Someone charged me unilaterally: change the transaction state to CONTESTED
   *   Someone invoiced me: change the transaction state to DENIED
   *
   * SELECT (*) FROM mcapi_transactions WHERE () ORDER BY xid DESC LIMIT 1
   * WHERE payer=me / payee=me / payer=me OR payee=me
   * AND payer=other OR payee=other
   */
  function do_undo() {
    extract($this->parms, EXTR_PREFIX_ALL, 'my');	// who and/or tofrom

    if (isset($my_tofrom)) {
      $my_tofrom = strtolower($my_tofrom);
      if ($my_tofrom === 'to') {
        $where = "payer={$this->uid}";
      } else $where = "payee={$this->uid}";
    } else $where = "(payee={$this->uid} OR payer={$this->uid})";

    if (isset($my_who)) {
      if (!is_numeric($other_uid = $this->identify($my_who))) return $other_uid; // return if error
      $where .= " AND ";
      if (!isset($my_tofrom)) {
        $where .= "(payee=$other_uid OR payer=$other_uid)";
      } elseif ($my_tofrom === 'to') {
        $where .= "payee=$other_uid";
      } else $where .= "payer=$other_uid";
    }

    $sql = "SELECT * FROM mcapi_transactions WHERE $where ORDER BY xid DESC LIMIT 1";
    $result = db_query($sql)->fetchAssoc();
    if (!$result) return tt('undo no match');
    $result['date'] = strftime('%m/%d/%Y',$result['created']);
    extract($result, EXTR_PREFIX_ALL, 'old');

    if (!$this->confirmed) {
      $this->parms['xid'] = $result['xid'];
      return $this->confirm('undo', $result);
    } elseif ($result['xid'] != $my_xid) {
      return tt('undo moving target');
    // !!! Here we would test for pending transactions
    } else {
      $rewards = unserialize($result['data']);
      list ($message, $args, $confirm) = r\transfer('payment', $result['payer'],
        $result['payee'], -$result['amount'], 'undo '.$result['payer_for'],
	      TRUE, $rewards);
    }
    return tt('report undo', $result);
  }

  /**
   * Give funds to the caller from someone else.
   */
  function do_charge() {
    return $this->do_transfer('charge');
  }

  /**
   * Give the caller's funds to someone else.
   * Syntax:
   *   pay (pay the most recent invoice from anyone)
   *   pay who (pay the most recent invoice from someone)
   *   other syntaxes including who and amount (create a new transaction)
   */
  function do_payment() {
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, what, and maybe xid
    if (isset($my_amount)) return $this->do_transfer('payment'); // complete syntax
    
    if ($this->confirmed and isset($my_xid)) {
      $where = "xid=$my_xid";
    } else {
      $where = "state=:state AND payer={$this->uid}";
      if (isset($my_who)) {
        if (!is_numeric($other_uid = $this->identify($my_who))) return $other_uid; // return if error
        $where .= " AND payee=$other_uid";
      }
    }
    $sql = "SELECT * FROM mcapi_transactions WHERE $where ORDER BY xid DESC LIMIT 1";
    $result = db_query($sql, array(':state' => TX_PENDING))->fetchAssoc();
    if (!$result) return tt('no matching invoice');

    extract($result, EXTR_PREFIX_ALL, 'old');

    if (!$this->confirmed) {
      $this->xid = $old_xid; // save for after possible confirmation
      $other = r\user_field('full_name', $old_payee);
      $amount = r\format_amount($old_amount);
      $date = strftime('%m/%d/%Y',$result['created']);
      return $this->confirm('invoice', compact(r\ray('other amount date')));
    }

    $dbtx = db_transaction();
    list ($message, $args, $confirm) = r\transfer('invoice payment', $this->uid, $old_payee, $old_amount, $old_payee_for, TRUE);
    if (isset($args['success'])) {
      // FUNCTION ALWAYS FAILS r\set_tx_field('state', TX_DELETED, $old_xid);
      \db_query('UPDATE mcapi_transactions SET state=:state WHERE serial=:serial', array(':state' => TX_DELETED, ':serial' => $old_serial));
    } else $dbtx->rollback();
    return tt($message, $args);
  }

  /**
   * Create a transaction for the caller.
   */
  function do_transfer($type) {
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, and what

    if (!is_numeric($other_uid = $this->identify($my_who, TRUE))) return $other_uid; // return if error
    list ($message, $args, $confirm) = r\transfer($type, $this->uid, $other_uid, $my_amount, $my_what, $this->confirmed);
    return $confirm ? $this->confirm($message, $args) : tt($message, $args);
  }
  
  /**
   * Figure out who the caller meant.
   *
   * @parm string $who: how the user referred to the trader
   * @parm boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
   * @return: the trader's uid or a (non-numeric) error message
   *   Note that an error will be returned if the other is not allowed to be a trader
   */
  function identify($who, $create = FALSE) {
    if ($phone = r\format_phone($who, '+n')) $who = $phone;
  //  if (substr($who, 0, 7) == 'http://') $who = substr($who, 7);
  //  if (substr($who, 0, 4) == 'www.') $who = substr($who, 4);

    $who = strtolower(str_replace("'", '', $who)); // avoid SQL errors
    $sql = "SELECT u.uid FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
            WHERE '$who' IN (CONCAT('.', u.short_name), u.credit_id, s.number, u.phone, u.mail)";
    $result = db_query($sql)->fetchCol();
    if(count($result) > 1) return tt('ambiguous other', compact('who'));
    if (isset($result[0])) {
      $other_uid = $result[0];
      if ($other_uid == $this->uid) return tt('no self-trading');
      $other_account = \user_load($other_uid);
      $other = $other_account->full_name;
      if (!\user_access('transact', $other_account)) return tt('non trader', compact('other'));

      return $other_uid;
    }
    
  //  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
    $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : '');
    if ((!$key) or !$create) return tt('unknown user', compact('who'));
    return create_account(array($key => $who));
  }

} // end of SMS call class definition
