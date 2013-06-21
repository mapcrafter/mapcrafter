==========
mapcrafter
==========

.. image:: mapcrafter.png
	:align: center
	:height: 500px

Welcome
=======

mapcrafter is a fast Minecraft World Renderer written in C++. It renders
Minecraft Worlds to a bunch of images, which are viewable in any webbrowser
using the Google Maps API.

mapcrafter is not yet finished. At the moment a few blocks are not supported, 
but the basic rendering routines are implemented. The renderer works with 
the Anvil World format and the Minecraft 1.5 Texture Packs.

mapcrafter is free software and available under the GPL license. You can
access the latest source code of mapcrafter on GitHub:
http://github.com/m0r13/mapcrafter

There are a few example maps of the renderer on the 
`GitHub Wiki <https://github.com/m0r13/mapcrafter/wiki/Example-maps>`_. 
Please feel free to add your own map to this list.

Features
========

* Rendering Minecraft Worlds to maps viewable in any webbrowser
* Configuration files to control the renderer
* Four different directions to render your worlds from with an isometric 3D view
* Biome colors
* Incremental rendering
* Multithreading
* User-defined markers on your maps

Requirements
============

* A Linux-based or Mac operating system would be good, 
  building the renderer on Windows is possible but not easy.
* A C++ compiler (preferable gcc) and make to build mapcrafter.
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
	* Minecraft 1.5 Texture Packs

Help
====

Read :doc:`getting_started` to get a first insight how to use the renderer.
You can find a detailed documentation about the renderer in
:doc:`configuration` and :doc:`commandline`.

If you find bugs or problems when using mapcrafter or if you have ideas
for new features, then please feel free to add an issue to the 
`GitHub issue tracker <https://github.com/m0r13/mapcrafter/issues>`_.

Contents
========

.. toctree::
   :maxdepth: 2
   
   getting_started
   configuration
   commandline
   markers

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

