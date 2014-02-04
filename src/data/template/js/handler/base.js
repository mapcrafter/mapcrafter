/**
 * This is the base class to implement different handlers which are called
 * when the type or the rotation of the map is changed.
 */
function MapHandler() {
	this.ui = null;
}

MapHandler.prototype.create = function() {
};

MapHandler.prototype.onMapChange = function(name, rotation) {
};
