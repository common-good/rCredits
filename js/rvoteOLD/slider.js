//MODEL: <input id='slider1' class='CGslider orientation-horizontal target-display_slider size-200 position-100 stops-10' />

// CP variable additions
var maintaintop = false;
var lastsliderchanged = '';
var valueprefs = new Array();
var reacting = false;
var cgsliders = new Array();
var maxvalue99 = 100;
var maxvalue = 100;
var idealmaxratio = 1.2; // what should the maxvalue be, in relation to the highest value
var reactslider = false; // true = adjust other sliders to accommodate
var reacttext = true; // true = maintain text total of 100% if any slider is set

function setupSliders() {
	var nm;
	var inputs = CARPE.getElementsByClass(CGSlider.prototype.sliderClassName);
	for (var i in inputs) { // Loops through slider elements.
		cgsliders[cgsliders.length] = new CGSlider(inputs[i]); // Create the new slider object. // 20100321 CP
	}
	cgReactText(''); // CP
	return;
}

// The Slider class constructor: elmnt is a proper input element object or an id string.
// params is an object with the optional properties..
// 		orientation ('horizontal' or 'vertical'),
// 		value (number 0-1),
// 		position (number of pixels),
// 		size (number of pixels),
//		stops (number of stops that the slider can snap to),
// 		target (target element or object),
//		feedback (true or false, true means that snapping at the target controls snapping for the slider)
CGSlider = function(elmnt, params, location, style) {
	// The original (X)HTML slider element:
	if (elmnt) {
		if (typeof elmnt === 'string') {
			var id = elmnt;
			var actualElement = document.getElementById(elmnt) ? document.getElementById(elmnt) : null;
		} else if (elmnt.nodeType == 1) {
			if ((elmnt.tagName.toLowerCase() == 'input') || (elmnt.tagName.toLowerCase() == 'select')) {
				var actualElement = elmnt;
				var options = elmnt.options ? elmnt.options : null;
				var stops = elmnt.options ? elmnt.options.length : undefined;
			}
		}
	}
	// Create the element if it's not there.
	var startElmnt = actualElement ? actualElement : document.createElement('input');
	
	// The slider ID:
	if (!startElmnt.id && !id) {
		var id = 0,
			idBase = this.sliderClassName + this.idSeparator + this.idPrefix + this.idSeparator;
		while (document.getElementById(idBase + id)) id++; // Find a unique id.
		startElmnt.id = idBase + id;
	}
	this.id = startElmnt.id || id;
	
	// The name attribute needed to send values to server:
	this.name = startElmnt.name ? startElmnt.name : this.id;
	
	// The Slider Classname:
	if (startElmnt.className) {
		this.className = (startElmnt.className.indexOf(this.sliderClassName) == -1) ?
			(startElmnt.className + ' ' + this.sliderClassName) : startElmnt.className;
	} else {
		this.className = this.sliderClassName;
	}
	
	// Default values
	this.active = true; // CP
	this.unit = this.defaultUnit;
	this.hasSlit = true;
	this.orientation = this.defaultOrientation;
	this.position = this.defaultPosition;
	this.value = startElmnt.value ? parseFloat(startElmnt.value) : this.defaultValue;
	this.text = this.texts ? this.texts[startElmnt.selectedIndex] : "";
	this.from = this.defaultMinValue;
	this.to = this.defaultmaxvalue; // UNUSED (20100321 CP)
	this.sticky = false; // 20100526 CP
	this.stops = this.stops ? this.stops : this.defaultStops;
	this.size = startElmnt.style.width ? 
		parseInt(startElmnt.style.width, 10) : this.defaultSize;
		
	// Properties supplied as constructor arguments:
	if (params) { 
		this.orientation = params.orientation ? params.orientation : this.orientation;
		this.value = params.value ? parseFloat(params.value) : this.value;
		this.position = params.position ? parseInt(params.position, 10) : this.position;
		this.size = params.size ? parseInt(params.size, 10) : this.size;
		this.name = params.name ? params.name.toString() : this.name;
		this.stops = params.stops ? parseInt(params.stops, 10) : this.stops;
		if (params.target) {
			this.target = (typeof params.target == 'string') ?
				document.getElementById(params.target) : params.target;
		}
		this.feedback = !!params.feedback;
	}

	// Properties supplied by the class attribute:
	this.classNames = this.className ? this.className.split(' ') : [];
	for (var i = 0; i < this.classNames.length; i++) {
		var name = this.classNames[i].split(this.nameValueSeparator)[0];
		var val = this.classNames[i].substring(name.length + 1, this.classNames[i].length); 
		switch (name) {
		case 'orientation': this.orientation = val; break;
		case 'position':    this.position = parseInt(val, 10); break;
		case 'value':		this.value = parseFloat(val); break;
		case 'size':        this.size = parseInt(val, 10); break;
		case 'unit':        this.unit = (val == 'px' || val == 'mm' || val == 'em') ? val : this.unit; break;
		case 'slit':        this.hasSlit = (val == 'false' || val == 'no') ? false : this.hasSlit; break;
		case 'stops':       this.stops = parseInt(val, 10); break;
		case 'from':        this.from = parseInt(val, 10); break; // 20100319 CP
		case 'to':          this.to = parseInt(val, 10); break; // 20100319 CP
		case 'target':      this.target = document.getElementById(val) ?
			document.getElementById(val) : this.target; break;
		case 'feedback':    this.feedback = ((val === 'true') || (val === 'yes')) ? true : !!val; break;
		default: break;
		}
	}

	// Initial values for slider movement limitation and position:
	this.xMax = this.orientation == this.xOrientation ? this.size : 0;
	this.yMax = this.orientation == this.yOrientation ? this.size : 0;
	this.value = this.value >= maxvalue ? maxvalue : (this.value < this.from ? this.from : this.value);
	this.position = this.position > this.size ? this.size : this.position;
	if (this.value === this.defaultValue) {
		this.value = this.size > 0 ?
			(this.position / this.size) * (maxvalue - this.from) + this.from : maxvalue;
	} else {
		this.position = this.value <= maxvalue ? Math.round((this.value - this.from) /
			(maxvalue - this.from) * this.size) : this.size;
	}
	this.x = this.y = this.position;

	// Parent element and the slider's DOM location:
	if (startElmnt.parentNode) {
		this.parent = startElmnt.parentNode;
	} 
	else {
		this.parent = CARPE.defaultParentNode;
		if (location) {
			if (location.parent) { 
				this.parent = (typeof location.parent === 'string') ?
					document.getElementById(location.parent) : location.parent;
			} 
			if (location.before) {
				this.before = (typeof location.before === 'string') ?
					document.getElementById(location.before) : location.before;
					this.parent = this.before.parentNode;
			}
			if (location.after) {
				this.after = (typeof location.after === 'string') ?
					document.getElementById(location.after) : location.after;
					this.parent = this.after.parentNode;
			}
		}
		// The slider's location in the DOM
		if (!this.before && !this.after) {
			this.parent.appendChild(startElmnt);
		} else if (this.before) {
			this.parent.insertBefore(startElmnt, this.before);
		} else if (this.after) {
			var node = this.after.nextSibling;
			while (node.nodeType != 1) node = node.nextSibling;
			this.parent.insertBefore(startElmnt, node);
		}
	}
	
	// The new hidden value element:
	this.valueElmnt = document.createElement('input');
	this.valueElmnt.setAttribute('type', 'hidden');
	this.valueElmnt.setAttribute('name', this.name);
	this.valueElmnt.className = this.className;
	this.parent.insertBefore(this.valueElmnt, startElmnt);
	this.parent.removeChild(startElmnt);
	this.valueElmnt.id = this.id;
	this.valueElmnt.knob = this.knob;
	this.valueElmnt.panel = this.panel;
	this.valueElmnt.slit = this.slit;
	this.valueElmnt.setValue = this.setValue.bind(this);
	
	// The slider panel:
	this.panel = document.createElement('a');
	this.panel.setAttribute('href', 'javascript: void 0;');
	this.panel.style.cssText = startElmnt.style.cssText; // Copy styles from the user supplied input element.
	this.parent.insertBefore(this.panel, this.valueElmnt);
	this.panel.className = this.panelClassName + ' orientation-' + 
		this.orientation; // + ' target-' + this.target.id;
	this.panel.id = this.panelClassName + '-' + this.id;
	
	// The slider knob:
	this.knob = document.createElement('div');
	this.knob.className = this.knobClassName;
	this.knob.id = this.knobClassName + '-' + this.id;
	this.panel.appendChild(this.knob);
	while (!this.knob.width) {
		this.knob.width = parseInt(CARPE.getStyle(this.knob, 'width'), 10);
		window.setTimeout('', 100);
	}
	this.knob.height = parseInt(CARPE.getStyle(this.knob, 'height'), 10);
	if (this.orientation == this.xOrientation) {
		var width = this.size + this.knob.width;
		if (!window.opera) {
			width += parseInt(CARPE.getStyle(this.knob, 'border-left-width'), 10) +
			parseInt(CARPE.getStyle(this.knob, 'border-right-width'), 10);
		}
		this.panel.style.width = width + this.unit;
	} else {
		var height = this.size + this.knob.height;
		if (!window.opera) {
			height += parseInt(CARPE.getStyle(this.knob, 'border-top-width'), 10) +
			parseInt(CARPE.getStyle(this.knob, 'border-bottom-width'), 10);
		}
		this.panel.style.height = height + this.unit;
	}
	
	// The slider slit:
	if (this.hasSlit) {
		this.slit = document.createElement('div');
		this.slit.className = this.slitClassName;
		this.slit.id = this.slitClassName + '-' + this.id;
		this.panel.appendChild(this.slit);
		if (this.orientation == this.xOrientation) {
			this.slit.style.width = this.size + this.knob.width -
			parseInt(CARPE.getStyle(this.slit, 'border-left-width'), 10) -
			parseInt(CARPE.getStyle(this.slit, 'border-right-width'), 10) +
			this.unit;
			if (window.opera) {
				this.slit.style.width = parseInt(this.slit.style.width, 10) -
				parseInt(CARPE.getStyle(this.knob, 'border-left-width'), 10) -
				parseInt(CARPE.getStyle(this.knob, 'border-right-width'), 10) +
				this.unit;
			}
		}
		else {
			this.slit.style.height = this.size + this.knob.height -
			parseInt(CARPE.getStyle(this.slit, 'border-top-width'), 10) -
			parseInt(CARPE.getStyle(this.slit, 'border-bottom-width'), 10) +
			this.unit;
			if (window.opera) {
				this.slit.style.height = parseInt(this.slit.style.height, 10) -
				parseInt(CARPE.getStyle(this.knob, 'border-top-width'), 10) -
				parseInt(CARPE.getStyle(this.knob, 'border-bottom-width'), 10) +
				this.unit;
			}
		}
	}
	// Take care of the style argument.
	if (style) {
		for (var item in style) {
			if ((item == 'panel') || (item == 'knob') || (item == 'slit')) {
				for (var property in style[item]) {
					this[item].style[property] = style[item][property];
				}
			}
		}
	}
	
	// Event handlers:
	CARPE.addEventListener(this.knob, 'mousedown', this.slide.bind(this));
	CARPE.addEventListener(this.panel, 'mousedown', this.slideTo.bind(this));
	this.panel.onblur = this.makeBlurred.bind(this);
	if (window.opera) {
		this.panel.onkeypress = this.keyHandler.bind(this);
	} else {
		this.panel.onkeydown = this.keyHandler.bind(this);
	}
	
	// Move slider knob to initial position:
// cp 20100429 	this.moveToPos(this.position, true);
	this.update(); // cp 20100429
};

// The Slider class:
CGSlider.prototype = {
	defaultParentNode:  document.forms[0] ? document.forms[0] : document,
	defaultUnit:        'px',
	defaultPosition:	0,    // The initial position for the slider knob when no position is specified.
	defaultValue:		null, // The initial value for the hidden input element when no value is specified.
	defaultMinValue:	0,    // The minimum value for the hidden slider display.
	defaultmaxvalue:	1,    // The maximum value for the hidden slider display.
	defaultSize:        100,  // The distance a slider can be moved if no size is specified.
	defaultStops:       0,    // The number of stops that the slider snaps to if not specified.
							  // 0 means no snapping functionality.
	xOrientation:       'horizontal', // May be changed to make language versions of the script.
	yOrientation:       'vertical',   // May be changed to make language versions of the script.
	defaultOrientation: 'horizontal', // May be changed to alternate orientation string.
	nameValueSeparator:	'-',    // Separator used for name/value pairs sent by the class attribute.
	idPrefix:           'auto', // Prefix used for auto-generated slider element IDs.
	idSeparator:		'-',    // Separator string between id prefix and ID number.
	sliderClassName:    'CGslider',       // The class name for the silders.
	panelClassName:     'CGslider-panel', // CSS selector for the slider panel.
	slitClassName:      'CGslider-slit',  // CSS selector for the slider slit.
	knobClassName:      'CGslider-knob',  // CSS selector for the slider knob.

	// Class method 'makeFocused': handles added focus.
	makeFocused: function(evnt) {
		// this.slit.className = this.slitClassName + ' focus';
		// this.panel.focus();
		// this.temp = this.keyHandler.bind(this);
		// this.panel.onkeydown = this.temp;
	},
	// Class method 'makeBlurred': handles removed focus.
	makeBlurred: function(evnt) {
		if (this.hasSlit) {
			this.slit.className = this.slitClassName;
		}
		// this.panel.blur();
		// this.panel.onkeydown = null;
		return this;
	},
	// Class method 'keyHandler': handles arrow key input for slider.
	keyHandler: function(evnt) {
		evnt = evnt || window.event; // Get the key event.
		if (evnt) {
			var key = evnt.which || evnt.keyCode; // Get the key code.
			if ((key == CARPE.KEY_RIGHT) || (key == CARPE.KEY_UP)) { // Right or up.
				(this.orientation == this.xOrientation) ? this.moveInc(1) : this.moveInc(-1);
				return false;
			}
			else if ((key == CARPE.KEY_LEFT) || (key == CARPE.KEY_DOWN)) { // Left or down.
				(this.orientation == this.xOrientation) ? this.moveInc(-1) : this.moveInc(1);
				return false;
			}
		}
		return true;
	},
	// Class method 'moveInc': moves slider a number of steps (pixels if no 'steps' attribute is specified).
	moveInc: function(increment) {
		if (this.stops > 1) {
			return this.moveToPos(this.position + (this.size / (this.stops - 1)) * increment);
		} else {
			return this.moveToPos(this.position + increment);
		}
	},
	// Class method 'mouseUp': handles the end of the sliding process.
	mouseUp: function(evnt) {
		this.sliding = false; 
		if (this.stops > 1) { // Snap to allowed positions.
			this.moveToPos(parseInt(this.size * Math.round(this.position *
				(this.stops - 1) / this.size) / (this.stops - 1), 10));
		} else {
			this.updateTarget(this.feedback);
		}
		cgReactText('');
		CARPE.removeEventListener(document, 'mousemove', this.mouseMoveListener)
		CARPE.removeEventListener(document, 'mouseup', this.mouseUpListener)
		if (this.hasSlit) {
			this.slit.className = this.slitClassName;
		}
		this.panel.focus();
		return this;
	},
	// Class method 'slide': handles the start of the sliding process.
	slide: function(evnt) {
		evnt = evnt || window.evnt; // Get the mouse event causing the slider activation.
		CARPE.stopEvent(evnt);
		this.panel.focus();
		this.startOffsetX = this.x - evnt.screenX; // Horizontal slider-mouse offset at start of slide.
		this.startOffsetY = this.y - evnt.screenY; // Vertical slider-mouse offset at start of slide.
		this.sliding = true;
		this.mouseMoveListener = this.moveSlider.bind(this);
		CARPE.addEventListener(document, 'mousemove', this.mouseMoveListener); // Start the action if
			// the mouse is dragged.
		this.mouseUpListener = this.mouseUp.bind(this);
		CARPE.addEventListener(document, 'mouseup', this.mouseUpListener); // Stop sliding on mouseup.
		return true;
	},
	// Class method 'slideTo': handles an instant movement of the slider when user clicks on the panel.
	slideTo: function(evnt) {
		evnt = evnt || window.event; // Get the mouse event causing the slider activation.
		CARPE.stopEvent(evnt);
		if (this.orientation === this.xOrientation) { // Move slider to new horizontal position.
			this.moveToPos(evnt.clientX - (CARPE.getPos(this.knob).x -
				this.x + parseInt((this.knob.width / 2), 10) ));
		} else {
			this.moveToPos(evnt.clientY - (CARPE.getPos(this.knob).y -
				this.y + parseInt((this.knob.height / 2), 10) ));
		}
		this.slide(evnt);
		return true;
	},
	// Class method 'moveSlider': handles the movement of the slider while dragging.
	moveSlider: function(evnt) {
		evnt = evnt || window.event; // Get the mousemove event.
		if (this.sliding) { // Only if slider is being dragged.
			if (this.orientation == this.xOrientation) {
				this.moveToPos(this.startOffsetX + evnt.screenX, true); // Mouse position relative to
					// allowed slider position.
			} else {
				this.moveToPos(this.startOffsetY + evnt.screenY); // Mouse position relative to
					// allowed slider position.
			}
			return false;
		}
	},
	// Class method 'moveToPos': Moves the slider to a new pixel position and returns the slider object.
	moveToPos: function(position, preventTargetUpdate) {
		if (this.orientation == this.xOrientation) {
			this.x = (position > this.xMax) ? this.xMax : ((position < 0) ? 0 : position);
			this.value = this.x / this.size * (maxvalue - this.from) + this.from;
			this.position = CARPE.left(this.knob.id, this.x); // Move knob to new position, and
				// return new position.
		} else {
			this.y = (position > this.yMax) ? this.yMax : ((position < 0) ? 0 : position);
			this.value = 1 - (this.y / this.size * (maxvalue - this.from) + this.from);
			this.position = CARPE.top(this.knob.id, this.y); // Move knob to new position, and
				// return new position.
		}
		if(reactslider) this.cgReact(); // 20100319 CP
		if(reacttext) if(!preventTargetUpdate) cgReactText(''); // 20100319 CP

//		this.valueElmnt.value = this.value.toFixed(1); // Set the value of the hidden input slider display. // 20100319 CP
		this.text = this.texts ? this.texts[Math.round(this.value * (this.texts.length - 1))] : "";
//		if (this.target && !preventTargetUpdate) this.updateTarget();
		return this;
	},
	// Class method 'setValue': Sets value and positions knob accordingly.
	// Intended as a 'public' method for user scripts, and when a display element publishes a feedback.
	setValue: function(value, preventTargetUpdate) { // 20100321 CP
		value = (value > maxvalue) ? maxvalue : value; // 20100321 CP
		value = (value < this.from) ? this.from : value; // 20100321 CP
		return this.moveToPos(Math.round(value * this.size / (maxvalue - this.from), 10), preventTargetUpdate); // 20100321 CP
	},
	// Class method 'updateTarget': Sets the value for the target element.
	updateTarget: function(feedback) {
		if (this.target) {
			if (this.target.setValue) {
				this.target.setValue(this.valueElmnt.value);
			}
			else if (document.getElementById(this.target.id).setValue) {
					this.target = document.getElementById(this.target.id);
					this.target.setValue(this.valueElmnt.value);
					return this.updateTarget();
			}
			else this.target.value = this.valueElmnt.value;
		}
		return this;
	},
	// Class method 'update': Used only when the slider is a target for other form elements.
	update: function() {
		var value0 = this.value || this.target.value;
		this.setValue(value0, true); // cp 20100429
		this.value = this.target.value = parseFloat(value0); // don't lose the exact intended figure
		return this;
//		return this.setValue(this.value || this.valueElmnt.value, true);
	},

	// Class method 'kill': Removes the slider elements and the object.
	kill: function() {
		this.setValue('', true);
		for(i in cgsliders) if(i.id == this.id) this.splice(i,1); // remove from array
	}

};

	function cgReactText(si) {  // si only supplied when pct is changed directly (note that si can be 0, though)
		var that;
		var newvalue = (si == '') ? 0 : parseFloat(cgsliders[si].target.value);
//pr('si='+si+' newvalue='+newvalue);
		if(si != '') if(newvalue != cgsliders[si].target.value) alert('ERROR new='+newvalue+' old='+cgsliders[si]);
		var totalvalue = 0; // total value of the rest 
		var totalsticky = 0; // just the set ones
		var available = 100-newvalue; // amount available for the rest
		var dpy;
		var v;
		var allsticky = true;

		for (var i in cgsliders) {
			that = cgsliders[i];
			if(i != si) if(that.active) {
				v = (si == '') ? that.value : parseFloat(that.target.value);
				totalvalue += v; // get total value of the others
				if(that.sticky) totalsticky += v; else allsticky = false; // get total stickys among the others
			}
		}
//pr('available='+available+' totalsticky='+totalsticky+' si='+si+' newvalue='+newvalue+' totalvalue='+totalvalue);
		if((si == '') || (totalsticky + newvalue > 100) || (totalvalue <= totalsticky)) allsticky = true; // moving slider or over the top, so everything can move again
		if(!allsticky) {
			totalvalue -= totalsticky;
			available -= totalsticky;
		}
//pr('newvalue='+newvalue+' totalvalue='+totalvalue);
		for (var i in cgsliders) { // set displays to the right percentage
			that = cgsliders[i];
			if(allsticky) that.sticky = false; // if they're all sticky, any change makes them all unsticky again
			if(that.active && (i != si) && !that.sticky) { // don't change inactive/sticky sliders or newvalued slider (if any)
				v = (si == '') ? that.value : parseFloat(that.target.value);
				dpy = (totalvalue == 0) ? 0 : (v * available / totalvalue);
//pr('thatvalue='+that.value+' dpy='+dpy);
	//testpr('id='+that.id+' value='+that.value+' tot='+totalvalue+' dpy='+dpy);
				that.valueElmnt.value = dpy.toFixed(1); // Set the value of the percentage display.
//pr('thatelementvalue='+that.valueElmnt.value);
				that.updateTarget();
				if(si) that.setValue(that.valueElmnt.value, true); // set the slider too
			}
		}
	}

function pr(s) {
	var spot = document.getElementById("testoutput");
	spot.innerHTML = spot.innerHTML + '<br>' + s;
}
