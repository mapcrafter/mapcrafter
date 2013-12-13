=========================
Configuration File Format
=========================

To tell the mapcrafter which maps to render, simple INI-like configuration
files are used. With configuration files it is possible to render multiple
maps/rotations/rendermodes into one output file. 

A First Example
===============

Here is a simple example of a configuration file (let's call it
``render.conf``):

.. code-block:: ini

    output_dir = output

    [world:myworld]
    input_dir = worlds/myworld

    [map:map_myworld]
    name = My World world

As you can see the configuration files consist of different types of sections
(e.g. ``[section]``) and containing assignments of configuration options to
specific values (e.g. ``key = value``).  The sections have their names in
square brackets, where the prefix with the colon shows the type of the section.

There are two types (actually three, but more about that later) of sections:

* World sections (e.g. sections starting with ``world:``)
* Map sections (e.g. sections starting with ``map:``)

Every world section represents a Minecraft World you want to render and needs a
directory where it can find the Minecraft World (``input_dir`` of the world
section ``myworld`` in the example above).

Every map section represents a rendered Minecraft World. You can specifiy
things like rotation of the world, rendermode, texture pack and texture size
for each map.

In this example you can see that we have a world ``myworld`` in the directory
``worlds/myworld/`` which is rendered as the map ``map_myworld``.  The
directory ``output/`` is set as output directory. After rendering you can open
the ``index.html`` file in this directory and view your rendered map.

As you can see the configuration option ``output_dir`` is not contained in any
section - it's in the so called *root section*. That's because all maps are
rendered into this directory and viewable via one ``index.html`` file, so the
``output_dir`` option is the same for all maps in this configuration file.

Let's have a look at a more advanced configuration file.

A More Advanced Example
=======================

.. code-block:: ini

    output_dir = output
    
    [global:maps]
    rendermode = daylight
    rotations = top-left bottom-right
    
    [world:world]
    input_dir = worlds/world
    
    [world:creative]
    input_dir = worlds/creative
    
    [map:map_world_day]
    name = Normal World - Day
    world = world
    
    [map:map_world_night]
    name = Normal World - Night
    world = world
    rendermode = night
    
    [map:map_world_cave]
    name = Normal World - Cave
    world = world
    rendermode = cave
    
    [map:map_creative_day]
    name = Creative World - Day
    world = creative
    rotations = top-left top-right bottom-right bottom-left
    texture_dir = textures/special_textures
    texture_size = 16
    
    [map:map_creative_night]
    name = Creative World - Night
    world = creative
    rendermode = nightlight
    rotations = top-left top-right bottom-right bottom-left
    texture_dir = textures/special_textures
    texture_size = 16

Here we have some more worlds and maps defined. We have a "normal" world which
is rendered with the day, night and cave rendermode and we have a "creative"
world which is rendered super fancy with a special texture pack, higher texture
size and all available world rotations with the day and night rendermode.

As you can see there is a new section ``global:maps``. This section is used to
set default values for all map sections. Because of this in this example every
map has the daylight rendermode and the world rotations top-left and top-right
as default. Of course you can overwrite these settings in every map section.
There is also a global section ``global:worlds`` for worlds, but at the moment
there is only one configuration option for worlds (``input_dir``), so it
doesn't make much sense setting a default value here.

Furthermore every map has as option ``name`` a name which is used in the
webinterface of the output HTML-File. This can be anything suitable to identify
this map. In contrast to that the world and map names in the sections are used
for internal representation and therefore should be unique and contain only
alphanumeric chars and underscores.

When you have now your configuration file you can render your worlds with: (see
:ref:`command_line_options` for more options and usage)::

    mapcrafter -c render.conf

Available Options
=================

General Options
---------------

.. note::

    These options are relevant for all worlds and maps, so you have to put them
    in the header before the first section starts

``output_dir = <directory>``

    **Required**

    This is the directory where mapcrafter saves the rendered map. Every time
    you render your map the renderer copies the template files into this
    directory and overwrites them, if they already exist. The renderer creates
    an ``index.html`` file you can open with your webbrowser. If you want to
    customize this HTML-File, you should do this directly in the template (see
    ``template_dir``) because this file is overwritten every time you render
    the map.

``template_dir = <directory>``

    **Default:** default template directory (see :ref:`resources_textures`)

    This is the directory with the web template files. The renderer copies all
    files, which are in this directory, to the output directory and replaces
    the variables in the ``index.html`` file. The ``index.html`` file is also
    the file in the output directory you can open with your webbrowser after
    the rendering.

World Options
-------------

.. note::

    These options are for the worlds. You can specify them in the world
    sections (the ones starting with world:) or you can specify them in the
    global:worlds section.  If you specify them in the global section, these
    options are default values and inherited into the world sections if you do
    not overwrite them

``input_dir = <directory>``

    **Required**
	
    This is the directory of your Minecraft World. The directory should contain
    a directory ``region/`` with the .mca region files.


.. note::

    If you want to render the Nether or End of your world, just specify the path to the
    Nether or End of your world as world path. This should be my_world/DIM-1 or
    my_world_nether/DIM1 for the Nether and my_world/DIM1 or my_world_the_end/DIM1
    for the End.
    
    The only way to render the Nether is the cave rendermode at the moment.
    You will be able to skip the top bedrock layers with the next mapcrafter version.

Map Options
-----------

.. note::

    These options are for the maps. You can specify them in the map sections
    (the ones starting with map:) or you can specify them in the global:maps
    section.  If you specify them in the global section, these options are
    default values and inherited into the map sections if you do not overwrite
    them

``name = <name>``

    **Default:** ``<name of the section>``

    This is the name for the rendered map. You will see this name in the output
    file, so you should use here an human-readable name. The belonging
    configuration section to this map has also a name (in square brackets).
    Since the name of the section is used for internal representation, the name
    of the section should be unique and you should only use alphanumeric chars.

``rendermode = normal|cave|daylight|nightlight``
	
    **Default:** ``daylight``

    This is the rendermode to use when rendering the world. Possible
    rendermodes are:

    ``normal``
        The default rendermode.  
    ``daylight``
        Renders the world with lighting.
    ``nightlight``
        Like ``daylight``, but renders at night.
    ``cave``
        Renders only caves and colors blocks depending on their height 
        to make them easier to recognize.

``texture_dir = <directory>``

    **Default:** default texture directory (see :ref:`resources_textures`)

    This is the directory with the Minecraft Texture files.  The renderer works
    with the Minecraft 1.6 Resource Pack file format. You need here: 

    * directory ``chest/`` with normal.png, normal_double.png and ender.png 
    * directory ``colormap/`` with foliage.png and grass.png 
    * directory ``blocks/`` from your texture pack * endportal.png

    See also :ref:`resources_textures` to see how to get these files.

``texture_size = <number>``

    **Default:** ``12``

    This is the size (in pixels) of the block textures. The default texture
    size is 12px (16px is the size of the default Minecraft Textures).

    The size of a tile is ``32 * texture_size``, so the higher the texture
    size, the more image data the renderer has to process. If you want a high
    detail, use texture size 16, but texture size 12 looks still good and is
    faster to render.

``rotations = [top-left] [top-right] [bottom-right] [bottom-left]``

    **Default:** ``top-left``

    This is a list of directions to render the world from. You can rotate the
    world by n*90 degrees. Later in the output file you can interactively
    rotate your world. Possible values for this space-separated list are:
    ``top-left``, ``top-right``, ``bottom-right``, ``bottom-left``. 
	
    Top left means that north is on the top left side on the map (same thing
    for other directions).

``render_unknown_blocks = true|false``

    **Default:** ``false``

    With this option the renderer renders unknown blocks as red blocks (for
    debugging purposes).

``render_leaves_transparent = true|false``

    **Default:** ``true``

    You can specifiy this to use the transparent leaf textures instead of the
    opaque textures. Using transparent leaf textures can make the renderer a
    bit slower because the renderer also has to scan the blocks after the
    leaves to the ground.

``render_biomes = true|false``

    **Default:** ``true``

    This setting makes the renderer to use the original biome colors for blocks
    like grass and leaves. 

..
    At the moment the renderer does not use the biome colors for water because
    the renderer preblits the water blocks (which is a great performance
    improvement) and it is not very easy to preblit all biome color variants.
    And also, there is not a big difference with different water colors.

``use_image_mtimes = true|false``

    **Default:** ``true``

    This setting specifies the way the renderer should check if tiles 
    are required when rendering incremental. Different behaviors are:

    Use the tile image modification times (``true``):
        The renderer checks the modification times of the already rendered 
        tile images.  All tiles whoose chunk timestamps are newer than
        this modification time are required.
    Use the time of the last rendering (``false``):
        The renderer saves the time of the last rendering.  All tiles
        whoose chunk timestamps are newer than this last-render-time are
        required.
