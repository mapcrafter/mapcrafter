============
Installation
============

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

Building from Source
====================

At first you have to get the source code of mapcrafter. 
Clone it directly from GitHub if you want the newest version::

    git clone https://github.com/m0r13/mapcrafter.git

Make sure you have all requirements installed. If your are on a Debian-like
Linux system, you can install these packages with apt::

    sudo apt-get install libpng12-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev

Then you can go into the directory of the mapcrafter source code and build
mapcrafter with the following commands::

    cmake .
    make

If everything works, you should have an executable file ``mapcrafter`` in 
the ``src/`` directory.

You can now install mapcrafter system-wide for all users if you want::

    sudo make install

Make sure that you still have to install the texture files needed for mapcrafter.

Debian Packages
===============

Textures
========
