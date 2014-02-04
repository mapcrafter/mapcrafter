MarkerHandler.prototype = new BaseHandler();

/**
 * Is responsible to show the markers from the markers.js.
 */
function MarkerHandler(markers) {
	this.mapMarkers = [];
	this.markers = markers;
}

MarkerHandler.prototype.onMapChange = function(name, rotation) {
	for(var i = 0; i < this.mapMarkers.length; i++)
		this.ui.lmap.removeLayer(this.mapMarkers[i]);
	this.mapMarkers = [];
	
	var world = this.ui.getCurrentConfig().worldName;
	if(!(world in this.markers))
		return;
	for(var i = 0; i < this.markers[world].length; i++) {  
		var location = this.markers[world][i];
		
		var pos = location.pos;
		var marker = new L.Marker(this.ui.mcToLatLng(pos[0], pos[1], pos[2]), {
			title: location.title,
		});
		if(location.icon) {
			marker.setIcon(new L.Icon({
				iconUrl: location.icon,
				iconSize: (location.iconsize ? location.iconsize : [24, 24]),
			}));
		}
		marker.bindPopup(location.text ? location.text : location.title);
		marker.addTo(this.ui.lmap);
		
		this.mapMarkers.push(marker);
	}
};