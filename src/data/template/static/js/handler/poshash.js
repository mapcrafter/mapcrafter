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

PosHashHandler.prototype.onOverlayChange = function(overlay, enabled) {
	this.updateHash();
};

PosHashHandler.prototype.parseHash = function() {
	if(!location.hash)
		return null;
	
	var url = location.hash.substr(1);
	var split = url.split("/");
	
	if(split.length != 7)
		return null;
	for(var i = 2; i < 7; i++)
		split[i] = parseInt(split[i]);
	return split;
};

PosHashHandler.prototype.updateHash = function() {
	var type = this.ui.getCurrentMap();
	var overlays = this.ui.getEnabledOverlays().join(",");
	if(overlays == "")
		overlays = "-";
	var rotation = this.ui.getCurrentRotation();
	var xzy = this.ui.latLngToMC(this.ui.lmap.getCenter(), 64);
	for(var i = 0; i < 3; i++)
		xzy[i] = Math.round(xzy[i]);
	var zoom = this.ui.lmap.getZoom();
	window.location.replace("#" + type + "/" + overlays + "/" + rotation + "/" + zoom + "/" + xzy[0] + "/" + xzy[1] + "/" + xzy[2]);
};

PosHashHandler.prototype.gotoHash = function(hash) {
	if(!hash)
		return;

	var map = hash[0];
	var overlays = hash[1].split(",");
	var rotation = hash[2];
	var zoom = hash[3];
	var x = hash[4];
	var z = hash[5];
	var y = hash[6];
	
	if(!(map in this.ui.getMapConfigs()) 
			|| this.ui.getMapConfig(map).rotations.indexOf(rotation) < 0)
		return null;
		
	this.ui.setMapAndRotation(map, rotation);

	var availOverlays = this.ui.getCurrentOverlays();
	for(var i in availOverlays) {
		var overlay = availOverlays[i].id;
		var enabled = overlays.indexOf(overlay) != -1;
		this.ui.enableOverlay(overlays[i], enabled);
	}
		
	var latlng = this.ui.mcToLatLng(x, z, y);
	this.ui.lmap.setView(latlng, zoom);
};

