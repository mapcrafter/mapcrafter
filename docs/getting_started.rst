===============
Getting Started
===============

Compiling
=========

At first you have to build mapcrafter from source. Make sure that you
have all requirements installed.

If you are on a Debian-like Linux system, you can install these packages with
``apt``::

    sudo apt-get install libpng12-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev

Then you can go into the ``src`` directory and run make::

    make

If everything goes well, you should have an executable file ``mapcrafter``.

Textures 
========

Now you need to get your Minecraft Textures. You need the following files:

* chest.png * enderchest.png * largechest.png * foliagecolor.png *
  grasscolor.png * the blocks/ directory from your texture pack

You can get those files from your ``minecraft.jar`` file (default textures) or
from another texture pack. You should put those files in the
``src/data/textures`` directory. There are already ``endportal.png`` and
``fire.png``.

To extract the textures from the ``minecraft.jar`` there is a python script
``find_images.py`` in the ``src/data`` directory. Just go into the
``src/data/textures`` directory or in your user-defined textures directory and
run::

    python find_images.py /path/to/my/minecraft.jar

Your first render 
=================

At first you have to create a configuration file::

    output_dir = output
	template_dir = data/template
	textures_dir = data/textures
	
	[world]
	name = My World
	world = world
	
In the configuration file you define which worlds the renderer should render.
In this example is defined that the renderer should render the world in the
directory ``world`` into the output directory ``output``. Directories to the
template and textures are also defined.  All relative paths in this
configuration file are relative to the path of the configuration file.

Now it's time to render your first world::

    ./mapcrafter -c myworld.conf

To improve the performance you can also render the map with multiple threads::

    ./mapcrafter -c myworld.conf -j 2

2 is the number of threads. You should use the count of your CPU cores, but
often the input/output is the bottleneck that using more threads is not useful.

For more information about rendering maps, see :doc:`configuration` and
:doc:`commandline`.
