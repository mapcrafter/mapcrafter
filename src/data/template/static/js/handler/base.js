/**
 * This is the base class to implement different handlers which are called
 * when the type or the rotation of the map is changed.
 */
function BaseHandler() {
	this.ui = null;
}

BaseHandler.prototype.create = function() {
};

BaseHandler.prototype.onMapChange = function(name, rotation) {
};
