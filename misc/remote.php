<?php
echo <<<EOF
<script>
function setAction(form) {
  var i=form.id;
  //var dot=i.value.indexOf(".");

//  if (dot>0) {
//    form.action="http://"+i.value.substr(0,dot)+".rcredits.org/signin";
    form.action="http://new.rcredits.org/signin";
    form.action="http://localhost/rMembers/signin";
    return true;
  /*} else {
    alert("You must sign in with your complete username (for example xyz.janedough) or your complete account ID (for example xyz.aaa), where \"xyz\" is your region abbreviation.");
    i.focus();
    return false;
  }*/
}
</script>

<style>
.auth-form{width:200px;margin:60px auto}
.auth-form form{border-radius:4px;box-shadow:0 1px 3px rgba(0,0,0,0.075)}
.auth-form-header{position:relative;padding:10px 20px;margin:0;color:#fff;text-shadow:0 -1px 0 rgba(0,0,0,0.5);background-color:#6c8393;background-image:-moz-linear-gradient(#7f95a5, #6c8393);background-image:-webkit-linear-gradient(#7f95a5, #6c8393);background-image:linear-gradient(#7f95a5, #6c8393);background-repeat:repeat-x;border:1px solid #6e8290;border-bottom-color:#586873;border-radius:4px 4px 0 0}
.auth-form-header h1{margin-top:0;margin-bottom:0;font-size:16px}
.auth-form-header h1 a{color:#fff}
.auth-form-header .octicon{position:absolute;right:0;top:12px;right:10px;color:rgba(0,0,0,0.4);text-shadow:0 1px 0 rgba(255,255,255,0.1)}
.auth-form-body{padding:20px;font-size:14px;background-color:#fff;border:1px solid #d8dee2;border-top-color:white;border-radius:0 0 4px 4px}
.auth-form-body .input-block{margin-top:5px;margin-bottom:15px}
.auth-form-body p.small_notice{display:inline;padding:0 10px}
.auth-form-subheading{margin:0}
.auth-form-body p{margin:0 0 10px}
.auth-form-permissions{padding-bottom:20px;margin-bottom:20px;border-bottom:1px solid #d8dee2}
.auth-form-permissions li{list-style-position:inside;padding-left:15px}
.auth-form .note{margin:15px 0;color:#777;text-align:center}

.button:focus {outline:none;text-decoration:none;border-color:#51a7e8;box-shadow:0 0 5px rgba(81,167,232,0.5)}
.button:hover, .button:active {text-decoration:none; background-color:#dddddd; background-image:-moz-linear-gradient(#eee, #ddd); background-image:-webkit-linear-gradient(#eee, #ddd);background-image:linear-gradient(#eee, #ddd);background-repeat:repeat-x;border-color:#ccc}
.button:active,.button.selected,.button.selected:hover {background-color:#dcdcdc; background-image:none; border-color:#b5b5b5; box-shadow:inset 0 2px 4px rgba(0,0,0,0.15)}

#login {z-index:2; display:none;}
#login label {display:block;}
#login input {display:block;}

</style>

<div class="auth-form" id="login">
  <form accept-charset="UTF-8" action="" method="post" onsubmit="return setAction(this);">
  <div class="auth-form-header"><h1>Sign in</h1></div>
    <div class="auth-form-body">
      <label for="login_field">Username/ID:</label>
      <input autocapitalize="off" autocorrect="off" autofocus="autofocus" class="input-block" id="login_field" name="id" tabindex="1" type="text" />
      <label for="pw">
        Password: (<a href="http://new.rcredits.org/account/password">problems</a>?)
      </label>
      <input class="input-block" id="password" name="pw" tabindex="2" type="password" />
      <input class="button" name="commit" tabindex="3" type="submit" value="Sign in" />
    </div>
  </form>
</div>

EOF;
/*
<form method="POST" action="" onsubmit="return setAction(this);">
Sign in:
<input type="text" name="id" value="abeone" autofocus />
<input type="password" name="pw" value="123" />
<input type="submit" value="Go" />
</table>
</form>
*/