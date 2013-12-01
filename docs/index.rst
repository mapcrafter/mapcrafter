==========
mapcrafter
==========

Welcome
=======

mapcrafter is a fast Minecraft Map Renderer written in C++. It renders
Minecraft Worlds to a bunch of images which are viewable using Leaflet like a
big map in any webbrowser.

It runs on Linux and other Unix-like operating systems like Mac OS.  The
renderer works with the Anvil World format and the Minecraft 1.6 Resource
Packs.

mapcrafter is free software and available under the GPL license. You can access
the latest source code of mapcrafter on GitHub:
https://github.com/m0r13/mapcrafter

There are a few example maps of the renderer on the `GitHub Wiki
<https://github.com/m0r13/mapcrafter/wiki/Example-maps>`_.  Please feel free to
add your own map to this list.

.. image:: mapcrafter.png
	:align: center
	:width: 700px


Features
========

* **Web output**: Render your Minecraft Worlds to maps with an isometric 
  3D perspective viewable in any webbrowser!
* **Different directions**: Choose from four different directions to 
  render your worlds!
* **Different rendermodes**: Choose between different rendermodes like 
  day, night and cave for your maps!
* **Configuration files**: Control which worlds are rendered from which 
  direction and with which rendermodes!
* **Markers**: Define your own markers for your maps!
* **Other stuff**: Biome colors, incremental rendering, multithreading

Help
====

Read :doc:`using_mapcrafter` to get a first insight how to use the renderer.
You can find a detailed documentation about the render configuration file
format in :doc:`configuration`.

If you find bugs or problems when using mapcrafter or if you have ideas for new
features, then please feel free to add an issue to the `GitHub issue tracker
<https://github.com/m0r13/mapcrafter/issues>`_.

You can contact me in IRC (#mapcrafter on Freenode). Use the `webclient
<http://webchat.freenode.net/?channels=mapcrafter>`_ if you are new to IRC. I
will be there most of the time, but please bear in mind that I can't be
available all the time. If I'm not there wait some time or try another time of
the day.

Documentation Contents
======================

.. toctree::
   :maxdepth: 3
   
   installation
   using_mapcrafter
   configuration
   markers

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

