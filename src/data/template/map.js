/**
 * From Minecraft Overviewer.
 */
// our custom projection maps Latitude to Y, and Longitude to X as normal,
// but it maps the range [0.0, 1.0] to [0, tileSize] in both directions
// so it is easier to position markers, etc. based on their position
// (find their position in the lowest-zoom image, and divide by tileSize)
function MapProjection() {
	this.inverseTileSize = 1.0 / MapConfig.tileSize;
}

MapProjection.prototype.fromLatLngToPoint = function(latLng) {
	var x = latLng.lng() * MapConfig.tileSize;
	var y = latLng.lat() * MapConfig.tileSize;
	return new google.maps.Point(x, y);
};

MapProjection.prototype.fromPointToLatLng = function(point) {
	var lng = point.x * this.inverseTileSize;
	var lat = point.y * this.inverseTileSize;
	return new google.maps.LatLng(lat, lng);
};

function convertMCtoLatLng(x, z, y) {
	// converts Minecraft x,z,y to a Google Map lat/lng
	
	// rotate the position to the map rotation
	for(var i = 0; i < MapConfig.rotation; i++) {
		var nx = -z+512; // 512 blocks = one region
		var nz = x;
		x = nx;
		z = nz;
	}
	
	// the size of a 1/4 block image divided by the total size of all render tiles 
	// on the highest zoom level
	var block = (MapConfig.textureSize/2.0) / (MapConfig.tileSize * Math.pow(2, MapConfig.maxZoom));
	
	// at first calculate the row and the column of the block
	// column is just x+z 
	var col = x+z;
	// row is z-x, and every y to bottom adds 2 rows
	var row = z-x + (256-y)*2;

	// midpoint of the map is in lat/lng 0.5|0.5
	// we have to move the lng by the size of one tile
	// lng is now 2*block size for every column
	var lng = 0.5 - (1.0 / Math.pow(2, MapConfig.maxZoom + 1)) + col * 2*block;
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
	
	var lat = latlng.lat();
	var lng = latlng.lng();
	
	var tile = (1.0 / Math.pow(2, MapConfig.maxZoom + 1));
	var block = (MapConfig.textureSize/2.0) / (MapConfig.tileSize * Math.pow(2, MapConfig.maxZoom));
	
	var z = (4*y*block - 1024*block + 2*lat + lng + tile - 1.5) / (4*block);
	var x = (0.5 - tile - lng + 2*z*block) / (-2*block);
	
	// rotate the position in the other direction
	for(var i = 0; i < MapConfig.rotation; i++) {
		var nx = z; // 512 blocks = one region
		var nz = -x+512;
		x = nx;
		z = nz;
	}
	
	return [x, z, y];
}

function MousePosControl(div, map) {
	div.style.padding = "5px";
	
	var wrapper = document.createElement("div");
	wrapper.style.padding = "3px";
	wrapper.style.border = "1px solid gray";
	wrapper.style.backgroundColor = "white";
	
	var text = document.createElement("span");
	text.setAttribute("id", "mouse-move-div");
	text.innerHTML = "test";
	google.maps.event.addListener(map, "mousemove", function(event) {
		var xzy = convertLatLngToMC(event.latLng, 64);
		document.getElementById("mouse-move-div").innerHTML = "x: " + Math.round(xzy[0]) + " z: " + Math.round(xzy[1]) + " y: " + Math.round(xzy[2]);
	});
	
	wrapper.appendChild(text);
	div.appendChild(wrapper);
}

var MCMapOptions = {
	/**
	 * From Minecraft Overviewer.
	 */
	getTileUrl: function(tile, zoom) {
		var url = ".";
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
	tileSize: new google.maps.Size(MapConfig.tileSize, MapConfig.tileSize),
	maxZoom: MapConfig.maxZoom,
	minZoom: MapConfig.minZoom,
	isPng: true
};

var map;

function init() {
	var MCMapType = new google.maps.ImageMapType(MCMapOptions);
	MCMapType.name = "Minecraft Map";
	MCMapType.alt = "Minecraft Map";
	MCMapType.projection = new MapProjection();
	
	var mapOptions = {
		zoom: MapConfig.defaultZoom,
		center: new google.maps.LatLng(0.5, 0.5),
		
		navigationControl: true,
		scaleControl: false,
		mapTypeControl: false,
		streetViewControl: false,
	};
	
	map = new google.maps.Map(document.getElementById("mcmap"), mapOptions);
	map.mapTypes.set("mcmap", MCMapType);
	map.setMapTypeId("mcmap");
	
	var mouseDiv = document.createElement("div");
	var mousePos = new MousePosControl(mouseDiv, map);
	mouseDiv.index = 1;
	map.controls[google.maps.ControlPosition.BOTTOM_LEFT].push(mouseDiv);
}