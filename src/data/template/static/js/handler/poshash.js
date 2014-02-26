PosHashHandler.prototype = new BaseHandler();

/**
 * Is responsible for the url position hash.
 */
function PosHashHandler() {
}

PosHashHandler.prototype.create = function() {
	// check if the url has already a position hash
	// if yes, set the map to this hash
	var hash = this.parseHash();
	this.gotoHash(hash);
	this.updateHash();
	
	// and register event handlers to update the url position hash
	
	var handler = (function(self) {
		return function() {
			self.updateHash();
		};
	})(this);
	
	this.ui.lmap.on("dragend", handler);
	this.ui.lmap.on("zoomend", handler);
};

PosHashHandler.prototype.onMapChange = function(name, rotation) {
	this.updateHash();
};

PosHashHandler.prototype.parseHash = function() {
	if(!location.hash)
		return null;
	
	var url = location.hash.substr(1);
	var split = url.split("/");
	
	if(split.length != 6)
		return null;
	for(var i = 1; i < 6; i++)
		split[i] = parseInt(split[i]);
	return split;
};

PosHashHandler.prototype.updateHash = function() {
	var type = this.ui.getCurrentMap();
	var rotation = this.ui.getCurrentRotation();
	var xzy = this.ui.latLngToMC(this.ui.lmap.getCenter(), 64);
	for(var i = 0; i < 3; i++)
		xzy[i] = Math.round(xzy[i]);
	var zoom = this.ui.lmap.getZoom();
	window.location.replace("#" + type + "/" + rotation + "/" + zoom + "/" + xzy[0] + "/" + xzy[1] + "/" + xzy[2]);
};

PosHashHandler.prototype.gotoHash = function(hash) {
	if(!hash)
		return;
	
	if(!(hash[0] in this.ui.getConfig()) 
			|| this.ui.getMapConfig(hash[0]).rotations.indexOf(hash[1]) < 0)
		return null;
		
	this.ui.setMapAndRotation(hash[0], hash[1]);
		
	var latlng = this.ui.mcToLatLng(hash[3], hash[4], hash[5]);
	this.ui.lmap.setView(latlng, hash[2]);
};