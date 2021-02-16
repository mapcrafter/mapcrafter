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
		var url = this._url;
		if(tile.x < 0 || tile.x >= Math.pow(2, tile.z) || tile.y < 0 || tile.y >= Math.pow(2, tile.z)) {
			url += "/blank";
		} else if(tile.z == 0) {
			url += "/base";
		} else {
			for(var z = tile.z - 1; z >= 0; --z) {
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
		tileSize: L.point(mapConfig.tileSize[0], mapConfig.tileSize[1]),
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
		// all pixel units here are pixels on the highest zoom level
		// size of the map in pixels
		var mapSize = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		// size of a quarter block = texture size / 2
		var quarterBlockSize = mapConfig.textureSize / 2;
		// each block has a row/column depending on x/z/y
		// each row is a half block high, each column a quarter block wide
		// row = x+z, column = z - x + (256-y)*2
		// so we can get the correct pixel coordinates of the mc coordinates, and then lat/lng:
		// 1. calculate row/column, multiply with row/column size (now pixel coordinates)
		// 2. map range [-mapSize/2, mapSize/2] to [0, mapSize/2] (like unproject wants it)
		// 3. apply little offset that is needed in this view
		// 4. apply tile offset
		// 5. pixel coordinates -> leaflet lat/lng with unproject
		var point = L.point(2 * (x + z), z - x + (256 - y) * 2).multiplyBy(quarterBlockSize)
			.add(L.point(mapSize / 2, mapSize / 2))
			.add(L.point(-mapConfig.textureSize * 16, 0))
			.add(L.point(-tileOffset[0], -tileOffset[1]).multiplyBy(mapConfig.tileSize[0]));
		return lmap.unproject(point, mapConfig.maxZoom);
	},
	
	latLngToMC: function(latLng, y, lmap, mapConfig, tileOffset, tileWidth) {
		var mapSize = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		var quarterBlockSize = mapConfig.textureSize / 2;
		// do the inverse translation from above
		var point = lmap.project(latLng, mapConfig.maxZoom)
			.add(L.point(tileOffset[0], tileOffset[1]).multiplyBy(mapConfig.tileSize[0]))
			.add(L.point(mapConfig.textureSize * 16, 0))
			.add(L.point(-mapSize / 2, -mapSize / 2))
		// remove block sizes from it
		point.x /= 2*quarterBlockSize;
		point.y /= quarterBlockSize;
		// solve the row = ... col = ... equation system and you get this:
		var x = 0.5 * (point.x - point.y - 2*y + 512);
		var z = 0.5 * (point.x + point.y + 2*y - 512);
		return [x, z, y];
	}
};

/**
 * Functions to convert Minecraft x, z, y (topdown render view) <-> Leaflet latitute/longitude.
 */
var TopdownRenderView = {
	mcToLatLng: function(x, z, y, lmap, mapConfig, tileOffset, tileWidth) {
		// like the isometric render view, except we don't have to deal with the row/col hassle
		var mapSize = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		var blockWidth = mapConfig.tileSize[0] / (16.0 * tileWidth);
		var point = L.point(x, z).multiplyBy(blockWidth)
			.add(L.point(mapSize / 2, mapSize / 2))
			.add(L.point(-tileOffset[0], -tileOffset[1]).multiplyBy(mapConfig.tileSize[0]));
		return lmap.unproject(point, mapConfig.maxZoom);
	},
	
	latLngToMC: function(latLng, y, lmap, mapConfig, tileOffset, tileWidth) {
		var mapSize = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		var blockWidth = mapConfig.tileSize[0] / (16.0 * tileWidth);
		// inverse transformation from above again
		var mc = lmap.project(latLng, mapConfig.maxZoom)
			.add(L.point(tileOffset[0], tileOffset[1]).multiplyBy(mapConfig.tileSize[0]))
			.add(L.point(-mapSize / 2, -mapSize / 2))
			.divideBy(blockWidth);
		return [mc.x, mc.y, y];
	}
};

/**
 * Functions to convert Minecraft x, z, y (side render view) <-> Leaflet latitute/longitude.
 */
var SideRenderView = {
	mcToLatLng: function(x, z, y, lmap, mapConfig, tileOffset, tileWidth) {
		var mapWidth = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		var mapHeight = mapConfig.tileSize[1] * Math.pow(2, mapConfig.maxZoom);
		var blockWidth = mapConfig.tileSize[0] / (16.0 * tileWidth);
		var blockHeight = mapConfig.tileSize[1] / (8.0 * tileWidth);

		// (z-1) because in the tile renderer we also subtract blockHeight/2 from y
		// leaflet x = x * blockWidth
		// leaflet y = (z - 1) * blockHeight / 2 + (255 - y) * blockHeight / 2
		var point = L.point(x, z - 1).scaleBy(L.point(blockWidth, blockHeight / 2))
			.add(L.point(mapWidth / 2, mapHeight / 2))
			.add(L.point(0, (255 - y) * blockHeight / 2))
			.add(L.point(-tileOffset[0] * mapConfig.tileSize[0], -tileOffset[1] * mapConfig.tileSize[1]));
		return lmap.unproject(point, mapConfig.maxZoom);
	},

	latLngToMC: function(latLng, y, lmap, mapConfig, tileOffset, tileWidth) {
		var mapWidth = mapConfig.tileSize[0] * Math.pow(2, mapConfig.maxZoom);
		var mapHeight = mapConfig.tileSize[1] * Math.pow(2, mapConfig.maxZoom);
		var blockWidth = mapConfig.tileSize[0] / (16.0 * tileWidth);
		var blockHeight = mapConfig.tileSize[1] / (8.0 * tileWidth);

		// x = leaflet x / blockWidth
		// (z - 1) = (leaflet y - (255 - y) * blockHeight / 2) / (blockHeight / 2)
		var point = lmap.project(latLng, mapConfig.maxZoom)
			.add(L.point(tileOffset[0] * mapConfig.tileSize[0], tileOffset[1] * mapConfig.tileSize[1]))
			.add(L.point(-mapWidth / 2, -mapHeight / 2));
		var x = point.x / blockWidth;
		var z = (point.y - (255 - y) * blockHeight / 2) / (blockHeight / 2);
		return [x, z + 1, y];
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
		// reset zoom, that seems to be important for leaflet
		this.lmap.setZoom(0);
	}
	
	// set the new map and rotation
	this.currentMap = map;
	this.currentRotation = parseInt(rotation);
	
	// remove the old map layer and set the new map layer
	if(oldMapLayer != null)
		this.lmap.removeLayer(oldMapLayer);
	this.lmap.addLayer(this.layers[this.currentMap][this.currentRotation]);
	this.lmap.invalidateSize();
	
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
			var cx = mapConfig.tileSize[0] / 2;
			var cy = mapConfig.tileSize[1] / 2
			this.lmap.setView(this.lmap.unproject([cx, cy]), zoom, {animate: false});
		}

	} else {
		// same world, we can set the view to the view of the old map
		// but make sure that we set a valid zoom level
		var newZoom = Math.round(oldZoom / oldMapConfig.maxZoom * mapConfig.maxZoom);
		this.lmap.setZoom(newZoom, {animate : false});
		this.lmap.setView(this.mcToLatLng(oldView[0], oldView[1], oldView[2]), newZoom, {animate: false});
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
			if(control.usePanelWrapper()) {
				wrapper.setAttribute("class", "control-wrapper control-wrapper-panel panel panel-default");
				wrapper.setAttribute("id", "control-wrapper-" + control.getName());
			} else {
				wrapper.setAttribute("class", "control-wrapper control-wrapper-invisible");
				wrapper.setAttribute("id", "control-wrapper-" + control.getName());
			}

			// just a dirty hack to prevent the map getting all mouse click events
			wrapper.onpointerover = function() {
				map.dragging.disable();
			};
			wrapper.onpointerout = function() {
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
	if (mapConfig.renderView == "isometric") {
		return IsometricRenderView.mcToLatLng(x, z, y, this.lmap, mapConfig, tileOffset, tileWidth);
	} else if (mapConfig.renderView == "topdown") {
		return TopdownRenderView.mcToLatLng(x, z, y, this.lmap, mapConfig, tileOffset, tileWidth);
	} else if (mapConfig.renderView == "side") {
		return SideRenderView.mcToLatLng(x, z, y, this.lmap, mapConfig, tileOffset, tileWidth);
	}
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
	if (mapConfig.renderView == "isometric") {
		mc = IsometricRenderView.latLngToMC(latLng, y, this.lmap, mapConfig, tileOffset, tileWidth);
	} else if (mapConfig.renderView == "topdown") {
		mc = TopdownRenderView.latLngToMC(latLng, y, this.lmap, mapConfig, tileOffset, tileWidth);
	} else if (mapConfig.renderView == "side") {
		mc = SideRenderView.latLngToMC(latLng, y, this.lmap, mapConfig, tileOffset, tileWidth);
	}
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
