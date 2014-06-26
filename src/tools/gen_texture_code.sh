#!/bin/sh

python tools/gen_texture_code.py --header data/textures/blocks > mapcraftercore/renderer/blocktextures.h
python tools/gen_texture_code.py --source data/textures/blocks > mapcraftercore/renderer/blocktextures.cpp
