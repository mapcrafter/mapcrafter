==========
mapcrafter
==========

Welcome
=======

.. image:: mapcrafter.png
	:align: center
	:width: 700px

mapcrafter is a fast Minecraft World Renderer written in C++. It renders
Minecraft Worlds to a bunch of images, which are viewable using Leaflet like a big Google Map 
in any webbrowser. mapcrafter is not yet finished. A few blocks are not supported
yet, but all rendering routines are implemented. 

The renderer works with the Anvil World format and the Minecraft 1.6 Resource Packs.

mapcrafter is free software and available under the GPL license. You can
access the latest source code of mapcrafter on GitHub:
http://github.com/m0r13/mapcrafter

There are a few example maps of the renderer on the 
`GitHub Wiki <https://github.com/m0r13/mapcrafter/wiki/Example-maps>`_. 
Please feel free to add your own map to this list.

Features
========

* **Web output**: Render your Minecraft Worlds to maps with an isometric 3D perspective viewable in any webbrowser!
* **Different directions**: Choose from four different directions to render your worlds!
* **Different rendermodes**: Choose between different rendermodes like day, night and cave for your maps!
* **Configuration files**: Control which worlds are rendered from which direction and with which rendermodes!
* **Markers**: Define your own markers for your maps!
* **Other stuff**: Biome colors, incremental rendering, multithreading

Help
====

Read :doc:`getting_started` to get a first insight how to use the renderer.
You can find a detailed documentation about the renderer in
:doc:`configuration` and :doc:`commandline`.

If you find bugs or problems when using mapcrafter or if you have ideas
for new features, then please feel free to add an issue to the 
`GitHub issue tracker <https://github.com/m0r13/mapcrafter/issues>`_.

You can contact me in IRC (#mapcrafter on Freenode). Use the 
`webclient <http://webchat.freenode.net/?channels=mapcrafter>`_ if you
are new to IRC. I will be there most of the time, but please bear in mind
that I can't be available all the time. If I'm not there, wait some time or 
try another time of the day.

Documentation Contents
======================

.. toctree::
   :maxdepth: 2
   
   getting_started
   configuration
   commandline
   markers

Requirements
============

* A Linux-based or Mac operating system would be good, 
  building the renderer on Windows is possible but not easy.
* A C++ compiler (preferable gcc, minimum gcc 4.4), CMake and make to build mapcrafter.
* Some libraries:
	* libpng
	* libpthread
	* libboost-iostreams
	* libboost-system
	* libboost-filesystem
	* libboost-program-options
	* (libboost-test if you want to use the tests)
* For the Minecraft Worlds:
	* Anvil World Format
	* Minecraft 1.6 Resource Packs

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

