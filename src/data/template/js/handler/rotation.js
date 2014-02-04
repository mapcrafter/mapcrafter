MapRotationHandler.prototype = new MapHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function MapRotationHandler(control) {
	this.control = control
}

MapRotationHandler.prototype.create = function() {
}

MapRotationHandler.prototype.onMapChange = function(name, rotation) {
	this.update();
};

MapRotationHandler.prototype.update = function(text) {	
	var config = this.ui.getCurrentConfig();
	var currentRotation = this.ui.getCurrentRotation();
	
	var images = this.control.images;
	for(var i = 0; i < 4; i++) {
		if(config.rotations.indexOf(i) == -1) {
			images[2*i].style.display = "none";
			images[2*i+1].style.display = "none";
		} else {
			if(i == currentRotation) {
				images[2*i].style.display = "none";
				images[2*i+1].style.display = "inline";
			} else {
				images[2*i].style.display = "inline";
				images[2*i+1].style.display = "none";
			}
		}
	}
}
