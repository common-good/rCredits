<?php
namespace rCredits\SMS;
use \rCredits as r;
/**
 * @file
 * rCredits SMS Call class
 */
 
class call {
  var $message;
  var $number;
  var $caller;
  var $command;
  var $parameters;

  function __construct($number, $message) {
    $this->number = $number;
    $this->message = $message;
    r\r_log('SMSin', $message, $number);
  }

  /**
   * Handle Call
   *
   * Handle the incoming SMS message appropriately,
   * including ALWAYS return a reply, to send to the caller.
   *
   * @return string
   *   what to reply to the caller
   */
  function handle_call() {
      try {
        $this->validate_caller();
        $this->parse();
      }
      catch (\Exception $e) {
        return $e->getMessage(); // cannot return $e->message directly (protected)
      }
      return $this->rdo();
  }
  
  /**
   * Validate Caller
   *
   * Look up the calling number in the database to see who's calling.
   *
   * @throws \Exception
   *   message is the error message to send the caller
   */
  function validate_caller() {
  //throw new \Exception('test');
    if (!($this->caller = \sms_user_get_uid($this->number))) { 
      \sms_user_register_new_user($this->number, $this->message);
      todo($this->number, 'setup name');
      throw new \Exception(tt("what's your name?"));
    }
    if (!user_access('Transact via SMS', \user_load($this->caller))) {
      // send the user an email here?
  //    throw new \Exception(''); // user does not have permission to use the rCredits SMS interface (due to misbehavior?), so don't waste a call?
      throw new \Exception(tt('no SMS permission')); // be polite 
    }
    return $this;
  }

  /**
   * Parse the SMS
   *
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
    $message = trim($this->message);
    $commands = array( // these get abbreviated first
      'get' => array('get %rusd %amount', 'get %amount %rusd'), // best before pay, so that "g r 10" is interpreted as "get r" not "give who"
      'payment' => array(
        '(?:(?:payment|give|send) )?%amount to %who(?: for %what)?', 
        '(?:to|payment|give|send) %who %amount(?: for %what)?',
      ),
      'charge' => array(
        '(?:(?:charge|invoice|take|chg)? )?%amount from %who(?: for %what)?', 
        '(?:from|charge|invoice|chg) %who %amount(?: for %what)?',
      ),
      'undo' => array(
        '(?:undo|cancel)',
        '(?:undo|cancel) %tofrom',
        '(?:undo|cancel) %who',
        '(?:undo|cancel) %tofrom %who',
      ),
      'information' => array('information', 'balance'),
      'help' => 'help(?: %feature)?',
    );
    $GLOBALS['SMS commands'] = array_keys($commands);
    $helper_words = explode(',', 'to,from,usd,balance,name,lost,be,give,send,invoice,take'); 

    $tokens = array(
      '%tofrom' => '(to|from)',
      '%amount' => '([0-9]+(?:.[0-9][0-9]?)?)',
      '%who' => '([^ ]+)',
      '%what' =>  '(.+)',
      '%rusd' => '(r|usd)',
      '%currency' => '([a-z]+)',
      '%feature' => '([^ ]+)',
    );
//    $tokens['%feature'] = '(' . join('|', $GLOBALS['SMS commands']) . ')';
    $abbreviate = array_merge($GLOBALS['SMS commands'], $helper_words);
    $intent = ''; // the caller's intended major command (for help, in case they get the syntax wrong)
    foreach ($GLOBALS['SMS commands'] as $command) {
      $syntaxes = is_array($commands[$command]) ? $commands[$command] : array($commands[$command]);
      foreach($syntaxes as $syntax) {
        $pattern = str_replace(' ', ' +', trim($syntax)); // extra spaces are fine
        $pattern = strtr($pattern, $tokens);
        foreach ($abbreviate as $word) {
          $pattern = str_replace($word, $this->abbreviator($word), $pattern);
        }
        
        // isolate major command, if possible, potentially for more accurate help
        $intent = $this->intent($intent, $command, $pattern, $message);

        $pattern = "/^$pattern$/i";
        if (preg_match($pattern, $message, $matches)) {
          array_shift($matches);
          preg_match_all("/%[^ \(\)%]*/", $syntax, $keys);
          $keys = str_replace('%', '', $keys[0]);
          if (count($keys)) {
            if (count($matches) < count($keys)) $matches[] = ''; // make the optional final argument null, if omitted
            $this->parameters = array_combine($keys, $matches);
          } else $this->parameters = array();
          $this->command = $command;
          return $this;
        }
      }
    }

    //send help if the message is not successfully parsed
    throw new \Exception(tt('syntax') . ' ' . $this->do_help($intent));
  }

  /**
   * Do: handle the parsed command.
   *
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
   * Do Help: give the caller a helpful message
   *
   * @param string $feature (optional)
   *   what the caller wants help on
   *
   * @return
   *   the appropriate SMS reply
   *
   * @todo
   *   1. Possibly customize the message for the caller (write a function for that).
   *      For example, using a local phone number and local businesses in the examples.
   */
  function do_help($feature = '') {
    if (!$feature) {
      $feature = isset($this->parameters['feature']) ? $this->parameters['feature'] : '';
      if (!$feature) $feature = 'help';
    }
    $help_subs = array(
      '@commands' => join(' | ', $GLOBALS['SMS commands']),
      '@param' => $feature,
    );

    foreach($GLOBALS['SMS commands'] as $try) {
      if (substr($try, 0, strlen($feature)) == $feature) {
        // we might want to customize the messages here (see @todo above)
        return tt("help for $try", $help_subs);
      }
    }
    return tt('not a feature|help for help', $help_subs);
  }

  /**
   * Do Information: Give the caller useful information.
   */
  function do_information() {
    return tt('account info', r\credit_info($this->caller)->fancy);
  }

  /**
   * Do Get: Help the caller exchange rCredits for US Dollars or vice versa.
   */
  function do_get() {
    extract($this->parameters, EXTR_PREFIX_ALL, 'my');
    extract((array) r\credit_info($this->caller), EXTR_PREFIX_ALL, 'info');
    $info_fancy['@amount'] = r\format_amount($my_amount);
    if ($my_rusd == 'r') {
      r\demand($this->caller, $my_amount);
      return tt('your demand', $info_fancy);
    }
    if ($my_rusd == 'usd') {
      if ($my_amount > $info_cashable) return tt("can't cash incentives", $info_fancy);
      $type = 'payment';
      $amount = $my_amount;
      $payer = $creator = $this->caller;
      $payee = variable_get('rcredits_community_uid');
      $for = t('cash');
      r\transact(compact('type', 'amount', 'payer', 'payee', 'for', 'creator'));
      $info_fancy['@balance'] = r\format_amount($info_balance - $my_amount);
      return tt('get usd', $info_fancy);
    }
    return tt('get r/usd error');
  }

  /**
   * Do Undo: Undo a recent transaction by creating its opposite.
   */
  function do_undo() {
    return t('The undo feature is not yet available.');
  }

  /**
   * Do Charge: Give funds to the caller from someone else.
   */
  function do_charge() {
    return $this->do_transfer(TRUE);
  }

  /**
   * Do Payment: Give the caller's funds to someone else.
   */
  function do_payment() {
    return $this->do_transfer(FALSE);
  }

  /**
   * Do Transfer: Create a transaction for the caller.
   */
  function do_transfer($isfrom) {
    extract($this->parameters, EXTR_PREFIX_ALL, 'my'); // amount, who, and what

    $type = $isfrom ? 'charge' : 'payment';
    $other = identify($my_who);
    if (!$other) return t('Unknown user: "@user". ', array('@user' => $my_who)) . $this->do_help($type);
    if (!user_access('transact', user_load($other))) {
      return t('@name does not have permission to trade.', array('@name' => user_load($other)->name));
    }
    if ($other == $this->caller) return tt('no self-trading');

    $payer = $isfrom ? $other : $this->caller;
    $payee = $isfrom ? $this->caller : $other;
    $amount = $my_amount;
    $for = $my_what;
    
    try {
      $txs = r\transact(compact('type', 'amount', 'payer', 'payee', 'for')); // credit sender with reward
    }
    catch (\Exception $e) {return $e->getMessage();}
    
    $totals = \transaction_totals($this->caller);
    $tid = r\tid_from_xid($this->caller, $txs[0]->xid);
    $other_name = \user_load($other)->name;
    $reward_index = $isfrom ? 2 : 1;
    $reward = isset($txs[$reward_index]) ? $txs[$reward_index]->worth['und'][0]['quantity'] : 0;
    
    $args = array(
      '@type' => $isfrom ? t('charged') : t('paid'),
      '@amount' => r\format_amount($my_amount),
      '@tofrom' => $isfrom ? t('from') : t('to'),
      '@reward_type' => $isfrom ? t('bonus') : t('rebate'),
      '@reward_amount' => r\format_amount($reward),
      '@balance' => r\format_amount($totals->balance),
      '@tid' => $tid,
      '@other' => $other_name,
    );
    if (count($txs) > 1) return tt('report transaction', $args);
    
    $args['@type'] = 'traded';
    $args['@way'] = $isfrom ? t('cash for credit') : t('credit for cash');
    return tt('report exchange', $args);
  }

  function abbreviator($word) {
    $result = substr($word, 0, 1);
    for ($i = 2; $i <= strlen($word); $i++) {
      $result .= '|' . substr($word, 0, $i);
    }
    return "(?:$result)";
  }

  function intent($intent, $command, $pattern, $message) {
    if ($command == 'payment' | $command == 'charge') {
      $abbrev = $this->abbreviator($command == 'charge' ? 'from' : 'to');
      return preg_match("/ $abbrev /i", $message) ? $command : $intent;
    }
    $short_pattern = str_replace(')(', ')', '/^' . substr($pattern, 0, strpos("$pattern ", ' ')) . ' /i'); // )( happens in help command pattern
    return preg_match($short_pattern, $message, $matches) ? $command : $intent;
  }

  /**
   * Identify
   *
   * Figure out who the caller meant.
   *
   * @parm string $val: how the user referred to the trader
   * @return: the trader's uid (NULL if not found)
   */
  function identify($val) {
    $val = strtolower(str_replace("'", '', $val));
    $sql = "SELECT u.uid FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
            WHERE '$val' IN (LCASE(CONCAT('.', REPLACE(u.name, ' ', ''))), u.credit_id, s.number, LCASE(u.website), u.phone, LCASE(u.mail))";
    $result = db_query($sql)->fetchCol();
    return isset($result[0]) ? $result[0] : '';
  }
 
} // end of SMS call class definition
