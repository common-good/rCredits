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
