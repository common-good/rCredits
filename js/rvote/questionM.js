function vetoclickM(i) {
	var checked = $('#edit-veto' + i).prop('checked');
	var grades = $('#grades' + i);
	var votenotediv = $('#votenotediv' + i);
	if (checked) expand(-i);
	votenotediv.toggle(checked);
	grades.css('visibility', checked ? 'hidden' : 'visible');
}

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