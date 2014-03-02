MarkerHandler.prototype = new BaseHandler();

/**
 * Is responsible to show the markers from the markers.js.
 */
function MarkerHandler(markers) {
	this.layerGroups = {};
	this.markers = markers;
	this.visible = {};
}

MarkerHandler.prototype.onMapChange = function(name, rotation) {
	for(var group in this.layerGroups)
		this.ui.lmap.removeLayer(this.layerGroups[group]);
	this.layerGroups = {};
	
	var world = this.ui.getCurrentMapConfig().worldName;
	for(var i = 0; i < this.markers.length; i++) {
		var groupInfo = this.markers[i];
		if (!(world in groupInfo["markers"]))
			continue;
		var group = groupInfo["id"];
		var markers = groupInfo["markers"][world];
		var layerGroup = L.layerGroup();
		for (var j = 0; j < markers.length; j++) {
			var poi = markers[j];
			
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
		//layerGroup.addTo(this.ui.lmap);
		this.layerGroups[group] = layerGroup;
		if(!(group in this.visible))
			this.visible[group] = true;
	}
	
	for(var group in this.visible)
		this.show(group, this.visible[group]);
	
	//this.show("spawn", false);
	//this.show("spawn", true);
};

MarkerHandler.prototype.getMarkerGroups = function() {
	var groups = [];
	for(var i = 0; i < this.markers.length; i++)
		groups.push([this.markers[i]["id"], this.markers[i]["name"]]);
	return groups;
};

MarkerHandler.prototype.show = function(group, visible) {
	var layer = this.layerGroups[group];
	if(visible && !this.ui.lmap.hasLayer(layer))
		layer.addTo(this.ui.lmap);
	if(!visible && this.ui.lmap.hasLayer(layer))
		this.ui.lmap.removeLayer(layer);
	this.visible[group] = visible;
};
