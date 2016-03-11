function findslider(i) {
	for(var si in cgsliders) if(cgsliders[si].id == 'input_slider' + i) return si;
}

function findslidernum(id) {
	return parseInt(id.replace('input_slider', ''));
}

function vetoclick(i) {
	var veto = document.getElementById('input_veto' + i);
	var sliderdiv = document.getElementById('sliderdiv' + i);
	var vetonotediv = document.getElementById('vetonotediv' + i);
	var si = findslider(i);
	if(veto.checked) {
		cgsliders[si].setValue(0, true);
		cgsliders[si].target.value = '';
		expand(-i);
	}
	vetonotediv.style.display = veto.checked ? 'block' : 'none';
	sliderdiv.style.visibility = veto.checked ? 'hidden' : 'visible';
	cgsliders[si].active = !veto.checked;
	if(veto.checked) cgReactText('');
}

function changepct(i) {
	var si = findslider(i);
	var newvalue = cgsliders[si].target.value;
	newvalue = Math.max(0,Math.min(100,parseFloat(newvalue.replace(/[^\d\.]/g, '')))); // remove all but digits and decimal points
	cgsliders[si].setValue(newvalue, true);
	cgReactText(si);
	cgsliders[si].target.value = newvalue.toFixed(1) + '%';
}

function handleEnter(e) {
//alert('top');
	if(!e) e = window.event;
	var code = ((e.charCode) && (e.keyCode==0)) ? e.charCode : e.keyCode;
	var isCR = (code == 13);
//alert('code='+code);
	var it = document.activeElement;
//alert('it.id='+it.id);
	var i = it.id.replace('input_option', ''); // was input_sliderdpy
//alert('i='+i);
	var t = it.id.replace('input_vetonote', '');
//alert('t='+t);
	if(t != it.id) return true; // must be a textarea
	if(!isCR) return true;
	if(i != it.id) changepct(i); // must be a pct
//alert('here');
	return (it.id == 'submitvote');
}

function load(optcount) {
	document.onkeypress = handleEnter;
	CARPE.Sliders.init();
	for(var i=0; i<optcount; i++) vetoclick(i); // show veto notes if appropriate
}

