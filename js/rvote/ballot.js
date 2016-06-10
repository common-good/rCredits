var letters = 'EDCBA';
var optTypes = 'BMR';
var opts = [];
$('.optRow').each(function() {
  opt = opti($(this).attr('id'));
  opts[opt] = new Opt(opt, $(this).find('.gliss input'));
});

/**
 * Construct an option for a Budget-type question.
 * @param int i: the (internal) option number
 * @param DOM element: jQuery input element for BootstrapSlider slider (or null)
 */
function Opt(i, slider) {
  this.i = i;
  this.veto = $('#edit-veto' + i);
	this.votenotediv = $('#votenotediv' + i);
  this.votenote = $('#votenotediv' + i + ' textarea');
  this.optdetail = $('#optdetail' + i);
  this.input = $('#edit-option' + i);
  this.max = $('#edit-max');
  this.grades = $('#grades' + i);
  
  var t, ti;
  for (ti = 0; ti < optTypes.length; ti++) {
    t = optTypes.charAt(ti);
    if ($('.ballot.q' + t).length > 0) this.type = t;
  }

  /**
   * Toggle between grades: X, X+, and X-.
   * @param DOMelement me: the DOM input element of the current option
   * @param int opti: the option number
   */
  this.nudgeGrade = function(me) {
    this.lastMod = now();
    this.clearVeto(); // clear veto, if grading
    var prevValue = this.grades.find('input:checked').val();
    if (prevValue == null) return; // no previous value, nothing to nudge

    var oneletter;
    var basevalue = Math.round(me.value);
    var oldsign = (Math.round(prevValue) == basevalue) ? me.value - basevalue : -.333; // pretend X- if new letter
    var baseletter = letters.substr(basevalue, 1);
    var letter = $('#g' + baseletter + this.i);

    this.resetGrades(); // reset all the letters before fixing this one
    newsign = (oldsign < 0 ? 0 : (oldsign == 0 ? .333 : (basevalue == 0 ? 0 : -.333))); // toggle sign (no E-)
    me.value = basevalue + newsign;
    letter.html(baseletter + (newsign < 0 ? '<sup><b>&ndash;</b></sup>' : (newsign == 0 ? '' : '<sup>+</sup>')));
  }
    
  /**
   * Reset all grade values to their unplused, unminused value.
   */
  this.resetGrades = function() {
    for (i=0; i<letters.length; i++) {
// fails (missing id) but not needed      $('#edit-option' + this.i + '_' + i).val(i); // restore start value
      oneletter = letters.substr(i, 1);
      $('#g' + oneletter + this.i).html(oneletter);
    }
  }

  this.vetoClick = function() {
    this.lastMod = now();
    var doVeto = this.veto.prop('checked');
    this.optdetail.toggle(doVeto);
    this.votenotediv.toggle(doVeto);
    this.votenote.prop('required', doVeto);
    if (doVeto) {
      if (this.grades.length > 0) {
        this.grades.find('input:checked').prop('checked', false);
        this.resetGrades();
      }
      if (this.type == 'B') this.setSlider(0);
      this.votenote.focus();
    }
  }

  this.clearVeto = function() {
    if (this.veto.prop('checked')) {
      this.veto.prop('checked', false);
      this.vetoClick();
    }
  }

  if (this.optdetail.val() == '') this.optdetail.find('.optdetailheader, #optdetailtext' + i).hide();
  
  if (slider.length > 0) {
    this.slider = slider.bootstrapSlider({
      tooltip: 'always',
      tooltip_split: this.type == 'R',
      formatter: function(v) {
        var me = opts[opti(this.id)]; // "this" is the slider, not the Opt object
        var pct = me == null ? this.percent : me.percent; // slider formats while constructing
        if (pct) v += '%'; else v = '$' + v.toLocaleString(); // maybe use accounting.js here
        return v;
      }
    });

    this.percent = (slider.bootstrapSlider('getAttribute', 'max') == 100);
    
    /**
     * Set a slider to a value (type == 'B' only)
     */
    this.setSlider = function(v) {
      v = Math.round(Math.max(0, Math.min(100, v)));
      this.slider.bootstrapSlider('setValue', v);
      this.input.val(v);
      this.lastMod = now();
    }

    /**
     * Handle a change in a slider setting.
     */
    this.slider.on('slideStop', function(slideEvt) {
      var me = opts[opti(this.id)]; // "this" is the slider, not the Opt object
      me.lastMod = now();
      me.clearVeto(); // clear veto, if grading
      var v = slideEvt.value;
      if (Array.isArray(v)) { // me.type == 'R'
        me.max.val(v[1]);
        me.input.val(v[0]);
      } else { // me.type == 'B'
        me.input.val(v);
        var over, oldest;
        while (over = optsTotal() - 100) {
          oldest = oldestOpt();
          if (oldest == null) break;
          oldest.setSlider(oldest.input.val() - over);
        }
      }
    });

  } //else this.type = this.grades.length > 0 ? 'M' : '?';

  this.vetoClick();
  this.lastMod = 0;
}

function opti(s) {return s.replace(/^\D+/g, '');}

function now() {return (new Date()).getTime();}

/**
 * Return the last opt modified (null if not all have been set)
 */
function oldestOpt() {
  var i, oldest;
  var oldness = now();
  for (i = 0; i < opts.length; i++) {
    if (opts[i].lastMod < oldness) {
      if (opts[i].lastMod == 0) return null;
      oldest = opts[i];
      oldness = oldest.lastMod;
    }
  }
  return oldest;
}

function optsTotal() {
  var i;
  var sum = 0;
  for (i = 0; i < opts.length; i++) {
    sum += Math.round(opts[i].input.val());
  }
  return sum;
}

function expand(i) { // if i<0 expand only
	var expandonly = (i<0);
	if(expandonly) i = -i;
	var img = byid('expand' + i);
	var detail = byid('optdetail' + i);
	var detailtext = byid('optdetailtext' + i);
	var expand = (img.src.indexOf('expand')>0 ? true : false);
	if(expand) {
		img.src=img.src.replace('expand', 'contract');
		img.alt='hide detail';
		img.style.visibility = 'visible';
//		if(expandonly) detailtext.style.height = (detailtext.style.height ? '' : '150px'); // vetoing (kludge for MSIE)
		detail.style.display = 'block';
	} else {
		if(expandonly) return;
		img.src=img.src.replace('contract', 'expand');
		img.alt='show detail';
		detail.style.display = 'none';
	}
	img.title = img.alt;
}

function goback(optcount, qtype) {
	var goback = byid('goback'); 
	if(!checkform(goback.form, optcount, qtype)) return false;
	goback.value = 1; 
alert(goback.value);
alert(goback.id);
	goback.form.submit();
}

function checkform(form, optcount, qtype) {
	var vetoi, vetonotei, gradei, checked;

	for(var opti=0, blanks=0, vetos=0; opti<optcount; opti++) {
		vetoi = byid('input_veto' + opti);
		vetonotei = byid('input_vetonote' + opti);
		if(vetoi.checked && (vetonotei.value.length < 2)) return formerr(vetonotei, 'If you veto an option, you have to say why.');

		if(qtype == 'M') {
			gradei = form.elements['option' + opti]; // can't use byid because this is a radio button set
			for(var gi=0, isblank=true; gi<gradei.length; gi++) if(gradei[gi].checked) isblank = false;
		} else {
			gradei = byid('input_option' + opti);
			isblank = (gradei == 0);
		}
		if(vetoi.checked) vetos++; else if(isblank) blanks++;
	}
	if(blanks) {
		var blankmsg = 'You left %d options ungraded. Okay?';
		blankmsg = blankmsg.replace('%d', blanks);
		if(qtype == 'B') blankmsg = blankmsg.replace('ungraded', 'unfunded');
		if(!confirm(blankmsg)) return false;
	}
	if(optcount - vetos < 0) { // this restriction is UNUSED
		var vetomsg = 'Too many vetos. We will count them as "E" instead. Okay?';
		if(!confirm(vetomsg)) return false;
	}
	return true;
}

function formerr(fld, msg) {
	alert(msg);
	fld.focus();
	return false;
}

function byid(s) {return document.getElementById(s);}

function fmtAmt(n) {return n.toFixed(2).replace(/(\d)(?=(\d{3})+\.)/g, '$1,');}
