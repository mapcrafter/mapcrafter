OverlaySelectControl.prototype = new BaseControl("OverlaySelectControl");

/**
 * This control widget allows the user to rotate the map.
 */
function OverlaySelectControl() {
	this.handler = new OverlaySelectHandler(this);
	
	this.div = null;
};

OverlaySelectControl.prototype.create = function(wrapper) {
	this.div = wrapper;
};

OverlaySelectControl.prototype.getHandler = function() {
	return this.handler;
};

OverlaySelectControl.prototype.usePanelWrapper = function() {
	return true;
};
