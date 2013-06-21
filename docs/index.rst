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

mapcrafter is free software and available under the GPL license.  You can
access the latest source code of mapcrafter on github:
http://github.com/m0r13/mapcrafter

Features
========

* Rendering Minecraft Worlds to maps viewable in any webbrowser
* Configuration files to control the renderer
* Four different directions to render your worlds from with an isometric 3D view
* Biome colors
* Incremental rendering
* Multithreading

Requirements
============

* A Linux-based or Mac operating system would be good, 
  building the renderer on Windows is possible but not easy.
* A C++ compiler and ``make`` to build mapcrafter.
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

