# Mapcrafter [![Build Status](https://travis-ci.org/mapcrafter/mapcrafter.svg?branch=master)](https://travis-ci.org/mapcrafter/mapcrafter)

by Moritz Hilscher

Mapcrafter is a high performance Minecraft map renderer written in C++. It renders
Minecraft worlds to a bunch of tiles (images) which are viewable as a map in any browser
using Leaflet.js.

<p align="center">
  <img src="https://i.imgur.com/WyKXrgO.png" alt="Example map">
</p>

<!-- TOC -->

- [Features](#features)
- [Requirements](#requirements)
- [Example maps](#example-maps)
- [Help](#help)
  - [Documentation](#documentation)
  - [Reporting bugs and feature requests](#reporting-bugs-and-feature-requests)
  - [Community help](#community-help)
- [Acknowledgements](#acknowledgements)
- [License](#license)
- [Version history](#version-history)

<!-- /TOC -->

## Supported Minecraft versions

The current version supports Minecraft versions 1.13 through 1.16. Newer versions may partially work.
For Minecraft 1.12 (or older) please use [Mapcrafter 2.4](https://github.com/mapcrafter/mapcrafter/tree/v.2.4).

## Features

- **Web output:** Render your Minecraft worlds to maps viewable in any web browser!
- **Different render views:** Choose between different perspectives to render your world
  from! A 2D topdown and a 3D isometric render view are available!
- **Different rotations:** Choose from four different rotations to render your worlds from!
- **Different render modes:** Choose between different render modes like day, night and cave
  for your maps!
- **Different overlays:** Show additional information on your map! For example: Where can
  slimes spawn? Where can monsters spawn at night?
- **Configuration files:** Highly-customizable which worlds are rendered with which render
  view and other render parameters!
- **Markers:** Automatically generated markers from your Minecraft world data!
- **Other stuff:** Biome colors, incremental rendering, multithreading

## Requirements

- A Linux-based or Unix-like operating system like Mac OS
- A decent C++ compiler (preferable `gcc` >= 4.4, or `clang`), CMake and make if you want
  to build Mapcrafter from source
- Some libraries:
  - `libpng`
  - `libjpeg` (but you may use libjpeg-turbo as drop in replacement)
  - `libboost-iostreams`
  - `libboost-system`
  - `libboost-filesystem` (>= 1.42)
  - `libboost-program-options`
  - `libboost-test` (if you want to use the tests)
- For your Minecraft worlds:
  - Anvil world format
  - Minecraft resource packs

## Example maps

There are a few example maps of the renderer on the
[GitHub Wiki](https://github.com/mapcrafter/mapcrafter/wiki/Example-maps).
Please feel free to add your own maps to this list.

## Help

### Documentation

You can read the current documentation on [docs.mapcrafter.org](http://docs.mapcrafter.org)
and you can also [download other builds](https://readthedocs.org/projects/mapcrafter/downloads/)
from there. The documentation source is in the `docs/` directory and you can build it
yourself with Sphinx.

### Reporting bugs and feature requests

If you find bugs or problems when using Mapcrafter or if you have ideas for new
features, then please feel free to add an issue to the [GitHub issue
tracker](https://github.com/mapcrafter/mapcrafter/issues).

### Community help

[Join us on Discord](https://discord.gg/QNe8jXT), where you can meet other Mapcrafter users
and ask for help. Please be respectful with everyone and have patience with people who are
offering their time to help!

## Acknowledgements

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code
were very helpful. I also used the alpha blending code of pigmap and some maps
stuff of the template from Minecraft Overviewer.

## License

Mapcrafter is free software and available under the GPL license.  You can
access the latest source code of Mapcrafter on GitHub:
http://github.com/mapcrafter/mapcrafter

## Version history

See the [CHANGELOG](./CHANGELOG.md) for details.
