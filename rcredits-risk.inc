<?php
/**
 * @file
 * Calculation and management of fraud/money-laundering risks.
 */
 
namespace rCredits\Risk; // typically abbreviated as "k"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Testing as t;
use rCredits\Util as u;
use rCredits\Backend as be;
use rCredits\Risk as k;

cashin cashout p2p inhouse bysuspect tosuspect b2p absent invoiceless big often offline firstoffline shortpath suspectout

function calcTxRisk($tx) {
  extract($tx);
  $risk = 0; // start from scratch
  if (!$goods
}