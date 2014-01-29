#!/bin/sh

python2 tools/gen_texture_code.py --header data/textures/blocks > renderer/blocktextures.h
python2 tools/gen_texture_code.py --source data/textures/blocks > renderer/blocktextures.cpp
