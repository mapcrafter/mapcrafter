/**
 * This base class is used to implement different map control widgets.
 * 
 * These controls can also have their own handler. Just return
 * in getHandler() your handler.
 */
function BaseControl(type) {
	this.TYPE = type;
	
	this.ui = null;
}

BaseControl.prototype.TYPE = "BaseControl";

BaseControl.prototype.create = function(wrapper) {
};

BaseControl.prototype.getHandler = function() {
	return null;
};