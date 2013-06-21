====================
Command Line Options
====================

Here is a list of available command line options:

.. cmdoption:: -h, --help

	This shows a help about the command line options.

.. cmdoption:: -c <file>, --config <file>

	This is the path to the configuration file to use when rendering
	and is **required**.

.. cmdoption:: -s <maps>, --render-skip <maps>

	You can specify maps the renderer should skip when rendering. This is a
	comma-separated list of map names (the section names from the configuration
	file). You can also specify the rotation of the maps to skip by adding adding a
	':' and the short name of the rotation (``tl``, ``tr``, ``br``, ``bl``). 
	
	For example: ``world,world2`` or ``world:tl,world:bl,world2:bl,world3``.

.. cmdoption:: -r, --render-reset

	This option skips all maps and renders only the maps you explicitly specify
	with ``-a`` or ``-f``.

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
	The rendering performance also depends heavily on your disk. You can render the
	map to a solid state disk or a ramdisk to improve the performance.

	Every thread needs around 150MB ram.

.. cmdoption:: -b, --batch

	This option deactivates the animated progress bar. This is useful if you
	let the renderer run with a cronjob and pipe the output into a log file.
