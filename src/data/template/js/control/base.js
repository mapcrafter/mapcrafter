/**
 * This base class is used to implement different map control widgets.
 * 
 * These controls can also have their own handler. Just return
 * in getHandler() your handler.
 */
function MapControl() {
	this.ui = null;
}

MapControl.prototype.create = function(wrapper) {
}

MapControl.prototype.getHandler = function() {
	return null;
}

MapControl.prototype.getName = function(wrapper) {
	return "";
}