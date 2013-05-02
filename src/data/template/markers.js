var lastRender = new Date();
lastRender.setTime(MapConfig.lastRender * 1000);

var MARKERS = [
	// You can add here your own markers, for example:
	// {"world": "worldname", "x": 42, "z": 73, "y": 64, "title": "Spawnpoint"},
	
	// You have to specify a world name, this is the last part of the path to the world.
	// For exmaple: The name for "/this/is/my_world" is just "my_world".
	
	// You can add your own icon:
	// {"world": "worldname", "x": 42, "z": 73, "y": 64, "title": "Spawnpoint", "icon": "icons/myicon.png"},
	
	// and/or you can set the text of the marker window:
	// (text defaults to the title of the marker)
	// {"world": "worldname", "x": 42, "z": 73, "y": 64, "title": "Spawnpoint", "text": "This is the <b>spawnpoint</b>."},
	
	
	// You want to show when the map was rendered last time?
	// {"world": "worldname", "x": 0, "z": 0, "y": 64, "title": "Last render", "text": "Last render: " + lastRender.toLocaleString()},
];