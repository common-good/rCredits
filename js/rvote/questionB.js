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

  if (slider != null) {
    this.slider = slider.bootstrapSlider({
      formatter: function(v) {return v + '%';},
      tooltip: 'always'
    });
    this.slider.on('slideStop', function(slideEvt) {
      opts[i].input.val(slideEvt.value); // "this" fails here
    });
  }
  
  this.vetoClick = function() {
    var doVeto = this.veto.prop('checked');
    this.optdetail.toggle(doVeto);
    this.votenotediv.toggle(doVeto);
    if (doVeto) {
      if (slider != null) this.slider.bootstrapSlider('setValue', 0);
      this.input.val(0);
      this.votenote.focus();
    }
  }
  this.vetoClick();
}

/**
 * Toggle between grades: X, X+, and X-.
 * @param DOMelement me: the DOM input element of the current option
 * @param int opti: the option number
 */
function nudgegrade(me, opti) {
  var prevValue = $('#grades0').find('input:checked').val();
  if (prevValue == null) return; // no previous value, nothing to nudge

	var oneletter;
	var letters = 'EDCBA';
	var basevalue = Math.round(me.value);
	var oldsign = (Math.round(prevValue) == basevalue) ? me.value - basevalue : -.333; // pretend X- if new letter
	var baseletter = letters.substr(basevalue, 1);
	var letter = $('#g' + baseletter + opti);

	for(i=0; i<letters.length; i++) { // reset all the letters before fixing this one
		$('#edit-option' + opti + '_' + i).val(i); // restore start value
		oneletter = letters.substr(i, 1);
		$('#g' + oneletter + opti).html(oneletter);
	}
	newsign = (oldsign < 0 ? 0 : (oldsign == 0 ? .333 : (basevalue == 0 ? 0 : -.333))); // toggle sign (no E-)
	me.value = basevalue + newsign;
	letter.html(baseletter + (newsign < 0 ? '<sup><b>&ndash;</b></sup>' : (newsign == 0 ? '' : '<sup>+</sup>')));
}

function loadM(optcount) {
	for (var i=0; i<optcount; i++) vetoclickM(i); // show veto notes if appropriate
}