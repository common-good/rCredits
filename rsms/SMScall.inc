<?php

/*
 * SMScall
 */
 
$GLOBALS['SMS commands'] = ''; // defined in $this->parse()
$GLOBALS['SMS messages'] = array(
  // reports
  "account info" => t("Your provisional balance is @balance (accurate only if your transactions as an Active Participant are like your INFO transactions). The current demand for rCredits is @demand."),
  "your account is ready" => t("Thank you! Your new provisional balance is @balance."),
  "if you were active" => t("If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant."),
  "transaction done" => t("@inaword: @amount @tofrom user#@other (@reward_type: @reward_amount). Your new provisional balance is @balance (If this had been a real transaction, your balance would be @amount @moreless). Transaction ID #@transaction_id"),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. What's your name?"),
  "what's your email?" => t("Welcome to rCredits, @name. Last question: What is your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // help messages
  'help for payment' => t("Payment examples: 12.34 to 413.772.0000 | 12.34 to wgabcd for cash | 12.34 to jdough@example.com for wine and cheese (Include a description ONLY if it's \"cash\" or if you are providing real goods and services.)"),
  'help for charge' => t("Charge examples: 12.34 from 413.772.0000 | 12.34 from wgabcd for cash | 12.34 from jdough@example.com for wine and cheese (Include a description ONLY if it's \"cash\" or if real goods and services are provided.)"),
  'help for undo' => t("Examples: undo | undo to | undo 413.772.0000 | undo from wgabcd"),
  'help for get' => t("Examples: get r 12.34 | get usd 12.34"),
  'help for information' => t("Example: information (tells you your balance and the current demand for rCredits)"),
  'help for help' => t('Examples: @commands. For example, type "help undo".'),
  
  // error messages
  'syntax' => t('Syntax error.'),
  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'),
  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your balance available to exchange for US Dollars is $0."),
  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"@param" is not a feature.'),
);

class SMScall {
  var $message;
  var $number;
  var $caller;
  var $command;
  var $parameters;

  function __construct($number, $message) {
    $this->number = $number;
    $this->message = $message;
    rcredits_log('SMSin', $message, $number);
  }

  function send($number, $message) {
    sms_send($number, $message);
    rcredits_log('SMSout', $message, $number);
  }

  /*
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
      catch (Exception $e) {
        return $e->getMessage(); // cannot return $e->message directly (protected)
      }
      return $this->rdo();
  }
  
  /*
   * Validate Caller
   *
   * Look up the calling number in the database to see who's calling.
   *
   * @throws Exception
   *   message is the error message to send the caller
   */
  function validate_caller() {
  //throw new Exception('test');
    if (!($this->caller = sms_user_get_uid($this->number))) { 
      rcredits_create_user($this->number);
      throw new Exception(rsms_t("what's your name?"));
    }
    if (!user_access('Transact via SMS', user_load($this->caller))) {
      // send the user an email here?
  //    throw new Exception(''); // user does not have permission to use the rCredits SMS interface (due to misbehavior?), so don't waste a call?
      throw new Exception(rsms_t('no SMS permission')); // be polite 
    }
    return $this;
  }

  /*
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
      'payment' => array('(?:(?:payment|give|send) )?%amount to %who(?: for %what)?', '(?:to|payment|give|send) %who %amount(?: for %what)?'),
      'charge' => array('(?:charge|invoice|take|chg)? %amount from %who(?: for %what)?', '(?:from|charge|invoice|chg) %who %amount(?: for %what)?'),
      'undo' => array('undo', 'undo %tofrom', 'undo %who', 'undo %tofrom %who'),
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
    throw new Exception(rsms_t('syntax') . ' ' . $this->do_help($intent));
  }

  /*
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

  /*
   * Help: give the caller a helpful message
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
        return rsms_t("help for $try", $help_subs);
      }
    }
    return rsms_t('not a feature|help for help', $help_subs);
  }

  function do_information() {
    return rsms_t('account info', rcredits_account_information($this->caller));
  }

  function do_get() {
    extract($this->parameters);
    $args = rcredits_account_information($this->caller);
    $args['@amount'] = rcredits_amount($amount);
    if ($rusd == 'r') {
      return rsms_t("can't cash incentives", $args);
    }
    if ($rusd == 'usd') {
      return rsms_t('if you were active', $args);
    }
    return rsms_t('get r/usd error');
  }

  function do_undo() {
    return t('The undo feature is not yet available.');
  }

  function do_charge() {
    return $this->do_transfer(TRUE);
  }

  function do_payment() {
    return $this->do_transfer(FALSE);
  }

  function do_transfer($isfrom) {
    extract($this->parameters); // amount, who, and what

    $other = _sms_validate_trader($who);
    if (!$other) {
      return t('Unknown user: @user ', array('@user' => $who)) . $this->do_help($isfrom ? 'charge' : 'payment');
    }
    if (!user_access('transact', user_load($other))) {
      return t('@name does not have permission to trade.', array('@name' => user_load($other)->name));
    }

    $payer = $isfrom ? $other : $this->caller;
    $payee = $isfrom ? $this->caller : $other;

    $rewards = array(FALSE => .05, TRUE => .10);
    $reward_me = $amount * $rewards[$isfrom];
    $reward_other = $amount * $rewards[!$isfrom]; // not yet used

    try {
      rcredits_transact($reward_me, RCREDITS_COMMUNITY_UID, $this->caller, $what); // credit sender with reward
    }
    catch (Exception $e) {
      return $e->getMessage();
    }
    
    $transaction_totals = transaction_totals($this->caller);
    $args = array(
      '@inaword' => $isfrom ? t('INFO charge') : t('INFO payment'), // RECEIVED and PAID, when not a mere INFO transaction
      '@amount' => rcredits_amount($amount),
      '@tofrom' => $isfrom ? t('from') : t('to'),
      '@reward_type' => $isfrom ? t('bonus') : t('rebate'),
      '@reward_amount' => rcredits_amount($reward_me),
      '@balance' => rcredits_amount($transaction_totals->balance),
      '@moreless' => $isfrom ? t('more') : t('less'),
      '@transaction_id' => 'ABC123',
      '@other' => $other,
    );
    return rsms_t('transaction done', $args);
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
  
} // end of SMScall class definition
