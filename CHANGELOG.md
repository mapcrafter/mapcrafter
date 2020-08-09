# Mapcrafter version history

**v.3.0**

* Add support for Minecraft versions 1.13 through 1.16 (#279)

**v.2.4**

* Add support for Minecraft 1.11 and 1.12 blocks (thanks to Bjarno #256)

**v.2.3.1**

* Fixed wrong Leaflet coordinate conversion in topdown and isometric render view when
    using different tiles sizes and fixed wrong chunk <-> tile mapping which caused the
    issue in isometric render view (#198)
  * **Unfortunately you have to force-re-render your isometric maps as the isometric
    chunk <-> tile mapping has slightly changed.**

**v.2.3**

* Added support for Minecraft 1.10 blocks (#226)
* Added new bootstrap web UI (thanks to jamestaylr #205)
* Added `postfix` option for signs to also use a postfix instead of just a prefix (#223)
* Added `--force-render-all` to force-render all maps (#212)
* Fixed map selection on mobile devices (thanks to joshproehl #220 #123)
* Fixed a bug in the bilinear image resizing algorithm that caused dark sides of
    grass and other transparent blocks

**v.2.2.1**

* Updated Minecraft version file to 1.9

**v.2.2**

* Added support for 1.9 blocks (and hoppers)
* Fixed bug which caused water not being rendered (#178)
* Fixed circular crop overflowing bug (#179)
* Fixed a config validation bug (#185)

**v.2.1**

* Added support for corner stairs (thanks for some Minecraft Overviewer code!)
* Added signs to isometric and topdown render view
* Fixed a lighting bug that appeared using some Debian packages
* Fixed wrong slime overlay
* Fixed rotation of pumpkins in isometric render view

**v.2.0**

* Huge internal code refactoring™
* Support for multiple rendering engines: A 2D topdown and the usual 3D isometric render
  view are available
* Support for indexed png images, reducing size of output images drastically
* New overlay render modes (slime chunks, show where monsters can spawn at day/night)
* Configurable tile sizes (to improve performance for small texture sizes)
* Configurable water opacity/transparency
* Restructured internal render management components to allow integration of Mapcrafter
  rendering core into different applications
* Further integration of Mapcrafter into a graphical user interface
  ([mapcrafter-gui](https://github.com/mapcrafter/mapcrafter-gui))
* **There is a new repository for Debian/Ubuntu packages. Please have a look at the
    documentation and update your apt source listings!**

**v.1.5.4**

* Added a detection in the CMake files which compiler C++11 flag to use (for newer
  compilers which do not support the older ``-std=c++0x`` flag)
* Fixed bug with the marker tool (new json sign text format was not parsed properly)
* Fixed bug in texture extraction script (convert command was not properly detected)
* Fixed bug with some textures (#122) by making Mapcrafter able to read indexed PNG images
  and PNG images with different other color formats

**v.1.5.3**

* Better handling of corrupt world files (for now, bigger refactoring of that probably later)
* Added a fix using imagemagick in texture extraction script to prevent libpng warnings
* Fixed bug with boost program options which made it unable to use paths
  containing spaces for the ``--config`` command line option
* Fixed wrong error messages when loading configuration files (Mapcrafter tried
  to read not existing files and even directories if specified as configuration file)

**v.1.5.2**

* Fixed the marker tool to work with the new internal Minecraft 1.8 sign format
* Fixed bug with global logging configuration file installed and searched in wrong directory
* Fixed bug causing the Leaflet map being stuck
* Fixed some small issues with new 1.8 blocks

**v.1.5.1**

* Added ability to configure logging facility with configuration files
* Added colored terminal output for warning/error log messages
* Improved cave rendermode with a new high contrast block coloring (thanks to dtfinch, #100),
  old behavior is still available with a configuration option for compatibility reasons
* Added support for Minecraft 1.8
  * **Since the structure of the textures has changed a bit (especially chest textures),
    you should extract the texture files from a 1.8 Minecraft Jar file to a new, clean
    texture directory.**

**v.1.5**

* Started internal refactoring to improve integration with possible Mapcrafter GUI
  or other programs using Mapcrafter's functionality
* Added own logging facility, further configuration with different log sinks
  will follow soon™
* Changed global sections from ``[global:sections]`` to ``[global:section]`` to
  simplify internal parsing procedures
* Added option to specify a block mask and render only specific blocks
* Added option to hide chunks that are not populated yet

**v.1.4.3**

* Fixed a lighting bug on FreeBSD and Windows
* Improved build support on Windows for MinGW and Visual C++

**v.1.4.2**

* Fixed a small problem with the multi threading

**v.1.4.1**

* Fixed ``mapcrafter_markers`` not being installed
* Fixed debian package dependency issues with libjpeg-turbo
* Fixed issue with CMake not finding libjpeg outside the default search path

**v.1.4**

* Added option to use JPEGs as image output format
* Added option to change the background color of the map

**v.1.3.2**

* Added option to hide specific marker groups in the web interface by default
* Added a way to add more user-defined markers using the Leaflet API (#71)
* Changed ``markers.js`` file attribute ``icon_size`` to ``iconSize``
  * **You should update your ``markers.js`` file and re-run ``mapcrafter_markers``
    if you use markers.** 
* Fixed a bug causing Mapcrafter not to use the ``data/`` directory as
  resource directory (#70)

**v.1.3.1**

* Added simple progress output to marker generation program (optional)
* Added option to hide empty signs by default
* Fixed a bug (#65) causing invalid Javascript being generated
* Markers use the world section name to distinguish between the worlds,
  not the ``world_name`` attribute

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
  * **(You have to delete your old textures and get them new from a new Minecraft Jar file)**
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
  * Render Minecraft Worlds to tiles and an HTML-File to view them like a Google Map
  * Incremental rendering, multithreading
