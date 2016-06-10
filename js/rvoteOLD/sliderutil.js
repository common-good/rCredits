//---------------------------------+
//  CARPE                     0.2  |
//  2007 - 02 - 26                 |
//  By Tom Hermansson Snickars     |
//  Copyright CARPE Design         |
//  http://carpe.ambiprospect.com/ |
//  Contact for custom scripts     |
//  or implementation help.        |
//---------------------------------+

// 
var CARPE = {
	version: '0.1',
	KEY_LEFT: 37,
	KEY_UP: 38,
	KEY_RIGHT: 39,
	KEY_DOWN: 40,
	bind: function(obj) {
		var method = this,
		temp = function() {
			return method.apply(obj, arguments);
		};
	return temp;
	},
	// getElementsByClass: Cross-browser function that returns
	// an array with all elements that have a class attribute that
	// contains className
	getElementsByClass: function(className) {
		var classElements = [];
		var els = document.getElementsByTagName("*");
		var pattern = new RegExp("(^|\\s)" + className + "(\\s|$)"); //   "\\s" + className + "\\s")
		for (var i = 0, j = 0; i < els.length; i++) {
			if (pattern.test(els[i].className) ) {
				classElements[j] = els[i];
				j++;
			}
		}
		return classElements;
	},
	// left: Cross-browser version of "element.style.left"
	// Returns or sets the horizontal position of an element.
	left: function(elmnt, pos) {
		if (!(elmnt = document.getElementById(elmnt))) {
			return 0;
		}
		if (elmnt.style && (typeof(elmnt.style.left) == 'string')) {
			if (typeof(pos) == 'number') {
				elmnt.style.left = pos + 'px';
			}
			else {
				pos = parseInt(elmnt.style.left, 10);
				if (isNaN(pos)) {
					pos = 0;
				}
			}
		}
		else if (elmnt.style && elmnt.style.pixelLeft) {
			if (typeof(pos) == 'number') { elmnt.style.pixelLeft = pos; }
			else { pos = elmnt.style.pixelLeft; }
		}
		return pos;
	},
	// top: Cross-browser version of "element.style.top"
	// Returns or sets the vertical position of an element.
	top: function(elmnt, pos) {
		if (!(elmnt = document.getElementById(elmnt))) {
			return 0;
		}
		if (elmnt.style && (typeof(elmnt.style.top) == 'string')) {
			if (typeof(pos) == 'number') {
				elmnt.style.top = pos + 'px';
			}
			else {
				pos = parseInt(elmnt.style.top, 10);
				if (isNaN(pos)) {
					pos = 0;
				}
			}
		}
		else if (elmnt.style && elmnt.style.pixelTop) {
			if (typeof(pos) == 'number') {
				elmnt.style.pixelTop = pos;
			}
			else {
				pos = elmnt.style.pixelTop;
			}
		}
		return pos;
	},
	getPos: function (obj) {
		var curleft = 0;
		var curtop = 0;
		if (obj.offsetParent) {
			curleft = obj.offsetLeft;
			curtop = obj.offsetTop;
			while ((obj = obj.offsetParent)) {
				curleft += obj.offsetLeft;
				curtop += obj.offsetTop;
			}
		}
		return { x: curleft, y: curtop };
	},
	getStyle: function(element, style) { // Modified from Prototype 1.5
		style = CARPE.camelize(style);
		var value = element.style[style];
		if (!value) {
			if (document.defaultView && document.defaultView.getComputedStyle) {
				var css = document.defaultView.getComputedStyle(element, null);
				value = css ? css[style] : null;
			} else if (element.currentStyle) {
				value = element.currentStyle[style];
			}
		}
		return value;
	},
	camelize: function(s) {
		var parts = s.split('-');
		if (parts.length == 1) return parts[0];
		var camel = parts[0];
		var len = parts.length;
		for (var i = 1; i < len; i++) {
			camel += parts[i].charAt(0).toUpperCase() + parts[i].substring(1);
		}
	    return camel;
	},
	stopEvent: function(e) {
		if (e.preventDefault) {
			e.preventDefault();
			e.stopPropagation();
		}
		else {
			e.returnValue = false;
			e.cancelBubble = true;
		}
	},
	addLoadEvent: function(func) {
		var old = window.onload;
		if (typeof window.onload != 'function') {
			window.onload = func;
		}
		else {
			window.onload = function() {
				if (old) {
					old()
				}
				func();
			};
		}
	},
	addEventListener: function(elmnt, evnt, func) {
		if (elmnt.addEventListener) {
			elmnt.addEventListener(evnt, func, false)
		}
		else if (elmnt.attachEvent) {
			elmnt.attachEvent('on' + evnt, func);
		} else return false;
	},
	removeEventListener: function(elmnt, evnt, func) {
		if (elmnt.removeEventListener) { // Remove event listeners from element (W3C).
			elmnt.removeEventListener(evnt, func, false);
			elmnt.removeEventListener(evnt, func, false);
		}
		else if (elmnt.detachEvent) { // Remove event listeners from element (IE).
			elmnt.detachEvent('on' + evnt, func);
			elmnt.detachEvent('on' + evnt, func);
		}
		return;
	},
	listObj: function(o) {
		var s = '';
		for (i in o) {
			s = s + i + ': ' + o.toString() + ',';
		}
		alert(s);
	}
};

Function.prototype.bind = CARPE.bind;
