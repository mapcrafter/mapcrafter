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
		if(!(i in config.rotations)) {
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
	
	google.maps.event.addListener(this.ui.gmap, "dragend", handler);
	google.maps.event.addListener(this.ui.gmap, "zoom_changed", handler);
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
	var xzy = this.ui.latLngToMC(this.ui.gmap.getCenter(), 64);
	for(var i = 0; i < 3; i++)
		xzy[i] = Math.round(xzy[i]);
	var zoom = this.ui.gmap.getZoom();
	window.location.replace("#" + type + "/" + rotation + "/" + zoom + "/" + xzy[0] + "/" + xzy[1] + "/" + xzy[2]);
};

MapPosHashHandler.prototype.gotoHash = function(hash) {
	if(!hash)
		return;
	
	if(!(hash[0] in this.ui.getAllConfig()) 
			|| !(hash[1] in this.ui.getConfig(hash[0]).rotations))
		return null;
		
	this.ui.setMapType(hash[0], hash[1]);
		
	var latlng = this.ui.mcToLatLng(hash[3], hash[4], hash[5]);
	this.ui.gmap.setCenter(latlng);
	this.ui.gmap.setZoom(hash[2]);
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
		this.mapMarkers[i].setMap(null);
	this.mapMarkers = [];
	
	var infowindow = new google.maps.InfoWindow();
	var current = {};

	for(var i = 0; i < this.markers.length; i++) {  
		var location = this.markers[i];
		
		if(location.world != this.ui.getCurrentConfig().worldName)
			continue;
		
		var markerOptions = {
			position: this.ui.mcToLatLng(location.x, location.z, location.y),
			map: this.ui.gmap,
			title: location.title,
		};
		if(location.icon)
			markerOptions["icon"] = location.icon;
		var marker = new google.maps.Marker(markerOptions);
		this.mapMarkers.push(marker);

		google.maps.event.addListener(marker, "click", (function(ui, marker, location) {
			return function() {
				if(current == location) {
					infowindow.close();
					current = {};
					return;
				}
				infowindow.setContent(location.text ? location.text : location.title);
				infowindow.open(ui.gmap, marker);
				current = location;
			}
		})(this.ui, marker, location));
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
		img.setAttribute("src", "icons/" + names[i] + ".png");
		a.appendChild(img);
		
		var img2 = document.createElement("img");
		img2.setAttribute("id", "rotation-" + names[i] + "-active");
		img2.setAttribute("class", "rotation");
		img2.setAttribute("src", "icons/" + names[i] + "_active.png");
		
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
	
	google.maps.event.addListener(this.ui.gmap, "mousemove", (function(ui) {
		return function(event) {
			var xzy = ui.latLngToMC(event.latLng, 64);
			document.getElementById("mouse-move-div").innerHTML = "X: " + Math.round(xzy[0]) 
				+ " Z: " + Math.round(xzy[1]) + " Y: " + Math.round(xzy[2]);
		};
	})(this.ui));
	
	wrapper.appendChild(text);
}

/**
 * The main map class.
 */
function MapcrafterUI(config) {
	this.config = config;
	
	this.currentType = null;
	this.currentRotation = null;
	
	this.gmap = null;
	
	this.handlers = [];
	this.controlsNotCreated = [];
	this.handlersNotCreated = [];
	this.created = false
	
	this.addHandler(new MapPosHashHandler());
}

MapcrafterUI.prototype.init = function() {
	var mapOptions = {
		zoom: 0,
		center: new google.maps.LatLng(0.5, 0.5),
		
		navigationControl: true,
		scaleControl: false,
		mapTypeControl: false,
		streetViewControl: false,
	};
	
	this.gmap = new google.maps.Map(document.getElementById("mcmap"), mapOptions);
	
	this.handlers = [];
	
	var firstType = true;
	for(var type in this.config) {
		for(var rotation in this.config[type].rotations) {
			this.gmap.mapTypes.set(type + "-" + rotation, 
					this.createMapType(type, this.config[type], rotation));
			if(firstType) {
				this.currentType = type;
				this.currentRotation = rotation;
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
	var oldType = this.getCurrentConfig();
	var newType = this.getConfig(type);
	
	xzy = this.latLngToMC(this.gmap.getCenter(), 64);
	this.currentType = type;
	this.currentRotation = parseInt(rotation);
	
	var oldZoom = this.gmap.getZoom();
	this.gmap.setMapTypeId(type + "-" + rotation);
	if(oldType.worldName != newType.worldName) {
		this.gmap.setCenter(new google.maps.LatLng(0.5, 0.5));
		this.gmap.setZoom(0);
	} else {
		this.gmap.setCenter(this.mcToLatLng(xzy[0], xzy[1], xzy[2]));
		
		// adjust the zoom level
		// if one switches between maps with different max zoom levels
		if(oldType.maxZoom != newType.maxZoom) {
			this.gmap.setZoom(oldZoom + newType.maxZoom - oldType.maxZoom);
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

/**
 * From Minecraft Overviewer.
 */
// our custom projection maps Latitude to Y, and Longitude to X as normal,
// but it maps the range [0.0, 1.0] to [0, tileSize] in both directions
// so it is easier to position markers, etc. based on their position
// (find their position in the lowest-zoom image, and divide by tileSize)
function MapProjection(config) {
	this.config = config
	this.inverseTileSize = 1.0 / this.config.tileSize;
}

MapProjection.prototype.fromLatLngToPoint = function(latLng) {
	var x = latLng.lng() * this.config.tileSize;
	var y = latLng.lat() * this.config.tileSize;
	return new google.maps.Point(x, y);
};

MapProjection.prototype.fromPointToLatLng = function(point) {
	var lng = point.x * this.inverseTileSize;
	var lat = point.y * this.inverseTileSize;
	return new google.maps.LatLng(lat, lng);
};

MapcrafterUI.prototype.createMapType = function(name, config, rotation) {
	var type = new google.maps.ImageMapType({
		getTileUrl: function(tile, zoom) {
			var url = name + "/" + ["tl", "tr", "br", "bl"][rotation];
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
		tileSize: new google.maps.Size(config.tileSize, config.tileSize),
		maxZoom: config.maxZoom,
		minZoom: 0,
		isPng: true
	});
	type.name = config.name;
	type.alt = name;
	type.projection = new MapProjection(config);
	return type;
};

MapcrafterUI.prototype.addControl = function(control, pos, index) {
	if(!this.created) {
		this.controlsNotCreated.push([control, pos, index]);
		return;
	}
	
	var wrapper = document.createElement("div");
	wrapper.setAttribute("class", "control-wrapper");
	wrapper.setAttribute("id", "control-wrapper-" + name);
	
	control.ui = this;
	control.create(wrapper);
	wrapper.index = index;
	
	var handler = control.getHandler();
	if(handler != null) {
		this.addHandler(handler);
	}
	
	this.gmap.controls[pos].push(wrapper);
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
	
	return new google.maps.LatLng(lat, lng);
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
	
<<<<<<< HEAD
	var lat = latlng.lat();
	var lng = latlng.lng();
	
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
