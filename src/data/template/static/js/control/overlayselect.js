OverlaySelectControl.prototype = new BaseControl("OverlaySelectControl");

/**
 * This control widget allows the user to rotate the map.
 */
function OverlaySelectControl() {
	this.handler = new OverlaySelectHandler(this);
	
	this.div = null;
}

OverlaySelectControl.prototype.create = function(wrapper) {
	this.div = wrapper;

	this.ui.addHandler(this.handler);
};
