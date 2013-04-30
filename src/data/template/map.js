function convertMCtoLatLng(x, z, y) {
	// converts Minecraft x,z,y to a Google Map lat/lng
	
	var config = getCurrentConfig();
	
	// rotate the position to the map rotation
	for(var i = 0; i < currentRotation; i++) {
		var nx = -z+512; // 512 blocks = one region
		var nz = x;
		x = nx;
		z = nz;
	}
	
	// the size of a 1/4 block image divided by the total size of all render tiles 
	// on the highest zoom level
	var block = (config.textureSize/2.0) / (config.tileSize * Math.pow(2, getCurrentMaxZoom()));
	
	// at first calculate the row and the column of the block
	// column is just x+z 
	var col = x+z;
	// row is z-x, and every y to bottom adds 2 rows
	var row = z-x + (256-y)*2;

	// midpoint of the map is in lat/lng 0.5|0.5
	// we have to move the lng by the size of one tile
	// lng is now 2*block size for every column
	var lng = 0.5 - (1.0 / Math.pow(2, getCurrentMaxZoom() + 1)) + col * 2*block;
	// lat is now one block size for every row 
	var lat = 0.5 + row * block;
	
	return new google.maps.LatLng(lat, lng);
}

function convertLatLngToMC(latlng, y) {
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
	
	var config = getCurrentConfig();
	
	var lat = latlng.lat();
	var lng = latlng.lng();
	
	var tile = (1.0 / Math.pow(2, getCurrentMaxZoom() + 1));
	var block = (config.textureSize/2.0) / (config.tileSize * Math.pow(2, getCurrentMaxZoom()));
	
	var z = (4*y*block - 1024*block + 2*lat + lng + tile - 1.5) / (4*block);
	var x = (0.5 - tile - lng + 2*z*block) / (-2*block);
	
	// rotate the position in the other direction
	for(var i = 0; i < currentRotation; i++) {
		var nx = z; // 512 blocks = one region
		var nz = -x+512;
		x = nx;
		z = nz;
	}
	
	return [x, z, y];
}

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

function createMapType(path, rotation) {
	var config = getConfig(path);
	var type = new google.maps.ImageMapType({
		getTileUrl: function(tile, zoom) {
			var url = path + "/" + ["tl", "tr", "br", "bl"][rotation];
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
			return(url);
		},
		tileSize: new google.maps.Size(config.tileSize, config.tileSize),
		maxZoom: config.zoomLevels[rotation],
		minZoom: 0,
		isPng: true
	});
	type.name = config.name;
	type.alt = path;
	type.projection = new MapProjection(getConfig(path));
	console.log(config);
	return type;
}

var map;
var currentType, currentRotation;

function getConfig(type) {
	return MapConfig[type];
}

function getCurrentConfig() {
	return getConfig(currentType);
}

function getCurrentMaxZoom() {
	return getCurrentConfig().zoomLevels[currentRotation];
}

function setMapType(type, rotation) {
	var oldType = getConfig(currentType);
	var newType = getConfig(type);
	
	xzy = convertLatLngToMC(map.getCenter(), 64);
	currentType = type;
	currentRotation = parseInt(rotation);
	
	map.setMapTypeId(type + "-" + rotation);
	if(oldType.worldName != newType.worldName) {
		map.setCenter(new google.maps.LatLng(0.5, 0.5));
		map.setZoom(0);
	} else {
		//console.log("Current position: " + xzy);
		
		map.setCenter(convertMCtoLatLng(xzy[0], xzy[1], xzy[2]));
		
		//console.log("New position: " + convertLatLngToMC(map.getCenter(), 64));
	}
	
	updateRotationSelect();
	PosHash.updateHash()
}

function updateRotationSelect(text) {	
	if(typeof text == "undefined")
		text = document.getElementById("map-rotation-select");
	if(!text)
		return;
	text.innerHTML = "<span>Rotation: </span>";
	
	for(var i in getCurrentConfig().zoomLevels) {
		var elem;
		if(i != currentRotation) {
			elem = document.createElement("a");
			elem.setAttribute("href", "#");
			elem.setAttribute("data-rotation", i);
			elem.onclick = function() {
				setMapType(currentType, this.getAttribute("data-rotation"));
				return false;
			};
		} else {
			elem = document.createElement("b");
		}
		
		elem.innerHTML = ["top-left", "top-right", "bottom-right", "bottom-left"][i];
		elem.style.marginRight = "5px";
		elem.style.marginLeft = "5px";
		text.appendChild(elem);
	}
}

function addControl(pos, index, func) {
	var wrapper = document.createElement("div");
	wrapper.style.margin = "5px";
	wrapper.style.padding = "3px";
	wrapper.style.border = "1px solid gray";
	wrapper.style.backgroundColor = "white";
	
	func(wrapper);
	wrapper.index = pos;
	
	map.controls[pos].push(wrapper);
}

function initMarkers() {
	var infowindow = new google.maps.InfoWindow();
	var current = {};

	for(var i = 0; i < MARKERS.length; i++) {  
		var location = MARKERS[i];
		
		var markerOptions = {
			position: convertMCtoLatLng(location.x, location.z, location.y),
			map: map,
			title: location.title,
		};
		if(location.icon)
			markerOptions["icon"] = location.icon;
		var marker = new google.maps.Marker(markerOptions);

		google.maps.event.addListener(marker, "click", (function(marker, location) {
			return function() {
				if(current == location) {
					infowindow.close();
					current = {};
					return;
				}
				infowindow.setContent(location.text ? location.text : location.title);
				infowindow.open(map, marker);
				current = location;
			}
		})(marker, location));
	}
}

var PosHash = {
	"parseHash": function() {
		if(!location.hash)
			return null;
		
		var url = location.hash.substr(1);
		var split = url.split("/");
		
		if(split.length != 6)
			return null;
		for(var i = 1; i < 6; i++)
			split[i] = parseInt(split[i]);
		return split;
	},
	
	"updateHash": function() {
		var type = currentType;
		var rotation = currentRotation;
		var xzy = convertLatLngToMC(map.getCenter(), 64);
		for(var i = 0; i < 3; i++)
			xzy[i] = Math.round(xzy[i]);
		var zoom = map.getZoom();
		window.location.replace("#" + type + "/" + rotation + "/" + zoom + "/" + xzy[0] + "/" + xzy[1] + "/" + xzy[2]);
	},
	
	"gotoHash": function(hash) {
		if(!hash)
			return;
		
		if(!(hash[0] in MapConfig) || !(hash[1] in MapConfig[hash[0]]))
			return null;
			
		setMapType(hash[0], hash[1]);
			
		var latlng = convertMCtoLatLng(hash[3], hash[4], hash[5]);
		map.setCenter(latlng);
		map.setZoom(hash[2]);
	},
};

function init() {
	var mapOptions = {
		zoom: 0,
		center: new google.maps.LatLng(0.5, 0.5),
		
		navigationControl: true,
		scaleControl: false,
		mapTypeControl: false,
		streetViewControl: false,
	};
	
	map = new google.maps.Map(document.getElementById("mcmap"), mapOptions);
	
	var firstType = true;
	for(var type in MapConfig) {
		for(var rotation in MapConfig[type].zoomLevels) {
			map.mapTypes.set(type + "-" + rotation, createMapType(type, rotation));
			if(firstType) {
				currentType = type;
				currentRotation = rotation;
				setMapType(type, rotation);
				firstType = false;
			}
		}
	}
	
	// init position hash and register event handlers
	PosHash.gotoHash(PosHash.parseHash());
	PosHash.updateHash();
	google.maps.event.addListener(map, "dragend", PosHash.updateHash);
	google.maps.event.addListener(map, "zoom_changed", PosHash.updateHash);

	// widget to select the world
	addControl(google.maps.ControlPosition.TOP_RIGHT, 1, function(wrapper) {
		var select = document.createElement('select');
		select.style.padding = "5px";
		select.style.border = "1px solid gray";
		select.style.backgroundColor = "white";
		
		for(var type in MapConfig) {
			var option = document.createElement("option");
			option.innerHTML = MapConfig[type].name;
			option.setAttribute("value", type);
			select.appendChild(option);
		}
		
		select.onchange = function() {
			for(rotation in getConfig(select.value).zoomLevels) {
				setMapType(select.value, rotation);
				break;
			}
		};
		
		var text = document.createElement("span");
		text.innerHTML = "Map type: ";
		
		wrapper.appendChild(text);
		wrapper.appendChild(select);
	});
	
	// widget to select the rotation of the current world
	addControl(google.maps.ControlPosition.BOTTOM_CENTER, 1, function(wrapper) {
		var text = document.createElement("span");
		text.setAttribute("id", "map-rotation-select");
		text.innerHTML = "no update";
		updateRotationSelect(text);
		
		wrapper.appendChild(text);
	});
	
	// widget to show the current cursor position in Minecraft coordinates
	addControl(google.maps.ControlPosition.BOTTOM_LEFT, 1, function(wrapper) {
		var text = document.createElement("span");
		text.setAttribute("id", "mouse-move-div");
		
		google.maps.event.addListener(map, "mousemove", function(event) {
			var xzy = convertLatLngToMC(event.latLng, 64);
			document.getElementById("mouse-move-div").innerHTML = "x: " + Math.round(xzy[0]) 
				+ " z: " + Math.round(xzy[1]) + " y: " + Math.round(xzy[2]);
		});
		
		wrapper.appendChild(text);
	});

}
