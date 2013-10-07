================
Using mapcrafter
================

First Render
===========

At first you have to create a configuration file like this::

	output_dir = output
	
	[world:myworld]
	input_dir = world
	
	[map:mymap]
	name = My Map
	world = myworld

In the configuration file you define which worlds and maps the renderer should
render.  Worlds are the different Minecraft Worlds, maps are the rendered
Minecraft Worlds.  They can be rendered with different rendermodes and
different rotations.

You have to declare every world you want to use in a section which name is
prefixed with ``world:``.  Then you can use these worlds for your different
maps. Every map section name is also prefixed with ``map:``.  In this example
is the world ``myworld`` with the input directory ``world/`` declared.  The
world ``myworld`` is rendered as map ``mymap`` into the output directory
``output/``.

All relative paths in configuration files are relative to the path of the
configuration file.

Now it's time to render your first world::

    mapcrafter -c render.conf

To improve the performance you can also render the map with multiple threads::

    mapcrafter -c render.conf -j 2

2 is the number of threads the renderer should use. You should use the count of
your CPU cores, but often the input/output is the bottleneck so using more
threads is not useful.

You can see your map by opening the ``index.html`` file in the output directory
with your webbrowser.

For more information about rendering maps, see :doc:`configuration` and the
next section about command line options.

.. _command_line_options:

Command Line Options
====================

Here is a list of available command line options:

.. cmdoption:: -h, --help

	This shows a help about the command line options.

.. cmdoption:: -v, --version

	Shows the version of mapcrafter.

.. cmdoption:: --find-resources

	Shows the resource directories of mapcrafter. See also
	:ref:`resources_textures`.

.. cmdoption:: -c <file>, --config <file>

	This is the path to the configuration file to use when rendering
	and is **required**.

.. cmdoption:: -s <maps>, --render-skip <maps>

	You can specify maps the renderer should skip when rendering. This is a
	comma-separated list of map names (the section names from the configuration
	file). You can also specify the rotation of the maps to skip by adding a
	``:`` and the short name of the rotation (``tl``, ``tr``, ``br``, ``bl``). 
	
	For example: ``world,world2`` or ``world:tl,world:bl,world2:bl,world3``.

.. cmdoption:: -r, --render-reset

	This option skips all maps and renders only the maps you explicitly specify
	with ``-a`` or ``-f``.

	.. note::

		This option is useful if you want to update only the template
		of your rendered map::
		
			mapcrafter -c render.conf -r

.. cmdoption:: -a <maps>, --render-auto <maps>

	You can specify maps the renderer should render automatically. This means that
	the renderer renders the map incrementally, if something was already rendered,
	or renders the map completely, if this is the first rendering. Per default the
	renderer renders all maps automatically. See ``--render-skip`` for the format to
	specify maps.

.. cmdoption:: -f <maps>, --render-force <maps>

	You can specify maps the renderer should render completely. This means that the
	renderer renders all tiles, not just the tiles, which might have changed. See
	``--render-skip`` for the format to specify maps.

.. cmdoption:: -j <number>, --jobs <number>

	This is the count of threads to use (defaults to one), when rendering the map.
	Using as much threads as CPU cores you have is good, but the rendering 
	performance also depends heavily on your disk. You can render the map 
	to a solid state disk or a ramdisk to improve the performance.

	Every thread needs around 150MB ram.

.. cmdoption:: -b, --batch

	This option deactivates the animated progress bar. This is useful if you
	let the renderer run with a cronjob and pipe the output into a log file.
