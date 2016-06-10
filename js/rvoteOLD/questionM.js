function vetoclickM(i) {
	var veto = document.getElementById('input_veto' + i);
	var grades = document.getElementById('grades' + i);
	var vetonotediv = document.getElementById('vetonotediv' + i);
	if(veto.checked) expand(-i);
	vetonotediv.style.display = veto.checked ? 'block' : 'none';
	grades.style.visibility = veto.checked ? 'hidden' : 'visible';
}

function nudgegrade(me,opti) {
	var one, oneletter;
	var letters = 'EDCBA';
	var oldvalue = me.value;
	var basevalue = Math.round(oldvalue);
	var oldsign = oldvalue - basevalue; // -.33, 0, or .33
	var baseletter = letters.substr(basevalue, 1);
	var letter = document.getElementById('g' + baseletter + opti);

	for(i=0; i<letters.length; i++) { // reset all the letters before fixing this one
		one = document.getElementById('input_option' + opti + '_' + i);
		one.value = i - 0.1; // restore start value (fakes minus, so next click is signless)
		oneletter = letters.substr(i, 1);
		one = document.getElementById('g' + oneletter + opti);
		one.innerHTML = oneletter;
	}
	newsign = (oldsign < 0 ? 0 : (oldsign == 0 ? .333 : (basevalue == 0 ? 0 : -.333))); // toggle sign (no E-)
	me.value = basevalue + newsign;
	letter.innerHTML = baseletter + (newsign < 0 ? '<sup><b>&ndash;</b></sup>' : (newsign == 0 ? '' : '<sup>+</sup>'));
}

function loadM(optcount) {
	for(var i=0; i<optcount; i++) vetoclickM(i); // show veto notes if appropriate
}