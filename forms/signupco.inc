<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Prepare to open a company account.
 * This form creates an invite and redirects to signup, using that code.
 * @see also: formAgent
 */
function formSignupCo($form, &$sta) {
  $a = r\agent();
  if (!$a->cAdmin and !$a->can(B_MEMBER)) return w\go('', 'members only', 'ERR');
  
  w\focusOn('fullname');
  //t('an additional personal account that I will manage'), 
  $title = t('Open a Company Account');
  $relations = array(t('a company and I own at least 10% of it'), t('a company or organization whose accounts I manage,<br>but I am not a 10%-or-more owner'), t('neither of the above') . ($a->cAdmin ? '' : t(' (send an <%a>invitation</a> instead!)', '_a', atag('/community/invite'))));
  $relationPrompt = item(t('What is the new account for?'));
  $relation = radiosFld(t(''), '', dft(2), $relations);
//  $flows = array(t('neither way (recommended in most cases)'), t('Your personal account can cover the new account\'s debts and overdrafts.'), t('The new account can cover your personal account\'s debts and overdrafts.'), t('both ways'));
//  $flowPrompt = item(t('Which way can the credit flow, when you need it to?'));
//  $flow = radiosFld(t(''), '', required(0), $flows);
  $flow = hidFld(0);
  $submit = t('Open Account');
  return cgform(compact(ray('title relationPrompt relation flowPrompt flow submit')));
}

function formSignupCo_validate($form, &$sta) {
  extract(just('relation flow', $sta['input']));
  if ($relation == 2) say('insufficient relation', 'relation');
}

function formSignupCo_submit($form, &$sta) {
  global $signupArgs; // for testing
  $a = r\agent();
//  if ($a->id == 1) $a = r\acct(R_ADMIN_QID); // use admin's personal account

  extract(just('relation flow', $sta['input']));

  $employee = $relation < 2; // let signupCo info override this
  $personal = ($relation > 1);
  if (!$personal) {
    if ($a->cAdmin) $a = r\acct(); // let admins open company accounts on behalf of others
    extract(just('company companyPhone owner employee', $a->signupCo)); // predict co name
    if (!findCompany(@$company, @$companyPhone)) list ($fullName, $phone) = [@$company, @$companyPhone];
  }
  $owner = $relation == 0; // don't let signupCo info influence this

  $helper = $a->mainQid;
  $code = r\invite($a->email, $a->id);
  $flow += 0;

  $signupArgs = compact(ray('code personal owner employee flow helper fullName phone'));
  $args = http_build_query($signupArgs);
///  debug($args); return;
  if ($personal) \session_destroy();
  w\go('signup/' . $args);
}
