/**
 * @file
 * javascript for the bottom of every page
 */

var vs = parseUrlQuery($('#script-scraps').attr('src').replace(/^[^\?]+\??/,'').replace('&amp;', '&'));
args = decodeURIComponent(vs['args']);
//alert(args);
args = JSON.parse(args);
for (var what in args) doit(what, parseUrlQuery(args[what]));
 
function doit(what, vs) {
  function fid(field) {return '#edit-' + vs[field].toLowerCase();}
  function fform(fid) {return $(fid).parents('form:first');}

  switch(what) {

    case 'funding-criteria': jQuery('.critLink').click(function () {jQuery('#criteria').modal('show');}); break;
    case 'download': window.open(vs['url'] + '&download=1', 'download'); break;

    case 'chimp':
      var imp = $('.form-item-chimpSet');
      $('#edit-chimp-1').click(function() {imp.hide();});
      $('#edit-chimp-0').click(function() {imp.show();});
      break;

    case 'get-ssn': get('ssn', {}, function () {}); break;
    
    case 'deposits':
      $('.filename').click(function () {
        var area = document.createElement('textarea');
        area.value = $(this).attr('data-flnm');
        document.body.appendChild(area);
        area.select();
        alert(document.execCommand('copy') ? 'filename copied to clipboard' : 'copy to clipboard failed');
      });
      break;

    case 'summary':
      $('#activate-credit').click(function () {
        post('setBit', {bit:'debt', on:1}, function (j) {
          $.alert(j.message, 'Success');
        });
      });
      break;

    case 'change-ctty':
      $('#edit-community').on('change', function() {
        var newCtty = this.value;
        changeCtty(newCtty, false);
      });

      function changeCtty(newCtty, retro) {
        post('changeCtty', {newCtty:newCtty, retro:retro}, function(j) {
      //    var jo = JSON.parse(j);
          if (!j.ok) $.alert(j.message, 'Error');
        });
      }
      break;

    case 'focus-on': $('#edit-' + vs['field']).focus(); break;
    
    case 'advanced-dates':
      if (!vs['showingAdv']) showAdv();
      $('#showAdvanced').click(function () {showAdv();});
      function showAdv() {jQuery('#advanced').show(); jQuery('#simple').hide();}
      $('#edit-period').change(function () {
        var id='#edit-submitPeriod'; if (!$(id).is(':visible')) id='#edit-downloadPeriod'; $(id).click();
      });
      $('#showSimple').click(function () {jQuery('#advanced').hide(); jQuery('#simple').show();});
      break;
      
    case 'paginate':
      $('#txlist #txs-links .showMore a').click(function () {showMore(0.1);});
      $('#txlist #txs-links .dates a').click(function () {
        $('#dateRange, #edit-submitPeriod, #edit-submitDates').show(); $('#edit-downloadPeriod, #edit-downloadDates, #edit-downloadMsg').hide();
      });
      $('#txlist #txs-links .download a').click(function () {
        $('#dateRange, #edit-downloadPeriod, #edit-downloadDates, #edit-downloadMsg').show(); $('#edit-submitPeriod, #edit-submitDates').hide();
      });
      $('#txlist #txs-links a.prevPage').click(function () {showPage(-1);});
      $('#txlist #txs-links a.nextPage').click(function () {showPage(+1);});
      break;
      
    case 'reverse-tx':
      $('.txRow .buttons a[title="' + vs['title'] + '"]').click(function () {
        var url = this.href;
        yesno(vs['msg'], function () {location.href=url;}); 
        return false;
      });
      break;

    case 'addr':
      print_country(vs['country'], vs['state'], vs['state2']);
      $('#frm-signup, #frm-contact').submit(function() {
        $('#edit-hidcountry').val($('#edit-country').val());
        $('#edit-hidstate').val($('#edit-state').val());
        $('#edit-hidstate2').val($('#edit-state2').val());
      });
      $('.form-item-country select').change(function() {
        print_state(this.options[this.selectedIndex].value,'','state');
        print_state(this.options[this.selectedIndex].value,'','state2');
      });
      $('.form-item-sameAddr input[type="checkbox"]').change(function () {setPostalAddr(true);});
      break;

    case 'legal-name':
      $('edit-fullname').change(function () {
        var legal=jQuery('#edit-legalname'); 
        if (legal.val()=='') legal.val(this.value);
      });
      break;
      
    case 'which':
      var fid = fid('field');
//      if ($(fid).val() == '') break; // don't suggest everyone
      var form = fform(fid);
//      this.form.elements[vs['field']].value=this.options[this.selectedIndex].text;
      $('#which').modal('show');
      break;

    case 'suggest-who':
      var fid = fid('field');
      var form = fform(fid);
      suggestWho(fid, vs['coOnly']);
      $(fid).focus(); // must be after suggestWho
      form.submit(function (e) {
        if ($(fid).val() == '') return true; // field is not required if we're here, so accept empty val
        return who(form, fid, vs['question'], vs['amount'] || $('input[name=amount]', form).val(), vs['allowNonmember'], vs['coOnly']);
      });
      
      function suggestWho(sel, coOnly) {
        var members = new Bloodhound({
        //  datumTokenizer: Bloodhound.tokenizers.obj.whitespace('value'),
          datumTokenizer: Bloodhound.tokenizers.whitespace,
          queryTokenizer: Bloodhound.tokenizers.whitespace,
          prefetch: {
            url: ajaxUrl + '?op=typeWho&data=' + coOnly + '&sid=' + ajaxSid,
            cache: false
          }
        });

        $(sel).wrap('<div></div>').typeahead(
          {
            minLength: 3,
            highlight: true
          },
          {
            name: 'rMembers',
        //    display: 'value',
            source: members
          }
        );
      }
      
      break;

    case 'invest-proposal':
      $('#add-co').click(function () {
        $('.form-item-fullName, .form-item-city, .form-item-serviceArea, .form-item-dob, .form-item-gross, .form-item-bizCats').show();
        $('#edit-fullname, #edit-city, #edit-servicearea, #edit-dob, #edit-gross, #edit-bizcats').attr('required', 'yes');
        $('.form-item-company').hide();
        $('#edit-company').removeAttr('required');
        $('#edit-fullname').focus();
      });
      break;
      
    case 'on-submit':
      var formid = '#rcreditsweb' + vs[caller];
      switch (caller) {
        case '': $(formid).submit(function() {}); break;
      }
      break;

    case 'advanced-prefs':
      toggleFields(vs['advancedFields'], false);
      $('#edit-showAdvancet').click(function() { $(this).hide(); toggleFields(vs['advancedFields'], true); });
      break;

    case 'bank-prefs':
      function showBank(show) {
        $('#connectFields2').toggle(show);
        $('#edit-routingnumber, #edit-bankaccount, #edit-bankaccount2, #edit-refills-0, #edit-refills-1').attr('required', show);
        var text = show ? vs['connectLabel'] : vs['saveLabel'];
        $('#edit-submit').val(text);
        $('#edit-submit .ladda-label').html(text);
      }
      if ($('#edit-connect-1')[0]) {
        showBank($('#edit-connect-1').attr('checked') == 'checked');
        $('#edit-connect-0').click(function() {showBank(false);});
        $('#edit-connect-1').click(function() {showBank(true);});
      }

      function showTarget(show) {
        $('#targetFields2').toggle(show);
        $('#edit-target, #edit-achmin').attr('required', show);
      }
      showTarget($('#edit-refills-1').attr('checked') == 'checked');
      $('#edit-refills-0').click(function() {showTarget(false);});
      $('#edit-refills-1').click(function() {
        showTarget(true); 
        if ($('#edit-target').val() == '$0') $('#edit-target').val('$' + vs['mindft']);
      });
      break;

    case 'signup':
      var form = $('#frm-signup');
      if (vs['clarify'] !== 'undefined') $('#edit-forother a').click(function () {alert(vs['clarify']);});
      form.submit(function (e) {return setPostalAddr(false);});
      break;

    case 'prejoint': $('#edit-old-0').click(function() {this.form.submit();}); break;

    case 'invite-link': $('#inviteLink').click(function () {SelectText(this.id);}); break;

    case 'gift':
      var other = jQuery('.form-item-amount'); 
      var gift = jQuery('#edit-gift');
      if (gift.val() == -1) other.show(); else other.hide();
      
      gift.change(function () {
        if(gift.val() == -1) {
          other.show(); 
          jQuery('#edit-amount').focus();
        } else other.hide();
      });
      
      $('#edit-amount').change(function () {
        if ($(this).val() == 0) $('#edit-often').val('Y');
      });
      break;
 
    case 'contact':
      var form = $('#frm-contact');
      $('#edit-fullname', form).focus();
      $('#edit-email', form).change(function () {$('.form-item-pass').show();}); // currently fails because no pw field
      form.submit(function (e) {return setPostalAddr(false);});
      break;

    case 'veto':
      $('.veto .checkbox input').change(function () {
        var opti = this.name.substring(4);
        opts[opti].noteClick();
      });
      break;
      
    case 'back-button': $('.btn-back').click(function () {history.go(-1); return false;}); break;
    
    case 'tickle': 
      $('.tickle').click(function () {
        var tickle = $(this).attr('tickle');
        if (tickle != 'NONE') $('#edit-tickle').val(tickle);
        //fform(this).submit();
        $('#edit-submit').click();
      });
      break;
      
    case 'coupons':
      $('#edit-automatic-0').click(function() {
        $('.form-item-automatic').hide();
        var min = $('#edit-minimum').val();
        $('.form-item-on').show();
        $('#edit-on').val(min > 0 ? 'your purchase of $' + min + ' or more' : 'any purchase');
      });
      break;
      
    case 'dispute':
      $('#dispute-it').click(function () {
        $('#denySet').show(); 
        $('.form-item-pay').hide();
      });
      break;

    case 'followup-email':
      $('#email-link').click(function () {
        var L = $(this);
        L.html('<h3 style="color:red;">Press Ctrl-C, Enter (then Ctrl-V in email)</h3>');
        var d=$('#email-details');
        d.attr('tabindex', '99'); // oddly, Chrome requires this for keydown
        d.attr('class', ''); // show
        d.focus();
        SelectText(d[0].id);
        d.bind('keydown', function(event) {
          if (event.keyCode!=13) return;
          $('#email-do')[0].click();
          d.attr('class', 'collapse'); // hide
          L.html('email');
        });
      });
      break;

    case 'invoices':
      $('#txlist tr td').not('#txlist tr td:last-child').click(function () {
        var nvid = $(this).siblings().first().html();
        location.href = baseUrl + '/handle-invoice/nvid=' + nvid + vs['args'];
      });
      break;
      
/*    case 'relations':
      $('div.checkbox').click(function() {
        var box = $('input', this);
        alert(box.prop('checked'));
        //box.prop('checked', !box.prop('checked'));
      });
      break;*/
    
    default:
      alert('ERROR: there is no default script.');
      alert($('#script-scraps').attr('src').replace(/^[^\?]+\??/,''));
      
  }
}
