/**
 * This is the base class to implement different handlers which are called
 * when the type or the rotation of the map is changed.
 */
function MapHandler() {
	this.ui = null;
}

MapHandler.prototype.create = function() {
};

MapHandler.prototype.onMapChange = function(name, rotation) {
};

MapRotationHandler.prototype = new MapHandler();

/**
 * Updates the control widget with the map rotation selection.
 */
function MapRotationHandler(control) {
	this.control = control
}

MapRotationHandler.prototype.create = function() {
}

MapRotationHandler.prototype.onMapChange = function(name, rotation) {
	this.update();
};

MapRotationHandler.prototype.update = function(text) {	
	var config = this.ui.getCurrentConfig();
	var currentRotation = this.ui.getCurrentRotation();
	
	var images = this.control.images;
	for(var i = 0; i < 4; i++) {
		if(config.rotations.indexOf(i) == -1) {
			images[2*i].style.display = "none";
			images[2*i+1].style.display = "none";
		} else {
			if(i == currentRotation) {
				images[2*i].style.display = "none";
				images[2*i+1].style.display = "inline";
			} else {
				images[2*i].style.display = "inline";
				images[2*i+1].style.display = "none";
			}
		}
	}
}

MapPosHashHandler.prototype = new MapHandler();

/**
 * Is responsible for the url position hash.
 */
function MapPosHashHandler() {
}

MapPosHashHandler.prototype.create = function() {
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

MapPosHashHandler.prototype.onMapChange = function(name, rotation) {
	this.updateHash();
};

MapPosHashHandler.prototype.parseHash = function() {
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

MapPosHashHandler.prototype.updateHash = function() {
	var type = this.ui.getCurrentType();
	var rotation = this.ui.getCurrentRotation();
	var xzy = this.ui.latLngToMC(this.ui.lmap.getCenter(), 64);
	for(var i = 0; i < 3; i++)
		xzy[i] = Math.round(xzy[i]);
	var zoom = this.ui.lmap.getZoom();
	window.location.replace("#" + type + "/" + rotation + "/" + zoom + "/" + xzy[0] + "/" + xzy[1] + "/" + xzy[2]);
};

MapPosHashHandler.prototype.gotoHash = function(hash) {
	if(!hash)
		return;
	
	if(!(hash[0] in this.ui.getAllConfig()) 
			|| this.ui.getConfig(hash[0]).rotations.indexOf(hash[1]) < 0)
		return null;
		
	this.ui.setMapTypeAndRotation(hash[0], hash[1]);
		
	var latlng = this.ui.mcToLatLng(hash[3], hash[4], hash[5]);
	this.ui.lmap.setView(latlng, hash[2]);
};

MapMarkerHandler.prototype = new MapHandler();

/**
 * Is responsible to show the markers from the markers.js.
 */
function MapMarkerHandler(markers) {
	this.mapMarkers = [];
	this.markers = markers;
}

MapMarkerHandler.prototype.onMapChange = function(name, rotation) {
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

/**
 * This base class is used to implement different map control widgets.
 * 
 * These controls can also have their own handler. Just return
 * in getHandler() your handler.
 */
function MapControl() {
	this.ui = null;
}

MapControl.prototype.create = function(wrapper) {
}

MapControl.prototype.getHandler = function() {
	return null;
}

MapControl.prototype.getName = function(wrapper) {
	return "";
}

MapSelectControl.prototype = new MapControl();

/**
 * This control widget allows the user to select a map.
 */
function MapSelectControl() {
}

MapSelectControl.prototype.create = function(wrapper) {
	var select = document.createElement("select");
	select.setAttribute("id", "map-select");
	
	for(var type in this.ui.getAllConfig()) {
		var option = document.createElement("option");
		option.innerHTML = this.ui.getConfig(type).name;
		option.setAttribute("value", type);
		if(this.ui.getCurrentType() == type)
			option.setAttribute("selected", true);
		select.appendChild(option);
	}
	
	select.addEventListener("change", (function(ui) {
		return function() {
			ui.setMapType(select.value);
		}
	})(this.ui));
	
	var text = document.createElement("span");
	text.innerHTML = "Map type: ";
	
	wrapper.appendChild(text);
	wrapper.appendChild(select);
};

MapSelectControl.prototype.getName = function() {
	return "map-select";
}

RotationSelectControl.prototype = new MapControl();

/**
 * This control widget allows the user to rotate the map.
 */
function RotationSelectControl() {
	this.handler = new MapRotationHandler(this);
	
	this.images = [];
}

RotationSelectControl.prototype.create = function(wrapper) {
	var names = ["tl", "tr", "br", "bl"];
	for(var i = 0; i < 4; i++) {
		var a = document.createElement("a");
		a.setAttribute("href", "#");
		a.setAttribute("data-rotation", i);
		a.addEventListener("click", (function(ui) {
			return function(event) {
				event.preventDefault();
				ui.setMapRotation(this.getAttribute("data-rotation"));
				return false;
			};
		})(this.ui));
		
		img = document.createElement("img");
		img.setAttribute("id", "rotation-" + names[i]);
		img.setAttribute("class", "rotation");
		img.setAttribute("src", "images/" + names[i] + ".png");
		a.appendChild(img);
		
		var img2 = document.createElement("img");
		img2.setAttribute("id", "rotation-" + names[i] + "-active");
		img2.setAttribute("class", "rotation");
		img2.setAttribute("src", "images/" + names[i] + "_active.png");
		
		wrapper.appendChild(a);
		wrapper.appendChild(img2);
		
		this.images.push(a);
		this.images.push(img2);
	}
};

RotationSelectControl.prototype.getHandler = function() {
	return this.handler;
}

RotationSelectControl.prototype.getName = function() {
	return "rotation-select";
}

MousePosControl.prototype = new MapControl();

/**
 * This control widget shows the current position of the mouse in Minecraft.
 */
function MousePosControl() {
}

MousePosControl.prototype.create = function(wrapper) {
	var text = document.createElement("span");
	text.setAttribute("id", "mouse-move-div");
	
	this.ui.lmap.on("mousemove", (function(ui) {
		return function(event) {
			var xzy = ui.latLngToMC(event.latlng, 64);
			document.getElementById("mouse-move-div").innerHTML = "X: " + Math.round(xzy[0]) 
				+ " Z: " + Math.round(xzy[1]) + " Y: " + Math.round(xzy[2]);
		};
	})(this.ui));
	
	wrapper.appendChild(text);
}

HTMLControl.prototype = new MapControl();

/**
 * A simple control with user-defined content.
 */
function HTMLControl(html) {
	this.html = html;
}

HTMLControl.prototype.create = function(wrapper) {
	wrapper.innerHTML = this.html;
};

var MCTileLayer = L.TileLayer.extend({
	initialize: function(url, options) {
		this._url = url;
		
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
		url = url + ".png";
		return url;
	},
});

/**
 * The main map class.
 */
function MapcrafterUI(config) {
	this.config = config;
	
	this.currentType = null;
	this.currentRotation = null;
	
	this.lmap = null;
	this.layers = {};
	
	this.handlers = [];
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
	this.created = false
	
	this.addHandler(new MapPosHashHandler());
}

MapcrafterUI.prototype.init = function() {
	this.lmap = L.map("mcmap", {
		crs: L.CRS.Simple
	}).setView([0, 0], 0);
	
	var firstType = true;
	for(var type in this.config) {
		this.layers[type] = {};
		for(var i in this.config[type].rotations) {
			var rotation = this.config[type].rotations[i];
			this.layers[type][rotation] = this.createTileLayer(type, this.config[type], rotation);
			if(firstType) {
				this.setMapTypeAndRotation(type, rotation);
				firstType = false;
			}
		}
	}
	
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

MapcrafterUI.prototype.getAllConfig = function() {
	return this.config;
};

MapcrafterUI.prototype.getCurrentType = function() {
	return this.currentType;
};

MapcrafterUI.prototype.getCurrentRotation = function() {
	return this.currentRotation;
}

MapcrafterUI.prototype.getConfig = function(type) {
	return this.config[type];
};

MapcrafterUI.prototype.getCurrentConfig = function() {
	return this.getConfig(this.currentType);
};

MapcrafterUI.prototype.setMapTypeAndRotation = function(type, rotation) {
	var oldConfig = this.getCurrentConfig();
	var oldRotation = this.currentRotation;
	var oldLayer = null;
	var xzy = null;
	if(this.currentType != null && this.currentRotation != null) {
		oldLayer = this.layers[this.currentType][this.currentRotation];
		xzy = this.latLngToMC(this.lmap.getCenter(), 64);
	}
	
	this.currentType = type;
	this.currentRotation = parseInt(rotation);
	var config = this.getCurrentConfig();
	
	var oldZoom = this.lmap.getZoom();
	if(oldLayer != null) {
		this.lmap.removeLayer(oldLayer);
	}
	this.lmap.addLayer(this.layers[this.currentType][this.currentRotation]);
	
	if(oldLayer == null || oldConfig.worldName != config.worldName) {
		// set view to the center
		this.lmap.setView(this.lmap.unproject([config.tileSize/2, config.tileSize/2]), 0);
	} else /*if(this.currentRotation != oldRotation)*/ {
		this.lmap.setView(this.mcToLatLng(xzy[0], xzy[1], xzy[2]), oldZoom);
		
		// adjust the zoom level
		// if one switches between maps with different max zoom levels
		if(oldConfig.maxZoom != config.maxZoom) {
			this.lmap.setZoom(oldZoom + config.maxZoom - oldConfig.maxZoom);
		}
	}
	
	for(var i = 0; i < this.handlers.length; i++) {
		this.handlers[i].onMapChange(this.currentType, this.currentRotation);
	}
};

MapcrafterUI.prototype.setMapType = function(type) {
	var current = this.getCurrentConfig();
	var other = this.getConfig(type);
	
	var sameWorld = current.worldName == other.worldName;
	if(sameWorld && this.currentRotation in other.rotations)
		this.setMapTypeAndRotation(type, this.currentRotation);
	else
		this.setMapTypeAndRotation(type, other.rotations[0]);
};

MapcrafterUI.prototype.setMapRotation = function(rotation) {
	this.setMapTypeAndRotation(this.currentType, rotation);
};

MapcrafterUI.prototype.createTileLayer = function(name, config, rotation) {
	var layer = new MCTileLayer(name + "/" + ["tl", "tr", "br", "bl"][rotation], {
		maxZoom: config.maxZoom,
		tileSize: config.tileSize,
		noWrap: true,
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
			}
			
			control.ui = self;
			control.create(wrapper);
			wrapper.index = index;
			
			var handler = control.getHandler();
			if(handler != null) {
				self.addHandler(handler);
			}
			
			return wrapper;
		},
	});
	var lcontrol = new ControlType({
		position: pos,
	});
	this.lmap.addControl(lcontrol);
};

MapcrafterUI.prototype.addHandler = function(handler) {
	if(!this.created) {
		this.handlersNotCreated.push(handler);
		return;
	}
	
	handler.ui = this;
	handler.create();
	handler.onMapChange(this.currentType, this.currentRotation);
	this.handlers.push(handler);
};

MapcrafterUI.prototype.mcToLatLng = function(x, z, y) {
	// converts Minecraft x,z,y to a Google Map lat/lng
	
	var config = this.getCurrentConfig();
	
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
	
	// at first calculate the row and the column of the block
	// column is just x+z 
	var col = x+z;
	// row is z-x, and every y to bottom adds 2 rows
	var row = z-x + (256-y)*2;

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
	
	var config = this.getCurrentConfig();
	
	// same way like in the other method
	// we convert the lat/lng coordinates to pixel coordinates
	// then we need to convert the pixel coordinates to lat/lng coordinates in the range [0; 1]
	// to use them for the lat/lng -> MC algorithm
	var point = this.lmap.project(latlng);
	var size = config.tileSize * Math.pow(2, this.lmap.getZoom());
	var lat = point.y / size;
	var lng = point.x / size;
	
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
