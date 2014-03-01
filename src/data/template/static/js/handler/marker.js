MarkerHandler.prototype = new BaseHandler();

/**
 * Is responsible to show the markers from the markers.js.
 */
function MarkerHandler(markers) {
	this.layerGroups = {};
	this.markers = markers;
}

MarkerHandler.prototype.onMapChange = function(name, rotation) {
	for(var group in this.layerGroups)
		this.ui.lmap.removeLayer(this.layerGroups[group]);
	this.layerGroups = {};
	
	var world = this.ui.getCurrentMapConfig().worldName;
	for(var group in this.markers) {
		if (!(world in this.markers[group]["markers"]))
			continue;
		var groupInfo = this.markers[group];
		var markers = groupInfo["markers"][world];
		var layerGroup = L.layerGroup();
		for (var i = 0; i < markers.length; i++) {
			var poi = markers[i];
			
			var pos = poi.pos;
			var marker = new L.Marker(this.ui.mcToLatLng(pos[0], pos[1], pos[2]), {
				title: poi.title,
			});
			if(groupInfo.icon) {
				marker.setIcon(new L.Icon({
					iconUrl: groupInfo.icon != "" ? "static/markers/" + groupInfo.icon : null,
					iconSize: (groupInfo.icon_size ? groupInfo.icon_size : [24, 24]),
				}));
			}
			marker.bindPopup(poi.text ? poi.text : poi.title);
			marker.addTo(layerGroup);
		}
		layerGroup.addTo(this.ui.lmap);
		this.layerGroups[group] = layerGroup;
	}
	
	//this.show("spawn", false);
	//this.show("spawn", true);
};

MarkerHandler.prototype.show = function(group, visible) {
	this.layerGroups[group].eachLayer(function(layer) {
		layer.setOpacity(visible ? 1 : 0);
	});
};
