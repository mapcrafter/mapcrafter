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
	
	var createDefaultMarker = function(ui, groupInfo, markerInfo) {
		var pos = markerInfo.pos;
		var marker = new L.Marker(ui.mcToLatLng(pos[0], pos[1], pos[2]), {
			title: markerInfo.title,
		});
		if(groupInfo.icon) {
			marker.setIcon(new L.Icon({
				iconUrl: groupInfo.icon != "" ? "static/markers/" + groupInfo.icon : null,
				iconSize: (groupInfo.iconSize ? groupInfo.iconSize : [24, 24]),
			}));
		}
		marker.bindPopup(markerInfo.text ? markerInfo.text : markerInfo.title);
		return marker;
	};
	
	var world = this.ui.getCurrentMapConfig().world;
	for(var i = 0; i < this.markers.length; i++) {
		var groupInfo = this.markers[i];
		var group = groupInfo.id;
		if(!(world in groupInfo.markers)) {
			// create empty layer group
			this.layerGroups[group] = L.layerGroup();
			continue;
		}
		
		if(!groupInfo.createMarker)
			groupInfo.createMarker = createDefaultMarker;
		
		var markers = groupInfo.markers[world];
		var layerGroup = L.layerGroup();
		for(var j = 0; j < markers.length; j++) {
			var markerInfo = markers[j];
			var marker = groupInfo.createMarker(this.ui, groupInfo, markerInfo);
			if(marker != null)
				marker.addTo(layerGroup);
		}
		
		this.layerGroups[group] = layerGroup;
		if(!(group in this.visible))
			this.visible[group] = true;
		if("showDefault" in groupInfo && !groupInfo.showDefault) {
			document.getElementById("cb_group_" + group).checked = false;
			this.visible[group] = false;
		}
	}
	
	for(var group in this.visible)
		this.show(group, this.visible[group]);
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
