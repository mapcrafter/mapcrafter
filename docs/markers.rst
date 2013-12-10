=======
Markers
=======

You can add your own markers to your map by editing the file ``markers.js``
in the output directory. This is a Javascript file which is included
in the web interface and contains definitions for the map markers. You 
can edit this file directly in the output directory (not in the template 
directory) because the renderer does not overwrite this file if it already 
exists.

The file has a associative Javascript-Array called ``MARKERS``. In this 
array keys are assigned to values. The keys are strings which represent 
names of rendered Minecraft Worlds. Each world has an array with markers
for this world.

For example:

.. code-block:: javascript

	var MARKERS = {
		"world1": [
			// markers for world1
		],
		
		"world2": [
			// markers for world2
		],
	};

The markers for the worlds ``world1`` and ``world2`` are now defined in
the places between the square brackets of the array.

.. note::

	This is Javascript syntax. Do not forget the quotation marks around
	strings or the commas after array elements. The lines starting with
	a ``//`` are comments and ignored by Javascript.

Marker Options
==============

The definition of markers is now also done with associative arrays::

	{"pos": [0, 0, 64], "title": "Spawnpoint"},

You can see here a simple marker with the title ``Spawnpoint`` and the
position ``0; 0; 64``. The position is always specified as array in the
form of ``[x, z, y]`` (x, z and then y because x and z are the horizontal
axes and y is the vertical axis).

Here are all available options for the markers:

``pos``

	**Required, Example:** ``[12, 34, 64]``

	This is the position of the marker in the form of ``[x, z, y]``.

``title``

	This is the title of the marker.

``text``

	**Default:** ``<title>``

	This is the text of the marker window. If you do not specifiy a text,
	the title of the marker is used as text.

``icon``

	This is the name of an icon image for the marker. If you do not specify
	an icon, the default icon is used. If you specifiy your own icon,
	you should also specify the size of the image (especially for the
	Leaflet template). You can copy your own icons to the ``icons``
	directory of the template and then use them with something
	like ``icons/myicon.png``.

``iconsize``

	**Default:** ``[24, 24]`` (if an icon is specified)

Now you can add these markers to the belonging worlds:

.. code-block:: javascript

	var MARKERS = {
		"world1": [
			// markers for world1
			{"pos": [0, 0, 64], "title": "Spawnpoint"},
			{"pos": [12, 34, 64], "title": "Marker", "icon": "icons/myicon.png", "iconsize": [32, 32]},
			{"pos": [34, 12, 64], "title": "Another Marker", "text": "This is <b>another</b> marker"},
		],
		
		"world2": [
			// markers for world2
			{"pos": [0, 0, 64], "title": "Marker"},
			{"pos": [-100, 100, 64], "title": "And yet another Marker"},
		],
	};

Minecraft Server
================

If you want player markers from your Minecraft Server on your map, please 
have a look at the `mapcrafter-playermarkers <https://github.com/m0r13/mapcrafter-playermarkers>`_
project.

The plugin adds to your map animated markers of the players on your Minecraft
Server.
