MapSelectHandler.prototype = new BaseHandler();

function MapSelectHandler(control) {
	this.control = control;
}

MapSelectHandler.prototype.onMapChange = function(map, rotation) {
	this.update();
};

MapSelectHandler.prototype.update = function() {
	var currentMap = this.ui.getCurrentMap();
	var mapConfig = this.ui.getCurrentMapConfig();

	this.control.button.innerHTML = mapConfig.name + " <span class='caret'></span>";

	var maps = this.control.dropdown.childNodes;
	for (var i = 0; i < maps.length; i++) {
		var current = maps[i].getAttribute("data-map") == currentMap;
		maps[i].setAttribute("class", current ? "active" : "");
	}
};

