=========================
Configuration File Format
=========================

To tell the Mapcrafter which maps to render, simple INI-like configuration
files are used. With configuration files it is possible to render maps with
multiple rotations and render modes into one output file. 

Basic Example
=============

Here is the most basic example of a configuration file which will make 
Mapcrafter render something (let's call it ``render.conf``):

.. code-block:: ini

    # Directory where Mapcrafter will save rendered maps
    output_dir = /home/user/myworld_mapcrafter

    # Directory of your Minecraft world
    [world:myworld]
    input_dir = /home/user/.minecraft/worlds/myworld

    # One map to render, called "My World"
    [map:myworld_isometric_day]
    world = My World

Once you replace the sample `output_dir` and `input_dir`, you can run Mapcrafter
like so (see :ref:`command_line_options` for more details): ::

    $ mapcrafter -c render.conf

If all went well, you should be able to open `/home/user/.minecraft/worlds/myworld/index.html`
in your web-browser and see your map!


A More Advanced Example
=======================

Configuration files are very versatile. Each configuration file lets you render 
to exactly one output directory, but can read from multiple Minecraft worlds 
(either totally different worlds, or different dimensions of the same world), 
and have many rendered maps (eg: one isometric, another top-town, and yet another 
with night-lighting).

A more complex example, with comments to summarise each section, is given below. 
Scroll down for a reference of all possible options.

.. code-block:: ini

    # Directory where Mapcrafter will save rendered maps
    output_dir = /home/user/myworld_mapcrafter


    # Directory of your Minecraft worlds.
    [world:world]
    input_dir = /home/user/.minecraft/worlds/world

    # This is the same world as above, but the end dimension.
    [world:world_end]
    input_dir = /home/user/.minecraft/worlds/world
    dimension = end
    
    # This is a completely different world.
    [world:creative]
    input_dir = /home/user/.minecraft/worlds/creative


    # Global options set defaults for all below map sections.    
    [global:map]
    world = world
    render_view = isometric
    render_mode = daylight
    rotations = top-left bottom-right
    texture_size = 12
    image_format = png

    
    # A map which picks up all defaults, with a nice name.
    [map:world_isometric_day]
    name = Normal World - Day
    
    # This map renders at night time.
    [map:world_isometric_night]
    name = Normal World - Night
    render_mode = nightlight
    
    # Cave render mode shows underground caves and structures.
    [map:world_isometric_cave]
    name = Normal World - Cave
    render_mode = cave

    # A shrunken top-down map.
    # NOTE: texture_size = 6 requires blockcrafter; scroll down more more details.
    [map:world_topdown_day]
    name = Normal World - Topdown overview
    render_view = topdown
    texture_size = 6
    texture_blur = 2
    tile_width = 3

    # A world of The End dimension.
    [map:world_isometric_end]
    name = Normal World - The End
    world = world_end
    

    # The creative world, with custom block sprites and all 4 rotations.
    # NOTE: Use Blockcrafter to generate custom sprites.
    [map:creative_isometric_day]
    name = Creative World - Day
    world = creative
    render_mode = daylight
    rotations = top-left top-right bottom-right bottom-left
    block_dir = data/special_blocks
    texture_size = 16
    
    # The creative world, rendered at night.
    [map:creative_isometric_night]
    name = Creative World - Night
    world = creative
    render_mode = nightlight
    rotations = top-left top-right bottom-right bottom-left
    block_dir = data/special_blocks
    texture_size = 16

You can find other `example maps <https://github.com/mapcrafter/mapcrafter/wiki/Example-maps>`_ 
on the Github Wiki. Other configuration examples may be available there.

There are tons of other options to customize your rendered maps. Before a
reference of fundamentals and all available options, here is a quick overview 
of interesting things you can do:

* Default view / zoom level / rotation in web interface
* World cropping (only render specific parts of your world)
* Block mask (skip rendering / render only specific types blocks)
* Different render views, render modes, overlays
* Use custom texture packs, or texture sizes
* Custom tile widths
* Different image formats
* Custom lighting intensity


Configuration File Fundamentals
===============================

Configuration files consist of several sections (e.g. ``[section]``) and 
*key+value* pairs (e.g. ``key = value``). Sections have two parts to them:
their type and their name, separated by a colon (eg: ``[type:name]``). 
Lines starting with a hash (``#``) are ignored and can be used as comments 
or notes.

There are four section types which can configure Mapcrafter output:

* The General section (values entered before any section),
* World sections (e.g. sections starting with ``world:``),
* Map sections (e.g. sections starting with ``map:``),
* and Marker sections (e.g. sections starting with ``marker:``, also see :ref:`markers`).

The *General* section specifies the ``output_dir`` where your map(s) will be
rendered. A single ``index.html`` will created in this folder, which you can
open in your web-browser and view all maps in your config. See `General Options`_ 
for more details.

Every *World* section represents a Minecraft world you want to render and needs 
an ``input_dir``. You can also choose the dimension to render and a variety of 
cropping options. See `World Options`_ for more details.

Every *Map* section represents an actual rendered map of a Minecraft world. You
can specify things like rotation of the world, render view, render mode, block sprites, 
texture size, etc. Each ``[map:...]`` will render a unique map you can
view in your web-browser. Oh, and you need at least one map, or Mapcrafter
can't render anything! See `Map Options`_ for more details.

Marker sections allow you to add map markers based on Minecraft signs. It is
outlined in more detail in :ref:`markers`.

Section names (eg: ``[map:a_section_name]``) are up to you. They are used to
refer back to other section (eg: in a ``map`` section, to specify which ``world``
to render). So it is recommended to use a fixed and consistent format (for example 
``<world name>_<render view>_<render mode>`` for maps). Section names are also 
used as internal identifiers in Mapcrafter and HTML files, and should only 
contain letters, numbers and underscores (definitely no quotes or spaces).

You can define defaults for each section using the special ``[global:<section type>]``
section, eg: ``[global:map]``. So, if you prefer to use PNGs rather than JPEGs, 
you can set ``image_format = png`` once in ``[global:map]`` rather than in every 
``[map:...]`` you configure.

In addition to the section name ``[section:name]``, you can set a "pretty" name
for each map using the ``name`` option. This is used in the web interface 
to identify your map. It can be anything including spaces, symbols and even Unicode. 


.. _Mapcrafter URL:

Mapcrafter URLs
=================

Mapcrafter maps are displayed in your web-browser. The URL in your browser's
address bar can be shared with others or linked to from other websites (if
your maps are hosted on a public web server). The URL includes details of which
map is selected, and the position and zoom of your current display. You can
read the zoom level or map position from the URL (see `Cropping Your World`_).

`Here is an example URL <https://minecraft.ligos.net/worlds/Mapcrafter_Test/index.html#MapcrafterTest_Overworld_isometric/0/5/369/1059/64>`_, 
which shows the village used throughout this page:

``https://../index.html#MapcrafterTest_Overworld_isometric/0/5/369/1059/64``

Here are the parts of the URL:

``{index.html}#{Map}/{Orientation}/{Zoom}/{X}/{Z}/{Y}``

=========================  ======================================================================
Part                       Description                                       
=========================  ======================================================================
``{index.html}``           Path to Mapcrafter index.html file, which includes the domain name.
``{Map}``                  Map being displayed, see ``map:<name>``.
``{Orientation}``          The displayed orientation. see ``rotations``.
``{Zoom}``                 Current zoom level, see ``default_zoom``.
``{X}``, ``{Y}``, ``{Z}``  Current co-ordinates of the center of your screen, see `Cropping Your World`_.
=========================  ======================================================================

----

Available Options
=================

This is a reference section for all available options for Mapcrafter.


General Options
---------------

.. note::

    These options are relevant for all worlds and maps, so you must put them
    in the header before the first section starts

**Output Directory:** ``output_dir = <directory>``

    **Required**

    This is the directory where Mapcrafter saves your rendered map(s). Every time
    you run Mapcrafter, the renderer copies the template files from ``template_dir``
    into this directory and overwrites them, if they already exist. The renderer 
    creates an ``index.html`` file you can open with your web-browser. If you want
    to customize this HTML file, you should do this directly in the ``template_dir``
    because this file is overwritten every time you render the map (see :doc:`hacking`).

**Template Directory:** ``template_dir = <directory>``

    **Default:** default template directory (see :ref:`resources_textures`)

    This is the directory with the web template files. The renderer copies all
    files from this directory to the output directory. You should open the
    ``index.html`` file the output directory with your web-browser to see your
    rendered map!


**Background Color:** ``background_color = <hex color>``

    **Default:** ``#DDDDDD``

    This is the background color of your rendered map. You have to specify
    it like an HTML hex color (``#rrggbb``).
    
    The background color of the map is set with a CSS option in the template.
    Because the JPEG image format does not support transparency and some tiles
    are not completely used, you have to re-render your maps which use JPEGs
    if you change the background color.

-----


World Options
-------------

.. note::

    These options are for worlds. You can specify them in the world
    sections (starting with ``world:``) or you can specify them in the
    ``global:world`` section.  If you specify them in the global section, these
    options are default values and inherited into the world sections if you do
    not overwrite them.

**Input Directory:** ``input_dir = <directory>``

    **Required**

    This is the directory of your Minecraft world. The directory should contain
    a directory ``region/`` with the .mca region files.

**Dimension:** ``dimension = nether|overworld|end``

    **Default**: ``overworld``
    
    You can specify with this option the dimension of the world Mapcrafter should render.
    If you choose The Nether or The End, Mapcrafter will automagically detect the
    corresponding region directory. It will try the Bukkit region directory
    (for example ``myworld_nether/DIM-1/region``) first and then the directory of a normal
    vanilla server/client (for example ``myworld/DIM-1/region``).

.. note::

    If you want to render The Nether and want to see something, you should use 
    ``render_mode = cave`` or the ``crop_max_y`` option to remove the top bedrock layers.

**World Name:** ``world_name = <name>``

    **Default**: ``<name of the world section>``
    
    This is another name of the world, the name of the world the server uses.
    You don't usually need to specify this manually unless your server uses different
    world names and you want to use the `mapcrafter-playermarkers 
    <https://github.com/mapcrafter/mapcrafter-playermarkers>`_ script.

**Default View:** ``default_view = <x>,<z>,<y>``

    **Default**: Center of the map (0,0,64)
    
    You can specify the default center of the map with this option. Just specify a
    position in your Minecraft world you want as center when you open the map.

    This is useful if you want to crop your map and focus on the cropped part (see below).

**Default Zoom:** ``default_zoom = <zoomlevel>``

    **Default**: ``0``
    
    This is the default zoom level shown when you open the map. The default zoom level
    is 0 (completely zoomed out) and the maximum zoom level (completely zoomed in) can
    be read from the `Mapcrafter URL`_ in your web-browser.

**Default Rotation:** ``default_rotation = top-left|top-right|bottom-right|bottom-left``

    **Default**: First available rotation of the map
    
    This is the default rotation shown when you open the map. You can specify one of the 
    four available rotations. If a map doesn't have this rotation, the first available
    rotation will be shown. 

Cropping Your World
~~~~~~~~~~~~~~~~~~~

By using the following options you can crop your world and render only 
a specific part of it. You can combine vertical, horizontal and block mask
options in the same map.

**Vertical Cropping**

With these two options you can skip blocks above or below a specific level:

``crop_min_y = <number>``

    **Default:** -infinity

    This is the minimum y-coordinate of blocks Mapcrafter will render.
    0 is the lowest y-coordinate. In the overworld, bedrock kicks in at 4-8
    and sealevel is 64.

``crop_max_y = <number>``

    **Default:** infinity

    This is the maximum y-coordinate of blocks Mapcrafter will render.
    256 is the highest y-coordinate. In the overworld, most interesting things 
    happen below 128. For example, if you specify 32, you will "cut open" your world
    to see underground structures (see also ``render_mode = cave``).

    .. image:: img/world_crop_level32.png
       :align: center
       :alt: A world cropped at level 32 to show underground structures (crop_max_y = 32).


**Horizontal Cropping**

Furthermore there are two different types of world cropping:

1. Rectangular cropping:

  * You can specify limits for the x- and z-coordinates.
    The renderer will render only blocks contained in these boundaries.
    All are optional and default to infinite (or -infinite for minimum limits):
    
    * ``crop_min_x`` (minimum limit of x-coordinate)
    * ``crop_max_x`` (maximum limit of x-coordinate)
    * ``crop_min_z`` (minimum limit of z-coordinate)
    * ``crop_max_z`` (maximum limit of z-coordinate)

    .. image:: img/world_crop_rectangular.png
       :align: center
       :alt: A rectangular cropped world.


2. Circular cropping:

  * You can specify a block position as center and a radius.
    The renderer will render only blocks contained in this circle:
    
    * ``crop_center_x`` (**required**, x-coordinate of the center)
    * ``crop_center_z`` (**required**, z-coordinate of the center)
    * ``crop_radius`` (**required**, radius of the circle)

    .. image:: img/world_crop_circular.png
       :align: center
       :alt: A circular cropped world.

.. note::

    It's best to read positions from a ``render_view = topdown``, from 
    `Mapcrafter URLs`_, or using the ``F3`` debug screen within Minecraft, 
    as the x and z co-ordinates in bottom left of isometric maps might 
    not be what you expect (they assume a specific `y` coordinate which 
    usually isn't the block / hight you are pointing at). 

.. note::

    The renderer automatically centers circular cropped worlds and rectangular
    cropped worlds which have all four limits specified so the maximum
    zoom level of the rendered map does not unnecessarily become as high as 
    the original map. 

.. note::

    Changing the center of an already rendered map is complicated and 
    therefore not supported by the renderer. Due to that you should 
    completely re-render the map when you want to change the boundaries of 
    a cropped world. This also means that you should delete the already 
    rendered map (delete ``<output_dir>/<map_name>``).


**Block Mask Cropping**

Block mask is an extremely powerful cropping tool to hide or show specific block
types. It's use requires a little knowledge about how Minecraft stores block information,
and how Mapcrafter works with blocks.

.. note::

    Using numeric *block ids*, as described below, is a temporary thing. It works today,
    but at some point, it will no longer be supported. Minecraft textual block ids 
    (eg: ``minecraft:oak_wood``) will be implemented in the future. Beware!


Minecraft stores two pieces of information about each block: a *block id* and optional
*block data*. You can find details of *block ids* in the of `data values <https://minecraft.gamepedia.com/Java_Edition_data_values>`_
on the Minecraft wiki. *Block data* is different for each *block id* and can be
found on each block page on the Minecraft wiki. Since the *"flattening"* in Minecraft 
1.13, there are now many more *block ids* and less usage of *block data*.

Eg: `Wood <https://minecraft.gamepedia.com/Wood>`_ (which make up tree trunks) has
*block id* of ``minecraft:oak_wood`` (plus 11 other variations), and *block data* 
``axis`` of either ``x``, ``y`` or ``z``, which is the direction of the wood.

Mapcrafter doesn't work with ``minecraft:oak_wood axis=x``, it translates those into
simple numbers to render each block. You can find Mapcrafter ``blockid``'s by locating 
the *block id* + *block data* in one of the `texture block files <https://github.com/mapcrafter/mapcrafter/blob/world113/src/data/blocks/isometric_0_16.txt>`_,
and subtracting 2 from the line number. So the three axis of ``minecraft:oak_wood`` 
= ``4137-4139``. (That magic number *2* comes from a zero indexed array with one 
extra header line).

.. code-block:: text

    ....
    line 4138 - minecraft:oak_trapdoor facing=west,....
    line 4139 - minecraft:oak_wood axis=x color=4532,uv=4533
    line 4140 - minecraft:oak_wood axis=y color=4534,uv=4535
    line 4141 - minecraft:oak_wood axis=z color=4536,uv=4537
    line 4142 - minecraft:observer facing=down,powered=false color=4538,uv=4539
    ....

.. note::

    If you use :doc:`blockcrafter` to create your own block data files and block sprites, 
    the example ``blockid`` numbers given here will not work! Nor will the linked texture 
    block data file above. These examples only apply for vanilla Minecraft 1.13 - different 
    Minecraft versions, the presence of mods or custom resource packs will change these.



``block_mask = <block mask>``

    **Default:** *show all blocks*
    
    The block mask is a space separated list of block groups you want to 
    hide/show. If a ``!`` precedes a block group, all blocks of this block group are
    hidden, otherwise they are shown. Per default, all blocks are shown.
    All block ids should be entered as decimal numbers, based on their
    location in block data files. Possible block groups are:
    
    * All blocks:
      
      * ``*``
    
    * A single block (independent of block data):
      
      * ``[blockid]``
    
    * A single block with specific block data:
      
      * ``[blockid]:[blockdata]``
    
    * A range of blocks:
      
      * ``[blockid1]-[blockid2]``
    
    * All blocks with a specific id and ``(block data & bitmask) == specified data``:
      
      * ``[blockid]:[blockdata]b[bitmask]``
    
    For example:
    
    * Hide all blocks except blocks with id 1,7,8,9 or id 3 / data 2:
    
      * ``!* 1 3:2 7-9``
    
    * Show all blocks except grass (in various forms), dirt and coarse dirt:
    
      * ``!2376-2381 !1296 !2175``

    .. image:: img/world_crop_blockmask.png
       :align: center
       :alt: A world with grass and dirt removed.

-----


Map Options
-----------

.. note::

    These options are for maps. You can specify them in the map sections
    (the ones starting with ``map:``) or you can specify them in the ``global:map``
    section.  If you specify them in the global section, these options become
    default values and are inherited into the map sections if you do not overwrite
    them.

**Name:** ``name = <name>``

    **Default:** ``<name of the section>``

    .. image:: img/map_name.png
       :align: center
       :alt: Your map name appears in the drop down list of maps.

    This is the name for the rendered map. You will see this name in the dropdown 
    list of maps, so you should use a human-readable name (spaces, numbers, symbols, 
    even unicode are all OK). 
    
    The configuration section for this map has also a name (in square brackets).
    This section name is used for internal representation, command line arguments, 
    and on-disk directory names, so the section name should be unique and only use
    alphanumeric chars and underscore (definitely no spaces).


**Render View:** ``render_view = isometric|topdown|side``

    **Default:** ``isometric``

    This is the view that your world is rendered from. You can choose from
    different render views:

    :Isometric:
        .. image:: img/map_render_view_isometric.png

        A 3D isometric view with up to 4 rotations.
    
    :Topdown:
        .. image:: img/map_render_view_topdown.png

        A simple 2D top view.

    :Side:
        .. image:: img/map_render_view_side.png

        A 2.5D view similar to ``topdown``, but tilted.


**Render Mode:** ``render_mode = daylight|nightlight|plain|cave|cavelight``
	
    **Default:** ``daylight``

    This is the render mode to use when rendering the world. Possible
    render modes are:

    :Daylight:      
        .. image:: img/map_render_mode_daylight.png

        High quality render with daylight lighting.
    
    :Plain:
        .. image:: img/map_render_mode_plain.png

        Renders the world without any special lighting.                     
        Slightly faster than ``daylight`` but not as pretty.

    :Nightlight:    
        .. image:: img/map_render_mode_nightlight.png
                      
        Like ``daylight``, but renders at night.
        Hope your world has lots of torches!
    
    :Cave Light:    
        .. image:: img/map_render_mode_cavelight.png
                      
        Renders only underground caves so you can see underground.
        Blocks are colored based on their height to make them easier
        to recognize.

    :Cave:          
        .. image:: img/map_render_mode_cave.png

        Same as ``cavelight`` but with ``plain`` lighting.

.. note::

    The old option name ``rendermode`` is still available, but deprecated.
    Therefore you can still use it in old configuration files, but Mapcrafter
    will show a warning.

**Overlay:** ``overlay = slime|spawnday|spawnnight``

    **Default:** ``none``

    Additionally to a render mode, you can specify an overlay. An overlay is a
    special render mode that is rendered on top of your map and the selected
    render mode. The following overlays are used to show some interesting
    additional data extracted from the Minecraft world data:

    :Slime:
        .. image:: img/map_overlay_spawnslime.png
                    
        Highlights the chunks where slimes can spawn. 
        Note that other conditions need to be met for slimes to spawn 
        in the overlay areas (eg: swamps or flat worlds).

    :Spawnday:
        .. image:: img/map_overlay_spawnday.png
                    
        Shows where monsters can spawn at day. 
        You'll need to find dark caves to see this overlay (or use 
        ``render_mode = cave``).

    :Spawnnight:    
        .. image:: img/map_overlay_spawnnight.png
                    
        Shows where monsters can spawn at night. 
        This covers most of the map, except for areas with light sources.
    

    At the moment there is only one overlay per map section allowed because the overlay
    is rendered just like a render mode on top of the world. If you want to render
    multiple overlays, you need multiple map sections. This behavior might change in
    future Mapcrafter versions so you will be able to dynamically switch multiple
    overlays on and off in the web interface.

**Rotations** ``rotations = [top-left] [top-right] [bottom-right] [bottom-left]``

    **Default:** ``top-left``

    +----------------------------------------------+----------------------------------------------+
    | .. image:: img/map_rotations_topleft.png     | .. image:: img/map_rotations_topright.png    |
    |                                              |                                              |
    | **top-left**                                 | **top-right**                                |
    +----------------------------------------------+----------------------------------------------+
    | .. image:: img/map_rotations_bottomright.png | .. image:: img/map_rotations_bottomleft.png  |  
    |                                              |                                              |
    | **bottom-right**                             | **bottom-left**                              |
    +----------------------------------------------+----------------------------------------------+

    This is a list of directions to render the world from. You can rotate the
    world by n*90 degrees. Later in the output file you can interactively
    rotate your world. Possible values for this space-separated list are:
    ``top-left``, ``top-right``, ``bottom-right``, ``bottom-left``. 

    Top left means that north is on the top left side on the map (same thing
    for other directions).

**Texture Size** ``texture_size = 16|12|blockcrafter``

    **Default:** ``12``

    This is the size (in pixels) of the block textures. The default Minecraft
    textures are 16px, which gives this highest level of detail.

    The size of a tile is ``32 * texture_size``, so the higher the texture
    size, the more image data the renderer has to process and the more disk-space 
    is required. If you want a high detail, use texture size 16, but texture 
    size 12 looks still good and is faster to render.

    Mapcrafter's pre-rendered block sprites include sizes 16 and 12. If you want to 
    use other sizes, or custom resource packs you will need to generate them using 
    :doc:`blockcrafter`.


**Block Directory** ``block_dir = <directory>``

    **Default:** ``data/blocks``

    This is the folder where block sprite files are located. You can set this
    to use textures from a custom resource pack. 

    The standard Minecraft block sprites are included with Mapcrafter, if you 
    want to use sprites from a custom resource pack, you should use :doc:`blockcrafter`
    to render new images.
    
    You can use the following command to find the location of the ``block_dir``
    Mapcrafter is using::

        $ mapcrafter --find-resources


**Tile Width** ``tile_width = <number>``

    **Default:** ``1``

    This lets you reduce the number of tiles / files Mapcrafter renders by merging 
    them together. Individual tiles will take longer to render, but fewer files 
    will be written to disk.

    This is a factor that is applied to the tile size. Every (square) tile is 
    usually one chunk wide (1:1). That is, one image at highest zoom generated 
    by Mapcrafter corresponds to one anvil chunk (or 16x16 Minecraft blocks). 
    If you set ``tile_width = 2`` one Mapcrafter tile will correspond to 2x2 
    anvil chunks (32x32 blocks), which is a 1:2 factor. So a larger
    ``tile_width`` will create a smaller number of larger image files, and each 
    change in your Minecraft world will cause a larger area to be re-rendered.
    
    Usually ``tile_width = 1`` works very well. But there are some circumstances
    where a larger tile width is beneficial, including: smaller ``texture_size``
    values, rendering to slower hard disks, and Windows systems. These all benefit 
    from fewer files of larger size. 

.. note::

    A larger ``tile_width`` requires considerably more RAM during rendering and 
    viewing, as more tiles and chunks are kept in memory and browsers need to work
    with larger images. Please increase this slowly and test carefully!

.. note::

    If you change a map's ``tile_width``, you need to delete existing files
    for your map so that smaller tiles are removed.
    

**Image Format** ``image_format = png|jpeg``

    **Default:** ``png``
    
    This is the image format the renderer uses for the tile images.
    You can render your maps to PNGs or to JPEGs. PNGs are lossless, 
    JPEGs are faster to write and need less disk space. Also consider
    the ``png_indexed`` and ``jpeg_quality`` options.

**PNG Indexed** ``png_indexed = true|false``

    **Default:** ``false``

    With this option you can make the renderer write indexed PNGs. Indexed PNGs
    use a color table with 256 colors instead of writing the RGBA values for 
    every pixel. 256 colors is usually enough for Mapcrafter's images, and 
    requires ~¼ of the disk-space.

**JPEG Quality** ``jpeg_quality = <number between 0 and 100>``

    **Default:** ``85``
    
    This is the quality to use for the JPEGs. It should be a number
    between 0 and 100, where 0 is the worst quality which needs the least disk space
    and 100 is the best quality which needs the most disk space.

**Lighting Intensity** ``lighting_intensity = <number>``

    **Default:** ``1.0``
    
    This is the lighting intensity, i.e. the strength the renderer applies the
    lighting to the rendered map. You can specify a value from 0.0 to 1.0, 
    where 1.0 means full lighting and 0.0 means no lighting.

**Lighting Water Intensity** ``lighting_water_intensity = <number>``

    **Default:** ``0.85``

    This is like the normal lighting intensity option, but used for blocks that are under
    water. Usually the effect of opaque looking deep water is created by rendering just
    the top water layer and then applying the lighting effect on the (dark) floor of the
    water. By decreasing the lighting intensity for blocks under water you can make the
    water look "more transparent".

    You might have to play around with this to find a configuration that you like.
    For me ``water_opacity=0.75`` and ``lighting_water_intensity=0.6`` didn't look bad.

**Render Biomes** ``render_biomes = true|false``

    **Default:** ``true``

    This setting makes the renderer to use the original biome colors for blocks
    like grass and leaves. 


**Use Image Mtimes** ``use_image_mtimes = true|false``

    **Default:** ``true``

    This setting specifies the way the renderer should check if re-rendering 
    tiles is required. This only applies when re-rendering an existing map.
    Different behaviors are:

    Use the tile image modification times (``true``):
        The renderer checks the modification times of the already rendered 
        tile images. Any tiles with chunk timestamps newer than
        this modification time are re-rendered.
    Use the time of the last rendering (``false``):
        The renderer saves the time of the last rendering.  All tiles
        with chunk timestamps newer than this last-render-time are
        re-rendered.

    You can force re-rendering all tiles using the ``-f`` command line option.

.. note::

    **Obsolete and Changed Options**

    Several options were removed or changed in the Minecraft 1.13 overhaul in 
    Mapcrafter 3.0. Some of these have been moved into the `BlockCrafter 
    <https://github.com/mapcrafter/blockcrafter>`_ project.

    Instead of ``texture_dir``, you should use ``block_dir`` to pick up custom 
    textures or sprites.

    Options moved to :doc:`blockcrafter`: ``texture_blur``, ``water_opacity``, 
    ``render_leaves_transparent``. However, these are not currently configurable
    in BlockCrafter.

    Options removed entirely: ``crop_unpopulated_chunks`` (unpopulated chunks
    are always cropped), ``render_unknown_blocks`` (unknown blocks are extremely
    rare, and always rendered).
    
    
    
-----

.. _config_marker_options:

Marker Options
--------------

.. note::

    These options are for the marker groups. You can specify them in the marker
    sections (the ones starting with ``marker:``) or you can specify them in the 
    ``global:marker`` section.  If you specify them in the global section, these
    options are default values and inherited into the marker sections if you 
    do not overwrite them.

.. note::

    Marker options have no direct effect in Mapcrafter. You must use the separate 
    ``mapcrafter_markers`` executable. See :doc:`markers` for more information.

**Name:** ``name = <name>``

    **Default:** ``<name of the section>``
    
    .. image:: img/markers_list.png
       :align: center
       :alt: The names of marker groups.

    This is the name of the marker group. You should use a human-readable
    name  (spaces, numbers, symbols, even unicode are all OK) since this 
    name is displayed in the web-browser.

    The configuration section for each marker group has also a name (in square brackets).
    This section name is used for internal identifiers, so should be unique and only use
    alphanumeric chars and underscore (definitely no spaces).

**Matching Prefix:** ``prefix = <prefix>``

    **Default:** *Empty*
    
    This is the prefix a sign must have to be recognized as marker
    of this marker group. Example: If you choose ``[home]`` as prefix,
    all signs whose text starts with ``[home]`` are displayed as markers
    of this group.

    If you leave this empty, the marker section will match all signs.

**Matching Postfix:** ``postfix = <postfix>``

    **Default:** *Empty*
    
    This is the postfix a sign must have to be recognized as marker
    of this marker group.

.. note::

    Note that ``prefix`` and ``postfix`` may not overlap in the text sign to be
    matched. Example: If you have prefix ``foo`` and postfix ``oo bar`` and
    your sign text says ``foo bar``, it won't be matched. A sign with text
    ``foo ooaoo bar`` would be matched.

.. note::

    Marker configuration sections will only match each sign in your world once.
    You should order your ``[marker:..]`` sections to be from most specific to
    most generic. See :doc:`markers` for an example.


**Title Format:** ``title_format = <format>``

    **Default:** ``%(text)``
    
    You can change the title used for markers (the name shown when you 
    hover over a marker) by using different placeholders:
    
    =============== =======
    Placeholder     Meaning
    =============== =======
    ``%(text)``     Complete text of the sign without the prefix/postfix.
    ``%(prefix)``   Configured prefix of this marker group.
    ``%(postfix)``  Configured postfix of this marker group.
    ``%(textp)``    Complete text of the sign with the prefix/postfix.
    ``%(line1)``    First line of the sign.
    ``%(line2)``    Second line of the sign.
    ``%(line3)``    Third line of the sign.
    ``%(line4)``    Fourth line of the sign.
    ``%(x)``        X coordinate of the sign position.
    ``%(z)``        Z coordinate of the sign position.
    ``%(y)``        Y coordinate of the sign position.
    =============== =======
    
    The title of markers defaults to the text (without the prefix/postfix) of 
    the belonging sign, e.g. the placeholder ``%(text)``.
    
    You can use different placeholders and other text in this format
    string as well, for example ``Marker at x=%(x), y=%(y), z=%(z): %(text)``.
    
    You can add line breaks to marker text with ``<br>``.

**Text Format:** ``text_format = <format>``

    **Default:** *Same as title_format*
    
    You can change the text shown in the marker popup windows as well. 
    You can use the same placeholders you can use for the marker title. 
    And, you can use HTML for formatting.

**Icon:** ``icon = <icon>``

    **Default:** *Default Leaflet marker icon*

    This is the icon used for the markers of this marker group. 
    If left blank, the default Leaflet marker icon is used (a blue
    map marker).

    When using a custom icon, you should also specify the size of your 
    custom icon in ``icon_size``.

    You can put your own icons into the ``static/markers/`` directory
    of your template directory. Then you only need to specify the
    filename of the icon, the path ``static/markers/`` is automatically
    prepended. 

**Icon Size:** ``icon_size = <size>``

    **Default:** ``[24, 24]``

    This is the size of your icon. Specify it like ``[width, height]``.
    The icon size defaults to 24x24 pixels.

**Match Empty Signs:** ``match_empty = true|false``

    **Default:** ``false``
    
    This option specifies whether empty signs can be matched as markers.
    You have to set this to ``true`` and ``prefix`` to empty to 
    match empty signs.

**Show Markers By Default:** ``show_default = true|false``

    **Default:** ``true``
    
    With this option you can hide a marker group in the web interface by
    default. If ``true``, the markers appear when you map loads. If ``false``
    the markers are hidden until you click on the marker list.
