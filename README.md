# mapcrafter #

by Moritz Hilscher

mapcrafter is a fast Minecraft World renderer written in C++. It renders
Minecraft Worlds to a bunch of images, which are viewable in any webbrowser
using the Google Maps API.

mapcrafter runs on linux (maybe also Mac OS or W****** with cygwin) and needs
libpng, libpthread, libboost-iostreams,libboost-system, libboost-filesystem,
and libboost-program-options (and libboost-test if you want to use the tests).
You can build it with g++ and the supplied makefile.

mapcrafter is not yet finished. At the moment a few blocks are not supported,
but the basic rendering routines are implemented. The renderer works with the
Anvil World Format and the new Minecraft 1.5 Texture Packs.

Some features of the renderer are:

* Rendering Minecraft Worlds to maps viewable in any webbrowser
* Configuration files to control the renderer
* Four different directions to render your worlds from with an isometric 3D view
* Different rendermodes
* Biome colors
* Incremental rendering, multithreading
* User-defined markers on your maps

mapcrafter is free software and available under the GPL license.  You can
access the latest source code of mapcrafter at:
http://github.com/m0r13/mapcrafter

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code
were very helpful. I also used the alpha blending code of pigmap and some maps
stuff of the template from Minecraft Overviewer.

## Documentation ##

The documentation is in the `docs/` directory and you can build it yourself
with Sphinx. You can read a built version of the documentation on
[Read the Docs](http://mapcrafter.readthedocs.org) and you can also [download
other builds](https://readthedocs.org/projects/mapcrafter/downloads/).

## Examples ##

There are a few example maps of the renderer on the [GitHub
Wiki](https://github.com/m0r13/mapcrafter/wiki/Example-maps).  Please feel free
to add your own map to this list.

## Help ##

Please read the [documentation](http://mapcrafter.readthedocs.org) to find out
how to use the renderer.

If you find bugs or problems when using mapcrafter or if you have ideas for new
features, then please feel free to add an issue to the [GitHub issue
tracker](https://github.com/m0r13/mapcrafter/issues).

You can also ask questions in the [Minecraft Forum
thread](http://www.minecraftforum.net/topic/1632003-mapcrafter-fast-minecraft-map-renderer/).

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
