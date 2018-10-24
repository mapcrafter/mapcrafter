.. _markers:

=======
Markers
=======

Mapcrafter allows you to add different markers easily to your rendered
maps. Markers are organized in marker groups, this allows you show
and hide different marker groups on the rendered map.

Automatically Generated Markers
===============================

Mapcrafter is able to automatically generate markers from `signs <TODO link to minecraft signs>`_ you place
in your Minecraft world.

A special marker section type is used to configure automatically generated
marker groups. Here is an example:

.. code-block:: ini

    [marker:homes]
    name = Homes
    prefix = [home]
    icon = home.png
    icon_size = [32, 32]

This section defines a marker group showing different homes in your 
Minecraft world. Every sign that starts with the prefix ``[home]`` is 
shown on the map as marker in the *Homes* group.

See :ref:`config_marker_options` for a reference of marker section options.

To automatically generate these markers, use the ``mapcrafter_markers``
program with your configuration file::

    $ mapcrafter_markers -c render.conf

This program generates your defined marker groups and writes them to a
``markers-generated.js`` file in your output directory. You do not need
to worry about manually specified markers being overwritten.

If you have a very big world and want some progress information, use
the verbose flag::

    $ mapcrafter_markers -v -c render.conf

The same markers and marker groups are applied to all maps in your configuration;
you cannot apply markers to just one map when you have many.

Marker configuration sections will only match each sign in your world once.
You should order your ``[marker:..]`` sections to be from most specific to
most generic: 

.. code-block:: ini

    # Matches all signs starting with "["
    [marker:bracket]
    prefix = [

    # Will not match anything because all signs starting with "[" were matched above.
    [marker:homes]
    prefix = [home]

    # Will match all signs.
    # Always put this last or it will match everything!
    [marker:all]    
    prefix = 
    # If false, it will not match blank signs.
    match_empty = true      


Manually Specifying Markers
===========================

You can also add markers manually to your map. These markers are not tied to
signs (or any other entity) in your Minecraft world; you have total control
over them!

You define markers by editing the ``markers.js`` file in your output directory 
(it is not overwritten by  Mapcrafter if it already exists). The ``markers.js`` 
file is a Javascript  file which is included by the web interface and contains 
definitions for the map markers.

Here is an example ``markers.js`` file in (mostly) `JSON format <https://en.wikipedia.org/wiki/JSON>`_:

.. code-block:: javascript

    // Add your own markers to this file.
    
    var MAPCRAFTER_MARKERS = [
        // just one example marker group
        {
            // id of the marker group, without spaces/other special chars
            "id" : "signs",
            // name of the marker group, displayed in the web-interface
            "name" : "Signs",
            // icon of the markers belonging to that group (optional)
            "icon" : "sign.png",
            // size of that icon
            "iconSize" : [32, 32],
            // whether this marker group is shown by default (optional)
            "showDefault" : true,
            // markers of this marker group...
            "markers" : {
                // ...in the world "world"
                "world" : [
                    // example marker, pretty format:
                    {
                        // position ([x, z, y])
                        "pos" : [35, -21, 64],
                        // title when you hover over the marker
                        "title" : "Sign1",
                        // text in the marker popup window
                        "text" : "Hello."
                        // override the icon of a single marker (optional)
                        "icon" : "player.png",
                        // override the size of the marker icon (optional)
                        "iconSize" : [16, 32]
                    },
                    // more markers (compact format, without custom icon):
                    {"pos" : [100, 100, 64], "title" : "Test1"},
                    {"pos" : [100, 200, 64], "title" : "Test2"},
                    {"pos" : [500, 30, 64], "title" : "Test2"},
                ],
            },
        },
        
        // another marker group
        {
            "id" : "homes",
            "name" : "Homes",
            "icon" : "home.png",
            "iconSize" : [32, 32],
            "markers" : {
                "world" : [
                    {"pos" : [42, 73, 64], "title" : "Steve's home"},    
                ],
                "world2" : [
                    {"pos" : [73, 42, 64], "title" : "Steve's other home"},    
                ],
            },
        },
    ];

As you can see there is a bit `Javascript syntax <https://en.wikipedia.org/wiki/JSON>`_ 
involved here. Do not forget quotation marks around strings or the commas 
after array elements. The lines starting with a ``//`` are comments and 
ignored by Javascript.

The file has a Javascript-Array called ``MAPCRAFTER_MARKERS`` which
contains the different marker groups. The elements are associative 
Javascript-Arrays and contain the options of the different marker groups.

These options are similar to the :ref:`config_marker_options` configuration section.
Every marker group has an unique ID and a name displayed in the web interface.
You can also use an icon with a specific size (optional).

The actual markers are specified per world in an associative array with
the name ``markers``. You must the same world name your world section
name.

The definition of markers is also done with associative arrays::

	{"pos" : [42, 73, 64], "title" : "Steve's home"},

Here you can see a simple marker with the title ``Steve's home`` and the
position ``42, 73, 64``. The position is always specified as array in the
form of ``[x, z, y]`` (x, z and then y because x and z are the horizontal
axes and y is the vertical axis).

Here are the available options for the markers:

``pos``

	**Required**

	This is the position of the marker in the form of ``[x, z, y]``.
	Example: ``[12, 34, 64]``

``title``

	**Required**

	This is the title of the marker you can see when you hover over the
	marker.

``text``

	**Default:** *Title of the marker*

	This is the text of the marker popup window.
	If you do not specify anything, the title of the marker is used.

    TODO: work out if HTML is possible here.

``icon``

    **Default:** *Group icon*

    An override for the icon for this specific marker.
    If you do not specify an icon, the icon set at the group level is used. Or,
    if there is no group-level icon, the default icon is used.

    This option may be used independently of the marker icon size override.

``iconSize``

    **Default:** *Group icon size*

    An override for the size of the icon for this specific marker.
    If you do not specify a size, the icon size set at the group level is used.
    Or, if there is no group-level icon size, the default icon size is used.

    This option may be used independently of the marker icon override.


Custom Leaflet Marker Objects
=============================

Furthermore you can customize your markers by specifying a function which
creates the actual Leaflet marker objects with the marker data. This function
is called for every marker in the marker group and should return a marker-like
object displayable by Leaflet. Please have a look at the
`Leaflet API <http://leafletjs.com/reference.html>`_ for more information.

Here is a simple example which shows two areas on the map:

.. code-block:: javascript

    {
        "id" : "test",
        "name" : "Test",
        "createMarker" : function(ui, groupInfo, markerInfo) {
            var latlngs = [];
            // use the ui.mcToLatLng-function to convert Minecraft coords to LatLngs
            latlngs.push(ui.mcToLatLng(markerInfo.p1[0], markerInfo.p1[1], 64));
            latlngs.push(ui.mcToLatLng(markerInfo.p2[0], markerInfo.p2[1], 64));
            latlngs.push(ui.mcToLatLng(markerInfo.p3[0], markerInfo.p3[1], 64));
            latlngs.push(ui.mcToLatLng(markerInfo.p4[0], markerInfo.p4[1], 64));
            latlngs.push(ui.mcToLatLng(markerInfo.p1[0], markerInfo.p1[1], 64));
            
            return L.polyline(latlngs, {"color" : markerInfo.color});
        },
        "markers" : {
            "world" : [
                {
                    "p1" : [42, 0],
                    "p2" : [0, 0],
                    "p3" : [0, 42],
                    "p4" : [42, 42],
                    "color" : "red",
                },
                {
                    "p1" : [73, -42],
                    "p2" : [-42, -42],
                    "p3" : [-42, 73],
                    "p4" : [73, 73],
                    "color" : "yellow",
                },
            ],
        },
    },

You should use the ``ui.mcToLatLng`` method to convert Minecraft
coordinates (x, z and then y) to Leaflet latitude/longitute coordinates.
You can also use arbitrary data in the associative marker arrays and access
them with the ``markerInfo`` parameter of your function (same with ``groupInfo``
and the fields of the marker group).

Minecraft Server
================

If you want player markers from your Minecraft Server on your map, please 
have a look at the `mapcrafter-playermarkers <https://github.com/mapcrafter/mapcrafter-playermarkers>`_
project.

The plugin adds to your map animated markers of the players on your Minecraft
Server.
