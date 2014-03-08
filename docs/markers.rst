.. _markers:

=======
Markers
=======

Mapcrafter allows you to add different markers easily to your rendered
maps. Markers are organized in marker groups, this allows you show
and hide different marker groups on the rendered map.

Automatically Generated Markers
===============================

Mapcrafter is able to automatically generate markers from specific signs
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
shown on the map as marker of this marker group.

See :ref:`config_marker_options` for a reference of marker section options.

To automatically generate these markers, use the ``mapcrafter_markers``
program with your configuration file::

    mapcrafter_markers -c render.conf

This program generates your defined marker groups and writes them to a
``markers-generated.js`` file in your output directory. You do not need
to worry about manually specified markers being overwritten.

Manually Specifying Markers
===========================

Of course it is still possible to add markers manually to your map.
You can do this by editing the ``markers.js`` file in your output directory
(it is not overwritten by Mapcrafter if it already exists).
The ``markers.js`` file is a Javascript file which is included
by the web interface and contains definitions for the map markers.

Here is an example ``markers.js`` file:

.. code-block:: javascript

    // Add your own markers to this file.
    
    var MAPCRAFTER_MARKERS = [
        // just one example marker group
        {
            // id of the marker group, without spaces/other special chars
            "id" : "signs",
            // name of the marker group, displayed in the webinterface
            "name" : "Signs",
            // icon of the markers belonging to that group
            "icon" : "sign.png",
            // size of that icon
            "icon_size" : [32, 32],
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
                    },
                    // more markers:
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
            "icon_size" : [32, 32],
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

As you can see there is a bit Javascript syntax involved here. Do not forget
quotation marks around strings or the commas after array elements. The
lines starting with a ``//`` are comments and ignored by Javascript.

The file has a Javascript-Array called ``MAPCRAFTER_MARKERS`` which
contains the different marker groups. The elements are associative 
Javascript-Arrays and contain the options of the different marker groups.

These options are similar to the marker section configuration options.
Every marker group has an unique ID and a name displayed in the web interface.
You can also use an icon with a specific size (optional).

The actual markers are specified per world in an associative array with
the name ``markers``. You have to use as world name your world section
name or the value of the ``world_name`` attribute if you specified another
world name.

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
	If you do not specifiy a text, the title of the marker is used as text.

Minecraft Server
================

If you want player markers from your Minecraft Server on your map, please 
have a look at the `mapcrafter-playermarkers <https://github.com/m0r13/mapcrafter-playermarkers>`_
project.

The plugin adds to your map animated markers of the players on your Minecraft
Server.
