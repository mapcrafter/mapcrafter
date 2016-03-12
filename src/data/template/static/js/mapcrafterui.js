/**
 * A tile layer class which uses the quadtree data structure of rendered tiles.
 */
var MCTileLayer = L.TileLayer.extend({
	initialize: function(url, options) {
		this._url = url;
		this._imageFormat = options["imageFormat"];
		
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
		url = url + "." + this._imageFormat;
		return url;
	},
});

/**
 * Creates a tile layer of a map with a specific rotation
 */
function createMCTileLayer(mapName, mapConfig, mapRotation) {
	return new MCTileLayer(mapName + "/" + ["tl", "tr", "br", "bl"][mapRotation], {
		maxZoom: mapConfig.maxZoom,
		tileSize: mapConfig.tileSize,
		noWrap: true,
		continuousWorld: true,
		imageFormat: mapConfig.imageFormat,
	});
};

/**
 * Functions to convert Minecraft x, z, y (isometric render view) <-> Leaflet latitute/longitude.
 */
var IsometricRenderView = {
	mcToLatLng: function(x, z, y, lmap, mapConfig, tileOffset, tileWidth) {
		// the size of a 1/4 block image divided by the total size of all render tiles 
		// on the highest zoom level
		var block = (mapConfig.textureSize/2.0) / (mapConfig.tileSize / tileWidth * Math.pow(2, mapConfig.maxZoom));
		
		// at first calculate the row and the column of the block:
		// column is just x+z
		// also don't forget the possible tile offset: one tile has 32 columns
		var col = x+z - tileOffset[0]*tileWidth*32;
		// row is z-x, and every y to bottom adds 2 rows
		// tile offset again: one tile has 64 rows
		var row = z-x + (256-y)*2 - tileOffset[1]*tileWidth*64;
	
		// midpoint of the map is in lat/lng 0.5|0.5
		// we have to move the lng by the size of one tile
		// lng is now 2*block size for every column
		var lng = 0.5 - (1.0 / Math.pow(2, mapConfig.maxZoom + 1)) + col * 2*block;
		// lat is now one block size for every row 
		var lat = 0.5 + row * block;
	
		// now we have coordinates in the range [0; 1]
		// we use the unproject method of leaflet to convert pixel coordinates
		// to real lat/lng coordinates
		// every zoom level has tileSize * 2^zoom pixels, so just multiplicate
		// the [0; 1] coordinates with this pixel count and use the unproject method
		var size = mapConfig.tileSize * Math.pow(2, lmap.getZoom());
		return lmap.unproject([lng * size, lat * size]);
	},
	
	latLngToMC: function(latLng, y, lmap, mapConfig, tileOffset, tileWidth) {
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
		
		// same way like in the other method:
		// we convert the lat/lng coordinates to pixel coordinates
		// in the range [0; count of tiles * tile size]
		var point = lmap.project(latLng);
		// size = count of tiles on this zoom level * tile size
		var size = mapConfig.tileSize / tileWidth * Math.pow(2, lmap.getZoom());
		// then we need to convert the pixel coordinates to lat/lng coordinates
		// in the range [0; 1] to use them for the lat/lng -> MC algorithm
		var lat = point.y / size;
		var lng = point.x / size;
		// add tile offset to lat/lng coordinates,
		// as we have now lat/lng coordinates in the range [0; 1],
		// divide tile offset by the count of tiles on max zoom level 
		lat += tileOffset[1] * tileWidth / Math.pow(2, mapConfig.maxZoom);
		lng += tileOffset[0] * tileWidth / Math.pow(2, mapConfig.maxZoom);
		
		// convert lat/lng coordinates to Minecraft coordinates
		var tile = (1.0 / Math.pow(2, mapConfig.maxZoom + 1));
		var block = (mapConfig.textureSize/2.0) / (mapConfig.tileSize * Math.pow(2, mapConfig.maxZoom));
		
		z = (4*y*block - 1024*block + 2*lat + lng + tile - 1.5) / (4*block);
		x = (0.5 - tile - lng + 2*z*block) / (-2*block);
		return [x, z, y];
	}
};

/**
 * Functions to convert Minecraft x, z, y (topdown render view) <-> Leaflet latitute/longitude.
 */
var TopdownRenderView = {
	mcToLatLng: function(x, z, y, lmap, mapConfig, tileOffset, tileWidth) {
		var block = mapConfig.textureSize / (mapConfig.tileSize / tileWidth * Math.pow(2, mapConfig.maxZoom));
		
		var lng = 0.5 + (x - tileOffset[0]*tileWidth*16) * block;
		var lat = 0.5 + (z - tileOffset[1]*tileWidth*16) * block;
		
		var size = mapConfig.tileSize / tileWidth * Math.pow(2, lmap.getZoom());
		return lmap.unproject([lng * size, lat * size]);
	},
	
	latLngToMC: function(latLng, y, lmap, mapConfig, tileOffset, tileWidth) {
		var point = lmap.project(latLng);
		var size = mapConfig.tileSize / tileWidth * Math.pow(2, lmap.getZoom());
		
		var lat = point.y / size;
		var lng = point.x / size;
		lat += tileOffset[1] * tileWidth / Math.pow(2, mapConfig.maxZoom);
		lng += tileOffset[0] * tileWidth / Math.pow(2, mapConfig.maxZoom);

		var block = mapConfig.textureSize / (mapConfig.tileSize / tileWidth * Math.pow(2, mapConfig.maxZoom));
		x = (lng - 0.5) / block;
		z = (lat - 0.5) / block;
		return [x, z, y];
	}
};

/**
 * This is the main class which manages the whole map ui.
 */
function MapcrafterUI(config) {
	this.config = config;
	
	// current map (map name as string)
	this.currentMap = null;
	// and current rotation
	this.currentRotation = null;
	
	// leaflet map object
	this.lmap = null;
	// leaflet layers of different maps/rotations, access them with layers[map][rotation]
	this.layers = {};
	
	// array of handlers to be called when map/rotation is changed
	this.handlers = [];
	// cache controls/handlers when map is not yet properly initialized
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
	this.created = false;
	
	this.addHandler(new PosHashHandler());
}

/**
 * Call this when the document is ready and completely loaded. Creates the leaflet map
 * object, initializes the available maps, sets the view to the first available map and
 * also initializes control widgets and handlers that were added to the ui earlier.
 */
MapcrafterUI.prototype.init = function() {
	// create the leaflet map object
	this.lmap = L.map("mcmap", {
		crs: L.CRS.Simple
	}).setView([0, 0], 0, {animate: false});
	this.lmap.attributionControl.addAttribution("Map rendered with <a href='http://mapcrafter.org'>Mapcrafter</a>");
	
	// initialize the maps
	var firstMap = null;
	for(var i in this.config["mapsOrder"]) {
		var map = this.config["mapsOrder"][i];
		var mapConfig = this.config["maps"][map];
		this.layers[map] = {};
		for(var i2 in mapConfig.rotations) {
			var rotation = mapConfig.rotations[i2];
			this.layers[map][rotation] = createMCTileLayer(map, mapConfig, rotation);
			if(firstMap === null)
				firstMap = map;
		}
	}
	
	this.setMap(firstMap);
	this.created = true;
	
	// initialize controls and handlers that aren't initialized yet
	for(var i = 0; i < this.controlsNotCreated.length; i++) {
		var control = this.controlsNotCreated[i];
		this.addControl(control[0], control[1], control[2]);
	}
	
	for(var i = 0; i < this.handlersNotCreated.length; i++)
		this.addHandler(this.handlersNotCreated[i]);
		
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
};

/**
 * Returns the current map (as map name) that is showed.
 */
MapcrafterUI.prototype.getCurrentMap = function() {
	return this.currentMap;
};

/**
 * Returns the current rotation of the map.
 */
MapcrafterUI.prototype.getCurrentRotation = function() {
	return this.currentRotation;
};

/**
 * Returns the configuration object of a specific tile set group.
 */
MapcrafterUI.prototype.getTileSetGroupConfig = function(group) {
	return group in this.config["tileSetGroups"] ? this.config["tileSetGroups"][group] : null;
};

/**
 * Returns the associative array with the map configuration objects.
 */
MapcrafterUI.prototype.getMapConfigs = function() {
	return this.config["maps"];
};

/**
 * Returns the array with the order of maps.
 */
MapcrafterUI.prototype.getMapConfigsOrder = function() {
	return this.config["mapsOrder"];
};

/**
 * Returns the configuration object of a specific map.
 */
MapcrafterUI.prototype.getMapConfig = function(map) {
	return map in this.config["maps"] ? this.config["maps"][map] : null;
};

/**
 * Returns the configuration object of the current map.
 */
MapcrafterUI.prototype.getCurrentMapConfig = function() {
	return this.getMapConfig(this.currentMap);
};

/**
 * Sets the current map and rotation.
 */
MapcrafterUI.prototype.setMapAndRotation = function(map, rotation) {
	var oldMapConfig = this.getCurrentMapConfig();
	var mapConfig = this.getMapConfig(map);
	
	// try to get the old map layer and view of the map (center in Minecraft coordinates, zoom)
	var oldMapLayer = null;
	var oldView = null;
	var oldZoom = 0;
	if(this.currentMap != null && this.currentRotation != null) {
		oldMapLayer = this.layers[this.currentMap][this.currentRotation];
		oldView = this.latLngToMC(this.lmap.getCenter(), oldMapConfig.worldSeaLevel);
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
	}
	
	// call handlers
	for(var i = 0; i < this.handlers.length; i++)
		this.handlers[i].onMapChange(this.currentMap, this.currentRotation);
};

/**
 * Sets the current map. Tries to keep the current rotation if available, otherwise
 * uses the first available rotation of the new map.
 */
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

/**
 * Sets the rotation of the current map.
 */
MapcrafterUI.prototype.setMapRotation = function(rotation) {
	this.setMapAndRotation(this.currentMap, rotation);
};

/**
 * Adds a control widget to the leaflet ui.
 * 
 * control should be an instance of a subclass of BaseControl.
 * position is the position of the control widget (have a look at the leaflet reference).
 * index is a number which determines how to order the control widgets at the same position.
 */
MapcrafterUI.prototype.addControl = function(control, position, index) {
	// if map is not created yet, add this widget later
	if(!this.created) {
		this.controlsNotCreated.push([control, position, index]);
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
	
	this.lmap.addControl(new ControlType({
		position: position,
	}));
	
	// also add the handler of the control widget if it has one
	if(control.getHandler())
		this.addHandler(control.getHandler());
};

/**
 * Adds a handler to the map ui which is called every time the map or rotation has changed.
 * 
 * handler should be an object of a subclass of BaseHandler.
 */
MapcrafterUI.prototype.addHandler = function(handler) {
	// if map is not created yet, add this handler later
	if(!this.created) {
		this.handlersNotCreated.push(handler);
		return;
	}
	
	handler.ui = this;
	handler.create();
	handler.onMapChange(this.currentMap, this.currentRotation);
	this.handlers.push(handler);
};

/**
 * Converts Minecraft coordinates to Leaflet latitute/longitute.
 */
MapcrafterUI.prototype.mcToLatLng = function(x, z, y) {
	var mapConfig = this.getCurrentMapConfig();
	var tileSetGroup = this.getTileSetGroupConfig(mapConfig.tileSetGroup);
	var tileOffset = tileSetGroup.tileOffsets[this.currentRotation];
	var tileWidth = tileSetGroup.tileWidth;
	
	// rotate the position to the map rotation
	for(var i = 0; i < this.currentRotation; i++) {
		var nx = -z+512; // 512 blocks = one region
		var nz = x;
		x = nx;
		z = nz;
	}
	
	// do the conversion depending on the current render view
	if (mapConfig.renderView == "isometric")
		return IsometricRenderView.mcToLatLng(x, z, y, this.lmap, mapConfig, tileOffset, tileWidth);
	return TopdownRenderView.mcToLatLng(x, z, y, this.lmap, mapConfig, tileOffset, tileWidth);
};

/**
 * Converts a Leaflet latitute/longitute to Minecraft coordinates. You have to specify
 * an y-coordinate since the map is a projection from 3d to 2d.
 */
MapcrafterUI.prototype.latLngToMC = function(latLng, y) {
	var mapConfig = this.getCurrentMapConfig();
	var tileSetGroup = this.getTileSetGroupConfig(mapConfig.tileSetGroup);
	var tileOffset = tileSetGroup.tileOffsets[this.currentRotation];
	var tileWidth = tileSetGroup.tileWidth;

	// do the conversion depending on the current render view
	var mc;
	if (mapConfig.renderView == "isometric")
		mc = IsometricRenderView.latLngToMC(latLng, y, this.lmap, mapConfig, tileOffset, tileWidth);
	else
		mc = TopdownRenderView.latLngToMC(latLng, y, this.lmap, mapConfig, tileOffset, tileWidth);
	var x = mc[0], z = mc[1];
	
	// rotate the position in the other direction back from map rotation
	for(var i = 0; i < this.currentRotation; i++) {
		var nx = z; // 512 blocks = one region
		var nz = -x+512;
		x = nx;
		z = nz;
	}
	
	return [x, z, y];
};
