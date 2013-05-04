# mapcrafter #

by Moritz Hilscher

mapcrafter is a fast Minecraft world renderer written in C++. It renders
Minecraft worlds to a bunch of images, which are viewable in any webbrowser
using the Google Maps API.

mapcrafter runs on linux (maybe also Mac OS or W****** with cygwin) and needs
libpng, libpthread, libboost-iostreams,libboost-system, libboost-filesystem,
and libboost-program-options (and libboost-test if you want to use the tests).
You can build it with g++ and the supplied makefile.

mapcrafter is not yet finished. At the moment a few blocks are not supported,
but the basic rendering routines are implemented. The renderer works with the
Anvil world format and the new Minecraft 1.5 texture packs.

Some features of the renderer are:

* configuration files to control the renderer
* incremental rendering
* multithreading
* rotated worlds
* biome colors

mapcrafter is free software and available under the GPL license.  You can
access the latest source code of mapcrafter at:
http://github.com/m0r13/mapcrafter

Please feel free to report bugs and errors you find when using mapcrafter.

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code
were very helpful. I also used the alpha blending code of pigmap and some maps
stuff of the template from Minecraft Overviewer.

## How to use the renderer ##

### Configuration file format ###

To tell the mapcrafter which maps to render, simple INI-like configuration
files are used. Here is an example of a configuration file render.conf:

	output_dir = output
	template_dir = mapcrafter/src/data/template
	textures_dir = mapcrafter/src/data/textures

	texture_size = 12

	[main]
	name = Main World
	world = worlds/main
	rotations = top-left top-right bottom-left bottom-right

	[creative]
	name = Creative World
	world = worlds/creative
	textures_dir = my/special/textures
	rotations = top-left bottom-left

You can render this worlds with (see command line options for more options):

	./mapcrafter -c render.conf

The configuration files consist of `key = value` pairs and sections (in square
brackets) with options. The sections of the configuration files are the maps to
render. 

All these maps are rendered into one output directory, so you can view them in
one HTML-File. For every map you can specify a list of rotations. When you
decide to render all four directions of a map, you can interactively rotate the
world on the webbrowser. Relative paths in the configuration file are relative
to the path of the configuration file.

Here is a list of available options:

**These options are relevant for all maps, so you have to put them in the
header before the first section starts:**

`output_dir = <directory>`

This is the directory, where mapcrafter saves the rendered map. Every time you
render your map the renderer copies the template files into this directory and
overwrites them, if they already exist. The renderer creates an index.html file
you can open with your webbrowser. If you want to customize this HTML-File, you
should do this directly in the template (see `template_dir`).

`template_dir = <directory>`

This is the directory with the web template files. The renderer copies all
files, which are in this directory, to the output directory and replaces the
variables in the index.html file. The index.html file is also the file in the
output directory you can open with your webbrowser after the rendering.

**These options are for the maps. You can specify them in the sections or you
can specify them in the header. If you specify them in the header, these
options are default values and inherited in the sections if you do not
overwrite them:**

`name = <name>`

This is the name for the rendered map. You will see this name in the output file,
so you should use here an human-readable name. The belonging configuration
section to this map has also a name (in square brackets). Since the name of the
section is used for internal representation, the name of the section should be
unique and you should only use alphanumeric chars.

`world = <directory>`

This is the directory of the Minecraft world to render. The directory should
contain a directory 'region' with the .mca region files.

`textures_dir = <directory>`

This is the directory with the Minecraft texture files.  The renderer works
with the Minecraft 1.5 texture file format. You need here: 

* chest.png
* enderchest.png
* largechest.png
* foliagecolor.png
* grasscolor.png
* the blocks/ directory from your texture pack

Probably you can get everything from your minecraft.jar. You can use the python
script `find_images.py` from the data directory to extract the images from your
minecraft.jar.

`texture_size = <number>`

This is the size (in pixels) of the block textures. The default texture size is
12px (16px is the size of the default Minecraft textures).

The size of a tile is `32 * texture_size`, so the higher the texture size, the
more image data the renderer has to process. If you want a high detail, use
texture size 16, but texture size 12 looks still good and is faster to render.

`rotations = [top-left] [top-right] [bottom-right] [bottom-left]`

This is a list of rotations to render the world from. You can rotate the world
by n*90 degrees. Later in the output file you can interactively rotate your
world. Possible values for this space separated list are: top-left, top-right,
bottom-right, bottom-left. Top left means that north is on the top left side on
the map (same thing for other directions). This option defaults to top-left.

`render_unknown_blocks = 1|0`

With this option the renderer renders unknown blocks as red blocks (for
debugging purposes). Per default the renderer just ignores unknown blocks and
does not render them.

`render_leaves_transparent = 1|0`

You can specifiy this to use the transparent leaf textures instead of the
opaque textures. Using transparent leaf textures can make the renderer a bit
slower, because the renderer also has to scan the blocks after the leaves to
the ground. Per default the renderer renders leaves transparent.

`render_biomes = 1|0`

This setting makes the renderer to use the original biome colors for blocks
like grass and leaves. At the moment the renderer does not use the biome
colors for water because the renderer preblits the water blocks (great
performance improvement) and it is not very easy to preblit all biome color
variants. And also, there is not a big difference with different water colors.
Per default the renderer renders biomes.

### Command line options ###

Here is a list of available command line options:

`-h [--help]`

This shows a help about the command line options.

`-c [--config=] file`

This is the path to the configuration file to use when rendering.

`-s [--render-skip=] maps`

`-r [--render=] maps`

`-f [--render-force] maps`

`-j [--jobs=] number`

This is the count of threads to use (defaults to one), when rendering the map.
The rendering performance also depends heavily on your disk. You can render the
map to a solid state disk or a ramdisk to improve the performance.

Every thread needs around 150MB ram.

`-b [--batch]`

This option deactivates the animated progress bar.


## Version history ##

**v.0.4.1**

* Fixed a bug preventing rotated worlds
* Fixed a small compiler error for some gcc versions
* Fixed the problem with the world scanning (world scanning seems to be stuck)
  when the max zoom level is higher
* Added compatibility for older boost filesystem versions

**v.0.4**

* Added support for biomes
* Added support for Minecraft 1.5 texture packs
* Added some new blocks
* Some template things: url hash with current view, better support for markers

**v.0.3**

* Reworked multithreading
* Added possibility to render the world rotated
* Reworked block image generation to work with the rotated worlds, fixed
  rotation of blocks
* Added nicer beacon, fixed wrong stone slab texture

**v.0.2**

* Added support for chests, fences, fence gates
* Added option --batch, to render without the animated progress bar
* Small fixes for end portal frame, enchantment table to work with all texture
  sizes
* Some code reworking, more comments

**v.0.1 (December 2012)**

* First version on github, features:
* Rendering minecraft worlds to tiles and an html file to view them like a
  Google Map
* Incremental rendering, multithreading
