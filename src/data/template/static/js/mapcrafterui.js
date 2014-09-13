var MCTileLayer = L.TileLayer.extend({
	initialize: function(url, options) {
		this._url = url;
		
		this.imageFormat = options["imageFormat"];
		
		L.setOptions(this, options);
	},
	
	getTileUrl: function(tile) {
		var zoom = this._map.getZoom();
		var url = this._url;
		if(tile.x < 0 || tile.x >= Math.pow(2, zoom) || tile.y < 0 || tile.y >= Math.pow(2, zoom)) {
			url += "/blank";
		} else if(zoom == 0) {
			url += "/base";
		} else {
			for(var z = zoom - 1; z >= 0; --z) {
				var x = Math.floor(tile.x / Math.pow(2, z)) % 2;
				var y = Math.floor(tile.y / Math.pow(2, z)) % 2;
				url += "/" + (x + 2 * y + 1);
			}
		}
		url = url + "." + this.imageFormat;
		return url;
	},
});

/**
 * The main map class.
 */
function MapcrafterUI(config) {
	this.config = config;
	
	this.currentMap = null;
	this.currentRotation = null;
	
	this.lmap = null;
	this.layers = {};
	
	this.handlers = [];
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
	this.created = false;
	
	this.addHandler(new PosHashHandler());
}

MapcrafterUI.prototype.init = function() {
	this.lmap = L.map("mcmap", {
		crs: L.CRS.Simple
	}).setView([0, 0], 0, {animate: false});
	this.lmap.attributionControl.addAttribution("Map rendered with <a href='http://mapcrafter.org'>Mapcrafter</a>");
	
	var firstMap = null;
	for(var type in this.config) {
		this.layers[type] = {};
		for(var i in this.config[type].rotations) {
			var rotation = this.config[type].rotations[i];
			this.layers[type][rotation] = this.createTileLayer(type, this.config[type], rotation);
			if(firstMap === null)
				firstMap = type;
		}
	}
	
	this.setMap(firstMap);
	
	this.created = true;
	
	for(var i = 0; i < this.controlsNotCreated.length; i++) {
		var control = this.controlsNotCreated[i];
		this.addControl(control[0], control[1], control[2]);
	}
	
	for(var i = 0; i < this.handlersNotCreated.length; i++)
		this.addHandler(this.handlersNotCreated[i]);
		
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
};

MapcrafterUI.prototype.getCurrentMap = function() {
	return this.currentMap;
};

MapcrafterUI.prototype.getCurrentRotation = function() {
	return this.currentRotation;
};

MapcrafterUI.prototype.getConfig = function() {
	return this.config;
};

MapcrafterUI.prototype.getMapConfig = function(map) {
	return map in this.config ? this.config[map] : null;
};

MapcrafterUI.prototype.getCurrentMapConfig = function() {
	return this.getMapConfig(this.currentMap);
};

MapcrafterUI.prototype.setMapAndRotation = function(map, rotation) {
	var oldMapConfig = this.getCurrentMapConfig();
	var mapConfig = this.getMapConfig(map);
	
	// try to get the old map layer and view of the map (center in Minecraft coordinates, zoom)
	var oldMapLayer = null;
	var oldView = null;
	var oldZoom = 0;
	if(this.currentMap != null && this.currentRotation != null) {
		oldMapLayer = this.layers[this.currentMap][this.currentRotation];
		oldView = this.latLngToMC(this.lmap.getCenter(), 64);
		oldZoom = this.lmap.getZoom();
	}
	
	// set the new map and rotation
	this.currentMap = map;
	this.currentRotation = parseInt(rotation);
	
	// remove the old map layer and set the new map layer
	if(oldMapLayer != null)
		this.lmap.removeLayer(oldMapLayer);
	this.lmap.addLayer(this.layers[this.currentMap][this.currentRotation]);
	//this.lmap.invalidateSize();
	
	// check whether we are switching to a completely different map
	if(oldMapLayer == null || oldMapConfig.world != mapConfig.world) {
		// completely different map, reset view
		
		// reset zoom level, 0 or user-defined default zoom level
		var zoom = 0;
		if("defaultZoom" in mapConfig)
			zoom = mapConfig.defaultZoom;
		
		// set view to the map center or a user-defined default center
		if("defaultView" in mapConfig) {
			var x = mapConfig.defaultView[0];
			var z = mapConfig.defaultView[1];
			var y = mapConfig.defaultView[2];
			this.lmap.setView(this.mcToLatLng(x, z, y), zoom, {animate: false});
		} else {
			var center = mapConfig.tileSize / 2;
			this.lmap.setView(this.lmap.unproject([center, center]), zoom, {animate: false});
		}

	} else {
		// same world, we can set the view to the view of the old map
		this.lmap.setView(this.mcToLatLng(oldView[0], oldView[1], oldView[2]), oldZoom, {animate: false});
		
		// same world, so should have same zoom levels
		//if(oldMapConfig.maxZoom != mapConfig.maxZoom)
		//	this.lmap.setZoom(oldZoom + mapConfig.maxZoom - oldMapConfig.maxZoom, {animate: false});
	}
	
	// call handlers
	for(var i = 0; i < this.handlers.length; i++)
		this.handlers[i].onMapChange(this.currentMap, this.currentRotation);
};

MapcrafterUI.prototype.setMap = function(map) {
	var oldMapConfig = this.getCurrentMapConfig();
	var mapConfig = this.getMapConfig(map);
	
	// check whether this the same world and the new map has the current rotation as well
	// we can use the current rotation then, use the default/first available rotation else
	var sameWorld = oldMapConfig == null ? false : oldMapConfig.world == mapConfig.world;
	if(sameWorld && mapConfig.rotations.indexOf(this.currentRotation) != -1) {
		this.setMapAndRotation(map, this.currentRotation);
	} else {
		var rotation = -1;
		if("defaultRotation" in mapConfig)
			rotation = mapConfig.defaultRotation;
		// use first available rotation if given default rotation is not available
		if(mapConfig.rotations.indexOf(rotation) == -1)
			rotation = mapConfig.rotations[0];
		this.setMapAndRotation(map, rotation);
	}
};

MapcrafterUI.prototype.setMapRotation = function(rotation) {
	this.setMapAndRotation(this.currentMap, rotation);
};

MapcrafterUI.prototype.createTileLayer = function(name, config, rotation) {
	var layer = new MCTileLayer(name + "/" + ["tl", "tr", "br", "bl"][rotation], {
		maxZoom: config.maxZoom,
		tileSize: config.tileSize,
		noWrap: true,
		continuousWorld: true,
		imageFormat: config.imageFormat,
	});
	
	return layer;
};

MapcrafterUI.prototype.addControl = function(control, pos, index) {
	if(!this.created) {
		this.controlsNotCreated.push([control, pos, index]);
		return;
	}
	
	var self = this;
	var ControlType = L.Control.extend({
		onAdd: function(map) {
			var wrapper = document.createElement("div");
			wrapper.setAttribute("class", "control-wrapper");
			wrapper.setAttribute("id", "control-wrapper-" + name);
			
			// just a dirty hack to prevent the map getting all mouse click events
			wrapper.onmouseover = function() {
				map.dragging.disable();
			};
			wrapper.onmouseout = function() {
				map.dragging.enable();
			};
			
			control.ui = self;
			control.create(wrapper);
			wrapper.index = index;
			
			return wrapper;
		},
	});
	var lcontrol = new ControlType({
		position: pos,
	});
	this.lmap.addControl(lcontrol);
	
	if(control.getHandler())
		this.addHandler(control.getHandler());
};

MapcrafterUI.prototype.addHandler = function(handler) {
	if(!this.created) {
		this.handlersNotCreated.push(handler);
		return;
	}
	
	handler.ui = this;
	handler.create();
	handler.onMapChange(this.currentMap, this.currentRotation);
	this.handlers.push(handler);
};

MapcrafterUI.prototype.mcToLatLng = function(x, z, y) {
	// converts Minecraft x,z,y to a Google Map lat/lng
	
	var config = this.getCurrentMapConfig();
	var rotation = this.getCurrentRotation();
	
	// rotate the position to the map rotation
	for(var i = 0; i < this.currentRotation; i++) {
		var nx = -z+512; // 512 blocks = one region
		var nz = x;
		x = nx;
		z = nz;
	}
	
	// the size of a 1/4 block image divided by the total size of all render tiles 
	// on the highest zoom level
	var block = (config.textureSize/2.0) / (config.tileSize * Math.pow(2, config.maxZoom));
	
	// at first calculate the row and the column of the block:
	// column is just x+z
	// also don't forget the possible tile offset: one tile has 32 columns
	var col = x+z - config.tileOffsets[rotation][0]*32;
	// row is z-x, and every y to bottom adds 2 rows
	// tile offset again: one tile has 64 rows
	var row = z-x + (256-y)*2 - config.tileOffsets[rotation][1]*64;

	// midpoint of the map is in lat/lng 0.5|0.5
	// we have to move the lng by the size of one tile
	// lng is now 2*block size for every column
	var lng = 0.5 - (1.0 / Math.pow(2, config.maxZoom + 1)) + col * 2*block;
	// lat is now one block size for every row 
	var lat = 0.5 + row * block;

	// now we have coordinates in the range [0; 1]
	// we use the unproject method of leaflet to convert pixel coordinates
	// to real lat/lng coordinates
	// every zoom level has tileSize * 2^zoom pixels, so just multiplicate
	// the [0; 1] coordinates with this pixel count and use the unproject method
	var size = config.tileSize * Math.pow(2, this.lmap.getZoom());
	return this.lmap.unproject([lng * size, lat * size]);
};

MapcrafterUI.prototype.latLngToMC = function(latlng, y) {
	// this is the equivalent of the Minecraft x,z,y to lat/lng converter
	// have some fun with the formulas from above!
	
	//    tile = 1 / 2^(maxZoom + 1)
	// 1) lng = 0.5 - tile + (x + z) * 2 * block
	// 2) lat = 0.5 + (z - x + (256 - y) * 2) * block
	
	// solve 1) for x:
	//    lng - (x + z) * 2 * block = 0.5 - tile
	//    - (x + z) * 2 * block = 0.5 - tile - lng
	//    -2x*block - 2z*block = 0.5 - tile - lng
	//    -2x*block = 0.5 - tile - lng + 2z*block
	// => x = (0.5 - tile - lng + 2z*block) / (-2*block)
	
	// put x from 1) into 2):
	//    lat = 0.5 + (z - x + (256 - y) * 2) * block
	//    lat = 0.5 + (z - ((0.5 - tile - lng + 2z*block) / (-2*block)) + (256 - y) * 2) * block
	// lets solve for z (yes, I was too lazy and used my CAS):
	// => z = (4y*block - 1024*block + 2*lat + lng + tile - 1.5) / (4*block)
	
	var config = this.getCurrentMapConfig();
	var rotation = this.getCurrentRotation();
	
	// same way like in the other method:
	// we convert the lat/lng coordinates to pixel coordinates
	// in the range [0; count of tiles * tile size]
	var point = this.lmap.project(latlng);
	// size = count of tiles on this zoom level * tile size
	var size = config.tileSize * Math.pow(2, this.lmap.getZoom());
	// then we need to convert the pixel coordinates to lat/lng coordinates
	// in the range [0; 1] to use them for the lat/lng -> MC algorithm
	var lat = point.y / size;
	var lng = point.x / size;
	// add tile offset to lat/lng coordinates,
	// as we have now lat/lng coordinates in the range [0; 1],
	// divide tile offset by the count of tiles on max zoom level 
	lat += config.tileOffsets[rotation][1] / Math.pow(2, config.maxZoom);
	lng += config.tileOffsets[rotation][0] / Math.pow(2, config.maxZoom);
	
	// convert lat/lng coordinates to Minecraft coordinates
	var tile = (1.0 / Math.pow(2, config.maxZoom + 1));
	var block = (config.textureSize/2.0) / (config.tileSize * Math.pow(2, config.maxZoom));
	
	var z = (4*y*block - 1024*block + 2*lat + lng + tile - 1.5) / (4*block);
	var x = (0.5 - tile - lng + 2*z*block) / (-2*block);
	
	// rotate the position in the other direction
	for(var i = 0; i < this.currentRotation; i++) {
		var nx = z; // 512 blocks = one region
		var nz = -x+512;
		x = nx;
		z = nz;
	}
	
	return [x, z, y];
};
