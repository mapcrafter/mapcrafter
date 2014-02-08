/**
 * This base class is used to implement different map control widgets.
 * 
 * These controls can also have their own handler. Just return
 * in getHandler() your handler.
 */
function BaseControl() {
	this.ui = null;
}

BaseControl.prototype.create = function(wrapper) {
}

BaseControl.prototype.getName = function(wrapper) {
	return "";
}