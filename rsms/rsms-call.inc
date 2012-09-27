<?php
namespace rCredits\SMS; // typically abbreviated as "rSMS"
use \rCredits\API as api;
use \rCredits\Utilities as u;

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
  var $uid; // record id of caller's record in user table (duplicates $cid)
  var $xid; // record id of transaction, related to an operation awaiting confirmation (negative for asif transactions)
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
    u\log('rSMS', 'in', compact(u\ray('type message number')));
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
    if (!($this->uid = \sms_user_get_uid($this->number))) $this->uid = api\create_user(array('number' => $this->number));
    if (!$this->uid) die('missing uid');
    u\cid($this->uid, TX_SMS); // set $userid, $cid, and $channel
    
    $account = \user_load($this->uid);
    $todo = api\todo($this->number);
    if (is_object($todo)) $todo = $todo->todo;
    if (api\is_temp_name($account->name)) {
      if($todo != 'setup name') {
        api\todo($this->number, 'setup name', '');
        return tt("what's your name?");
      }
      $name = setup_name($account, $this->message); // actually full_name
      api\todo($this->number, 'setup email', '');
      $qid = r\qid(); //$account->credit_id;
      return tt("what's your email?", compact('qid', 'name'));
    } elseif ($todo == 'setup email' and !$account->mail) {
      setup_email($account, $this->message);
      api\todo($this->number, '', '');
      $balance = u\format_amount(api\give_signup_incentive($this->uid));
      return tt('your account is ready', compact('balance'));
/*    } elseif (!\user_access('transact via sms', $account)) { // user does not have permission to use the rCredits SMS interface
      // send the user an email here?
      // throw new \Exception('You do not have permission to use this system.'); // (due to misbehavior?), so don't waste a call?
      return tt('no SMS permission'); // be polite 
      */
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
    $todo = api\todo($this->number);
    api\todo($this->number, '', ''); // todo and nonce are only good once

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
    $info['nonce'] = api\todo($this->number, serialize(get_object_vars($this)));
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
   *   Possibly customize the message for the caller (write a function for that).
   *   For example, using a local phone number and local businesses in the examples.
   */
  function do_helpme($feature = '') {
    if (!$feature) $feature = @$this->parms['feature'] ?: 'helpme';

    foreach(array_keys($GLOBALS['SMS commands']) as $try) {
      if (u\abbreviates($feature, $try)) {
        // we might want to customize the messages here (see @todo above)
        api\todo($this->number, "help {$try}2", '');
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
      api\todo($this->number, $todo . '2', ''); // queue up tertiary help
      return tt($todo);
    }
    return tt('help helpme');
  }
    
  /**
   * Give the caller useful information.
   */
  function do_information() {
//    api\todo($this->number, 'account info2', ''); // queue up more info (not used yet)
    return tt('account info', api\credit_info()->fancy);
  }

  /**
   * Help the caller exchange rCredits for US Dollars or vice versa.
   */
  function do_get() {
    extract($this->parms, EXTR_PREFIX_ALL, 'my');
    list ($message, $args, $confirm) = api\get(strtolower($my_rusd), $my_amount, $this->uid, $this->confirmed);
    return $confirm ? $this->confirm($message, $args) : tt($message, $args);
  }

  /**
   * Undo, cancel, contest, or deny a recent transaction.
   * Syntax:
   *   undo (undo my last transaction) (see also "pay" with no arguments)
   *   undo who (undo my last transaction with who) (see also "pay who")
   *   undo to|from (undo my last transaction to anyone or from anyone
   *   undo to|from who (undo my last transaction to someone or from someone)
   * SELECT transactions ORDER BY xid DESC LIMIT 1
   * WHERE payer=me / payee=me / payer=me OR payee=me
   * AND payer=other OR payee=other
   */
  function do_undo() {
    global $cid;
    extract($this->parms, EXTR_PREFIX_ALL, 'my');	// who and/or tofrom

    if ($this->confirmed) {
      list ($message, $args) = api\undo_tx($my_xid);
      return tt($message, $args);
    }
    
    if (isset($my_tofrom)) {
      $my_tofrom = strtolower($my_tofrom);
      $where = $my_tofrom === 'to' ? 'payer=:cid' : 'payee=:cid';
    } else $where = '(payee=:cid OR payer=:cid)';

    if (isset($my_who)) {
      if (is_array($other_uid = api\identify($my_who))) return tt($other_uid); // return if error
      $where .= " AND ";
      if (isset($my_tofrom)) {
        $where .= $my_tofrom === 'to' ? 'payee=:other_uid' : 'payer=:other_uid';
      } else $where .= '(payee=:other_uid OR payer=:other_uid)';
    }

    if (!($result = api\last_tx($where, compact(u\ray('cid other_uid'))))) return tt('undo no match');
    $this->parms['xid'] = $result['xid'];
    return $this->confirm('undo', r\tx_args($result));
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
    global $cid;
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, what, and maybe xid
    if (isset($my_amount)) return $this->do_transfer('payment'); // complete syntax
    
    if ($this->confirmed) {
      list ($message, $args) = api\pay_invoice($my_xid);
      return tt($message, $args);
    }
    
    $where = 'state=:TX_PENDING AND payer=:cid';
    if (isset($my_who)) {
      if (is_array($other_uid = api\identify($my_who))) return tt($other_uid); // return if error
      $where .= ' AND payee=:other_uid';
    }
    if(!($result = api\last_tx($where, compact(u\ray('cid other_uid'))))) return tt('no matching invoice');
    $this->parms['xid'] = $result['xid'];
    return $this->confirm('pay invoice', r\tx_args($result));
  }
  
  /**
   * Create a transaction for the caller.
   */
  function do_transfer($type) {
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, and what

    if (is_array($other_uid = api\identify($my_who, TRUE))) return tt($other_uid); // return if error
    list ($message, $args, $confirm) = api\transfer($type, $other_uid, $my_amount, $my_what, $this->confirmed);
    return $confirm ? $this->confirm($message, $args) : tt($message, $args);
  }
  
} // end of SMS call class definition
