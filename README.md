# Mapcrafter #

by Moritz Hilscher

Mapcrafter is a high performance Minecraft map renderer written in C++. It renders
Minecraft worlds to a bunch of images which are viewable in any webbrowser
using Leaflet.js.

Mapcrafter is free software and available under the GPL license.  You can
access the latest source code of Mapcrafter on GitHub:
http://github.com/m0r13/mapcrafter

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code
were very helpful. I also used the alpha blending code of pigmap and some maps
stuff of the template from Minecraft Overviewer.

<div align="center">
    <img src="http://i.imgur.com/WyKXrgO.png" />
</div>

## Features ##

* **Web output:** Render your Minecraft worlds to maps with an isometric 3D perspective 
      viewable in any webbrowser!
* **Different directions:** Choose from four different directions to render your worlds!
* **Different rendermodes:** Choose between different rendermodes like day, night and cave 
     for your maps!
* **Configuration files:** Control which worlds are rendered from which direction and with 
     which rendermodes!
* **Markers:** Automatically generated markers from your Minecraft world data!
* **Other stuff:** Biome colors, incremental rendering, multithreading

## Requirements ##

* A Linux-based or Unix-like operating system like Mac OS
* A decent C++ compiler (gcc >= 4.4, clang), CMake and make if you want to build
      Mapcrafter from source
* Some libraries:
  * libpng
  * libboost-iostreams
  * libboost-system
  * libboost-filesystem
  * libboost-program-options
  * (libboost-test if you want to use the tests)
* For your Minecraft worlds:
  * Anvil world format
  * Minecraft 1.6 resource packs

## Examples ##

There are a few example maps of the renderer on the [GitHub
Wiki](https://github.com/m0r13/mapcrafter/wiki/Example-maps).  Please feel free
to add your own map to this list.

## Documentation and Help ##

The documentation is in the `docs/` directory and you can build it yourself
with Sphinx. You can read a built version of the documentation
[here](http://docs.mapcrafter.org) and you can also [download
other builds](https://readthedocs.org/projects/mapcrafter/downloads/).

If you find bugs or problems when using Mapcrafter or if you have ideas for new
features, then please feel free to add an issue to the [GitHub issue
tracker](https://github.com/m0r13/mapcrafter/issues).

You can contact me in IRC (#mapcrafter on Freenode). Use the 
[webclient](http://webchat.freenode.net/?channels=mapcrafter>) if you
are new to IRC. I will be there most of the time, but please bear in mind
that I can't be available all the time. If I'm not there, wait some time or 
try another time of the day.

You can also ask questions in the [Minecraft Forum
thread](http://www.minecraftforum.net/topic/1632003-mapcrafter-fast-minecraft-map-renderer/).

## Version history ##

**v.1.3**

* Added ability to automatically generate markers from signs in the
  Minecraft world
  * **The format to specify markers has changed, please have a look
    at the documentation and update your ``markers.js`` file.**

**v.1.2**

* Started refactoring the web template
  * **Since some files were split up and are now in a separate ``static/`` directory, 
    you should manually delete the old *.js *.css files in your map directories and
    update the maps with mapcrafter to prevent a mess with old files.**
* Added new options to specify the default view of a map
* Added new option to specify the dimension of a world
* Fixed lighting of The End
* Fixed a texture bug (#61) causing segfaults

**v.1.1.3**

* Fixed the batch mode showing an animated progress bar

**v.1.1.2**

* Fixed a bug (#58) with mapcrafter hanging when there are no tiles to render

**v.1.1.1**

* Refactored multi threading and some other things
* Fixed some small block issues:
  * Fixed downward spreading lava
  * Fixed not rendered nether portal blocks
  * Improved appearance of flat snow
  * Added some special (not used in the game) slabs
* Fixed a configuration bug

**v.1.1**

* Added ability to crop the world and render only a specific part of it

**v.1.0.1**

* Fixed jungle wooden planks rendering as birch planks
* Changed worldName attribute in template from part of the world path 
  to the name of the world section
* Added hint about rendering the Nether/End to documentation

**v.1.0**

* Added new configuration file format (consult the documentation for more information):
  * Different section types for Minecraft Worlds/rendered maps
  * Better validation of configuration files
* Some internal reworkings

**v.0.6**

* Added support for Minecraft 1.7 blocks and biomes
* Added some more blocks such as trip wire, trip wire hook and *magic floating* cocoa beans

**v.0.5.1**

* Added support for Minecraft 1.6 Resource Packs
  *  **(You have to delete your old textures and get them new from a new Minecraft Jar file)**
* Added some new blocks from Minecraft 1.6
* Better compatibility for older gcc (4.4 +) and boost versions
* Some code reworkings, replaced plain Makefile with CMake

**v.0.5**

* Added a new configuration file format to specify worlds to render
* Added different rendermodes: default, lighting (day/night), cave
* Added ability to render different worlds/rotations/rendermodes into one output file
* Added new blocks: dragon egg, redstone wires, some redstone/quartz stuff from Minecraft 1.5
* Added alternative way to check if tiles are required when rendering incremental (image file timestamps)
* Added alternative Leaflet template

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

* First version on GitHub
* Features:
  * Render Minecraft Worlds to tiles and an HTML-File to view them like a
    Google Map
  * Incremental rendering, multithreading
