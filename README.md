# mapcrafter #

by Moritz Hilscher

mapcrafter is a fast Minecraft world renderer written in C++. It renders
Minecraft worlds to a bunch of images which are viewable in any webbrowser
using the Google Maps API.

mapcrafter runs on linux (maybe also Mac OS or W****** with cygwin) and needs
libpng, libpthread, libboost-iostreams,libboost-system, libboost-filesystem,
and libboost-program-options (and libboost-test if you want to use the tests).
You can build it with g++ and the supplied makefile.

mapcrafter is not yet finished. At the moment a few blocks are not supported,
but the basic rendering routines are implemented. The renderer works with the
Anvil world format and the new Minecraft 1.5 texture packs.

Some features of the renderer are:
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
stuff of template from Minecraft Overviewer.

## How to use the renderer ##

Here is a list of available command line options:

-h [--help]

This shows a help about the command line options.

-i [--input-dir=] directory

This is the directory of the Minecraft world to render. The directory should
contain a directory 'region' with the .mca region files.

-o [--output-dir=] directory

This is the directory, where mapcrafter saves the rendered map. Every time you
render your map, the renderer copies the template files into this directory and
overwrites them, if they already exist. The renderer creates an 'index.html'
file you can open with your webbrowser. If you want to customize this html
file, you should do this directly in the template (see template).

--template=directory

This is the directory with the web template files (defaults to data/template).
The renderer copies all files, which are in this directory, to the output
directory and replaces the variables in the index.html file. The index.html
file is also the file in the output directory you can open with your webbrowser
after the rendering. 

--textures=directory

This is the directory with the Minecraft texture files (defaults to
data/textures).  The renderer works with the Minecraft 1.5 texture files. You
need here: 
* chest.png
* enderchest.png
* largechest.png
* foliagecolor.png
* grasscolor.png
* the blocks/ directory from your texture pack

Probably you can get everything from your minecraft.jar. You can use the python
script 'find_images.py' in the data directory to extract the images from your
minecraft.jar.

-j [--jobs=] number

This is the count of threads to use (defaults to one), when rendering the map.
The rendering performance also depends heavily on your disk. You can render the
map to a solid state disk or a ramdisk to improve the performance.

Every thread needs around 150MB ram.

-u [--incremental]

When you specify this, the renderer checks, which chunks were changed since the
last rendering of the world. Then it calculates the tiles, which need to get
rendered. You can use this, if you already rendered your map completely.
Incremental rendering is much faster than a full rendering, if there are only a
few changes in your minecraft world.

-b [--batch]

This option deactivates the animated progress bar.


**Here are some settings you can only specifiy, if you do a full rendering. In
case of incremental rendering, the renderer reads this settings from the file
'map.settings' in the output directory.**


--texture-size=number

This is the size (in pixels) of the block textures. The default texture size is
12px (16px is the size of the default Minecraft textures).

The size of a tile is 32*texturesize, so the higher the texture size, the more
image data the renderer has to process. If you want a high detail, use texture
size 16, but texture size 12 looks still good and is faster to render.

--north-dir=direction

With this setting, you can rotate the world by n*90 degrees. Possible values
are: top-left (default), top-right, bottom-right, bottom-left. Top left means
that north is on the top left side on the map (same thing for other
directions).

--render-unknown-blocks

With this setting, the renderer renders unknown blocks as red blocks (for
debugging purposes). Per default, the renderer just ignores unknown blocks and
does not render them.

--render-leaves-transparent

You can specifiy this to use the transparent leave textures instead of the
opaque textures. This option can make the renderer a bit slower, because the
renderer also has to scan the blocks after the leaves to the ground.

--render-biomes

This setting makes the renderer to use the original biome colors for blocks
like grass and leaves. At the moment, the renderer does not use the biome
colors for water, because the renderer preblits the water blocks (great
performance improvement) and it is not very easy to preblit all biome color
variants. And also, there is not a big difference with different water colors. 

## Version history ##

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

* first version on github, features:
* rendering minecraft worlds to tiles and an html file to view them like a
  Google Map
* incremental rendering, multithreading
