============
Installation
============

Requirements
============

* A Linux-based or Mac operating system would be good, 
  building the renderer on Windows is possible but not easy.
* A decent C++ compiler (preferable gcc >= 4.4, or clang), CMake and make to build Mapcrafter.
* Some libraries:

  * libpng
  * libjpeg (but you should use libjpeg-turbo as drop in replacement)
  * libboost-iostreams
  * libboost-system
  * libboost-filesystem (>= 1.42)
  * libboost-program-options
  * (libboost-test if you want to use the tests)
* For your Minecraft worlds:

  * Anvil world format
  * Minecraft 1.6 resource packs

Building from Source
====================

General Instructions
--------------------

At first you have to get the source code of Mapcrafter.  Clone it directly from
GitHub if you want the newest version::

    git clone https://github.com/m0r13/mapcrafter.git

Make sure you have all requirements installed. If you are on a Debian-like
Linux system, you can install these packages with apt::

    sudo apt-get install libpng-dev libjpeg-dev libboost-iostreams-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev

Then you can go into the directory with the Mapcrafter source (for example
``mapcrafter/``, not ``mapcrafter/src/``) and build it with the following
commands::

    cmake .
    make

If everything works, you should have an executable file ``mapcrafter`` in the
``src/`` directory.

You can now install Mapcrafter system-wide for all users if you want::

    sudo make install

If you get an error concerning ``libmapcraftercore.so`` not found, you have to run 
``ldconfig`` (as root).

Don't forget that you still have to install the texture files needed for Mapcrafter.
If you install the texture files to ``src/data/textures``, they will be copied
to a path Mapcrafter will automatically detect when installing Mapcrafter with ``make install``.

FreeBSD 10
----------

Mapcrafter builds fine on FreeBSD 10, 9 is not tested but could also build there.

For this guide we will be using ports, but could work with packages from pkgng (untested).

First step is to install prerequisites::

    cd /usr/ports/devel/git
    make install clean; rehash
    cd /usr/ports/devel/boost-all
    make install clean; rehash
    cd /usr/ports/devel/cmake
    make install clean; rehash
    cd /usr/ports/misc/compat8x
    make install clean; rehash
    cd /usr/ports/graphics/png
    make install clean; rehash

Or if you got portmaster installed::

    portmaster devel/git devel/boost-all devel/cmake misc/compat8x graphics/png

Once this is done compiling (takes a long time), you can go ahead with the normal steps::

    git clone https://github.com/m0r13/mapcrafter.git
    cd mapcrafter
    cmake .
    make

Mac OS X
--------

Currently there are no pre built packages available for Mac OS X but building it is relatively simple.

Prerequisites:

* `Xcode <https://itunes.apple.com/us/app/xcode/id497799835?ls=1&mt=12>`_ 
* `Homebrew <http://brew.sh/>`_ or `Macports <http://www.macports.org/>`_

Depending on your version of OS X you may or may not have git installed. 
Starting from 10.9 Mavericks git is installed with Xcode, if you got 10.8 Mountain Lion or older, 
you must install command line tools from Xcode and run the following command::

    brew install git

On 10.9 Mavericks systems you will have to run the following command after you've installed Xcode::

    xcode-select --install

and select install in the window that pops up, and accept the EULA.

First you will have to clone the latest Mapcrafter source by running::

    git clone https://github.com/m0r13/mapcrafter.git

After this, install the dependencies using brew::

    brew install boost libpng cmake libjpeg-turbo

Or install the dependencies using port::

    port install boost libpng cmake libjpeg-turbo

Once you have run this, you should have a working build system for Mapcrafter::

    cd mapcrafter
    cmake .
    make

This will build Mapcrafter and put the ready to use binary in the ``src/`` directory.

**Note**: With homebrew you will have to run the following CMake command::

	cmake . -DJPEG_INCLUDE_DIR=/usr/local/opt/jpeg-turbo/include/ -DJPEG_LIBRARY=/usr/local/opt/jpeg-turbo/lib/libjpeg.dylib


Arch Linux
==========

If you are running Arch Linux as operating system, you can install Mapcrafter
from the `AUR <https://aur.archlinux.org/packages/mapcrafter-git/>`_. 

Debian Packages
===============

If you are running Debian, Ubuntu or another Debian-like operating system you
can use the already built Mapcrafter Debian packages.  This is the recommended
practice though you can easily upgrade Mapcrafter everytime to the newest
stable version.

Create the file ``/etc/apt/sources.list.d/mapcrafter.list`` with the 
following content::

    deb http://mapcrafter.org/debian ./

Run ``sudo apt-get update`` to update your sources. If you get an error
concerning GPG keys, you have to import the GPG key of the Mapcrafter project
manually::

    gpg --keyserver pgp.mit.edu --recv 0xb6f77e28fe4f4eac && gpg --export --armor 0xb6f77e28fe4f4eac | sudo apt-key add -

Now you can run ``sudo apt-get install mapcrafter`` to install Mapcrafter.
During this process it will automatically download a temporary Minecraft Jar
file and unpack required texture files.

.. _resources_textures:

Resources and Textures
======================

.. note::

    You don't need to install the Minecraft texture files manually if you
    installed Mapcrafter from the AUR or with the Debian package.

Mapcrafter needs some resources to render maps: Minecraft texture files and
some template files for the web output.

There are different directories Mapcrafter searches these files:

1. ``$HOME/.mapcrafter``
2. ``$PREFIX/share/mapcrafter``
3. ``$MAPCRAFTER/data``

``$HOME`` is your home directory (usually ``/home/<username>``).  ``$PREFIX``
is the directory where Mapcrafter is installed (mostly ``/usr`` or
``/usr/local``, if installed via Debian package or ``make install``).
``$MAPCRAFTER`` is the directory of the Mapcrafter executable. The third path
is used if you built Mapcrafter from source and run it directly without
installing.

The template and texture files in these resource directories are expected by
the renderer in ``template/``, the texture files in ``textures/``.

You can get the paths to the resource directories of Mapcrafter by running
``mapcrafter --find-resources``. For example, when I installed the Debian
package::

    $ mapcrafter --find-resources
    Your home directory: /home/moritz
    mapcrafter binary: /usr/bin/mapcrafter
    Resource directories:
      1. /home/moritz/.mapcrafter
      2. /usr/share/mapcrafter
    Template directories:
      1. /usr/share/mapcrafter/template
    Texture directories:
      1. /usr/share/mapcrafter/textures

You can see that Mapcrafter found a resource directory in the home directory
but no ``template/`` or ``textures/`` directory in it. So it's just using the
template and texture directories in ``/usr/share/mapcrafter``. The numbers in
front of the paths are the order Mapcrafter is using these directories.  If you
want to overwrite the default textures, you can just create a new texture
directory ``.mapcrafter/textures`` in your home directory.

Now you have to install the Minecraft texture files. You need the following
files in your texture directory:

* directory ``chest/`` with normal.png, normal_double.png and ender.png 
* directory ``colormap/`` with foliage.png and grass.png
* directory ``blocks/`` from your texture pack
* endportal.png

You can get those files from your Minecraft Jar file (default textures) or from
another resource pack. To extract these texture files there is a python script
``mapcrafter_textures.py`` (``src/tools/mapcrafter_textures.py`` in the
Mapcrafter source if you didn't install Mapcrafter on your system). Run the
python script with the Minecraft Jar file and the texture directory as
arguments::

    mapcrafter_textures.py /path/to/my/minecraft/jar/1.7.2.jar /my/texture/directory

You will probably find your Minecraft Jar file in
``~/.minecraft/versions/%version%/%version%.jar``.
