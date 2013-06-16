// Here you can add your own markers.

var MARKERS = {
	// You have to specify a world name for the markers.
	// This is the last part of the path to the world.
	// For example: The name for "/this/is/my_world" is just "my_world".
	"worldname": [
		// A simple example for a marker:
		{"pos": [0, 0, 64], "title": "Spawnpoint"},
		// The position of a marker is always in the form [x, z, y].
		
		// You can add your own icon:
		{"pos": [12, 34, 64], "title": "Marker", "icon": "icons/myicon.png"},
		// Just copy your icon file into your template directory to use it here.
		// A size of 24x24 pixel for your icons would be good.
		// Otherwise you should also specify the size of your icon (for the Leaflet template):
		{"pos": [12, 34, 64], "title": "Marker", "icon": "icons/myicon.png", "iconsize": [32, 32]},
		
		// And/or you can set the text of the marker window:
		{"pos": [34, 12, 64], "title": "Another Marker", "text": "This is <b>another</b> marker"},
	],
	
	// And you can specify the markers of your other worlds:
	"world2": [
		{"pos": [0, 0, 64], "title": "Marker"},
	],
	
	"world2": [
		{"pos": [0, 0, 64], "title": "Marker"},
	],
	
	// etc.
};