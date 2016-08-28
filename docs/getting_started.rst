===============
Getting Started
===============

What does Mapcrafter do?
========================

Mapcrafter is a high performance Minecraft renderer running on the command line
which means that it takes Minecraft worlds and a configuration as input and
outputs a bunch of rendered images and some static HTML file which allows you
to look at your rendered worlds in a webbrowser.  You do not need a running
webserver to look at rendered worlds, you can also just open the created
``index.html`` file locally from the filesystem.

Mapcrafters core rendering engine allows it to render worlds from two different
perspectives: A 3D isometric render view and a 2D topdown render view are
available, also worlds can be rotated by 90, 180, or 270 degrees. On top of
those different views Mapcrafter can render overlays above maps like daylight,
nightlight, where mobs can spawn at day/night, or in which chunks slimes can
spawn for example. These overlays can be enabled or disabled in all different
combinations in the user interface.

Such a rendered configuration (render view, rotation, overlays, etc.) of a
*world* is called a *map*. Because Mapcrafter uses configuration files you can
render multiple worlds with different configurations (as different maps) to a
single output directory.

A simple example
================

As already mentioned Mapcrafter is using configuration files to make you able to tell
it which worlds should be rendered with which rendering configuration into a specific
output directory. Here you can see a simple example (``my_world.conf``):

.. code-block:: ini

	output_dir = my_world_mapcrafter
	
	[world:my_world]
	input_dir = worlds/my_world
	
	[map:my_world_isometric]
	name = My World - Isometric View
	world = my_world
	render_view = isometric
	rotations = top-left bottom-right

	[map:my_world_topdown]
	name = My World - Topdown View
	world = my_world
	render_view = topdown
	overlays = day night spawn-night

The config file format is a simple INI-like format with sections
(``[world:my_world]`` with type ``world`` and name ``my_world``) and
key/value-pairs (``output_dir = my_world_mapcrafter``). Sections are used to
create different objects for the renderer (Minecraft worlds to use, maps to
render or custom overlays for example) while the key/value-pairs are used to
configure them.

In this example there is a world ``my_world`` used with the path
``worlds/my_world``.  With this world there are two maps rendered: One with the
isometric view, the other one with the topdown view. The map with the isometric
view is also rendered twice as the world is rotated, while the topdown map is
rendered with different overlays.

To render those two maps you use Mapcrafter on the command line::

	mapcrafter -c my_world.conf

To improve the performance you can also render the maps with multiple threads::

	mapcrafter -c my_world.conf -j 2

``-j 2`` means that the renderer uses two threads. For the maximum CPU
utlization you should use the number of CPU cores as thread count. With
increasing thread count the input/output (reading the world, writing the
rendered tiles to disk) often becomes the bottleneck so using more threads than
CPU cores is not useful.

You can see your rendered maps by opening the ``index.html`` file in the output
directory with your favorite webbrowser.

For more information about configuring your maps to render see
:doc:`configuration_of_mapcrafter` and :doc:`usage` about the command line
usage of Mapcrafter.

Migrating from Mapcrafter 2.2 and older
=======================================

TODO

