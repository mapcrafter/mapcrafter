MapSelectHandler.prototype = new BaseHandler();

function MapSelectHandler(control) {
	this.control = control;
}

MapSelectHandler.prototype.onMapChange = function(map, rotation) {
	this.update();
};

MapSelectHandler.prototype.update = function() {
	var map = this.ui.getCurrentMapConfig().name;
	this.control.button.innerHTML = map + " <span class='caret'></span>";
};

