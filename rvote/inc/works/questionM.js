function vetoclick(i) {
	var veto = document.getElementById('input_veto' + i);
	var grades = document.getElementById('grades' + i);
	var vetonotediv = document.getElementById('vetonotediv' + i);
	if(veto.checked) expand(-i);
	vetonotediv.style.display = veto.checked ? 'block' : 'none';
	grades.style.visibility = veto.checked ? 'hidden' : 'visible';
}

function load(optcount) {
	for(var i=0; i<optcount; i++) vetoclick(i); // show veto notes if appropriate
}