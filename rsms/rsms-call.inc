<?php
namespace rCredits\SMS;
use rCredits as r;
use rCredits\Backend as be;
use rCredits\Util as u;

/**
 * @file
 * rCredits SMS Call class
 *
 * This class includes functions that directly reference the call's parameters (message, number, etc).
 * All other rSMS functions go in rsms.module or rsms.inc instead.
 */

class rSMSexception extends \Exception {} // so we can use \Exception for real errors

class call {
  var $message; // the text message received
  var $number; // calling phone number
  var $uid; // record id of caller's record in user table (duplicates $mya->id and can probably be eliminated, but look carefully
    // at storing $this for todo and retrieving
  var $xid; // record id of transaction, related to an operation awaiting confirmation
  var $command; // the major command operation (as parsed)
  var $parms; // parameters to the command (as parsed)
  var $confirmed; // whether the operation has been confirmed

  /**
   * Instantiate a call.
   */
  function __construct($number, $message) {
    global $channel, $rsms_number; 
    $channel = TX_SMS;
    $rsms_number = $this->number = $number = u\fmtPhone($number, '+n');
    u\EXPECT(compact(u\ray('number message')), 'string&notempty string&notempty');
    $this->message = trim($message);
    $this->confirmed = FALSE;
    u\loga('in', compact('message'), $number);
///    die('in new call ' . print_r($this, 1));
  }

  /**
   * Handle the incoming SMS message appropriately,
   * including ALWAYS return a reply, to send to the caller.
   *
   * @return string
   *   what to reply to the caller
   */
  function handle_call() {
    if (!r\up()) return tt('system down');
    if ($mya = r\acct() and !$mya->cttyUp) return tt('your community down', u\ray('contact', $mya->cAdminContact()));
    try {
      if($error_message = $this->validate_caller()) return $error_message;
      $this->parse();
    }
    catch (rSMSexception $e) {return $e->getMessage();}
    return $this->rdo();
  }
  
  /**
   * Look up the calling number in the database to see who's calling.
   *
   * If the caller does not have a record in our database, create one.
   * If the record is not yet adequate, ask for more information.
   * The caller is not validated unless we have a name and email address for him or her.
   * @returns string: error message to send to the caller (null if success).
   * @throws rSMSexception
   *   Message is the error message to send the caller.
   */
  function validate_caller() {
    $number = $this->number;
///    debug($number);
    $myid = $this->uid = uid($number);
    if (!$myid) {
      return tt('you must setup');
      //$mya = new r\acct(compact('number')); // this will crash if someone (VERY) quickly calls again from the same number
      //$myid = $mya->id;
    } else $mya = r\acct($myid);
///    print_r(compact('number','myid','mya')); die();
    u\EXPECT($mya, 'failed to get acct');
    r\acct::setDefault($mya);
    
    $todo = @be\todo($number)->todo;
    /*
    if (be\isTempName($mya->name)) {
      if($todo != 'setup name') {
        be\todo($number, 'setup name', '');
        return tt("what's your name?");
      }
      $fullName = setup_name($mya, $this->message);
      be\todo($number, 'setup email', '');
      $quid = r\quid($mya->id, $mya->agentId, $mya->regionId); //$mya->credit_id;
      return tt("what's your email?", compact('quid', 'fullName'));
    } elseif ($todo == 'setup email' and !$mya->mail) {
      setup_email($mya, $this->message);
      be\todo($number, '', '');
      $balance = u\fmtAmt(be\fund($this->uid, TX_SIGNUP));
      return tt('your account is ready', compact('balance'));
/*    } elseif (!\user_access('transact via sms')) { // user does not have permission to use the rCredits SMS interface
      // send the user an email here?
      // throw new rSMSexception('You do not have permission to use this system.'); // (due to misbehavior?), so don't waste a call?
      return tt('no SMS permission'); // be polite 
    }
    */
    return ''; // success
  }

  /**
   * Decide what syntax best matches the caller's message.
   * Identify the command and its parameters
   * @throws error message to reply to the caller if message cannot be completely parsed
   * @todo: "who" identifiers chosen in the "name" command cannot be me, r, u, us, or usd.
   */
  function parse() {
    $todo = be\todo($this->number);
    be\todo($this->number, '', ''); // todo and nonce are only good once

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
    if ($unparseable) throw new rSMSexception(tt('syntax') . ' ' . $this->do_helpme($intent));
    if (strtoupper($message) != $todo->nonce) throw new rSMSexception(tt('wrong nonce'));

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
    $info['nonce'] = be\todo($this->number, serialize(get_object_vars($this)));
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
    be\deleteCell($this->number);
  }

  /**
   * Handle a user's request to (again) accept SMS messages from us (already handled by the gateway).
   */
  function do_start() {
    return tt('already started');
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
        be\todo($this->number, "help {$try}2", '');
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
    if (isset($GLOBALS[TX_SMS . ' messages'][$todo])) {
      be\todo($this->number, $todo . '2', ''); // queue up tertiary help
      return tt($todo);
    }
    return tt('help helpme');
  }
    
  /**
   * Give the caller useful information.
   */
  function do_information() {
//    be\todo($this->number, 'account info2', ''); // queue up more info (not used yet)
    $mya = r\acct();
    $quid = $mya->mainQid;
    $totalDemand = r\totals()->fancy['demand'];
    list ($balance, $rewards) = array(u\fmtAmt($mya->r + $mya->usd), u\fmtAmt($mya->rewards));
    return tt('account info', compact(u\ray('quid balance rewards totalDemand')));
  }

  /**
   * Handle a user's request to use the phone for someone else's account (Until the user types "be me").
   */
  function do_be() {
    return tt('nyi');
  }
  
  /**
   * Help the caller exchange rCredits for US Dollars or vice versa.
   */
  function do_get() {
    return tt('nyi');
    extract($this->parms, EXTR_PREFIX_ALL, 'my');
    list ($message, $args, $confirm) = be\get(strtolower($my_rusd), $my_amount, $this->uid, $this->confirmed);
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
    $mya = r\acct();
    $myid = $mya->id;
    extract($this->parms, EXTR_PREFIX_ALL, 'my');	// who and/or tofrom

    if ($this->confirmed) {
      list ($message, $args) = be\undoTx($my_xid);
      return tt($message, $args);
    }
    
    if (isset($my_tofrom)) {
      $my_tofrom = strtolower($my_tofrom);
      $where = $my_tofrom === 'to' ? 'payer=:myid' : 'payee=:myid';
    } else $where = '(payee=:myid OR payer=:myid)';

    if (isset($my_who)) {
      if (!u\isAcct($acct = be\identify($my_who))) return tt($acct); // return if error
      $otherUid = $acct->id;
      $where .= " AND ";
      if (isset($my_tofrom)) {
        $where .= $my_tofrom === 'to' ? 'payee=:otherUid' : 'payer=:otherUid';
      } else $where .= '(payee=:otherUid OR payer=:otherUid)';
    }

    $oldRecord = be\lastTx($where, compact(u\ray('myid otherUid')));
    if ($error = r\nonoUndo($oldRecord, $subs)) return array($error, $subs);
    $this->parms['xid'] = $oldRecord['xid'];
    return $this->confirm('undo', r\txReportArgs($mya, $oldRecord, FALSE));
  }

  /**
   * Give funds to the caller from someone else.
   */
  function do_charge() {
    return tt('no charging');
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
    $mya = r\acct();
    $myid = $mya->id;
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, what, and maybe xid
    if (isset($my_amount)) return $this->do_transfer('payment'); // complete syntax
    
    // confirmed invoice payment
    if ($this->confirmed) {
      list ($message, $args) = be\payInvoice($my_xid);
      return tt($message, $args);
    }
    
    // unconfirmed invoice payment
    $where = 'state=:TX_PENDING AND payer=:myid';
    if (isset($my_who)) {
      if (!u\isAcct($acct = be\identify($my_who))) return tt($acct); // return if error
      $otherUid = $acct->id;
      $where .= ' AND payee=:otherUid';
    }
    if(!$result = be\lastTx($where, compact(u\ray('myid otherUid')))) return tt('no matching invoice');
    $this->parms['xid'] = $result['xid'];
    return $this->confirm('pay invoice', r\txReportArgs($mya, $result));
  }
  
  /**
   * Create a transaction for the caller.
   */
  function do_transfer($type) {
    extract($this->parms, EXTR_PREFIX_ALL, 'my'); // amount, who, and what

    $acct = be\identify($my_who);
    if ($acct != 'ALL' and !u\isAcct($acct)) return tt($acct); // return the array if error

    list ($message, $args, $confirm) = be\transfer($type, r\acct(), $acct, $my_amount, $my_what, !u\forCash($my_what), $this->confirmed);
    return $confirm ? $this->confirm($message, $args) : tt($message, $args);
  }
  
} // end of SMS call class definition
