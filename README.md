# mapcrafter #

mapcrafter is a fast minecraft world renderer written in C++. It renders minecraft
worlds to a bunch of images which are viewable in any webbrowser using the Google Maps API.

mapcrafter runs on linux (maybe also Mac OS or w****** with cygwin) and needs libpng, 
libpthread, libboost-iostreams,libboost-system, libboost-filesystem, and 
libboost-program-options (and libboost-test if you want to use the tests). 
You can build it with g++ and the supplied makefile.

mapcrafter is not yet finished. At the moment not all blocks are supported, but the
basic rendering routines (also incremental rendering, multithreading) are implemented.
The renderer works with the 'new' minecraft anvil worlds.

mapcrafter is free software and available under the GPL license.
You can access the latest source of mapcrafter at: http://github.com/m0r13/mapcrafter

Please feel free to report bugs and errors you find when using mapcrafter.

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code were 
very helpful. I also used the alpha blending code of pigmap and some maps stuff in the 
template from Minecraft Overviewer.

## How to use the renderer ##

Here is a list of available command line options:

-h [--help]

This shows a help about the command line options.

-i [--input-dir=] (directory)

This is the directory of the minecraft world to render. The directory should contain a 
directory 'region' with the *.mca region files.


-o [--output-dir=] (directory)

This is the directory, where mapcrafter saves the rendered map. Every time you render your
map, the renderer copies the template files into this directory and overwrites them, if 
they already exist. The renderer creates an 'index.html' file you can open with your 
webbrowser. If you want to customize this html file, you should do this directly in the 
template (see data dir).

-d [--data-dir=] (directory)

This is the directory with a template directory and the minecraft images needed for the 
rendering. 

You need here the terrain.png, chest.png, enderchest.png and largechest.png (probably 
from your minecraft.jar). You can use the python script in the data directory to extract 
the images from your minecraft.jar. 

The web templates are stored inside the data directory in the directory 'template'. You 
can make a copy of the data directory and customize the templates.

-j [--jobs=] (number)

This is the count of threads to use (default 1), when rendering the highest zoom level 
tiles (the tiles which are directly rendered from the world data). The rendering 
performance also depends heavily on your disk. You can render the map to a solid state 
disk or a ramdisk to improve the performance. 

At the moment the other tiles (composite tiles), which are composed of the top level 
tiles and other composite tiles, are rendered by a single thread.

-u [--incremental]

When you specify this, the renderer checks, which chunks were changed since the last 
rendering of the world. It calculates then the tiles, which need to get rendered. You can 
use this, if you already rendered your map completely. Incremental rendering is much 
faster than a full rendering, if there are only a few changes in your minecraft world.



**Here are some settings you can only specifiy, if you do a full rendering. In case of 
incremental rendering, the renderer reads this settings from the file 'map.settings' in 
the output directory.**


--texture-size=(a number)

This is the size (in pixels) of the block textures. The default texture 
size is 12px (16px is the size of the default minecraft textures). The size (width and 
height) of your terrain.png must be a multiple of 16 and width and height must be equal.

The size of a tile is 32*texturesize, so the higher the texture size, the more image data 
the renderer has to process. If you want a high detail, use texture size 16, but texture
size 12 looks still good and is faster to render.

--render-unknown-blocks

With this setting the renderer renders unknown blocks as red blocks (for debugging 
purposes). Per default the renderer just ignores unknown blocks and doesn't render them.

--render-leaves-transparent

You can specifiy this to use the transparent leaves textures instead of the opaque
textures. This option can make the renderer a bit slower, because the renderer has to scan
the blocks after the leaves to the ground also.

## Version history ##

**v.0.1 (December 2012)**

* first version on github, features:
* rendering minecraft worlds to tiles and an html file to view them like a Google Map
* incremental rendering, multithreading