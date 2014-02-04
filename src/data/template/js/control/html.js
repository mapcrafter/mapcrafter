HTMLControl.prototype = new MapControl();

/**
 * A simple control with user-defined content.
 */
function HTMLControl(html) {
	this.html = html;
}

HTMLControl.prototype.create = function(wrapper) {
	wrapper.innerHTML = this.html;
};