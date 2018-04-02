var letters = 'EDCBA';
var optTypes = 'BMR';
var opti;
var opts = [];
$('.optRow').each(function() {
  opti = optI($(this).attr('id'));
  opts[opti] = new Opt(opti, $(this).find('.gliss input'));
  if (opts[opti].slider != null) opts[opti].slider.bootstrapSlider('refresh'); // rerun slider formatter
});

$('.optImg.expand').click(function () {expandOpt(this.attrib('index'));});
$('.grade-letter').mousedown(function () {opts[this.attrib('index')].nudgeGrade(this);}
$('.optRow .veto input[type="checkbox"]').change(function () {opts[this.attrib('index')].noteClick();});
$('.qdetailer').click(function () {$('#qdetails' + this.attr('index')).toggle();}

/**
 * Construct an option for a Budget-type question.
 * @param int i: the (internal) option number
 * @param DOM element: jQuery input element for BootstrapSlider slider (or null)
 */
function Opt(i, slider) {
  this.i = i;
  this.veto = $('#edit-veto' + i);
  this.note = $('#edit-note' + i);
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

  this.noteClick = function() {
    var doVeto = this.veto.prop('checked');
    if (!doVeto && this.votenote.val() != '') this.note.prop('checked', true);
    var show = doVeto || this.note.prop('checked');
    this.optdetail.toggle(show);
    this.votenotediv.toggle(show);
    this.votenote.prop('required', doVeto);
    if (doVeto) {
      if (this.grades.length > 0) {
        this.grades.find('input:checked').prop('checked', false);
        this.resetGrades();
      }
      if (this.type == 'B') this.setSlider(0); else this.lastMod = now();
      this.votenote.prop('placeholder', 'Reason for Veto: How is this option evil, immoral or unethical?');
    } else this.votenote.prop('placeholder', 'Comments / Suggestions for improvement');
    if (show) this.votenote.focus();
  }

  this.clearVeto = function() {
    if (this.veto.prop('checked')) {
      this.veto.prop('checked', false);
      this.noteClick();
    }
  }

  if (this.optdetail.val() == '') this.optdetail.find('.optdetailheader, #optdetailtext' + i).hide();
  
  if (slider.length > 0) {
    this.slider = slider.bootstrapSlider({
      tooltip: 'always',
      tooltip_split: this.type == 'R',
      formatter: function(v) {
        var me = opts[optI(this.id)]; // "this" is the slider, not the Opt object
        v = roundTo(v, 2);
        if (me != null && me.type == 'B') v += '%'; else v = v.toLocaleString(); // maybe use accounting.js here (don't add $)
        return v;
      }
    });
    
    /**
     * Set a slider to a value (type == 'B' only)
     */
    this.setSlider = function(v0, setTime) {
      v = Math.round(Math.max(0, Math.min(100, v0)));
      this.slider.bootstrapSlider('setValue', v);
      this.input.val(v);
      if (setTime == null || setTime || v != v0) this.lastMod = now();

      var over, oldestI;
      if (over = optsTotal() - 100) {
        oldestI = oldestOpt(this.i, over);
        if (oldestI >= 0) opts[oldestI].setSlider(opts[oldestI].input.val() - over, false);
      }
    }

    /**
     * Handle a change in a slider setting.
     */
    this.slider.on('slideStop', function(slideEvt) {
      var me = opts[optI(this.id)]; // "this" is the slider, not the Opt object
      me.lastMod = now();
      me.clearVeto(); // clear veto, if grading
      var v = slideEvt.value;
      if (Array.isArray(v)) { // me.type == 'R'
        me.max.val(v[1]);
        me.input.val(v[0]);
      } else me.setSlider(v); // me.type == 'B'
    });

  } //else this.type = this.grades.length > 0 ? 'M' : '?';

  this.noteClick();
  this.lastMod = 0;
}

function optI(s) {return s.replace(/^\D+/g, '');}

function now() {return (new Date()).getTime();}

/**
 * Return the last eligible opt modified (null if not all have been set)
 * @param Opt myI: index to an Opt to disqualify from being oldest (the current one)
 * @param int over: by how much we need to reduce something
 */
function oldestOpt(myI, over) {
  var i, oldest, v;
  var oldness = now();
  for (i = 0; i < opts.length; i++) {
    v = opts[i].input.val();
    if (i != myI && opts[i].lastMod < oldness && (over < 0 ? (v < 100) : (v > 0))) {
//      if (opts[i].lastMod == 0) return -1;
      oldest = i;
      oldness = opts[oldest].lastMod;
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

function expandOpt(i) { // if i<0 expand only
	var expandonly = (i<0);
	if(expandonly) i = -i;
	var img = byid('expand' + i);
	var detail = '#optdetail' + i;
	var detailtext = '#optdetailtext' + i;
	var expand = (img.src.indexOf('expand')>0 ? true : false);
	if(expand) {
		img.src=img.src.replace('expand', 'contract');
		img.alt='hide detail';
		img.style.visibility = 'visible';
//		if(expandonly) detailtext.style.height = (detailtext.style.height ? '' : '150px'); // vetoing (kludge for MSIE)
		$(detail).show();
    $(detailtext).show();
	} else {
		if(expandonly) return;
		img.src=img.src.replace('contract', 'expand');
		img.alt='show detail';
		$(detail).hide();
    $(detailtext).hide();
	}
	img.title = img.alt;
}

function roundTo(num, n) {
  if (num == 0) return 0;

  var absNum = num < 0 ? -num: num;
  var d = Math.ceil(Math.log(absNum)/Math.log(10)); // MSIE 11 does not support Math.log10()
  var power = n - d;

  var magnitude = Math.pow(10, power);
  var shifted = Math.round(num * magnitude);
  return shifted / magnitude;
}

function byid(s) {return document.getElementById(s);}
function fmtAmt(n) {return n.toFixed(2).replace(/(\d)(?=(\d{3})+\.)/g, '$1,');}
