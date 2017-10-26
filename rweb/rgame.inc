<?php

define('INVITES_NEEDED', 3); // how many invitations (on average) before a person joins
define('GAME_MIN_ID', 4);
define('GAME_MAX_ID', 24);
define('GAME_GROCER_ID', 16);
define('GAME_CGF_ID', 6);

/*
To do:
. day 5, 6, and 7 (do inflation right by refactoring into cron-subs)
  - Players can also finance new or old companies, in whole or in part. Type company ID (buying price is 10,000 times company ID) and ownership percentage. New companies can even be created in the triangles between other companies. Their ID can be any unused positive decimal number. Their neighbors are the three companies at the points of the triangle.
  - Players can adjust g() parameters
*/

/**
 * Play the rCredits Game
 */
function formGame($form, &$sta, $args = '') {
  if (isPRODUCTION) return r\go('community', t('The game is not yet available.'));
  extract(just('ctty memberCo acct', $args)); // ctty recovers a stored game

  if (@$ctty == 'reset') {
    w\svar('ctty', '');
    db\q('TRUNCATE r_txs; TRUNCATE r_usd; DELETE FROM users WHERE uid<0 OR uid>1'); // temporary
  } else if ($id = @$memberCo) {
    $a = gameCompany($id, w\svar('ctty')); // ctty from svar not from args
    $not = $a->ok ? t(' NOT') : '';
    gameActivate($a, !$a->ok);
    say(t('Company #@id is now@not a Member Company.', ['@id'=>$id, '@not'=>$not]));
  }
  
  if (w\svar('ctty')) return formGame2(); // already playing
  
  $title = item(t('Play the rCredits Game'));
  $subtext = item(t('<p>Play this game to learn how the rCredits system works and how to create a Common Good Community. First download and print the <@a1>instructions</a> and <@a2>game board</a>.</p>', ['@a1' => 'a target=_blank href=https://docs.google.com/document/d/1RfdYmmz2jb99UzfppuhH_vBcg3fZ3denF_fH_wFFg9U/edit', '@a2' => 'a target=_blank href=http://commongoodfinance.org/files/rCreditsGameBoard.png']));
  $count = textField(t('How many players?'));
  $learn = boolField(t('Learning?'), t('Answer Yes if you want to play the special Week One "Learning the Game" round.'), FALSE);
  $submit = submit(t('Go'));
    
  return compact(u\ray('title subtext learn count submit'));  
}

function formGame2() {
  global $base_url;
  
  extract(just('week day ctty learn', gameVars()));
  if ($week > 0) {
    $title = item(t('<h2>Week @week, Day @day</h2>', $timeSubs = ['@week' => $week, '@day' => $day]));
    $subtext = item(t(<<<EOF
<ol>
  <li><b>Move @day space(s)</b>. Enter the number of your new location in the "Goto" column below. OR leave it blank to move to a random circle.</li>
  <li><b>Maybe invite someone</b>, for yourself and each of your followers. In the "New Followers" column, enter the IDs of any new followers you get, separated by spaces.<br>OR, if you want to automate your invitations to a certain number per day, type any decimal number between zero and one (inclusive).</li>
  <li><b>Maybe trade some US Dollars for rCredits</b>. Enter that amount in the "Trade" column or leave it blank. Type a <i>negative</i> number to trade your rCredits for US Dollars.<br>OR type a greater-than sign (">") and a number, to keep your personal rCredits balance at that amount or more (and the same for your followers).</li>
  <li><b>Buy something</b>, if possible.</li>
</ol>
<p>Press "Go" when you're ready.</p>
EOF
    , $timeSubs));
  $hint = item(<<<EOF
<div style="width:200px; padding:15px; border:5px solid darkgreen; background-color:white; position:relative; z-index:9999999; top:-700px; left:1000px;">
  <h1 style="margin:0;">1. Move<br>2. Invite<br>3. Trade $/r<br>4. Buy</h1>
</div>
EOF
  );

  } else {
    $title = item(t('The rCredits Game'));
    $subtext = item(t('Each player add four dice to get your ID. Enter each ID here and place your token there on the game board.'));
  }
  
  $q = db\q('SELECT uid,fullName,r,:IS_OK AS ok,theme AS id FROM users WHERE community=:ctty ORDER BY uid', compact('ctty'));
  $fields = u\ray('id fullName members usd r rewards goto newFollowers exchange');
  $heads = u\ray('ID,Player,Members,$,r,Rew,Goto,New Followers,Trade');
  for ($i = GAME_MIN_ID; $i <= GAME_MAX_ID; $i++) $fields[] = $heads[] = $i;

  $lines = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    list ($fullName) = explode('-', $fullName);
    $usd = number_format($a->usd);
    $r = number_format($r);
    $rewards = number_format($a->rewards);
    $members = $a->members ?: 0;
    if ($a->co) {
      if ($a->id < 0) { // allow grants to ctty
        if (!($week < 2 and @$day < 7)) $usd = "<input name=\"usd-$fullName\" value=\"$usd\">";
      } else $usd = '<span class="subdue">?</span>'; // USD not tracked for companies
      $class = $a->ok ? ' class="ok"' : '';
      $fullName = "<a$class href=\"$base_url/community/game/memberCo=$id\">$fullName</a>";
      $goto = $newFollowers = $exchange = '';
      if ($id == GAME_MIN_ID) $lines .= "<tr><td colspan=\"6\"></td><td colspan=\"" . GAME_MAX_ID . "\">
      <input type=\"button\" name=\"go\" value=\"Go\" onClick=\"this.form.submit();\">
      <input type=\"button\" name=\"reset\" value=\"Start over\" onClick=\"location.href='$base_url/community/game/ctty=reset';\">
      </td></tr>\n";
    } else {
      foreach (u\ray('goto newFollowers') as $k) $$k = "<input name=\"$k-$fullName\" />";
      $value = $a->minimum > 0 ? ('value=">' . number_format($a->minimum, 0) . '"') : '';
      $exchange = "<input name=\"exchange-$fullName\"$value />";
      foreach (u\ray('r usd') as $k) {$$k = "<input name=\"$k-$fullName\" value=\"${$k}\" />";}
      $legalName = strpos($a->legalName, '-') ? "<input name=\"legalName-$fullName\" />" : $a->legalName;
      if ($a->locus) $fullName .= " @ #$a->locus"; // show where each player is
      $fullName .= ": $legalName";
      if ($week < 1) $newFollowers = $exchange = ''; // setting up: just get IDs for players
      if ($week < 2 and $learn and @$day < 7) foreach (u\ray('r usd') as $k) $$k = number_format($a->$k); // not changeable yet
    }
    $line = '';
    foreach ($fields as $k) {
      if (is_numeric($k)) {
        $v = @$a->boughtAt[$k] ? (in_array($k, [GAME_GROCER_ID,20,21]) ? $a->boughtAt[$k] : '&check;') : '';
        $k = 'n'; // single class for all boughtAt fields
      } else $v = $$k;
      $line .= "<td class=\"game-$k\">$v</td>";
    }
    $lines .= "<tr>$line</tr>\n";
  }
  $header = '';
  $heads = array_combine($fields, $heads);
  foreach ($heads as $k => $v) { // also +USD to handle chance card
    if (is_numeric($k)) {
      $k = 'n'; // single class for all boughtAt fields
      $v = join('<br>', str_split($v));
    }
    $header .= "<th class=\"game-$k\">$v</th>";
  }
  $table = item("<table id=\"game-table\"><tr>$header</tr>$lines</table>\n");
//  $posttext = item('<span class="ok">*</span> Member Company');
  $reset2 = submit();
  $go2 = submit(); // these are hidden, but, by existing, let the go and reset buttons work
    
  return compact(u\ray('title subtext table posttext submit reset hint go2 reset2'));  
}

function formGame_validate($form, &$sta) {
  extract(just('count', $sta['input']));
  extract(just('ctty', gameVars()));
  
  if (!@$ctty and $err = u\badAmount($count, '>0', 0, 30)) return sayFieldErr($err, 'count');
  
  foreach ($sta['input'] as $k0 => $v) if (strpos($k0, '-')) { // compound params
    list ($k, $name) = explode('-', $k0);
    $a = gamePlayer($name, $ctty);
    if (($k == 'r' or $k == 'usd') and $err = u\badAmount($v, '', 0)) return sayFieldErr($err, $k0);
    if ($k == 'goto' and $v and $err = badCircle($v)) return sayFieldErr($err, $k0);
    if ($k == 'newFollowers') {
      $newFollowers = [];
      if (!$v) $v = $a->autoInvite;
      if (is_numeric($v) and $v > 0 and $v <= 1) { // request to automate followers
        for ($i = 0; $i < $a->members; $i++) {
          srand();
          if ((rand() / getrandmax()) <= $v and rand(1, INVITES_NEEDED) == 1) {
            srand();
            $newFollowers[] = rand(GAME_MIN_ID, GAME_MAX_ID);
          }
        }
      } elseif ($v) $newFollowers = u\ray($v);
      if (@$newFollowers) foreach ($newFollowers as $id) {
        if ($err = badCircle($id)) return sayFieldErr("$err ($id)", $k0);
        $coA = gameCompany($id, $ctty);
        $coMembers[$id] = (@$coMembers[$id] ?: $coA->members) + 1;
        if ($coMembers[$id] > $id) {
          if (is_numeric($v) and $v <= 1) {
            if ($coA->members < $id) say(tt('Circle #@id is now full.', compact('id')));
            $coMembers[$id] = $id;
          } else return say(t('Circle #@id has too many people.'), compact('id'), $k0);
        }
        $pNewMembers[$name] = @$pNewMembers[$name] + 1;
        $pNewMembersUSD[$name] = @$pNewMembersUSD[$name] + gameStartingUSD($id);
      }
    }
    if ($k == 'exchange' and $v) {
      if (substr($v, 0, 1) == '>') $v = substr($v, 1); // ignore the minimum symbol, for syntax testing
      if ($err = u\badAmount($v, '!=0', '0')) return sayFieldErr($err, $k0);
      $short = $v < 0 ? -$v - $a->r : $v - $a->usd;
// go into debt instead.      if ($short > 0) return sayFieldErr(t('Player @name is @short short for that exchange', ['@name' => $name, '@short' => u\fmtAmt($short, $v < 0 ? 'sr' : 's$')]), $k0);
    }
  }
  u\preray(compact(u\ray('pNewMembers pNewMembersUSD coMembers')), $sta['input']);
}

function gameStartingUSD($id, $learning = FALSE) {
  return $id < 10 ? ($learning ? 750 : 500) : ($id < 20 ? 5000 : 50000);
}

function gameCompany($id, $ctty) {
  if (!$uid = db\lookup('uid', 'users', ':IS_CO AND theme=:id AND community=:ctty', compact('id', 'ctty'))) return FALSE;
  return r\acct($uid);
}

function gamePlayer($name, $ctty) {
  if (!$uid = db\lookup('uid', 'users', 'fullName=CONCAT(:name, :ctty)', compact('name', 'ctty'))) return FALSE;
  return r\acct($uid);
}

function badCircle($id) {
  if ($err = u\badAmount($id, '>0', 0, '')) return $err;
  if ($id < GAME_MIN_ID or $id > GAME_MAX_ID) return tt('IDs must be between @GAME_MIN_ID and @GAME_MAX_ID, inclusive');
  return FALSE;
}

function gameVars() {
  foreach (u\ray('week day ctty learn') as $k) $res[$k] = w\svar($k);
  return $res;
}

function formGame_submit($form, &$sta) {
  extract($all = just('count learn pNewMembers pNewMembersUSD coMembers', $params = $sta['input']));
  if (@$count) return gameSetup($count, $learn);
  extract(just('week day ctty learn', gameVars()));

  foreach ($params as $k => $v) if (u\starts($k, 'goto')) {unset($params[$k]); $params[$k] = $v;} // reorder
  
  foreach ($params as $k0 => $v) if (strpos($k0, '-')) { // compound params
    list ($k, $name) = explode('-', $k0);
    $a = gamePlayer($name, $ctty);
    u\EXPECT($week > 0 or in_array($k, ['goto', 'legalName']), 'should be only goto or legalName');
    if ($k == 'legalName') {
      $a->update($k, $v);
    } elseif ($k == 'r' or $k == 'usd') {
      $a->update($k, str_replace(',', '', $v) + 0);
    } elseif ($k == 'newFollowers') {
      if ($v and is_numeric($v) and $v <= 1) $a->update('autoInvite', $v); // set latest auto-invite rate
      if (@$pNewMembers[$name]) {
        list ($n, $usd) = [$pNewMembers[$name], $pNewMembersUSD[$name]];
        $a->update('members usd toBona', $a->members + $n, $a->usd + $usd, $a->toBona + $n);
        sayPlayer($name, t(': @n new Followers got @usd Starting Money.', ['@usd'=>u\fmtAmt($usd), '@n'=>$n]));
      }
    } elseif ($k == 'exchange') {
      $v = str_replace(',', '', $v);
      if (substr($v, 0, 1) == '>') {
        $a->update('minimum', substr($v, 1));
        $v = ''; // no explicit trade
      }
      
      if ($v) {
        $a->update('minimum', 0); // if trading explicitly, stop automatic trades
      } elseif ($v = max(0, min($a->usd, $a->members * $a->minimum - $a->r))) { // shortfall (for autotrade)
        $way = $v < 0 ? t('rCredits for US Dollars') : t('US Dollars for rCredits');
        say(t('Player') . " $name " . t('(and Followers) exchanged') . " $v $way.");
      }
      
      if ($v) {
        $completed = r\rTime(); // pass by ref
        $a->bank(-$v, $completed);
        $a->update('usd', $a->usd - $v);
        $cttyA = $a->cttyA;
        $cttyA->update('usd r', $cttyA->usd + $v, $cttyA->r - $v);
        $v = u\fmtAmt(abs(@$v), 's');
      }
    } elseif ($k == 'goto') {
      if (!$locus = $v) { // new location and maybe ID
        if ($day < 7) $locus = rand(GAME_MIN_ID, GAME_MAX_ID); // no locus so randomize (go nowhere on day 7)
      }
      
      if ($week < 1) { // just starting
        $a->update('usd', $usd = gameStartingUSD($locus, $learn));
        sayPlayer($name, t('got @usd Starting Money.', ['@usd' => u\fmtAmt($usd)]));
        $a->update('theme locus', $locus, $locus);
      } elseif ($locus) { // visiting somewhere, buy something
        if (in_array($locus, [6,7,8,9]) and !gameCompany($locus, $a->community)->ok) {
          if ($week >= 1) sayPlayer($name, tt('visits #@locus, buys nothing.', compact('locus')));
        } else {
          $boughtAt = $a->boughtAt ?: [];
          $boughtAt[$locus] = @$boughtAt[$locus] + 1;
          $first = ($learn ? (@$week == 1 and @$day == 4) : !$a->can(B_BONA));
          sayPlayer($name, tt('buys at #@locus.', compact('locus')));
          gameBuy($a, $locus, $first ? arbitraryInviter($a) : '');
        }
        $a->update(compact(u\ray('locus boughtAt')));
      }
      if (@$day == 5) { // payday
        $amt = round($a->theme * $a->members * 100); // don't track individual follower payscales yet
        $a->update('usd', $a->usd + $amt);
        sayPlayer($name, t('and any Followers received @amt in pay (total).', ['@amt'=>u\fmtAmt($amt)]));
      } elseif (@$day == 7) { // day of rest (and inflation)
        $amt = round($a->r * .1);
        $a->update('r', $a->r + $amt);
        sayPlayer($name, t('(and Followers) received inflation adjustments of @amt (total).', ['@amt'=>u\fmtAmt($amt)]));
      }
      if ($a->usd < 0) $a->update('usd', floor($a->usd * 1.01)); // after last possible usd adjustment
    }
  }
  if (@$coMembers) foreach ($coMembers as $id => $v) {
    $coA = gameCompany($id, $ctty);
    $coA->update('members', $v);
    if (!$coA->ok and $coA->members >= .20 * $id and !in_array($id, [6,7,8,9])) {
      gameActivate($coA);
      say(t('Company #@id has 20% of its employees participating in rCredits, and is now a Member Company!', ['@id'=>$id]));
    }
  }
  
  if ($week == 1 and $day == 1) {
//    setGlobals(); // any player will do -- just to set the right community, to see stats
    w\svar('myid', $a->id); // this gets checked ONLY in setGlobals()
//    $extra = "acct=$a->id";
  }
  
  w\svar('day', $day = $day == 7 ? 1 : $day + 1);
  if ($day == 1) w\svar('week', $week + 1);
  
  gameStats();
  r\go('community/game/' . @$extra);
}

function gameActivate($a, $ok = TRUE) {
  $a->setBit(B_OK, $ok); $a->update('activated', $ok ? r\rTime() : 0);
}

function gameStats() {
  extract(just('week day ctty', gameVars()));
  $asof = r\rTime();
  $monthAgo = u\plusMonths(-1, $asof);
  $q = db\q('SELECT uid FROM users WHERE uid>0 AND community=:ctty', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    $uid = $row['uid'];
    r\averageBalance($uid, $monthAgo, $asof, TRUE, $minPastMo); // get past month's (week's actually) min
    a($uid)->update('picture', 100 * $minPastMo); // r\getStats uses this
  }
  r\getStats($ctty, $asof);
}

function arbitraryInviter($a) {
  $ctty = $a->community;
  $list = u\ray(db\lst('uid', 'users', 'community=:ctty AND NOT :IS_CO', compact('ctty')));
  unset($list[$a->id]); // can't invite yourself
  return $list ? r\acct($list[rand(0, count($list) - 1)]) : '';
}

function gameBuy($a, $id, $helper) {
  global $boxUser;
  $coA = gameCompany($id, $a->community);
  $amount = $id * 10 * $a->members;
  if ($rAmt = $coA->ok ? min($amount, $a->r) : 0) { // can we buy some with rCredits?
    $boxUser = $a->members; // special use of box field in transactions, for the game
    $info = u\ray('amount payer payee for goods', $rAmt, $a->id, $coA->id, 'what', R_FOR_GOODS);
    $x = new r\x($info);
    if ($helper) giveBonuses($a, $helper); // first rCredits purchase, give bonuses
    if ($a->toBona) {
      giveBonuses($a, $a, $a->toBona);
      $a->update('toBona', 0);
    }
  }
  
  if ($usdAmt = $amount - $rAmt) {
    if ($short = max(0, $usdAmt - $a->usd)) sayBorrow($a, ceil($short));
    u\EXPECT(abs(ceil($short) - $short) < .001, 'orphan cents');
    $a->update('usd', $a->usd - $usdAmt);
  }
}

function giveBonuses($a, $helper, $count = 1) {
  global $boxUser; $boxUser = $count;
  $a->setBit(B_BONA);
  be\fund($a->id, TX_SIGNUP, $count * R_SIGNUP_BONUS, 'signup');
  sayPlayer($a, $a == $helper ? t(': @n Followers got signup bonuses for making their first rCredits purchase.', ['@n' => $count]) : t('got a signup bonus.'));
  be\fund($helper->id, TX_HELPER, $count * R_HELPER_BONUS, 'invite');
  sayPlayer($helper, $a == $helper ? t('got inviter bonuses for inviting @n Followers who just made their first rCredits purchase.', ['@n' => $count]) : t('got an inviter bonus for inviting player @name.', ['@name' => substr($a->fullName, 0, 1)]));
}
  
function sayBorrow($a, $short) {    
  sayPlayer($a, t('has borrowed @short from a payday lender, with interest at 1% per day (payable daily, in advance, rounded up)', ['@short' => u\fmtAmt($short)]));
}

function sayPlayer($player, $msg) {
  if (is_object($player)) $player = substr($player->fullName, 0, 1);
  return say(t('Player') . " $player $msg");
}

function gameSetup($count, $learn) {
  w\svar('ctty', $ctty = -rand(2, R_REGION_MAX * R_REGION_MAX));
  w\svar('learn', $learn);
  w\svar('week', 0);
  w\svar('day', '');
  
  $companies = u\ray(GAME_COMPANIES);
  db\q('DELETE FROM users WHERE community=:ctty', compact('ctty'));
  db\q('DELETE FROM users WHERE community<>:ctty', ['ctty'=>r\serverUid()]);
  
  $flags = u\bit(B_CO) | u\bit(B_OK) | u\bit(B_CGC);
  $info = u\ray('uid community fullName members email flags', $ctty, $ctty, "Community$ctty", (GAME_MAX_ID+1-GAME_MIN_ID)*(GAME_MIN_ID+GAME_MAX_ID)/2, "ctty$ctty@example.com", $flags);
  if (!$a = new r\acct($info)) return say(t('cannot create community account'), 'zot');

  for ($i = 0; $i < $count; $i++) { // make players
    $fullName = u\n2a($i, -1) . "$ctty";
    $email = "$fullName@example.com";
    $info = u\ray('fullName community members flags email activated', $fullName, $ctty, 1, u\bit(B_OK), $email, r\rTime());
    if (!$a = new r\acct($info)) return say(t('cannot create player account for ') . $fullName, 'zot');
  }
  for ($id = GAME_MIN_ID; $id <= GAME_MAX_ID; $id++) { // make companies
    $fullName = $companies[$id] . "$ctty";
    $flags = u\bit(B_CO);
    if ($id == GAME_CGF_ID and @$cgfVersion) $fullName = "CGF$ctty";
    $community = $ctty;
    $members = 0;
    $email = "$fullName@example.com";
    $theme = $id;
    if (!$a = new r\acct(compact(u\ray('fullName community members flags email theme')))) return say(t('cannot create company account for ') . $fullName, 'zot');
    if ($id == GAME_GROCER_ID or ($id == GAME_CGF_ID and @$cgfVersion)) gameActivate($a);
  }
  return r\go('community/game');
}

