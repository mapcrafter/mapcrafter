=========================
Configuration File Format
=========================

To tell the mapcrafter which maps to render, simple INI-like configuration
files are used. With configuration files it is possible to render multiple
maps/rotations/rendermodes into one output file. 

An Example
==========

Here is an example of a configuration file (render.conf):

.. code-block:: ini

	output_dir = output
	template_dir = mapcrafter/src/data/template
	textures_dir = mapcrafter/src/data/textures

	[world]
	name = Main World
	world = worlds/world
	rotations = top-left top-right bottom-left bottom-right

	[world2]
	name = World 2
	world = worlds/world2
	textures_dir = my/special/textures
	texture_size = 16
	rotations = top-left bottom-left
	rendermode = daylight

Here are two maps with different textures, texture sizes and rotations defined.
The first world is rendered with the default textures and from all available
directions. The second world is rendered with other textures, with texture
size 16 and only from two directions. Furthermore the lighting rendermode
is used.

You can render these worlds with (see :doc:`commandline` for more options)::

	./mapcrafter -c render.conf

The configuration files consist of ``[sections]`` and ``key = value`` option pairs .
The sections of the configuration files are the maps to render. The options in
the sections are settings for the rendered maps. You can also specify these
options in the head of the configuration file to set default option
values. In the head are also options, which are relevant to all worlds and
therefore must be defined in the head, for example ``output_dir`` and
``template_dir``.

The names of the sections (square brackets) are identifiers for the maps and
used for internal representations, so they should be unique and you should only
use alphanumeric chars. Additionally the sections have a normal name as an
option ``name``, which is also shown to the user in the output file.

All these maps are rendered into one output directory, so you can view them in
one HTML-File. For every map you can specify a list of rotations. You can
interactively rotate the world in the webbrowser, if you render different
rotations of a world. 

Relative paths in the configuration file are relative to the path of the
configuration file.

Available Options
=================

.. note::

	These options are relevant for all maps, so you have to put them in the
	header before the first section starts

``output_dir = <directory>``

	**Required**

	This is the directory, where mapcrafter saves the rendered map. Every time you
	render your map the renderer copies the template files into this directory and
	overwrites them, if they already exist. The renderer creates an ``index.html`` file
	you can open with your webbrowser. If you want to customize this HTML-File, you
	should do this directly in the template (see ``template_dir``) because this
	file is overwritten every time you render the map.

``template_dir = <directory>``

	**Default:** default template directory (see :ref:`resources_textures`)

	This is the directory with the web template files. The renderer copies all
	files, which are in this directory, to the output directory and replaces the
	variables in the ``index.html`` file. The ``index.html`` file is also the file in the
	output directory you can open with your webbrowser after the rendering.

.. note::

	These options are for the maps. You can specify them in the sections or you
	can specify them in the header. If you specify them in the header, these
	options are default values and inherited in the sections if you do not
	overwrite them

``name = <name>``

	**Default:** ``<name of the section>``

	This is the name for the rendered map. You will see this name in the output file,
	so you should use here an human-readable name. The belonging configuration
	section to this map has also a name (in square brackets). Since the name of the
	section is used for internal representation, the name of the section should be
	unique and you should only use alphanumeric chars.

``world = <directory>``

	**Required**

	This is the directory of the Minecraft World to render. The directory should
	contain a directory ``region`` with the .mca region files.

``rendermode = normal|cave|daylight|nightlight``
	
	**Default:** ``normal``

	This is the rendermode to use when rendering the world. Possible rendermodes are:

	``normal``
		The default rendermode.
	``cave``
		Renders only caves and colors blocks depending on their height to make 
		them better recognizable.
	``daylight``
		Renders the world with lighting. This lighting rendermode is still a bit 
		experimental, but already useable and looks good in most cases.
	``nightlight``
		Like ``daylight``, but renders at night.

``textures_dir = <directory>``

	**Default:** default texture directory (see :ref:`resources_textures`)

	This is the directory with the Minecraft Texture files.  The renderer works
	with the Minecraft 1.6 Resource Pack file format. You need here: 

	* directory ``chest/`` with normal.png, normal_double.png and ender.png 
	* directory ``colormap/`` with foliage.png and grass.png
	* directory ``blocks/`` from your texture pack
	* endportal.png

	See also :ref:`resources_textures` to see how to get these files.

``texture_size = <number>``

	**Default:** ``12``

	This is the size (in pixels) of the block textures. The default texture size is
	12px (16px is the size of the default Minecraft Textures).

	The size of a tile is ``32 * texture_size``, so the higher the texture size, the
	more image data the renderer has to process. If you want a high detail, use
	texture size 16, but texture size 12 looks still good and is faster to render.

``rotations = [top-left] [top-right] [bottom-right] [bottom-left]``

	**Default:** ``top-left``

	This is a list of directions to render the world from. You can rotate the world
	by n*90 degrees. Later in the output file you can interactively rotate your
	world. Possible values for this space-separated list are: 
	``top-left``, ``top-right``, ``bottom-right``, ``bottom-left``. 
	
	Top left means that north is on the top left side on the map (same thing for 
	other directions).

``render_unknown_blocks = 1|0``

	**Default:** ``0``

	With this option the renderer renders unknown blocks as red blocks (for
	debugging purposes).

``render_leaves_transparent = 1|0``

	**Default:** ``1``

	You can specifiy this to use the transparent leaf textures instead of the
	opaque textures. Using transparent leaf textures can make the renderer a bit
	slower, because the renderer also has to scan the blocks after the leaves to
	the ground.

``render_biomes = 1|0``

	**Default:** ``1``

	This setting makes the renderer to use the original biome colors for blocks
	like grass and leaves. At the moment the renderer does not use the biome
	colors for water because the renderer preblits the water blocks (which is a great
	performance improvement) and it is not very easy to preblit all biome color
	variants. And also, there is not a big difference with different water colors.

``incremental_detection = timestamp|filetimes``

	**Default:** ``timestamp``

	This setting specifies the way the renderer should check if tiles are required
	when rendering incremental.  Possible options are:

	``timestamp``
		The renderer saves the time of the last rendering.  All tiles whoose
		chunk timestamps are newer than this last-render-time are required.
	``filetimes``
		The renderer checks the modification times of the already rendered tile
		images.  All tiles whoose chunk timestamps are newer than this
		modification time are required.

