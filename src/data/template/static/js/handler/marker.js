MarkerHandler.prototype = new BaseHandler();

/**
 * Is responsible to show the markers from the markers.js.
 */
function MarkerHandler(control, markers) {
	this.control = control;
	this.layerGroups = {};
	this.markers = markers;
	this.markersByGroup = {};
	this.visible = {};
}

MarkerHandler.prototype.onMapChange = function(name, rotation) {
	for(var group in this.layerGroups)
		this.ui.lmap.removeLayer(this.layerGroups[group]);
	this.layerGroups = {};
	
	var createDefaultMarker = function(ui, groupInfo, markerInfo) {
		var pos = markerInfo.pos;
		// show on top center of block
		var marker = new L.Marker(ui.mcToLatLng(pos[0] + 0.5, pos[1] + 0.5, pos[2]), {
			title: markerInfo.title,
		});

		// The icon may be specified on either a marker or group level, with
		// preference for the marker-specific icon.
		var icon = markerInfo.icon ? markerInfo.icon : groupInfo.icon;
		var iconSize = markerInfo.iconSize ? markerInfo.iconSize : groupInfo.iconSize;

		if(icon) {
			marker.setIcon(new L.Icon({
				// The icon URL itself may be given relative to "static/markers"
				// or as an absolute URL (perhaps to a resource on a CDN).
				iconUrl: icon.match(/^\w+:\/\//) ? icon : "static/markers/" + icon,
				iconSize: (iconSize ? iconSize : [24, 24]),
			}));
		}
		marker.bindPopup(markerInfo.text ? markerInfo.text : markerInfo.title);
		return marker;
	};
	
	var world = this.ui.getCurrentMapConfig().world;
	for(var i = 0; i < this.markers.length; i++) {
		var groupInfo = this.markers[i];
		var group = groupInfo.id;
		this.markersByGroup[group] = groupInfo;
		
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
			this.control.uncheckGroup(group);
			this.visible[group] = false;
		}
	}
	
	for(var group in this.visible)
		this.show(group, this.visible[group]);

	this.updateMarkerCounts();
};

MarkerHandler.prototype.updateMarkerCounts = function() {
	var world = this.ui.getCurrentMapConfig().world;
	var buttons = this.control.buttons;
	for(var i = 0; i < buttons.length; i++) {
		var button = buttons[i];
		var group = button.getAttribute("data-group");
		var count = 0;
		if(world in this.markersByGroup[group].markers)
			count = this.markersByGroup[group].markers[world].length;

		var span = button.getElementsByTagName("span")[0];
		span.innerHTML = count;
	}
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
