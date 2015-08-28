=========================
Configuration File Format
=========================

To tell the Mapcrafter which maps to render, simple INI-like configuration
files are used. With configuration files it is possible to render maps with
multiple rotations and render modes into one output file. 

A First Example
===============

Here is a simple example of a configuration file (let's call it ``render.conf``):

.. code-block:: ini

    output_dir = myworld_mapcrafter

    [world:myworld]
    input_dir = worlds/myworld

    [map:myworld_isometric_day]
    world = myworld

As you can see the configuration files consist of different types of sections
(e.g. ``[section]``) and containing assignments of configuration options to
specific values (e.g. ``key = value``).  The sections have their names in
square brackets, where the prefix with the colon shows the type of the section.

There are three types (actually four, but more about that later) of sections:

* World sections (e.g. sections starting with ``world:``)
* Map sections (e.g. sections starting with ``map:``)
* Marker sections (e.g. sections starting with ``marker:``, also see :ref:`markers`)

Every world section represents a Minecraft world you want to render and needs a
directory where it can find the Minecraft world (``input_dir`` of the world
section ``myworld`` in the example above).

Every map section represents an actual rendered map of a Minecraft world. You
can specify things like rotation of the world, render view, render mode, texture
pack, texture size, etc. for each map.

In this example you can see that we have a world ``myworld`` in the directory
``worlds/myworld/`` which is rendered as the map ``myworld_isometric_day``.  The
directory ``output/`` is set as output directory. After the rendering you can
open the ``index.html`` file in this directory and view your rendered map.

As you can see the configuration option ``output_dir`` is not contained in any
section - it's in the so called *root section*. That's because all maps are
rendered into this directory and viewable via one ``index.html`` file, so the
``output_dir`` option is the same for all maps in this configuration file.

Also keep in mind that you can choose the section names (but not the section
types!) on your own, though it is recommended to use some kind of a fixed
format (for example ``<world name>_<render view>_<render mode>`` for maps) to
keep things consistent.

Let's have a look at a more advanced configuration file.

A More Advanced Example
=======================

.. code-block:: ini

    output_dir = output
    
    [global:map]
    world = world
    render_view = isometric
    render_mode = daylight
    rotations = top-left bottom-right
    texture_size = 12
    
    [world:world]
    input_dir = worlds/world
    
    [world:creative]
    input_dir = worlds/creative
    
    [map:world_isometric_day]
    name = Normal World - Day
    
    [map:world_isometric_night]
    name = Normal World - Night
    render_mode = nightlight
    
    [map:world_isometric_cave]
    name = Normal World - Cave
    render_mode = cave

    [map:world_topdown_day]
    name = Normal World - Topdown overview
    render_view = topdown
    texture_size = 6
    texture_blur = 2
    tile_width = 3
    
    [map:creative_isometric_day]
    name = Creative World - Day
    world = creative
    render_mode = daylight
    rotations = top-left top-right bottom-right bottom-left
    texture_dir = textures/special_textures
    texture_size = 16
    
    [map:creative_isometric_night]
    name = Creative World - Night
    world = creative
    render_mode = nightlight
    rotations = top-left top-right bottom-right bottom-left
    texture_dir = textures/special_textures
    texture_size = 16

Here we have some more worlds and maps defined. We have a "normal" world which
is rendered with the day, night, cave render mode, and also with the top view
and a lower texture size as overview map. Also we have a "creative" world which
is rendered with a special texture pack, higher texture size and all available
world rotations with the day and night render mode (super fancy!).

As you can see there is a new section ``global:map``. This section is used to
set default values for all map sections. Because of this in this example every
map has the world ``world``, the 3D isometric render view, the daylight render
mode, the world rotations top-left and top-right and the 12px texture size as
default. Of course you can overwrite these settings in every map section.  There
is also a global section ``global:world`` for worlds, but at the moment there is
only one configuration option for worlds (``input_dir``), so it doesn't make
much sense setting a default value here.

Furthermore every map has as option ``name`` a name which is used in the web
interface of the output HTML-File. This can be anything suitable to identify
this map. In contrast to that the world and map names in the sections are used
for internal representation and therefore should be unique and contain only
alphanumeric chars and underscores.

When you have now your configuration file you can render your worlds with (see
:ref:`command_line_options` for more options and usage)::

    mapcrafter -c render.conf

There are tons of other options to customize your rendered maps. Before a
reference of all available options, here is a quick overview of interesting
things you can do:

* Default view / zoom level / rotation in web interface
* World cropping (only render specific parts of your world)
* Block mask (skip rendering / render only specific types blocks)
* Different render views, render modes, overlays
* Use custom texture packs, texture sizes, apply a blur effect to textures
* Custom tile widths
* Different image formats
* Custom lighting intensity

Available Options
=================

General Options
---------------

.. note::

    These options are relevant for all worlds and maps, so you have to put them
    in the header before the first section starts

``output_dir = <directory>``

    **Required**

    This is the directory where Mapcrafter saves the rendered map. Every time
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

``background_color = <hex color>``

    **Default:** ``#DDDDDD``

    This is the background color of your rendered map. You have to specify
    it like an HTML hex color (``#rrggbb``).
    
    The background color of the map is set with a CSS option in the template.
    Because the JPEG image format does not support transparency and some tiles
    are not completely used, you have to re-render your maps which use JPEGs
    if you change the background color.

World Options
-------------

.. note::

    These options are for the worlds. You can specify them in the world
    sections (the ones starting with world:) or you can specify them in the
    global:world section.  If you specify them in the global section, these
    options are default values and inherited into the world sections if you do
    not overwrite them.

``input_dir = <directory>``

    **Required**

    This is the directory of your Minecraft world. The directory should contain
    a directory ``region/`` with the .mca region files.

``dimension = nether|overworld|end``

    **Default**: ``overworld``
    
    You can specify with this option the dimension of the world Mapcrafter should render.
    If you choose The Nether or The End, Mapcrafter will automagically detect the
    corresponding region directory. It will try the Bukkit region directory
    (for example ``myworld_nether/DIM-1/region``) first and then the directory of a normal
    vanilla server/client (for example ``myworld/DIM-1/region``).

.. note::

    If you want to render The Nether and want to see something, you should use the cave
    render mode or use the crop_max_y option to remove the top bedrock layers.

``world_name = <name>``

    **Default**: ``<name of the world section>``
    
    This is another name of the world, the name of the world the server uses.
    You don't usually need to specify this manually unless your server uses different
    world names and you want to use the mapcrafter-playermarkers script.

``default_view = <x>,<z>,<y>``

    **Default**: Center of the map
    
    You can specify the default center of the map with this option. Just specify a
    position in your Minecraft world you want as center when you open the map.

``default_zoom = <zoomlevel>``

    **Default**: ``0``
    
    This is the default zoom level shown when you open the map. The default zoom level
    is 0 (completely zoomed out) and the maximum zoom level (completely zoomed in) is the 
    one Mapcrafter shows when rendering your map.

``default_rotation = top-left|top-right|bottom-right|bottom-left``

    **Default**: First available rotation of the map
    
    This is the default rotation shown when you open the map. You can specify one of the 
    four available rotations. If a map doesn't have this rotation, the first available
    rotation will be shown. 

By using the following options you can crop your world and render only 
a specific part of it. With these two options you can skip blocks above or
below a specific level:

``crop_min_y = <number>``

    **Default:** -infinity

    This is the minimum y-coordinate of blocks Mapcrafter will render.

``crop_max_y = <number>``

    **Default:** infinity

    This is the maximum y-coordinate of blocks Mapcrafter will render.

Furthermore there are two different types of world cropping:

1. Rectangular cropping:

  * You can specify limits for the x- and z-coordinates.
    The renderer will render only blocks contained in these boundaries.
    You can use the following options whereas all options are optional
    and default to infinite (or -infinite for minimum limits):
    
    * ``crop_min_x`` (minimum limit of x-coordinate)
    * ``crop_max_x`` (maximum limit of x-coordinate)
    * ``crop_min_z`` (minimum limit of z-coordinate)
    * ``crop_max_z`` (maximum limit of z-coordinate)

2. Circular cropping:

  * You can specify a block position as center and a radius.
    The renderer will render only blocks contained in this circle:
    
    * ``crop_center_x`` (**required**, x-coordinate of the center)
    * ``crop_center_z`` (**required**, z-coordinate of the center)
    * ``crop_radius`` (**required**, radius of the circle)

.. note::

    The renderer automatically centers circular cropped worlds and rectangular
    cropped worlds which have all four limits specified so the maximum
    zoom level of the rendered map does not unnecessarily become as high as 
    the original map. 
    
    Changing the center of an already rendered map is complicated and 
    therefore not supported by the renderer. Due to that you should 
    completely rerender the map when you want to change the boundaries of 
    a cropped world. This also means that you should delete the already 
    rendered map (delete <output_dir>/<map_name>).

The provided options for world cropping are very versatile as you can see
with the next two options:

``crop_unpopulated_chunks = true|false``

    **Default:** ``false``
    
    If you are bored of the chunks with unpopulated terrain at the edges of
    your world, e.g. no trees, ores and other structures, you can skip rendering
    them with this option. If you are afraid someone might use this to find
    rare ores such as Diamond or Emerald, you should not enable this option.

``block_mask = <block mask>``

    **Default:** *show all blocks*
    
    With the block mask option it is possible to hide or shown only specific blocks.
    The block mask is a space separated list of block groups you want to 
    hide/show. If a ``!`` precedes a block group, all blocks of this block group are
    hidden, otherwise they are shown. Per default, all blocks are shown.
    Possible block groups are:
    
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
    
    * Show all blocks except jungle wood and jungle leaves:
    
      * ``!17:3b3 !18:3b3``
      * Jungle wood and jungle leaves have id 17 and 18 and use data value 3 for first two bits (bitmask 3 = 0b11)
      * other bits are used otherwise -> ignore all those bits


Map Options
-----------

.. note::

    These options are for the maps. You can specify them in the map sections
    (the ones starting with map:) or you can specify them in the global:map
    section.  If you specify them in the global section, these options are
    default values and inherited into the map sections if you do not overwrite
    them.

``name = <name>``

    **Default:** ``<name of the section>``

    This is the name for the rendered map. You will see this name in the output
    file, so you should use here an human-readable name. The belonging
    configuration section to this map has also a name (in square brackets).
    Since the name of the section is used for internal representation, the name
    of the section should be unique and you should only use alphanumeric chars.

``render_view = isometric|topdown``

    **Default:** ``isometric``

    This is the view that your world is rendered from. You can choose from
    different render views:

    ``isometric``
      A 3D isometric view looking at north-east, north-west, south-west or 
      south-east (depending on the rotation of the world).
    ``topdown``
      A simple 2D top view.

``render_mode = plain|daylight|nightlight|cave``
	
    **Default:** ``daylight``

    This is the render mode to use when rendering the world. Possible
    render modes are:

    ``plain``
        Plain render mode without lighting or other special magic.
    ``daylight``
        Renders the world with lighting.
    ``nightlight``
        Like ``daylight``, but renders at night.
    ``cave``
        Renders only caves and colors blocks depending on their height 
        to make them easier to recognize.

.. note::

    The old option name ``rendermode`` is still available, but deprecated.
    Therefore you can still use it in old configuration files, but Mapcrafter
    will show a warning.

``overlay = slime|spawn``

    **Default:** ``none``

    Additionally to a render mode, you can specify an overlay. An overlay is a
    special render mode that is rendered on top of your map and the selected
    render mode. The following overlays are used to show some interesting
    additional data extracted from the Minecraft world data:

    ``none``
      Empty overlay.
    ``slime``
      Highlights the chunks where slimes can spawn.
    ``spawnday``
      Shows where monsters can spawn at day.
    ``spawnnight``
      Shows where monsters can spawn at night.

    At the moment there is only one overlay per map section allowed because the overlay
    is rendered just like a render mode on top of the world. If you want to render
    multiple overlays, you need multiple map sections. This behavior might change in
    future Mapcrafter versions so you will be able to dynamically switch multiple
    overlays on and off in the web interface.

``rotations = [top-left] [top-right] [bottom-right] [bottom-left]``

    **Default:** ``top-left``

    This is a list of directions to render the world from. You can rotate the
    world by n*90 degrees. Later in the output file you can interactively
    rotate your world. Possible values for this space-separated list are:
    ``top-left``, ``top-right``, ``bottom-right``, ``bottom-left``. 

    Top left means that north is on the top left side on the map (same thing
    for other directions).

``texture_dir = <directory>``

    **Default:** default texture directory (see :ref:`resources_textures`)

    This is the directory with the Minecraft Texture files.  The renderer works
    with the Minecraft 1.6 resource pack file format. You need here: 

    * directory ``chest/`` with normal.png, normal_double.png and ender.png 
    * directory ``colormap/`` with foliage.png and grass.png 
    * directory ``blocks/`` from your texture pack
    * endportal.png

    See also :ref:`resources_textures` to see how to get these files.

``texture_size = <number>``

    **Default:** ``12``

    This is the size (in pixels) of the block textures. The default texture
    size is 12px (16px is the size of the default Minecraft Textures).

    The size of a tile is ``32 * texture_size``, so the higher the texture
    size, the more image data the renderer has to process. If you want a high
    detail, use texture size 16, but texture size 12 looks still good and is
    faster to render.

``texture_blur = <number>``

    **Default:** ``0``

    You can apply a simple blur filter with a radius of ``<number>`` pixels to
    the texture images. This might be useful if you are using a very low texture
    size because areas with their blocks sometimes look a bit "tiled".

``water_opacity = <number>``

    **Default:** ``1.0``

    With a factor from 0.0 to 1.0 you can modify the opacity of the used water texture
    before your map is rendered. 0 means that it is completely transparent and 1 means
    that the original opacity of the texture is kept. Also have a look at the
    ``lighting_water_intensity`` option.

.. note::

    Don't actually set the water opacity to 0.0, that's a bad idea regarding performance.
    If you don't want to render water, have a look at the ``block_mask`` option.

``tile_width = <number>``

    **Default:** ``1``

    This is a factor that is applied to the tile size. Every (square) tile is
    usually one chunk wide, but you can increase that size. The wider a tile
    is, the more blocks it contains and the longer it takes to render a tile,
    but the less tiles are to render overall and the less overhead there is
    when writing the tile images. Use this if your texture size is small and
    you want to prevent that a lot of very small tiles are rendered.

``image_format = png|jpeg``

    **Default:** ``png``
    
    This is the image format the renderer uses for the tile images.
    You can render your maps to PNGs or to JPEGs. PNGs are losless, 
    JPEGs are faster to write and need less disk space. Also consider
    the ``png_indexed`` and ``jpeg_quality`` options.

``png_indexed = true|false``

    **Default:** ``false``

    With this option you can make the renderer write indexed PNGs. Indexed PNGs
    are using a color table with 256 colors (which is usually enough for this
    kind of images) instead of writing the RGBA values for every pixel. Like
    using JPEGs, this is another way of drastically reducing the needed disk
    space of the rendered images.

``jpeg_quality = <number between 0 and 100>``

    **Default:** ``85``
    
    This is the quality to use for the JPEGs. It should be a number
    between 0 and 100, where 0 is the worst quality which needs the least disk space
    and 100 is the best quality which needs the most disk space.

``lighting_intensity = <number>``

    **Default:** ``1.0``
    
    This is the lighting intensity, i.e. the strength the renderer applies the
    lighting to the rendered map. You can specify a value from 0.0 to 1.0, 
    where 1.0 means full lighting and 0.0 means no lighting.

``lighting_water_intensity = <number>``

    **Default:** ``1.0``

    This is like the normal lighting intensity option, but used for blocks that are under
    water. Usually the effect of opaque looking deep water is created by rendering just
    the top water layer and then applying the lighting effect on the (dark) floor of the
    water. By decreasing the lighting intensity for blocks under water you can make the
    water look "more transparent". Use this option together with the ``water_opacity``
    option. You might have to play around with this to find a configuration that you like.
    For me ``water_opacity=0.75`` and ``lighting_water_intensity=0.6`` didn't look bad.

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

.. _config_marker_options:

Marker Options
--------------

.. note::

    These options are for the marker groups. You can specify them in the marker
    sections (the ones starting with marker:) or you can specify them in the 
    global:marker section.  If you specify them in the global section, these
    options are default values and inherited into the marker sections if you 
    do not overwrite them.

``name = <name>``

    **Default:** *Name of the section*
    
    This is the name of the marker group. You can use a human-readable
    name since this name is displayed in the webinterface.

``prefix = <prefix>``

    **Default:** *Empty*
    
    This is the prefix a sign must have to be recognized as marker
    of this marker group. Example: If you choose ``[home]`` as prefix,
    all signs whose text starts with ``[home]`` are displayed as markers
    of this group.


``title_format = <format>``

    **Default:** ``%(text)``
    
    You can change the title used for markers (the name shown when you 
    hover over a marker) by using different placeholders:
    
    =============== =======
    Placeholder     Meaning
    =============== =======
    ``%(text)``     Complete text of the sign without the prefix.
    ``%(prefix)``   Configured prefix of this marker group.
    ``%(textp)``    Complete text of the sign with the prefix.
    ``%(line1)``    First line of the sign.
    ``%(line2)``    Second line of the sign.
    ``%(line3)``    Third line of the sign.
    ``%(line4)``    Fourth line of the sign.
    ``%(x)``        X coordinate of the sign position.
    ``%(z)``        Z coordinate of the sign position.
    ``%(y)``        Y coordinate of the sign position.
    =============== =======
    
    The title of markers defaults to the text (without the prefix) of 
    the belonging sign, e.g. the placeholder ``%(text)``.
    
    You can use different placeholders and other text in this format
    string as well, for example ``Marker at x=%(x), y=%(y), z=%(z): %(text)``.

``text_format = <format>``

    **Default:** *Format of the title*
    
    You can change the text shown in the marker popup windows as well.
    You can use the same placeholders you can use for the marker title.

``icon = <icon>``

    **Default:** *Default Leaflet marker icon*

    This is the icon used for the markers of this marker group. You
    do not necessarily need to specify a custom icon, you can also
    use the default icon.
    
    You can put your own icons into the ``static/markers/`` directory
    of your template directory. Then you only need to specify the
    filename of the icon, the path ``static/markers/`` is automatically
    prepended. You should also specify the size of your custom icon.

``icon_size = <size>``

    **Default:** ``[24, 24]``

    This is the size of your icon. Specify it like ``[width, height]``.
    The icon size defaults to 24x24 pixels.

``match_empty = true|false``

    **Default:** ``false``
    
    This option specifies whether empty signs can be matched as markers.
    You have to set this to ``true`` if you set the prefix to an empty
    string to show all remaining unmatched signs as markers and if you
    want to show even empty signs as markers.

``show_default = true|false``

    **Default:** ``true``
    
    With this option you can hide a marker group in the web interface by
    default.
