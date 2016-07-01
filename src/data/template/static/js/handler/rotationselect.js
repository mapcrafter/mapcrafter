RotationSelectHandler.prototype = new BaseHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function RotationSelectHandler(control) {
	this.control = control
}

RotationSelectHandler.prototype.create = function() {
};

RotationSelectHandler.prototype.onMapChange = function(name, rotation) {
	this.update();
};

RotationSelectHandler.prototype.update = function(text) {
	var config = this.ui.getCurrentMapConfig();
	var currentRotation = this.ui.getCurrentRotation();
	
	for(var i = 0; i < 4; i++) {
		// hide buttons where rotations don't exist, set active/not active for others
		var button = this.control.buttons[i];
		if(config.rotations.indexOf(i) == -1) {
			button.style.display = "none";
		} else {
			button.style.display = "inline";
			if(i == currentRotation) {
				button.setAttribute("class", "btn btn-default active");
			} else {
				button.setAttribute("class", "btn btn-default");
			}
		}
	}
};

