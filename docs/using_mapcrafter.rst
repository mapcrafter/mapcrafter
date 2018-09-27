================
Using Mapcrafter
================

First Rendered World
====================

At first you have to create a configuration file like this:

.. code-block:: ini

	output_dir = output
	
	[world:myworld]
	input_dir = worlds/myworld
	
	[map:map_myworld]
	name = My World
	world = myworld

In the configuration file you define which worlds the renderer should render.
In this example is defined that the renderer should render the world in the
directory ``worlds/myworld/`` as the map ``map_myworld`` into the output
directory ``output/``. All relative paths in configuration files are relative
to the path of the configuration file.

Now it's time to render your first world::

	mapcrafter-legacy -c render.conf

To improve the performance you can also render the map with multiple threads::

	mapcrafter-legacy -c render.conf -j 2

2 is here the number of threads the renderer uses. You should use the count of
your CPU cores.  With increasing thread count I/O (reading the world, writing
the rendered tiles to disk) mostly becomes the bottleneck so using more threads
than CPU cores is not useful.

You can see your rendered map by opening the ``index.html`` file in the output
directory with your webbrowser.

For more information about rendering maps see :doc:`configuration` and the next
section about command line options.

.. _command_line_options:

Command Line Options
====================

Here is a list of available command line options:

General options
---------------

.. cmdoption:: -h, --help

    Shows a help about the command line options.

.. cmdoption:: -v, --version

    Shows the version of Mapcrafter.

Logging/output options
----------------------

.. cmdoption:: --logging-config <file>

    This option sets the global logging configuration file Mapcrafter's logging facility
    is using. You do not necessarily need to specify a logging configuration file,
    Mapcrafter is trying to determine it automatically. 

.. cmdoption:: --color <colored>

    This option specifies whether Mapcrafter's logging facility should use a colored
    terminal output for special messages (warnings/errors). Possible options are
    ``true``, ``false`` or ``auto`` (default). ``auto`` means that terminal colors are
    enabled if the output is connected to a tty (and not piped to a file for example).

.. cmdoption:: -b, --batch

    This option deactivates the animated progress bar and enables the progress logger
    instead as terminal output. It is also automatically enabled if the output is not
    connected to a tty (piped to a file for example).

Renderer options
----------------

.. cmdoption:: --find-resources

    Shows the resource directories of Mapcrafter. See also
    :ref:`resources_textures`.

.. cmdoption:: -c <file>, --config <file>

    This is the path to the configuration file to use when rendering and is
    **required**.

.. cmdoption:: -s <maps>, --render-skip <maps>

    You can specify maps the renderer should skip when rendering. This is a
    space-separated list of map names (the map section names from the
    configuration file). You can also specify the rotation of the maps to skip
    by adding a ``:`` and the short name of the rotation (``tl``, ``tr``,
    ``br``, ``bl``). 

    For example: ``-s world world2`` or ``-s world:tl world:bl world2:bl world3``.

.. cmdoption:: -r, --render-reset

    This option skips all maps and renders only the maps you explicitly specify
    with ``-a`` or ``-f``.

    .. note::

        This option is useful if you want to update only the template of your
        rendered map::

            mapcrafter-legacy -c render.conf -r

.. cmdoption:: -a <maps>, --render-auto <maps>

    You can specify maps the renderer should render automatically. This means
    that the renderer renders the map incrementally, if something was already
    rendered, or renders the map completely, if this is the first rendering.
    Per default the renderer renders all maps automatically. See
    ``--render-skip`` for the format to specify maps.

.. cmdoption:: -f <maps>, --render-force <maps>

    You can specify maps the renderer should render completely. This means that
    the renderer renders all tiles, not just the tiles, which might have
    changed. See ``--render-skip`` for the format to specify maps.

.. cmdoption:: -F, --render-force-all

    This option is similar to the ``-f`` option, but it makes Mapcrafter force-render
    all maps.

.. cmdoption:: -j <number>, --jobs <number>

    This is the count of threads to use (defaults to one), when rendering the
    map.  Using as much threads as CPU cores you have is good, but the
    rendering performance also depends heavily on your disk. You can render the
    map to a solid state disk or a ramdisk to improve the performance.

    Every thread needs around 150MB ram.
