/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "blockimages.h"

#include "../../biomes.h"
#include "../../../util.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace renderer {

FaceIterator::FaceIterator()
		: is_end(false), size(16), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::FaceIterator(int size)
		: is_end(false), size(size), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::~FaceIterator() {
}

void FaceIterator::next() {
	if(size == 0) {
		is_end = true;
		return;
	}
	// just iterate over the source pixels
	if (src_x == size - 1 && src_y == size - 1) {
		// set end if we are on bottom right
		is_end = true;
	} else if (src_y == size - 1) {
		// go one to the right (x+1) if we reached the bottom edge of the texture
		src_x++;
		src_y = 0;
	} else {
		// else just go one pixel forward
		src_y++;
	}
}

bool FaceIterator::end() const {
	return is_end;
}

SideFaceIterator::SideFaceIterator()
		: FaceIterator(), side(0), delta(0) {
}

SideFaceIterator::SideFaceIterator(int size, int side)
		: FaceIterator(size), side(side) {
	if (side == RIGHT) {
		delta = size / 2;
		dest_y = delta;
	} else if (side == LEFT) {
		delta = 0;
		dest_y = 0;
	}
}

void SideFaceIterator::next() {
	FaceIterator::next();
	if (src_y == 0 && src_x % 2 == 1) {
		delta += side;
	}
	dest_x = src_x;
	dest_y = src_y + delta;
}

TopFaceIterator::TopFaceIterator()
		: FaceIterator(), next_x(-1), next_y(-1) {
}

TopFaceIterator::TopFaceIterator(int size)
		: FaceIterator(size), next_x(-1), next_y(-1) {
	dest_x = size - 1;
}

void TopFaceIterator::next() {
	FaceIterator::next();
	if (src_y == 0) {
		dest_x = src_x + size - 1;
		dest_y = src_x / 2;
		next_x = dest_x - (src_x % 2 == 0 ? 1 : 0);
		next_y = dest_y + 1;
	} else if (next_x != -1 && next_y != -1) {
		dest_x = next_x;
		dest_y = next_y;
		next_x = -1;
		next_y = -1;
	} else if (src_x % 2 == 0 && src_y == size - 1) {
		dest_y += 1;
	} else {
		if (src_y % 2 == 1) {
			dest_y += 1;
		}
		dest_x -= 1;
	}
}

/**
 * Blits a face on a block image.
 */
void blitFace(RGBAImage& image, int face, const RGBAImage& texture, int xoff, int yoff,
		bool darken, double darken_left, double darken_right) {
	double d = 1;
	if (darken) {
		if (face == FACE_SOUTH || face == FACE_NORTH)
			d = darken_left;
		else if (face == FACE_WEST || face == FACE_EAST)
			d = darken_right;
	}

	int xsize = texture.getWidth();
	int ysize = texture.getHeight();
	int size = std::max(xsize, ysize);

	if (face == FACE_BOTTOM || face == FACE_TOP) {
		if (face == FACE_BOTTOM)
			yoff += ysize;
		for (TopFaceIterator it(size); !it.end(); it.next()) {
			uint32_t pixel = texture.getPixel(it.src_x, it.src_y);
			image.blendPixel(rgba_multiply(pixel, d, d, d), it.dest_x + xoff,
					it.dest_y + yoff);
		}
	} else {
		int itside = SideFaceIterator::LEFT;
		if (face == FACE_NORTH || face == FACE_SOUTH)
			itside = SideFaceIterator::RIGHT;

		if (face == FACE_EAST || face == FACE_SOUTH)
			xoff += xsize;
		if (face == FACE_WEST || face == FACE_SOUTH)
			yoff += ysize / 2;
		for (SideFaceIterator it(size, itside); !it.end(); it.next()) {
			uint32_t pixel = texture.getPixel(it.src_x, it.src_y);
			image.blendPixel(rgba_multiply(pixel, d, d, d), it.dest_x + xoff,
					it.dest_y + yoff);
		}
	}
}

/**
 * Blits the two faces (like a cross from top) to make an item-style block.
 */
void blitItemStyleBlock(RGBAImage& image, const RGBAImage& north_south, const RGBAImage& east_west) {
	int size = std::max(north_south.getWidth(), east_west.getWidth());
	SideFaceIterator it(size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		if (it.src_x > size / 2) {
			uint32_t pixel = east_west.getPixel(it.src_x, it.src_y);
			image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
		}
	}
	it = SideFaceIterator(size, SideFaceIterator::LEFT);
	for (; !it.end(); it.next()) {
		uint32_t pixel = north_south.getPixel(it.src_x, it.src_y);
		image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
	}
	it = SideFaceIterator(size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		if (it.src_x <= size / 2) {
			uint32_t pixel = east_west.getPixel(it.src_x, it.src_y);
			image.blendPixel(pixel, size / 2 + it.dest_x, size / 4 + it.dest_y);
		}
	}
}

/**
 * This function rotates the north-, south-, east- and west-oriented block images
 * by swapping their images. Used for special block images: stairs, rails
 */
void rotateImages(RGBAImage& north, RGBAImage& south, RGBAImage& east, RGBAImage& west, int rotation) {
	std::map<int, RGBAImage> images;
	images[util::rotateShiftRight(FACE_NORTH, rotation, 4)] = north;
	images[util::rotateShiftRight(FACE_SOUTH, rotation, 4)] = south;
	images[util::rotateShiftRight(FACE_EAST, rotation, 4)] = east;
	images[util::rotateShiftRight(FACE_WEST, rotation, 4)] = west;

	north = images[FACE_NORTH];
	south = images[FACE_SOUTH];
	east = images[FACE_EAST];
	west = images[FACE_WEST];
}

BlockImage::BlockImage(int type)
		: type(type) {
	for (int i = 0; i < 6; i++) {
		x_offsets[i] = 0;
		y_offsets[i] = 0;
	}
}

BlockImage::~BlockImage() {
}

/**
 * Sets a face of a block image. You can use this method also to set more than one face
 * to the same texture.
 */
BlockImage& BlockImage::setFace(int face, const RGBAImage& texture, int xoff, int yoff) {
	for (int i = 0; i < 6; i++)
		if (face & (1 << i)) {
			faces[i] = texture;
			x_offsets[i] = xoff;
			y_offsets[i] = yoff;
		}
	return *this;
}

/**
 * Returns the texture of a face.
 */
const RGBAImage& BlockImage::getFace(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return faces[i];
	return empty_image;
}

int BlockImage::getXOffset(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return x_offsets[i];
	return 0;
}

int BlockImage::getYOffset(int face) const {
	for (int i = 0; i < 6; i++)
		if (face == (1 << i))
			return y_offsets[i];
	return 0;
}

/**
 * Returns this block count*90 degrees rotated.
 */
BlockImage BlockImage::rotate(int count) const {
	count = count % 4;
	if (count == 0)
		return BlockImage(*this);

	BlockImage rotated(type);
	for (int i = 0; i < 4; i++) {
		int face = 1 << i;
		int new_face = util::rotateShiftLeft(face, count, 4);
		RGBAImage texture = getFace(face);
		// we have to flip the texture if it is moved from/to south|west to/from north|east
		if (((face == FACE_SOUTH || face == FACE_WEST) && (new_face == FACE_NORTH || new_face == FACE_EAST))
				|| ((new_face == FACE_SOUTH || new_face == FACE_WEST) && (face == FACE_NORTH || face == FACE_EAST)))
			texture = texture.flip(true, false);
		rotated.setFace(new_face, texture, getXOffset(face), getYOffset(face));
	}

	RGBAImage top = getFace(FACE_TOP).rotate(count);
	RGBAImage bottom = getFace(FACE_BOTTOM).rotate(count);
	rotated.setFace(FACE_TOP, top, getXOffset(FACE_TOP), getYOffset(FACE_TOP));
	rotated.setFace(FACE_BOTTOM, bottom, getXOffset(FACE_BOTTOM), getYOffset(FACE_BOTTOM));
	return rotated;
}

/**
 * Creates the block image from the textures.
 */
RGBAImage BlockImage::buildImage(double dleft, double dright) const {
	RGBAImage image;

	int size = 0;
	for(int i = 0; i < 6; i++)
		size = std::max(size, faces[i].getWidth());
	image.setSize(size * 2, size * 2);

	if (type == NORMAL) {
		int order[] = {FACE_BOTTOM, FACE_NORTH, FACE_EAST, FACE_WEST, FACE_SOUTH, FACE_TOP};

		for(int i = 0; i < 6; i++) {
			int face = order[i];
			int xoff = getXOffset(face), yoff = getYOffset(face);
			if (face == FACE_NORTH || face == FACE_EAST)
				blitFace(image, face, getFace(face).flip(true, false),
						xoff, yoff, true, dleft, dright);
			else
				blitFace(image, face, getFace(face),
						xoff, yoff, true, dleft, dright);
		}
	} else if (type == ITEM_STYLE) {
		blitItemStyleBlock(image, getFace(FACE_NORTH), getFace(FACE_EAST));
	}

	return image;
}

IsometricBlockImages::IsometricBlockImages()
	: AbstractBlockImages(),
	  dleft(1), dright(1) {
}

IsometricBlockImages::~IsometricBlockImages() {
}

void IsometricBlockImages::setBlockSideDarkening(double left, double right) {
	this->dleft = left;
	this->dright = right;
}

bool IsometricBlockImages::isBlockTransparent(uint16_t id, uint16_t data) const {
	return AbstractBlockImages::isBlockTransparent(id, data & ~(EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM));
}

RGBAImage IsometricBlockImages::getBiomeBlock(uint16_t id, uint16_t data,
		uint16_t extra_data, const Biome& biome) const {
	// return normal block for the snowy grass block
	if (id == 2 && (data & GRASS_SNOW))
		return getBlock(id, data, extra_data);
	return AbstractBlockImages::getBiomeBlock(id, data, biome, extra_data);
}

int IsometricBlockImages::getBlockSize() const {
	return texture_size * 2;
}

/**
 * This method filters unnecessary block data, for example the leaves decay counter.
 */
uint16_t IsometricBlockImages::filterBlockData(uint16_t id, uint16_t data) const {
	// call super method
	data = AbstractBlockImages::filterBlockData(id, data);

	if (id == 6)
		return data & (0xff00 | util::binary<11>::value);
	else if (id == 8 || id == 9) { // water
		// 0x8 bit means that this is a water block spreading downwards
		// -> return with Minecraft data 0 (full block)
		if (data & 0x8)
			data &= ~0x7;
		return data & (0xff00 | util::binary<11110111>::value);
	} else if (id == 10 || id == 11) { // lava
		// 0x8 bit means that this is a lava block spreading downwards
		// -> return data 0 (full block)
		if (data & 0x8)
			return 0;
		return data;
	} else if (id == 18 || id == 161) // leaves
		return data & (0xff00 | util::binary<11>::value);
	else if (id == 26) // bed
		return data & (0xff00 | util::binary<1011>::value);
	else if (id == 51) // fire
		return 0;
	else if (id == 53 || id == 67 || id == 108 || id == 109 || id == 114 || id == 128 || id == 134 || id == 135 || id == 136 || id == 156 || id == 163 || id == 164 || id == 180 || id == 203) {
		// stairs: ignore lower two bits
		return data & ~0x3;
	} else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 63) // signs
		return 0;
	else if (id == 64 || id == 71 || (id >= 193 && id <= 197)) // doors
		return data & util::binary<1111110000>::value;
	else if (id == 81 || id == 83 || id == 92) // cactus, sugar cane, cake
		return data & 0xff00;
	else if (id == 84) // jukebox
		return 0;
	else if (id == 90) // nether portal
		return 0;
	else if (id == 93 || id == 94) // redstone repeater
		return data & (0xff00 | util::binary<11>::value);
	else if (id == 117) // brewing stand
		return data & 0xff00;
	else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	else if (id == 127)
		return data & util::binary<1100>::value;
	else if (id == 131) // trip wire hook
		return data & util::binary<11>::value;
	else if (id == 132) // trip wire
		return data & ~0xf;
	else if (id == 149 || id == 150) // comparator // TODO!
		return data & ~(4 | 8);
	// the light sensor shouldn't have any data, but I had problems with it...
	else if (id == 151 || id == 178)
		return 0;
	else if (id == 154) // hopper
		return 0;
	else if (id == 199) // chrous plant
		return 0;
	return data;
}

/**
 * Checks, if a block images has transparent pixels.
 */
bool IsometricBlockImages::isImageTransparent(const RGBAImage& image) const {
	// TODO
	// Find out why blocks on Windows contain slightly transparency (>250)
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end(); it.next()) {
		if (rgba_alpha(image.getPixel(it.dest_x, it.dest_y + texture_size/2)) < 255)
			return true;
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end(); it.next()) {
		if (rgba_alpha(image.getPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2)) < 255)
			return true;
	}
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		if (rgba_alpha(image.getPixel(it.dest_x, it.dest_y)) < 255)
			return true;
	}
	return false;
}

/**
 * This method adds to the block image the dark shadow edges by blitting the shadow edge
 * masks and then stores the block image with the special data.
 */
void IsometricBlockImages::addBlockShadowEdges(uint16_t id, uint16_t data, const RGBAImage& block) {
	for (int n = 0; n <= 1; n++)
		for (int e = 0; e <= 1; e++)
			for (int b = 0; b <= 1; b++) {
				RGBAImage image = block;
				uint16_t extra_data = 0;
				if (n) {
					image.alphaBlit(shadow_edge_masks[0], 0, 0);
					extra_data |= EDGE_NORTH;
				}
				if (e) {
					image.alphaBlit(shadow_edge_masks[1], 0, 0);
					extra_data |= EDGE_EAST;
				}
				if (b) {
					image.alphaBlit(shadow_edge_masks[2], 0, 0);
					extra_data |= EDGE_BOTTOM;
				}
				block_images[id | ((data | extra_data) << 16)] = image;
			}
}

/**
 * Sets a block image in the block image list (and rotates it if necessary (later)).
 */
void IsometricBlockImages::setBlockImage(uint16_t id, uint16_t data, const BlockImage& block) {
	setBlockImage(id, data, buildImage(block.rotate(rotation)));
}

void IsometricBlockImages::setBedImage(uint16_t data, uint16_t extra_data, const BlockImage& block) {
    setBedImage(data, extra_data, buildImage(block.rotate(rotation)));
}

void IsometricBlockImages::setBlockImage(uint16_t id, uint16_t data, const RGBAImage& block) {
	AbstractBlockImages::setBlockImage(id, data, block);

	// if block is not transparent, add shadow edges
	if (!isBlockTransparent(id, data))
		addBlockShadowEdges(id, data, block);
}

void IsometricBlockImages::setBedImage(uint16_t data, uint16_t extra_data, const RGBAImage& block) {
    AbstractBlockImages::setBedImage(data, extra_data, block);
}

uint32_t IsometricBlockImages::darkenLeft(uint32_t pixel) const {
	return rgba_multiply(pixel, dleft, dleft, dleft);
}

uint32_t IsometricBlockImages::darkenRight(uint32_t pixel) const {
	return rgba_multiply(pixel, dright, dright, dright);
}

RGBAImage IsometricBlockImages::buildImage(const BlockImage& image) const {
	return image.buildImage(dleft, dright);
}

BlockImage IsometricBlockImages::buildSmallerBlock(const RGBAImage& left_texture,
        const RGBAImage& right_texture, const RGBAImage& top_texture, int y1, int y2) const {
	RGBAImage left = left_texture;
	RGBAImage right = right_texture;
	left.fill(0, 0, 0, texture_size, texture_size - y2);
	right.fill(0, 0, 0, texture_size, texture_size - y2);

	left.fill(0, 0, texture_size - y1, texture_size, y1);
	right.fill(0, 0, texture_size - y1, texture_size, y1);

	BlockImage block;
	block.setFace(FACE_EAST | FACE_WEST, left);
	block.setFace(FACE_NORTH | FACE_SOUTH, right);
	block.setFace(FACE_TOP, top_texture, 0, texture_size - y2);
	return block;
}

void IsometricBlockImages::buildCustomTextures() {
	shadow_edge_masks[0].setSize(getBlockSize(), getBlockSize());
	shadow_edge_masks[1].setSize(getBlockSize(), getBlockSize());
	shadow_edge_masks[2].setSize(getBlockSize(), getBlockSize());

	uint32_t darkness = rgba(0, 0, 0, 64);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		if (it.src_x < 1)
			shadow_edge_masks[0].setPixel(it.dest_x, it.dest_y, darkness);
		if (it.src_y < 1)
			shadow_edge_masks[1].setPixel(it.dest_x, it.dest_y, darkness);
		if (it.src_x == texture_size - 1 || it.src_y == texture_size - 1)
			shadow_edge_masks[2].setPixel(it.dest_x, it.dest_y + texture_size, darkness);
	}
}

void IsometricBlockImages::createBlock(uint16_t id, uint16_t data, const RGBAImage& texture) {
	createBlock(id, data, texture, texture);
}

void IsometricBlockImages::createBlock(uint16_t id, uint16_t data, const RGBAImage& side_texture,
        const RGBAImage& top_texture) {
	createBlock(id, data, side_texture, side_texture, top_texture);
}

void IsometricBlockImages::createBlock(uint16_t id, uint16_t data, const RGBAImage& left_texture,
        const RGBAImage& right_texture, const RGBAImage& top_texture) {
	BlockImage block;
	block.setFace(FACE_EAST | FACE_WEST, left_texture);
	block.setFace(FACE_NORTH | FACE_SOUTH, right_texture);
	block.setFace(FACE_TOP, top_texture);
	setBlockImage(id, data, block);
}

void IsometricBlockImages::createSmallerBlock(uint16_t id, uint16_t data,
        const RGBAImage& left_texture, const RGBAImage& right_texture, const RGBAImage& top_texture,
        int y1, int y2) {
	setBlockImage(id, data,
	        buildSmallerBlock(left_texture, right_texture, top_texture, y1, y2));
}

void IsometricBlockImages::createSmallerBlock(uint16_t id, uint16_t data, const RGBAImage& side_face,
        const RGBAImage& top_texture, int y1, int y2) {
	createSmallerBlock(id, data, side_face, side_face, top_texture, y1, y2);
}

void IsometricBlockImages::createSmallerBlock(uint16_t id, uint16_t data, const RGBAImage& texture,
		int y1, int y2) {
	createSmallerBlock(id, data, texture, texture, texture, y1, y2);
}

void IsometricBlockImages::createGlazedTerracotta(uint16_t id, const RGBAImage& texture) {
    RGBAImage rotated_texture_0 = texture.rotate(ROTATE_0);
    RGBAImage rotated_texture_90 = texture.rotate(ROTATE_90);
    RGBAImage rotated_texture_180 = texture.rotate(ROTATE_180);
    RGBAImage rotated_texture_270 = texture.rotate(ROTATE_270);

    BlockImage block_0; // south
    block_0.setFace(FACE_TOP, rotated_texture_270);
    block_0.setFace(FACE_NORTH, rotated_texture_90);
    block_0.setFace(FACE_SOUTH, rotated_texture_270);
    block_0.setFace(FACE_EAST, rotated_texture_180);
    block_0.setFace(FACE_WEST, rotated_texture_0);
    setBlockImage(id, 0, block_0);

    BlockImage block_1;
    block_1.setFace(FACE_TOP, rotated_texture_0);
    block_1.setFace(FACE_NORTH, rotated_texture_0);
    block_1.setFace(FACE_SOUTH, rotated_texture_180);
    block_1.setFace(FACE_EAST, rotated_texture_90);
    block_1.setFace(FACE_WEST, rotated_texture_270);
    setBlockImage(id, 1, block_1);

    BlockImage block_2;
    block_2.setFace(FACE_TOP, rotated_texture_90);
    block_2.setFace(FACE_NORTH, rotated_texture_270);
    block_2.setFace(FACE_SOUTH, rotated_texture_90);
    block_2.setFace(FACE_EAST, rotated_texture_0);
    block_2.setFace(FACE_WEST, rotated_texture_180);
    setBlockImage(id, 2, block_2);

    BlockImage block_3;
    block_3.setFace(FACE_TOP, rotated_texture_180);
    block_3.setFace(FACE_NORTH, rotated_texture_180);
    block_3.setFace(FACE_SOUTH, rotated_texture_0);
    block_3.setFace(FACE_EAST, rotated_texture_270);
    block_3.setFace(FACE_WEST, rotated_texture_90);
    setBlockImage(id, 3, block_3);
}

void IsometricBlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data, const RGBAImage& texture) {
    createRotatedBlock(id, extra_data, texture, texture, texture, texture);
}

void IsometricBlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const RGBAImage& front_texture, const RGBAImage& side_texture,
        const RGBAImage& top_texture) {
	createRotatedBlock(id, extra_data, front_texture, side_texture, side_texture,
	        top_texture);
}

void IsometricBlockImages::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const RGBAImage& front_texture, const RGBAImage& back_texture, const RGBAImage& side_texture,
        const RGBAImage& top_texture) {
	BlockImage block;
	block.setFace(FACE_NORTH, front_texture.flip(true, false));
	block.setFace(FACE_SOUTH, back_texture);
	block.setFace(FACE_EAST, side_texture.flip(true, false));
	block.setFace(FACE_WEST, side_texture);
	block.setFace(FACE_TOP, top_texture);

	setBlockImage(id, 2 | extra_data, block);
	setBlockImage(id, 3 | extra_data, block.rotate(2));
	setBlockImage(id, 4 | extra_data, block.rotate(3));
	setBlockImage(id, 5 | extra_data, block.rotate(1));
}

void IsometricBlockImages::createItemStyleBlock(uint16_t id, uint16_t data,
        const RGBAImage& texture) {
	createItemStyleBlock(id, data, texture, texture);
}

void IsometricBlockImages::createItemStyleBlock(uint16_t id, uint16_t data,
        const RGBAImage& north_south, const RGBAImage& east_west) {
	BlockImage block(BlockImage::ITEM_STYLE);
	block.setFace(FACE_NORTH | FACE_SOUTH, north_south);
	block.setFace(FACE_EAST | FACE_WEST, east_west);
	setBlockImage(id, data, block);
}

void IsometricBlockImages::createSingleFaceBlock(uint16_t id, uint16_t data, int face,
        const RGBAImage& texture) {
	setBlockImage(id, data, BlockImage().setFace(face, texture));
}

void IsometricBlockImages::createGrassBlock() { // id 2
	const BlockTextures& textures = resources.getBlockTextures();
	RGBAImage dirt = textures.DIRT;

	RGBAImage grass = dirt;
	RGBAImage grass_mask = textures.GRASS_SIDE_OVERLAY;
	grass.alphaBlit(grass_mask, 0, 0);

	RGBAImage top = textures.GRASS_TOP;

	BlockImage block;
	block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, grass);
	block.setFace(FACE_TOP, top);
	setBlockImage(2, 0, block);

	// create the snowy version
	grass = textures.GRASS_SIDE_SNOWED;
	top = textures.SNOW;

	BlockImage block_snow;
	block_snow.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, grass);
	block_snow.setFace(FACE_TOP, top);
	setBlockImage(2, GRASS_SNOW, block_snow);
}

void IsometricBlockImages::createWater() { // id 8, 9
	RGBAImage water = resources.getBlockTextures().WATER_STILL.colorize(0, 0.39, 0.89);
	for (int data = 0; data < 8; data++) {
		int smaller = data / 8.0 * texture_size;
		RGBAImage side_texture = water.move(0, smaller);

		RGBAImage block(getBlockSize(), getBlockSize());
		blitFace(block, FACE_WEST, side_texture, 0, 0, true, dleft, dright);
		blitFace(block, FACE_SOUTH, side_texture, 0, 0, true, dleft, dright);
		blitFace(block, FACE_TOP, water, 0, smaller, true, dleft, dright);
		setBlockImage(8, data, block);
		setBlockImage(9, data, block);
	}
	for (size_t i = 0; i <= util::binary<111>::value; i++) {
		bool west = i & util::binary<100>::value;
		bool south = i & util::binary<10>::value;
		bool top = i & util::binary<1>::value;

		RGBAImage block(getBlockSize(), getBlockSize());
		uint16_t extra_data = FULL_WATER;

		if (top) {
			blitFace(block, FACE_TOP, water, 0, 0, true, dleft, dright);
			extra_data |= FULL_WATER_TOP;
		}

		if (west) {
			blitFace(block, FACE_WEST, water, 0, 0, true, dleft, dright);
			extra_data |= FULL_WATER_WEST;
		}

		if (south) {
			blitFace(block, FACE_SOUTH, water, 0, 0, true, dleft, dright);
			extra_data |= FULL_WATER_SOUTH;
		}

		if (extra_data != 0) {
			setBlockImage(8, extra_data, block);
			setBlockImage(9, extra_data, block);
		}
	}
}

void IsometricBlockImages::createLava() { // id 10, 11
	RGBAImage lava = resources.getBlockTextures().LAVA_STILL;
	for (int data = 0; data < 8; data++) {
		int smaller = data / 8.0 * texture_size;
		RGBAImage side_texture = lava.move(0, smaller);

		BlockImage block;
		block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, side_texture);
		block.setFace(FACE_TOP, lava, 0, smaller);
		setBlockImage(10, data, block);
		setBlockImage(11, data, block);
	}
}

void IsometricBlockImages::createWood(uint16_t id, uint16_t data, const RGBAImage& side, const RGBAImage& top) { // id 17
	createBlock(id, data | 4, top, side.rotate(1), side);
	createBlock(id, data | 8, side.rotate(1), top, side.rotate(1));
	createBlock(id, data, side, side, top); // old format
	createBlock(id, data | 4 | 8, side, side, top);
}

/**
 * Makes the transparent leaf textures opaque. It seems the opaque leaf textures are just
 * the transparent ones with all transparent pixels replaced by a specific gray.
 */
RGBAImage makeLeavesOpaque(const RGBAImage& texture, uint8_t color) {
	RGBAImage opaque = texture;
	opaque.fill(rgba(color, color, color), 0, 0, opaque.getWidth(), opaque.getHeight());
	opaque.simpleAlphaBlit(texture, 0, 0);
	return opaque;
}

void IsometricBlockImages::createLeaves() { // id 18
	const BlockTextures& textures = resources.getBlockTextures();
	if (render_leaves_transparent) {
		createBlock(18, 0, textures.LEAVES_OAK); // oak
		createBlock(18, 1, textures.LEAVES_SPRUCE); // pine/spruce
		createBlock(18, 2, textures.LEAVES_OAK); // birch
		createBlock(18, 3, textures.LEAVES_JUNGLE); // jungle

		createBlock(161, 0, textures.LEAVES_ACACIA); // acacia
		createBlock(161, 1, textures.LEAVES_BIG_OAK); // dark oak
	} else {
		// have to create the opaque leaf textures on our own...
		createBlock(18, 0, makeLeavesOpaque(textures.LEAVES_OAK, 0x2c)); // oak
		createBlock(18, 1, makeLeavesOpaque(textures.LEAVES_SPRUCE, 0x31)); // pine/spruce
		createBlock(18, 2, makeLeavesOpaque(textures.LEAVES_OAK, 0x2c)); // birch
		createBlock(18, 3, makeLeavesOpaque(textures.LEAVES_JUNGLE, 0x4b)); // jungle

		createBlock(161, 0, makeLeavesOpaque(textures.LEAVES_ACACIA, 0x2c)); // acacia
		createBlock(161, 1, makeLeavesOpaque(textures.LEAVES_BIG_OAK, 0x2c)); // dark oak
	}
}

void IsometricBlockImages::createGlass(uint16_t id, uint16_t data, const RGBAImage& texture) { // id 20, 95
	BlockImage block(BlockImage::NORMAL);
	block.setFace(FACE_SOUTH | FACE_WEST | FACE_TOP, texture);
	setBlockImage(id, data, block.buildImage(dleft, dright));
}

void IsometricBlockImages::createDispenserDropper(uint16_t id, const RGBAImage& front) { // id 23, 158
	RGBAImage side = resources.getBlockTextures().FURNACE_SIDE;
	RGBAImage top = resources.getBlockTextures().FURNACE_TOP;

	createRotatedBlock(id, 0, front, side, top);
	createBlock(id, 0, side, top);
	createBlock(id, 1, side, front);
}

void IsometricBlockImages::createObserver(uint16_t id) { // id 23, 158
    RGBAImage side = resources.getBlockTextures().OBSERVER_SIDE;
    RGBAImage top = resources.getBlockTextures().OBSERVER_TOP;
    RGBAImage front = resources.getBlockTextures().OBSERVER_FRONT;
    RGBAImage back = resources.getBlockTextures().OBSERVER_BACK;

    createRotatedBlock(id, 0, front, back, side, top.rotate(ROTATE_90));

    BlockImage up_image;
    up_image.setFace(FACE_NORTH, top);
    up_image.setFace(FACE_EAST, side.rotate(ROTATE_90));
    up_image.setFace(FACE_SOUTH, top);
    up_image.setFace(FACE_WEST, side.rotate(ROTATE_90));
    up_image.setFace(FACE_TOP, back.rotate(ROTATE_90));

    setBlockImage(id, 0, up_image);

    BlockImage down_image;
    down_image.setFace(FACE_NORTH, top.rotate(ROTATE_180));
    down_image.setFace(FACE_EAST, side.rotate(ROTATE_90));
    down_image.setFace(FACE_SOUTH, top.rotate(ROTATE_180));
    down_image.setFace(FACE_WEST, side.rotate(ROTATE_90));
    down_image.setFace(FACE_TOP, front.rotate(ROTATE_90));

    setBlockImage(id, 1, down_image);
}

BlockImage buildBed(const RGBAImage& top,
		const RGBAImage& side_north, const RGBAImage& side_south,
		const RGBAImage& side_east, const RGBAImage& side_west) {
	BlockImage block;

	block.setFace(FACE_TOP, top, 0, top.getHeight() / 16. * 7.0);
	block.setFace(FACE_NORTH, side_north);
	block.setFace(FACE_SOUTH, side_south);
	block.setFace(FACE_EAST, side_east);
	block.setFace(FACE_WEST, side_west);

	return block;
}

void IsometricBlockImages::createBed(const BedTextures& textures) { // id 26
	// TODO: Add images for each bed colour
	// TODO: Check if right texture is assigned to the right face

	for (uint16_t colour = 0; colour < 16; colour++) {
		int offset = colour * BedTextures::DATA_SIZE;
		RGBAImage top_foot = textures[offset + BedTextures::TOP_FOOT];
		RGBAImage top_head = textures[offset + BedTextures::TOP_HEAD];
		RGBAImage side_head_left = textures[offset + BedTextures::SIDE_HEAD_LEFT];
		RGBAImage side_head_right = textures[offset + BedTextures::SIDE_HEAD_RIGHT];
		RGBAImage side_foot_left = textures[offset + BedTextures::SIDE_FOOT_LEFT];
		RGBAImage side_foot_right = textures[offset + BedTextures::SIDE_FOOT_RIGHT];
		RGBAImage side_head_end = textures[offset + BedTextures::SIDE_HEAD_END];
		RGBAImage side_foot_end = textures[offset + BedTextures::SIDE_FOOT_END];

		// Head
        setBedImage(0 | 8, colour, buildBed(top_head.rotate(ROTATE_90), 0, side_head_end, side_head_left.flip(true, false), side_head_right)); // Pointing south
        setBedImage(1 | 8, colour, buildBed(top_head.rotate(ROTATE_180), side_head_right.flip(true, false), side_head_left, 0, side_head_end)); // Pointing west
		setBedImage(2 | 8, colour, buildBed(top_head.rotate(ROTATE_270), side_head_end, 0, side_head_right.flip(true, false), side_head_left)); // Pointing north
        setBedImage(3 | 8, colour, buildBed(top_head, side_head_right, side_head_left.flip(true, false), side_head_end, 0)); // Pointing east

		// Foot
		setBedImage(0, colour, buildBed(top_foot.rotate(ROTATE_90), side_foot_end, 0, side_foot_left.flip(true, false), side_foot_right)); // Pointing south
        setBedImage(1, colour, buildBed(top_foot.rotate(ROTATE_180), side_foot_right.flip(true, false), side_foot_left, side_foot_end, 0)); // Pointing west
        setBedImage(2, colour, buildBed(top_foot.rotate(ROTATE_270), 0, side_foot_end, side_foot_right.flip(true, false), side_foot_left)); // Pointing north
        setBedImage(3, colour, buildBed(top_foot, side_foot_right, side_foot_left.flip(true, false), 0, side_foot_end)); // Pointing east
	}
}

void IsometricBlockImages::createStraightRails(uint16_t id, uint16_t extra_data,
        const RGBAImage& texture) { // id 27, 28, 66
	createSingleFaceBlock(id, 0 | extra_data, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 1 | extra_data, FACE_BOTTOM, texture);

	RGBAImage rotated_texture = texture.rotate(ROTATE_90);
	RGBAImage north(getBlockSize(), getBlockSize()),
			south(getBlockSize(), getBlockSize()),
			east(getBlockSize(), getBlockSize()),
			west(getBlockSize(), getBlockSize());

	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		east.setPixel(it.dest_x, it.dest_y + it.src_y,
				texture.getPixel(it.src_x, it.src_y));
		east.setPixel(it.dest_x + 1, it.dest_y + it.src_y,
		        texture.getPixel(it.src_x, it.src_y));

		west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
		west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
	}

	north = east.flip(true, false);
	south = west.flip(true, false);

	rotateImages(north, south, east, west, rotation);

	setBlockImage(id, 2 | extra_data, east);
	setBlockImage(id, 3 | extra_data, west);
	setBlockImage(id, 4 | extra_data, north);
	setBlockImage(id, 5 | extra_data, south);
}

BlockImage buildPiston(int frontface, const RGBAImage& front, const RGBAImage& back,
		const RGBAImage& side, const RGBAImage& top) {
	BlockImage block;

	block.setFace(FACE_TOP, top);
	block.setFace(frontface, front);
	if(frontface == FACE_NORTH || frontface == FACE_SOUTH) {
		block.setFace(FACE_EAST, side.flip(true, false));
		block.setFace(FACE_WEST, side);
	} else {
		block.setFace(FACE_NORTH, side.flip(true, false));
		block.setFace(FACE_SOUTH, side);
	}

	if(frontface == FACE_NORTH)
		block.setFace(FACE_SOUTH, back);
	else if(frontface == FACE_SOUTH)
		block.setFace(FACE_NORTH, back);
	else if(frontface == FACE_EAST)
		block.setFace(FACE_WEST, back);
	else
		block.setFace(FACE_EAST, back);

	return block;
}

void IsometricBlockImages::createPiston(uint16_t id, bool sticky) { //  id 29, 33
	const BlockTextures& textures = resources.getBlockTextures();
	RGBAImage front = sticky ? textures.PISTON_TOP_STICKY : textures.PISTON_TOP_NORMAL;
	RGBAImage side = textures.PISTON_SIDE;
	RGBAImage back = textures.PISTON_BOTTOM;

	createBlock(id, 0, side.rotate(ROTATE_180), back);
	createBlock(id, 1, side, front);

	// TODO: Bug fix rotations
	setBlockImage(id, 2, buildPiston(FACE_NORTH, front, back, side.rotate(3), side.rotate(3)));
	setBlockImage(id, 3, buildPiston(FACE_SOUTH, front, back, side.rotate(1), side.rotate(1)));
	setBlockImage(id, 4, buildPiston(FACE_WEST, front, back, side.rotate(3), side.rotate(2)));
	setBlockImage(id, 5, buildPiston(FACE_EAST, front, back, side.rotate(1), side));
}

void IsometricBlockImages::createSlabs(uint16_t id, SlabType type, bool double_slabs) { // id 43, 44, 125, 126
	const BlockTextures& textures = resources.getBlockTextures();
	std::map<int, RGBAImage> slab_textures;
	if (type == SlabType::STONE) {
		// stone slabs
		slab_textures[0x0] = textures.STONE_SLAB_SIDE;
		slab_textures[0x1] = textures.SANDSTONE_NORMAL;
		slab_textures[0x2] = textures.PLANKS_OAK;
		slab_textures[0x3] = textures.COBBLESTONE;
		slab_textures[0x4] = textures.BRICK;
		slab_textures[0x5] = textures.STONEBRICK;
		slab_textures[0x6] = textures.NETHER_BRICK;
		slab_textures[0x7] = textures.QUARTZ_BLOCK_SIDE;
	} else if (type == SlabType::STONE2) {
		// stone2 slabs
		slab_textures[0x0] = textures.RED_SANDSTONE_NORMAL;
	} else if (type == SlabType::WOOD) {
		// wooden slabs
		slab_textures[0x0] = textures.PLANKS_OAK;
		slab_textures[0x1] = textures.PLANKS_SPRUCE;
		slab_textures[0x2] = textures.PLANKS_BIRCH;
		slab_textures[0x3] = textures.PLANKS_JUNGLE;
		slab_textures[0x4] = textures.PLANKS_ACACIA;
		slab_textures[0x5] = textures.PLANKS_BIG_OAK;
	} else if (type == SlabType::PURPUR) {
		slab_textures[0x0] = textures.PURPUR_BLOCK;
	}
	for (auto it = slab_textures.begin(); it != slab_textures.end(); ++it) {
		RGBAImage side = it->second;
		RGBAImage top = it->second;

		if (it->first == 0 && type == SlabType::STONE)
			top = textures.STONE_SLAB_TOP;
		if (it->first == 1 && type == SlabType::STONE)
			top = textures.SANDSTONE_TOP;
		if (it->first == 0 && type == SlabType::STONE2)
			top = textures.RED_SANDSTONE_TOP;

		if (double_slabs) {
			createBlock(id, it->first, side, top);
		} else {
			createSmallerBlock(id, it->first, side, top, 0, texture_size / 2);
			createSmallerBlock(id, 0x8 | it->first, side, top, texture_size / 2, texture_size);
		}
	}

	// special double slabs
	if (type == SlabType::STONE && double_slabs) {
		createBlock(id, 0x8, textures.STONE_SLAB_TOP);
		createBlock(id, 0x9, textures.SANDSTONE_TOP);
		createBlock(id, 0xF, textures.QUARTZ_BLOCK_TOP);
	}
}

void IsometricBlockImages::createTorch(uint16_t id, const RGBAImage& texture) { // id 50, 75, 76
	createSingleFaceBlock(id, 1, FACE_WEST, texture);
	createSingleFaceBlock(id, 2, FACE_EAST, texture);
	createSingleFaceBlock(id, 3, FACE_NORTH, texture);
	createSingleFaceBlock(id, 4, FACE_SOUTH, texture);

	createItemStyleBlock(id, 5, texture);
	createItemStyleBlock(id, 6, texture);
}

namespace {

/**
 * Clips the texture for a side of a stairs quarter. You have to specify which part of
 * the texture this is (topleft/topright/bottomleft/bottomright).
 */
RGBAImage clipQuarterTexture(const RGBAImage& texture, bool left, bool top) {
	int s = texture.getWidth() / 2;
	int start_x = left ? 0 : s;
	int start_y = top ? 0 : s;
	return texture.clip(start_x, start_y, s, s);
}

}

/**
 * Builds the image of a stairs quarter. You have to specify which quarter this is, so
 * the correct textures can be used (top/bottom, north/west, east/west).
 */
RGBAImage IsometricBlockImages::buildStairsQuarter(const RGBAImage& texture_side,
		const RGBAImage& texture_top, bool top, bool north, bool east) const {
	RGBAImage quarter(getBlockSize() / 2, getBlockSize() / 2);
	blitFace(quarter, FACE_TOP, clipQuarterTexture(texture_top, north, east));
	blitFace(quarter, FACE_WEST, clipQuarterTexture(texture_side, north, top), 0, 0, true, dleft, dright);
	blitFace(quarter, FACE_SOUTH, clipQuarterTexture(texture_side, !east, top), 0, 0, true, dleft, dright);
	return quarter;
}

void IsometricBlockImages::createStairs(uint16_t id, const RGBAImage& texture,
		const RGBAImage& texture_top) { // id 53, 67, 108, 109, 114, 128, 134, 135, 136, 180
	// https://github.com/overviewer/Minecraft-Overviewer/blob/master/overviewer_core/src/iterate.c#L454
	/* 4 ancillary bits will be added to indicate which quarters of the block contain the 
	 * upper step. Regular stairs will have 2 bits set & corner stairs will have 1 or 3.
	 *     Southwest quarter is part of the upper step - 0x40
	 *    / Southeast " - 0x20
	 *    |/ Northeast " - 0x10
	 *    ||/ Northwest " - 0x8
	 *    |||/ flip upside down (Minecraft)
	 *    ||||/ has North/South alignment (Minecraft)
	 *    |||||/ ascends North or West, not South or East (Minecraft)
	 *    ||||||/
	 *  0b0011011 = Stair ascending north, upside up, with both north quarters filled
	 */

	for (int i = 0; i < 32; i++) {
		uint16_t data = i << 2;
		// whether a quarter is a top quarter
		bool south_west = data & 0x40;
		bool south_east = data & 0x20;
		bool north_east = data & 0x10;
		bool north_west = data & 0x8;
		bool upside_down = data & 0x4;

		RGBAImage block(getBlockSize(), getBlockSize());
		// draw bottom quarters
		// also create the quarters with the right part of the original textures
		int yoff = texture_size / 2;
		// north east
		if (!upside_down || north_east)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, false, true, true), texture_size/2, yoff);
		// north-west
		if (!upside_down || north_west)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, false, true, false), 0, texture_size/4 + yoff);
		// south east
		if (!upside_down || south_east)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, false, false, true), texture_size, texture_size/4 + yoff);
		// south west
		if (!upside_down || south_west)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, false, false, false), texture_size/2, texture_size/2 + yoff);

		// draw top quarters
		// the same as bottom quarters except that y offset 0
		yoff = 0;
		if (upside_down || north_east)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, true, true, true), texture_size/2, 0 + yoff);
		if (upside_down || north_west)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, true, true, false), 0, texture_size/4 + yoff);
		if (upside_down || south_east)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, true, false, true), texture_size, texture_size/4 + yoff);
		if (upside_down || south_west)
			block.simpleAlphaBlit(buildStairsQuarter(texture, texture_top, true, false, false), texture_size/2, texture_size/2 + yoff);

		setBlockImage(id, data, block);
	}
}

void IsometricBlockImages::createStairs(uint16_t id, const RGBAImage& texture) {
	createStairs(id, texture, texture);
}

void IsometricBlockImages::createShulkerBox(uint16_t id, int color_index, const ShulkerTextures& textures) { // id 219 - 234
	int offset = color_index * ShulkerTextures::DATA_SIZE;

	RGBAImage top = textures[offset + ShulkerTextures::TOP];
	RGBAImage side = textures[offset + ShulkerTextures::SIDE];
	RGBAImage bottom = textures[offset + ShulkerTextures::BOTTOM];

	createBlock(id, 0, side.rotate(ROTATE_180), bottom.rotate(ROTATE_270).flip(true, false));
	createBlock(id, 1, side, top);

	BlockImage north_block;
	north_block.setFace(FACE_NORTH, top);
	north_block.setFace(FACE_EAST, side.rotate(ROTATE_270));
	north_block.setFace(FACE_SOUTH, bottom.flip(true, false));
	north_block.setFace(FACE_WEST, side.rotate(ROTATE_270));
	north_block.setFace(FACE_TOP, side.rotate(ROTATE_270));

	setBlockImage(id, 2, north_block);

	BlockImage south_block;
	south_block.setFace(FACE_NORTH, bottom.flip(true, false));
	south_block.setFace(FACE_EAST, side.rotate(ROTATE_90));
	south_block.setFace(FACE_SOUTH, top);
	south_block.setFace(FACE_WEST, side.rotate(ROTATE_90));
	south_block.setFace(FACE_TOP, side.rotate(ROTATE_90));

	setBlockImage(id, 3, south_block);

	BlockImage east_block;
	east_block.setFace(FACE_NORTH, side.rotate(ROTATE_90));
	east_block.setFace(FACE_EAST, top);
	east_block.setFace(FACE_SOUTH, side.rotate(ROTATE_90));
	east_block.setFace(FACE_WEST, bottom.flip(true, false));
	east_block.setFace(FACE_TOP, side);

	setBlockImage(id, 5, east_block);

	BlockImage west_block;
	west_block.setFace(FACE_NORTH, side.rotate(ROTATE_270));
	west_block.setFace(FACE_EAST, bottom.flip(true, false));
	west_block.setFace(FACE_SOUTH, side.rotate(ROTATE_270));
	west_block.setFace(FACE_WEST, top);
	west_block.setFace(FACE_TOP, side.rotate(ROTATE_180));

	setBlockImage(id, 4, west_block);
}

void IsometricBlockImages::createChest(uint16_t id, const ChestTextures& textures) { // id 54, 130
	BlockImage chest;
	chest.setFace(FACE_SOUTH, textures[ChestTextures::FRONT]);
	chest.setFace(FACE_NORTH | FACE_EAST | FACE_WEST, textures[ChestTextures::SIDE]);
	chest.setFace(FACE_TOP, textures[ChestTextures::TOP]);

	setBlockImage(id, DATA_NORTH, buildImage(chest.rotate(2)));
	setBlockImage(id, DATA_SOUTH, buildImage(chest));
	setBlockImage(id, DATA_EAST, buildImage(chest.rotate(3)));
	setBlockImage(id, DATA_WEST, buildImage(chest.rotate(1)));
}

void IsometricBlockImages::createDoubleChest(uint16_t id, const DoubleChestTextures& textures) { // id 54
	BlockImage left, right;

	// left side of the chest, south orientation
	left.setFace(FACE_SOUTH, textures[DoubleChestTextures::FRONT_LEFT]);
	left.setFace(FACE_NORTH, textures[DoubleChestTextures::BACK_LEFT].flip(true, false));
	left.setFace(FACE_WEST, textures[DoubleChestTextures::SIDE]);
	left.setFace(FACE_TOP, textures[DoubleChestTextures::TOP_LEFT].rotate(3));

	// right side of the chest, south orientation
	right.setFace(FACE_SOUTH, textures[DoubleChestTextures::FRONT_RIGHT]);
	right.setFace(FACE_NORTH, textures[DoubleChestTextures::BACK_RIGHT].flip(true, false));
	right.setFace(FACE_EAST, textures[DoubleChestTextures::SIDE]);
	right.setFace(FACE_TOP, textures[DoubleChestTextures::TOP_RIGHT].rotate(3));

	int l = LARGECHEST_DATA_LARGE;
	setBlockImage(id, DATA_NORTH | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(2)));
	setBlockImage(id, DATA_SOUTH | l | LARGECHEST_DATA_LEFT, buildImage(left));
	setBlockImage(id, DATA_EAST | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(3)));
	setBlockImage(id, DATA_WEST | l | LARGECHEST_DATA_LEFT, buildImage(left.rotate(1)));

	setBlockImage(id, DATA_NORTH | l, buildImage(right.rotate(2)));
	setBlockImage(id, DATA_SOUTH | l, buildImage(right));
	setBlockImage(id, DATA_EAST | l, buildImage(right.rotate(3)));
	setBlockImage(id, DATA_WEST | l, buildImage(right.rotate(1)));
}

void IsometricBlockImages::createRedstoneWire(uint16_t id, uint16_t extra_data,
		uint8_t r, uint8_t g, uint8_t b) { // id 55
	RGBAImage redstone_cross = resources.getBlockTextures().REDSTONE_DUST_DOT;
	RGBAImage redstone_line = resources.getBlockTextures().REDSTONE_DUST_LINE0.rotate(1);
	redstone_cross.simpleAlphaBlit(redstone_line, 0, 0);
	redstone_cross.simpleAlphaBlit(redstone_line.rotate(1), 0, 0);

	//uint8_t color = powered ? 50 : 255;
	redstone_cross = redstone_cross.colorize(r, g, b);
	redstone_line = redstone_line.colorize(r, g, b);

	// 1/16 of the texture size
	double s = (double) texture_size / 16;

	// go through all redstone combinations
	for (uint16_t i = 0; i <= 0xff; i++) {
		// the redstone extra data starts at the fifth byte
		// so shift the data to the right
		uint16_t data = i << 4;

		// skip unpossible redstone combinations
		// things like neighbor topnorth but not north
		// what would look like redstone on a wall without a connection
		if (((data & REDSTONE_TOPNORTH) && !(data & REDSTONE_NORTH))
				|| ((data & REDSTONE_TOPSOUTH) && !(data & REDSTONE_SOUTH))
				|| ((data & REDSTONE_TOPEAST) && !(data & REDSTONE_EAST))
				|| ((data & REDSTONE_TOPWEST) && !(data & REDSTONE_WEST)))
			continue;

		BlockImage block;
		RGBAImage texture = redstone_cross;
		// remove the connections from the cross image
		// if there is no connection
		if(!(data & REDSTONE_NORTH))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*16, s*4);
		if(!(data & REDSTONE_SOUTH))
			texture.fill(rgba(0, 0, 0, 0), 0, s*12, s*16, s*4);

		if(!(data & REDSTONE_EAST))
			texture.fill(rgba(0, 0, 0, 0), s*12, 0, s*4, s*16);
		if(!(data & REDSTONE_WEST))
			texture.fill(rgba(0, 0, 0, 0), 0, 0, s*4, s*16);

		// check if we have a line of restone
		if (data == (REDSTONE_NORTH | REDSTONE_SOUTH))
			texture = redstone_line.rotate(ROTATE_90);
		else if (data == (REDSTONE_EAST | REDSTONE_WEST))
			texture = redstone_line;

		// check if
		if (data & REDSTONE_TOPNORTH)
			block.setFace(FACE_NORTH, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPSOUTH)
			block.setFace(FACE_SOUTH, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPEAST)
			block.setFace(FACE_EAST, redstone_line.rotate(ROTATE_90));
		if (data & REDSTONE_TOPWEST)
			block.setFace(FACE_WEST, redstone_line.rotate(ROTATE_90));

		// rotate the texture to fit the sky directions
		texture = texture.rotate(ROTATE_270);
		block.setFace(FACE_BOTTOM, texture);

		// we can add the block like this without rotation
		// because we calculate the neighbors on our own,
		// it does not depend on the rotation of the map
		setBlockImage(id, data | extra_data, buildImage(block));
	}
}

void IsometricBlockImages::createSign() { // id 63
	// simple version of a standing sign always pointing to south-west // TODO
	double ratio = (double) texture_size / 16.0;
	int h1 = 10 * ratio;
	int h2 = 8 * ratio;
	
	RGBAImage texture = resources.getBlockTextures().PLANKS_OAK;
	RGBAImage head = texture.clip(0, 0, texture_size, h1);
	RGBAImage post = texture.clip(0, 0, 2, h2).colorize(0.6, 0.6, 0.6);

	RGBAImage sign(texture_size, h1+h2);
	sign.simpleAlphaBlit(head, 0, 0);
	sign.simpleAlphaBlit(post, (texture_size - 2) / 2, h1);
	RGBAImage block(getBlockSize(), getBlockSize());
	block.simpleAlphaBlit(sign, (block.getWidth() - sign.getWidth()) / 2, (block.getHeight() - sign.getHeight()) / 2);

	setBlockImage(63, 0, block);
}

void IsometricBlockImages::createDoor(uint16_t id, const RGBAImage& texture_bottom,
        const RGBAImage& texture_top) { // id 64, 71
	// TODO sometimes the texture needs to get x flipped when door is opened
	for (int top = 0; top <= 1; top++) {
		for (int flip_x = 0; flip_x <= 1; flip_x++) {
			for (int d = 0; d < 4; d++) {
				RGBAImage texture = (top ? texture_top : texture_bottom);
				if (flip_x)
					texture = texture.flip(true, false);
				BlockImage block;

				uint16_t direction = 0;
				if (d == 0) {
					direction = DOOR_NORTH;
					block.setFace(FACE_NORTH, texture);
				} else if (d == 1) {
					direction = DOOR_SOUTH;
					block.setFace(FACE_SOUTH, texture);
				} else if (d == 2) {
					direction = DOOR_EAST;
					block.setFace(FACE_EAST, texture);
				} else if (d == 3) {
					direction = DOOR_WEST;
					block.setFace(FACE_WEST, texture);
				}
				uint16_t data = (top ? DOOR_TOP : 0) | (flip_x ? DOOR_FLIP_X : 0)
				        | direction;
				setBlockImage(id, data, block);
			}
		}
	}
}

void IsometricBlockImages::createRails() { // id 66
	RGBAImage texture = resources.getBlockTextures().RAIL_NORMAL;
	RGBAImage corner_texture = resources.getBlockTextures().RAIL_NORMAL_TURNED;

	createStraightRails(66, 0, texture);
	createSingleFaceBlock(66, 6, FACE_BOTTOM, corner_texture.flip(false, true));
	createSingleFaceBlock(66, 7, FACE_BOTTOM, corner_texture);
	createSingleFaceBlock(66, 8, FACE_BOTTOM, corner_texture.flip(true, false));
	createSingleFaceBlock(66, 9, FACE_BOTTOM, corner_texture.flip(true, true));
}

void IsometricBlockImages::createWallSign() { // id 68
	double ratio = (double) texture_size / 16.0;
	int h = 10 * ratio;
	
	RGBAImage texture = resources.getBlockTextures().PLANKS_OAK.clip(0, 0, texture_size, h);
	RGBAImage sign(texture_size, texture_size);
	sign.simpleBlit(texture, 0, (texture_size - h) / 2);

	createSingleFaceBlock(68, 2, FACE_SOUTH, sign);
	createSingleFaceBlock(68, 3, FACE_NORTH, sign);
	createSingleFaceBlock(68, 4, FACE_EAST, sign);
	createSingleFaceBlock(68, 5, FACE_WEST, sign);
}

void IsometricBlockImages::createButton(uint16_t id, const RGBAImage& tex) { // id 77, 143
	RGBAImage texture = tex;
	int s = texture.getWidth();
	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			if (x < s / 8 * 2 || x > s / 8 * 6 || y < s / 8 * 3 || y > s / 8 * 5)
				texture.setPixel(x, y, rgba(0, 0, 0, 0));
			else {
				uint32_t p = texture.getPixel(x, y);
				if (x == s / 8 * 2 || x == s / 8 * 6 || y == s / 8 * 3 || y == s / 8 * 5)
					p = rgba_multiply(p, 0.8, 0.8, 0.8);
				texture.setPixel(x, y, p);
			}
		}
	}

	createSingleFaceBlock(id, 1, FACE_WEST, texture);
	createSingleFaceBlock(id, 2, FACE_EAST, texture);
	createSingleFaceBlock(id, 3, FACE_NORTH, texture);
	createSingleFaceBlock(id, 4, FACE_SOUTH, texture);
}

void IsometricBlockImages::createSnow() { // id 78
	RGBAImage snow = resources.getBlockTextures().SNOW;
	for (int data = 0; data < 8; data++) {
		int height = (data+1) / 8.0 * texture_size;
		setBlockImage(78, data, buildSmallerBlock(snow, snow, snow, 0, height));
	}
}

void IsometricBlockImages::createIce(uint8_t id, uint16_t extra_data, const RGBAImage& texture) { // id 79, 212
	for (int w = 0; w <= 1; w++) {
		for (int s = 0; s <= 1; s++) {
			RGBAImage block(getBlockSize(), getBlockSize());
			uint16_t data = extra_data;
			if (w == 1)
				blitFace(block, FACE_WEST, texture, 0, 0, true, dleft, dright);
			else
				data |= DATA_WEST;

			if (s == 1)
				blitFace(block, FACE_SOUTH, texture, 0, 0, true, dleft, dright);
			else
				data |= DATA_SOUTH;
			blitFace(block, FACE_TOP, texture, 0, 0, true, dleft, dright);
			setBlockImage(id, data, block);
		}
	}
}

void IsometricBlockImages::createCactus() { // id 81
	BlockImage block;
	block.setFace(FACE_WEST, resources.getBlockTextures().CACTUS_SIDE, 2, 0);
	block.setFace(FACE_SOUTH, resources.getBlockTextures().CACTUS_SIDE, -2, 0);
	block.setFace(FACE_TOP, resources.getBlockTextures().CACTUS_TOP);
	setBlockImage(81, 0, buildImage(block));
}

/**
 * Creates the texture for a fence. The texture looks like a cross, you can set if you
 * want the left or/and right connection posts.
 */
RGBAImage createFenceTexture(bool left, bool right, RGBAImage texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	RGBAImage mask(size, size);
	mask.fill(rgba(255, 255, 255, 255), 0, 0, size, size);
	// the main post
	mask.fill(0, 6 * ratio, 0, 4 * ratio, 16 * ratio);
	// if set, left and right connection posts
	// note here when filling the width +2, it's because to make sure we fill enough
	// and don't forget a pixel
	if(left)
		mask.fill(0, 0, 4 * ratio, 6 * ratio, 4 * ratio);
	if(right)
		mask.fill(0, 10 * ratio, 4 * ratio, 6 * ratio + 2, 4 * ratio);

	// then apply mask to the texture
	for(int x = 0; x < size; x++)
		for(int y = 0; y < size; y++)
			if(mask.getPixel(x, y) != 0)
				texture.setPixel(x, y, 0);

	return texture;
}

/**
 * This method creates the fence block images. It generates textures for fences and makes
 * with this textures item style block images.
 */
void IsometricBlockImages::createFence(uint16_t id, uint16_t extra_data, const RGBAImage& texture) { // id 85, 113
	RGBAImage fence_empty = createFenceTexture(false, false, texture);
	RGBAImage fence_left = createFenceTexture(true, false, texture);
	RGBAImage fence_right = createFenceTexture(false, true, texture);
	RGBAImage fence_both = createFenceTexture(true, true, texture);

	// go through all neighbor combinations
	for (uint8_t i = 0; i < 16; i++) {
		RGBAImage left = fence_empty, right = fence_empty;

		uint16_t data = i << 4;
		// special data set by the tile renderer
		bool north = data & DATA_NORTH;
		bool south = data & DATA_SOUTH;
		bool east = data & DATA_EAST;
		bool west = data & DATA_WEST;

		// now select the needed textures for this neighbors
		if (north && south)
			left = fence_both;
		else if (north)
			left = fence_left;
		else if (south)
			left = fence_right;

		if (east && west)
			right = fence_both;
		else if (east)
			right = fence_right;
		else if (west)
			right = fence_left;

		BlockImage block(BlockImage::ITEM_STYLE);
		block.setFace(FACE_NORTH | FACE_SOUTH, left);
		block.setFace(FACE_EAST | FACE_WEST, right);
		setBlockImage(id, data | extra_data, buildImage(block));
	}
}

void IsometricBlockImages::createPumkin(uint16_t id, const RGBAImage& front) { // id 86, 91
	BlockImage pumpkin;
	pumpkin.setFace(FACE_SOUTH, front);
	pumpkin.setFace(FACE_NORTH | FACE_EAST | FACE_WEST, resources.getBlockTextures().PUMPKIN_SIDE);
	pumpkin.setFace(FACE_TOP, resources.getBlockTextures().PUMPKIN_TOP);

	setBlockImage(id, 0, pumpkin); // south
	setBlockImage(id, 1, pumpkin.rotate(1)); // west
	setBlockImage(id, 2, pumpkin.rotate(2)); // north
	setBlockImage(id, 3, pumpkin.rotate(3)); // east
}

void IsometricBlockImages::createCake() { // id 92
	BlockImage block;
	block.setFace(FACE_WEST, resources.getBlockTextures().CAKE_SIDE, 1, 0);
	block.setFace(FACE_SOUTH, resources.getBlockTextures().CAKE_SIDE, -1, 0);
	block.setFace(FACE_TOP, resources.getBlockTextures().CAKE_TOP, 0, 9);
	setBlockImage(92, 0, buildImage(block));
}

void IsometricBlockImages::createRedstoneRepeater(uint16_t id, const RGBAImage& texture) { // id 93, 94
	createSingleFaceBlock(id, 0, FACE_BOTTOM, texture.rotate(ROTATE_270));
	createSingleFaceBlock(id, 1, FACE_BOTTOM, texture);
	createSingleFaceBlock(id, 2, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 3, FACE_BOTTOM, texture.rotate(ROTATE_180));
}

void IsometricBlockImages::createTrapdoor(uint16_t id, const RGBAImage& texture) { // id 96, 167
	for (uint16_t i = 0; i < 16; i++) {
		if (i & 4) {
			int data = i & util::binary<11>::value;

			if (data == 0x0)
				createSingleFaceBlock(id, i, FACE_SOUTH, texture);
			else if (data == 0x1)
				createSingleFaceBlock(id, i, FACE_NORTH, texture);
			else if (data == 0x2)
				createSingleFaceBlock(id, i, FACE_EAST, texture);
			else if (data == 0x3)
				createSingleFaceBlock(id, i, FACE_WEST, texture);
		} else {
			if (i & 8)
				createSingleFaceBlock(id, i, FACE_TOP, texture);
			else
				createSingleFaceBlock(id, i, FACE_BOTTOM, texture);
		}
	}
}

BlockImage buildHugeMushroom(const RGBAImage& pores, const RGBAImage& cap = RGBAImage(),
		int cap_sides = 0, const RGBAImage& stem = RGBAImage(), int stem_sides = 0) {
	BlockImage block;
	block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST | FACE_TOP, pores);
	for (int i = 0; i < 6; i++) {
		int side = 1 << i;
		if (cap_sides & side)
			block.setFace(side, cap);
		else if (stem_sides & side)
			block.setFace(side, stem);
	}
	return block;
}

void IsometricBlockImages::createHugeMushroom(uint16_t id, const RGBAImage& cap) { // id 99, 100
	RGBAImage pores = resources.getBlockTextures().MUSHROOM_BLOCK_INSIDE;
	RGBAImage stem = resources.getBlockTextures().MUSHROOM_BLOCK_SKIN_STEM;

	setBlockImage(id, 0, buildHugeMushroom(pores));
	setBlockImage(id, 1, buildHugeMushroom(pores, cap, FACE_TOP | FACE_WEST | FACE_NORTH));
	setBlockImage(id, 2, buildHugeMushroom(pores, cap, FACE_TOP | FACE_NORTH));
	setBlockImage(id, 3, buildHugeMushroom(pores, cap, FACE_TOP | FACE_NORTH | FACE_EAST));
	setBlockImage(id, 4, buildHugeMushroom(pores, cap, FACE_TOP | FACE_WEST));
	setBlockImage(id, 5, buildHugeMushroom(pores, cap, FACE_TOP));
	setBlockImage(id, 6, buildHugeMushroom(pores, cap, FACE_TOP | FACE_EAST));
	setBlockImage(id, 7, buildHugeMushroom(pores, cap, FACE_TOP | FACE_SOUTH | FACE_WEST));
	setBlockImage(id, 8, buildHugeMushroom(pores, cap, FACE_TOP | FACE_SOUTH));
	setBlockImage(id, 9, buildHugeMushroom(pores, cap, FACE_TOP | FACE_EAST | FACE_SOUTH));
	setBlockImage(id, 10, buildHugeMushroom(pores, cap, 0, stem, util::binary<1111>::value));
	setBlockImage(id, 14, buildHugeMushroom(pores, cap, util::binary<111111>::value));
	setBlockImage(id, 15, buildHugeMushroom(pores, cap, 0, stem, util::binary<111111>::value));
}

void IsometricBlockImages::createBarsPane(uint16_t id, uint16_t extra_data,
		const RGBAImage& texture_left_right) { // id 101, 102, 160
	RGBAImage texture_left = texture_left_right;
	RGBAImage texture_right = texture_left_right;
	texture_left.fill(0, texture_size / 2, 0, texture_size / 2, texture_size);
	texture_right.fill(0, 0, 0, texture_size / 2, texture_size);

	createItemStyleBlock(id, 0, texture_left_right);
	for (uint8_t i = 1; i < 16; i++) {
		RGBAImage left = empty_texture, right = empty_texture;

		uint16_t data = i << 4;
		bool north = data & DATA_NORTH;
		bool south = data & DATA_SOUTH;
		bool east = data & DATA_EAST;
		bool west = data & DATA_WEST;

		if (north && south)
			left = texture_left_right;
		else if (north)
			left = texture_left;
		else if (south)
			left = texture_right;

		if (east && west)
			right = texture_left_right;
		else if (east)
			right = texture_right;
		else if (west)
			right = texture_left;

		BlockImage block(BlockImage::ITEM_STYLE);
		block.setFace(FACE_NORTH | FACE_SOUTH, left);
		block.setFace(FACE_EAST | FACE_WEST, right);
		setBlockImage(id, data | extra_data, buildImage(block));
	}
}

void IsometricBlockImages::createStem(uint16_t id) { // id 104, 105
	// build here only growing normal stem
	RGBAImage texture = resources.getBlockTextures().PUMPKIN_STEM_DISCONNECTED;

	for (int i = 0; i <= 7; i++) {
		double percentage = 1 - ((double) i / 7);
		int move = percentage * texture_size;

		if (i == 7)
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.6, 0.7, 0.01));
		else
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.3, 0.7, 0.01));
	}
}

void IsometricBlockImages::createVines() { // id 106
	RGBAImage texture = resources.getBlockTextures().VINE;

	createSingleFaceBlock(106, 0, FACE_TOP, texture);
	for (int i = 1; i < 16; i++) {
		BlockImage block;
		if (i & 1)
			block.setFace(FACE_SOUTH, texture);
		if (i & 2)
			block.setFace(FACE_WEST, texture);
		if (i & 4)
			block.setFace(FACE_NORTH, texture);
		if (i & 8)
			block.setFace(FACE_EAST, texture);
		setBlockImage(106, i, block);
	}
}

/**
 * Creates the texture for a fence gate, opened or closed.
 */
RGBAImage createFenceGateTexture(bool opened, RGBAImage texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	RGBAImage mask(size, size);
	mask.fill(rgba(255, 255, 255, 255), 0, 0, size, size);

	// left and right post
	mask.fill(0, 0, 0, 4 * ratio, 12 * ratio);
	mask.fill(0, 12 * ratio, 0, 4 * ratio, 12 * ratio);
	// if closed this middle part
	if(!opened)
		mask.fill(0, 4 * ratio, 3 * ratio, 8 * ratio, 6 * ratio);

	// then apply mask to the texture
	for(int x = 0; x < size; x++)
		for(int y = 0; y < size; y++)
			if(mask.getPixel(x, y) != 0)
				texture.setPixel(x, y, 0);

	return texture;
}

void IsometricBlockImages::createFenceGate(uint8_t id, RGBAImage texture) { // id 107, 183-187
	RGBAImage opened = createFenceGateTexture(true, texture);
	RGBAImage closed = createFenceGateTexture(false, texture);

	// go through states opened and closed
	for(int open = 0; open <= 1; open++) {
		RGBAImage tex = open ? opened : closed;
		RGBAImage north(texture_size * 2, texture_size * 2);
		RGBAImage east = north;
		// north and south block images are same
		// (because we ignore the direction of opened fence gates)
		blitFace(north, FACE_NORTH, tex, texture_size * 0.5, texture_size * 0.25, false);
		// also east and west
		blitFace(east, FACE_EAST, tex, -texture_size * 0.5, texture_size * 0.25, false);
		uint8_t extra = open ? 4 : 0;
		if (rotation == 0 || rotation == 2) {
			setBlockImage(id, 0 | extra, north);
			setBlockImage(id, 1 | extra, east);
			setBlockImage(id, 2 | extra, north);
			setBlockImage(id, 3 | extra, east);
		} else {
			setBlockImage(id, 0 | extra, east);
			setBlockImage(id, 1 | extra, north);
			setBlockImage(id, 2 | extra, east);
			setBlockImage(id, 3 | extra, north);
		}
	}
}

void IsometricBlockImages::createBrewingStand() { // id 117
	const BlockTextures& textures = resources.getBlockTextures();
	RGBAImage block(getBlockSize(), getBlockSize());
	blitFace(block, FACE_BOTTOM, textures.BREWING_STAND_BASE);
	blitItemStyleBlock(block, textures.BREWING_STAND, textures.BREWING_STAND);
	setBlockImage(117, 0, block);
}

void IsometricBlockImages::createCauldron() { // id 118
	RGBAImage side = resources.getBlockTextures().CAULDRON_SIDE;
	RGBAImage water = resources.getBlockTextures().WATER_STILL;

	for (int i = 0; i < 4; i++) {
		RGBAImage block(getBlockSize(), getBlockSize());
		blitFace(block, FACE_NORTH, side, 0, 0, true, dleft, dright);
		blitFace(block, FACE_EAST, side, 0, 0, true, dleft, dright);
		if (i == 3)
			blitFace(block, FACE_TOP, water, 0, 2, true, dleft, dright);
		else if (i == 2)
			blitFace(block, FACE_TOP, water, 0, 5, true, dleft, dright);
		else if (i == 1)
			blitFace(block, FACE_TOP, water, 0, 9, true, dleft, dright);
		blitFace(block, FACE_SOUTH, side, 0, 0, true, dleft, dright);
		blitFace(block, FACE_WEST, side, 0, 0, true, dleft, dright);
		setBlockImage(118, i, block);
	}
}

void IsometricBlockImages::createDragonEgg() { // id 122
	// create an half circle of the dragon egg texture
	// to create an item style block with this texture

	RGBAImage texture = resources.getBlockTextures().DRAGON_EGG;

	// the formula for an half circle is sqrt(r*r - x*x)
	// the circle would go from -r to +r
	// here we use 1.5 * sqrt(r*r - x*x) in the interval [0;texture_size/2],
	// reflect the quarter circle and remove all pixels which are not in the circle

	int r = (texture_size/2) * (texture_size/2);
	for (int x = 0; x <= texture_size/2; x++) {
		int height = 1.5*sqrt(r - x*x);
		for (int y = 0; y < texture_size-height; y++) {
			texture.setPixel(texture_size/2 + x, y, rgba(0, 0, 0, 0));
			texture.setPixel(texture_size/2 - x - 1, y, rgba(0, 0, 0, 0));
		}
	}

	createItemStyleBlock(122, 0, texture);
}

RGBAImage IsometricBlockImages::buildCocoa(int stage) {
	const BlockTextures& textures = resources.getBlockTextures();
	RGBAImage texture;
	if (stage == 0)
		texture = textures.COCOA_STAGE_0.getOriginal();
	else if (stage == 1)
		texture = textures.COCOA_STAGE_1.getOriginal();
	else if (stage == 2)
		texture = textures.COCOA_STAGE_2.getOriginal();

	// at first use the original size of the provided texture image
	int original_size = texture.getWidth();
	double r = (double) original_size / 16;
	// the sizes of the 16px texture size cocoa bean textures are 4/6/8px
	// multiply with r to get the correct size according to the texture
	int size = 2 * (stage+2) * r;

	// get the size * size top texture
	// only the top texture is used to create a cubic cocoa bean
	// because it's too difficult to use the original cocoa bean
	// proportions for all the texture sizes
	RGBAImage top = texture.clip(0, 0, size, size);

	// however, the size of the third stage is not 8px, it's 7px. why?
	// just resize it to 8px...
	if (stage == 2)
		texture.clip(0, 0, size-1, size-1).resize(top, size, size, InterpolationType::NEAREST);

	// now size according to the texture size the renderer should use
	r = (double) texture_size / 16;
	size = 2 * (stage+2) * r;
	// resize the texture to this size
	RGBAImage(top).resize(top, size, size, InterpolationType::NEAREST);

	// and create a simple cubic cocoa bean
	RGBAImage cocoa(size*2, size*2);
	blitFace(cocoa, FACE_WEST, top);
	blitFace(cocoa, FACE_SOUTH, top);
	blitFace(cocoa, FACE_TOP, top);
	return cocoa;
}

void IsometricBlockImages::createCocoas() { // id 127
	// create the cubic cocoa beans images
	// and just use a centered version of it as block images
	for (int i = 0; i < 3; i++) {
		RGBAImage cocoa = buildCocoa(i);
		RGBAImage block(texture_size * 2, texture_size * 2);
		int xoff = (block.getWidth() - cocoa.getWidth()) / 2;
		int yoff = (block.getHeight() - cocoa.getHeight()) / 2;
		block.simpleAlphaBlit(cocoa, xoff, yoff);

		uint16_t data = i == 0 ? 0 : (i == 1 ? util::binary<100>::value : util::binary<1000>::value);
		setBlockImage(127, data, block);
	}
}

void IsometricBlockImages::createTripwireHook() { // id 131
	RGBAImage tripwire = resources.getBlockTextures().REDSTONE_DUST_LINE0.colorize((uint8_t) 192, 192, 192).rotate(1);

	BlockImage block;
	block.setFace(FACE_NORTH, resources.getBlockTextures().TRIP_WIRE_SOURCE);
	block.setFace(FACE_BOTTOM, tripwire);

	setBlockImage(131, 0, block); // trip wire hook on the north side
	setBlockImage(131, 1, block.rotate(1)); // on the east side
	setBlockImage(131, 2, block.rotate(2)); // on the south side
	setBlockImage(131, 3, block.rotate(3)); // on the west side
}

void IsometricBlockImages::createCommandBlock(uint16_t id, const RGBAImage& front,
		const RGBAImage& back, const RGBAImage& side, const RGBAImage& conditional) { // id 137, 210, 211
	for (int i = 0; i <= 15; i++) {
		int rotation_data = i & ~0x8;
		RGBAImage side_texture = (i & 0x8) ? conditional : side;

		if (rotation_data <= 1 || rotation_data >= 6) {
			// a command block that's pointing up or down
			bool down = rotation_data == 0 || rotation_data == 6;
			int face_up = down ? FACE_BOTTOM : FACE_TOP;
			int face_down = down ? FACE_TOP : FACE_BOTTOM;
			int side_flip = down ? true : false;
			BlockImage block;
			block.setFace(face_up, front);
			block.setFace(FACE_NORTH | FACE_SOUTH | FACE_EAST | FACE_WEST, side_texture.flip(false, side_flip));
			block.setFace(face_down, back);
			setBlockImage(id, i, block);
		} else {
			// otherwise it's a command block showing in one of the other directions
			BlockImage block;
			block.setFace(FACE_NORTH, front);
			block.setFace(FACE_EAST | FACE_WEST | FACE_TOP | FACE_BOTTOM, side_texture.rotate(3));
			block.setFace(FACE_SOUTH, back);
			int rotation = -1;
			if (rotation_data == 2)
				rotation = 0;
			else if (rotation_data == 3)
				rotation = 2;
			else if (rotation_data == 4)
				rotation = 3;
			else if (rotation_data == 5)
				rotation = 1;
			setBlockImage(id, i, block.rotate(rotation));
		}
	}
}

void IsometricBlockImages::createBeacon() { // id 138
	const BlockTextures& textures = resources.getBlockTextures();
	RGBAImage beacon(texture_size * 2, texture_size * 2);

	// at first create this little block in the middle
	RGBAImage beacon_texture;
	textures.BEACON.resize(beacon_texture, texture_size * 0.75, texture_size * 0.75);
	RGBAImage smallblock(texture_size * 2, texture_size * 2);
	blitFace(smallblock, FACE_WEST, beacon_texture, 0, 0, true, dleft, dright);
	blitFace(smallblock, FACE_SOUTH, beacon_texture, 0, 0, true, dleft, dright);
	blitFace(smallblock, FACE_TOP, beacon_texture, 0, 0, true, dleft, dright);

	// then create the obsidian ground
	RGBAImage obsidian_texture = textures.OBSIDIAN;
	RGBAImage obsidian = buildImage(buildSmallerBlock(obsidian_texture, obsidian_texture,
			obsidian_texture, 0, texture_size / 4));

	// blit block and obsidian ground
	beacon.simpleAlphaBlit(obsidian, 0, 0);
	beacon.simpleAlphaBlit(smallblock, texture_size / 4, texture_size / 4);

	// then blit outside glass
	RGBAImage glass_texture = textures.GLASS;
	blitFace(beacon, FACE_WEST, glass_texture, 0, 0, true, dleft, dright);
	blitFace(beacon, FACE_SOUTH, glass_texture, 0, 0, true, dleft, dright);
	blitFace(beacon, FACE_TOP, glass_texture, 0, 0, true, dleft, dright);

	setBlockImage(138, 0, beacon);
}

void IsometricBlockImages::createFlowerPot() { // id 140
	const BlockTextures& textures = resources.getBlockTextures();
	double s = (double) textures.FLOWER_POT.getOriginal().getWidth() / 16;
	RGBAImage pot_texture = textures.FLOWER_POT.getOriginal().clip(s*5, s*10, s*6, s*6);
	s = (double) texture_size / 16;
	pot_texture = pot_texture.resize(s*6, s*6);

	int xoff = std::ceil(s*10);
	int yoff = std::ceil(s*16);

	RGBAImage pot(getBlockSize(), getBlockSize());
	blitFace(pot, FACE_NORTH, pot_texture, xoff, yoff, true, dleft, dright);
	blitFace(pot, FACE_EAST, pot_texture, xoff, yoff, true, dleft, dright);
	blitFace(pot, FACE_TOP, textures.DIRT.clip(0, 0, s*6, s*6), xoff, yoff+s*3);

	RGBAImage contents[] = {
		RGBAImage(),
		textures.FLOWER_ROSE,
		textures.FLOWER_DANDELION,
		textures.SAPLING_OAK,
		textures.SAPLING_SPRUCE,
		textures.SAPLING_BIRCH,
		textures.SAPLING_JUNGLE,
		textures.MUSHROOM_RED,
		textures.MUSHROOM_BROWN,
		RGBAImage(),
		textures.DEADBUSH,
		textures.FERN,
	};

	for (int16_t i = 0; i < 11; i++) {
		RGBAImage block = pot;

		if (i == 9) {
			RGBAImage cactus = getBlock(81, 0);
			RGBAImage content;
			cactus.resize(content, s*16, s*16, InterpolationType::NEAREST);
			block.alphaBlit(content, s*8, s*8);
		} else if (i != 0) {
			RGBAImage content(texture_size*2, texture_size*2);
			blitItemStyleBlock(content, contents[i], contents[i]);
			block.alphaBlit(content, 0, s*-3);
		}

		blitFace(block, FACE_WEST, pot_texture, xoff, yoff, true, dleft, dright);
		blitFace(block, FACE_SOUTH, pot_texture, xoff, yoff, true, dleft, dright);

		setBlockImage(140, i, block);
	}
}

void IsometricBlockImages::createHopper() { // id 154
	RGBAImage inside = resources.getBlockTextures().HOPPER_INSIDE;
	RGBAImage outside = resources.getBlockTextures().HOPPER_OUTSIDE;
	RGBAImage top = resources.getBlockTextures().HOPPER_TOP;

	RGBAImage block(getBlockSize(), getBlockSize());
	blitFace(block, FACE_NORTH, outside, 0, 0, true, dleft, dright);
	blitFace(block, FACE_EAST, outside, 0, 0, true, dleft, dright);
	blitFace(block, FACE_TOP, inside, 0, texture_size / 2);
	blitFace(block, FACE_SOUTH, outside, 0, 0, true, dleft, dright);
	blitFace(block, FACE_WEST, outside, 0, 0, true, dleft, dright);
	blitFace(block, FACE_TOP, top);

	setBlockImage(154, 0, block);
}

void IsometricBlockImages::createLargePlant(uint16_t data, const RGBAImage& texture, const RGBAImage& top_texture) { // id 175
	createItemStyleBlock(175, data, texture);
	createItemStyleBlock(175, data | LARGEPLANT_TOP, top_texture);
}

void IsometricBlockImages::createEndRod() { // id 198
	double s = (double) getTextureSize() / 16;
	int base_height = std::max(2.0, std::ceil(s*2));
	int base_width = std::max(4.0, std::ceil(s*6));
	int rod_width = std::max(2.0, std::ceil(s*2));
	int rod_length = s*16;

	RGBAImage texture = resources.getBlockTextures().END_ROD.getOriginal();
	s = (double) texture.getWidth() / 16;
	RGBAImage rod_side, rod_top, base_side, base_top;
	texture.clip(0, 0, s*2, s*14).resize(rod_side, rod_width, rod_length, InterpolationType::NEAREST);
	texture.clip(s*2, 0, s*2, s*2).resize(rod_top, rod_width, rod_width, InterpolationType::NEAREST);
	texture.clip(s*2, s*2, s*4, s*1).resize(base_side, base_width, base_height, InterpolationType::NEAREST);
	texture.clip(s*2, s*3, s*4, s*4).resize(base_top, base_width, base_width, InterpolationType::NEAREST);

	RGBAImage rod(getTextureSize(), getTextureSize());
	rod.simpleAlphaBlit(rod_side, (rod.getWidth() - rod_side.getWidth()) / 2, 0);
	RGBAImage base(getTextureSize(), getTextureSize());
	base.simpleAlphaBlit(base_top, (base.getWidth() - base_top.getWidth()) / 2, (base.getHeight() - base_top.getHeight()) / 2);

	BlockImage up, down;
	up.setFace(FACE_BOTTOM, base);
	up.setFace(FACE_NORTH, rod, getTextureSize() / 2, getTextureSize() / 4);
	down.setFace(FACE_NORTH, rod, getTextureSize() / 2, getTextureSize() / 4);
	down.setFace(FACE_TOP, base);
	setBlockImage(198, 0, down);
	setBlockImage(198, 1, up);

	BlockImage north, south, east, west;
	north.setFace(FACE_SOUTH, base);
	north.setFace(FACE_BOTTOM, rod.rotate(1), 0, -getTextureSize() / 2);
	south.setFace(FACE_NORTH, base);
	south.setFace(FACE_TOP, rod.rotate(1), 0, getTextureSize() / 2);
	west.setFace(FACE_EAST, base);
	west.setFace(FACE_TOP, rod, 0, getTextureSize() / 2);
	east.setFace(FACE_WEST, base);
	east.setFace(FACE_BOTTOM, rod, 0, -getTextureSize() / 2);
	setBlockImage(198, 2, buildImage(north));
	setBlockImage(198, 3, buildImage(south));
	setBlockImage(198, 4, buildImage(west));
	setBlockImage(198, 5, buildImage(east));
}

RGBAImage IsometricBlockImages::createUnknownBlock() const {
	RGBAImage texture(texture_size, texture_size);
	texture.fill(rgba(255, 0, 0, 255), 0, 0, texture_size, texture_size);
	return buildImage(BlockImage().setFace(util::binary<11111>::value, texture));
}

RGBAImage IsometricBlockImages::createBiomeBlock(uint16_t id, uint16_t data,
        const Biome& biome) const {
	if (!block_images.count(id | (data << 16)))
		return unknown_block;

	uint32_t color;
	// leaves have the foliage colors
	// for birches, the color x/y coordinate is flipped
	if (id == 18)
		color = biome.getColor(resources.getFoliageColors(), (data & util::binary<11>::value) == 2);
	else
		color = biome.getColor(resources.getGrassColors(), false);

	double r = (double) rgba_red(color) / 255;
	double g = (double) rgba_green(color) / 255;
	double b = (double) rgba_blue(color) / 255;

	// grass block needs something special
	if (id == 2) {
		RGBAImage block = block_images.at(id | (data << 16));
		RGBAImage side = resources.getBlockTextures().GRASS_SIDE_OVERLAY.colorize(r, g, b);

		// blit the side overlay over the block
		blitFace(block, FACE_WEST, side, 0, 0, false);
		blitFace(block, FACE_SOUTH, side, 0, 0, false);

		// now tint the top of the block
		for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
			uint32_t pixel = block.getPixel(it.dest_x, it.dest_y);
			block.setPixel(it.dest_x, it.dest_y, rgba_multiply(pixel, r, g, b));
		}

		return block;
	}

	return block_images.at(id | (data << 16)).colorize(r, g, b);
}

void IsometricBlockImages::createBlocks() {
	buildCustomTextures();

	const BlockTextures& t = resources.getBlockTextures();

	createBlock(1, 0, t.STONE); // stone
	createBlock(1, 1, t.STONE_GRANITE); // granite
	createBlock(1, 2, t.STONE_GRANITE_SMOOTH); // polished granite
	createBlock(1, 3, t.STONE_DIORITE); // diorite
	createBlock(1, 4, t.STONE_DIORITE_SMOOTH); // polished diorite
	createBlock(1, 5, t.STONE_ANDESITE); // andesite
	createBlock(1, 6, t.STONE_ANDESITE_SMOOTH); // polished andesite
	createGrassBlock(); // id 2
	createBlock(3, 0, t.DIRT); // dirt
	createBlock(3, 1, t.DIRT); // grassless dirt
	createBlock(3, 2, t.DIRT_PODZOL_SIDE, t.DIRT_PODZOL_SIDE, t.DIRT_PODZOL_TOP); // podzol
	createBlock(4, 0, t.COBBLESTONE); // cobblestone
	// -- wooden planks
	createBlock(5, 0, t.PLANKS_OAK); // oak
	createBlock(5, 1, t.PLANKS_SPRUCE); // pine/spruce
	createBlock(5, 2, t.PLANKS_BIRCH); // birch
	createBlock(5, 3, t.PLANKS_JUNGLE); // jungle
	createBlock(5, 4, t.PLANKS_ACACIA); // acacia
	createBlock(5, 5, t.PLANKS_BIG_OAK); // dark oak
	// --
	// -- saplings
	createItemStyleBlock(6, 0, t.SAPLING_OAK); // oak
	createItemStyleBlock(6, 1, t.SAPLING_SPRUCE); // spruce
	createItemStyleBlock(6, 2, t.SAPLING_BIRCH); // birch
	createItemStyleBlock(6, 3, t.SAPLING_JUNGLE); // jungle
	createItemStyleBlock(6, 4, t.SAPLING_ACACIA); // acacia
	createItemStyleBlock(6, 5, t.SAPLING_ROOFED_OAK); // dark oak
	// --
	createBlock(7, 0, t.BEDROCK); // bedrock
	createWater(); // id 8, 9
	createLava(); // id 10, 11
	createBlock(12, 0, t.SAND); // sand
	createBlock(12, 1, t.RED_SAND); // red sand
	createBlock(13, 0, t.GRAVEL); // gravel
	createBlock(14, 0, t.GOLD_ORE); // gold ore
	createBlock(15, 0, t.IRON_ORE); // iron ore
	createBlock(16, 0, t.COAL_ORE); // coal ore
	// -- wood
	createWood(17, 0, t.LOG_OAK, t.LOG_OAK_TOP); // oak
	createWood(17, 1, t.LOG_SPRUCE, t.LOG_SPRUCE_TOP); // pine/spruce
	createWood(17, 2, t.LOG_BIRCH, t.LOG_BIRCH_TOP); // birch
	createWood(17, 3, t.LOG_JUNGLE, t.LOG_JUNGLE_TOP); // jungle
	// --
	createLeaves(); // id 18
	createBlock(19, 0, t.SPONGE); // sponge
	createBlock(19, 1, t.SPONGE_WET); // wet sponge
	createGlass(20, 0, t.GLASS);
	createBlock(21, 0, t.LAPIS_ORE); // lapis lazuli ore
	createBlock(22, 0, t.LAPIS_BLOCK); // lapis lazuli block
	createDispenserDropper(23, t.DISPENSER_FRONT_HORIZONTAL); // dispenser
	// -- sandstone
	createBlock(24, 0, t.SANDSTONE_NORMAL, t.SANDSTONE_TOP); // normal
	createBlock(24, 1, t.SANDSTONE_CARVED, t.SANDSTONE_TOP); // chiseled
	createBlock(24, 2, t.SANDSTONE_SMOOTH, t.SANDSTONE_TOP); // smooth
	// --
	createBlock(25, 0, t.NOTEBLOCK); // noteblock
	createBed(resources.getBedTextures()); // id 26 bed
	createStraightRails(27, 0, t.RAIL_GOLDEN); // id 27 powered rail (unpowered)
	createStraightRails(27, 8, t.RAIL_GOLDEN_POWERED); // id 27 powered rail (powered)
	createStraightRails(28, 0, t.RAIL_ACTIVATOR); // id 28 detector rail
	createPiston(29, true); // sticky piston
	createItemStyleBlock(30, 0, t.WEB); // cobweb
	// -- tall grass
	createItemStyleBlock(31, 0, t.DEADBUSH); // dead bush style
	createItemStyleBlock(31, 1, t.TALLGRASS); // tall grass
	createItemStyleBlock(31, 2, t.FERN); // fern
	// --
	createItemStyleBlock(32, 0, t.DEADBUSH); // dead bush
	createPiston(33, false); // piston
	// id 34 // piston extension
	// -- wool
	createBlock(35, 0, t.WOOL_COLORED_WHITE); // white
	createBlock(35, 1, t.WOOL_COLORED_ORANGE); // orange
	createBlock(35, 2, t.WOOL_COLORED_MAGENTA); // magenta
	createBlock(35, 3, t.WOOL_COLORED_LIGHT_BLUE); // light blue
	createBlock(35, 4, t.WOOL_COLORED_YELLOW); // yellow
	createBlock(35, 5, t.WOOL_COLORED_LIME); // lime
	createBlock(35, 6, t.WOOL_COLORED_PINK); // pink
	createBlock(35, 7, t.WOOL_COLORED_GRAY); // gray
	createBlock(35, 8, t.WOOL_COLORED_SILVER); // light gray
	createBlock(35, 9, t.WOOL_COLORED_CYAN); // cyan
	createBlock(35, 10, t.WOOL_COLORED_PURPLE); // purple
	createBlock(35, 11, t.WOOL_COLORED_BLUE); // blue
	createBlock(35, 12, t.WOOL_COLORED_BROWN); // brown
	createBlock(35, 13, t.WOOL_COLORED_GREEN); // green
	createBlock(35, 14, t.WOOL_COLORED_RED); // red
	createBlock(35, 15, t.WOOL_COLORED_BLACK); // black
	// --
	createBlock(36, 0, empty_texture); // block moved by piston aka 'block 36'
	createItemStyleBlock(37, 0, t.FLOWER_DANDELION); // dandelion
	// -- poppy -- different flowers
	createItemStyleBlock(38, 0, t.FLOWER_ROSE); // poppy
	createItemStyleBlock(38, 1, t.FLOWER_BLUE_ORCHID); // blue orchid
	createItemStyleBlock(38, 2, t.FLOWER_ALLIUM); // azure bluet
	createItemStyleBlock(38, 3, t.FLOWER_HOUSTONIA); //
	createItemStyleBlock(38, 4, t.FLOWER_TULIP_RED); // red tulip
	createItemStyleBlock(38, 5, t.FLOWER_TULIP_ORANGE); // orange tulip
	createItemStyleBlock(38, 6, t.FLOWER_TULIP_WHITE); // white tulip
	createItemStyleBlock(38, 7, t.FLOWER_TULIP_PINK); // pink tulip
	createItemStyleBlock(38, 8, t.FLOWER_OXEYE_DAISY); // oxeye daisy
	// --
	createItemStyleBlock(39, 0, t.MUSHROOM_BROWN); // brown mushroom
	createItemStyleBlock(40, 0, t.MUSHROOM_RED); // red mushroom
	createBlock(41, 0, t.GOLD_BLOCK); // block of gold
	createBlock(42, 0, t.IRON_BLOCK); // block of iron
	createSlabs(43, SlabType::STONE, true); // double stone slabs
	createSlabs(44, SlabType::STONE, false); // normal stone slabs
	createBlock(45, 0, t.BRICK); // bricks
	createBlock(46, 0, t.TNT_SIDE, t.TNT_TOP); // tnt
	createBlock(47, 0, t.BOOKSHELF, t.PLANKS_OAK); // bookshelf
	createBlock(48, 0, t.COBBLESTONE_MOSSY); // moss stone
	createBlock(49, 0, t.OBSIDIAN); // obsidian
	createTorch(50, t.TORCH_ON); // torch
	createItemStyleBlock(51, 0, t.FIRE_LAYER_0); // fire
	createBlock(52, 0, t.MOB_SPAWNER); // monster spawner
	createStairs(53, t.PLANKS_OAK); // oak wood stairs
	createChest(54, resources.getNormalChest()); // chest
	createDoubleChest(54, resources.getNormalDoubleChest()); // chest
	createRedstoneWire(55, 0, 48, 0, 0); // redstone wire not powered
	createRedstoneWire(55, REDSTONE_POWERED, 192, 0, 0); // redstone wire powered
	createBlock(56, 0, t.DIAMOND_ORE); // diamond ore
	createBlock(57, 0, t.DIAMOND_BLOCK); // block of diamond
	createBlock(58, 0, t.CRAFTING_TABLE_SIDE, t.CRAFTING_TABLE_FRONT, t.CRAFTING_TABLE_TOP); // crafting table
	// -- wheat
	createItemStyleBlock(59, 0, t.WHEAT_STAGE_0); //
	createItemStyleBlock(59, 1, t.WHEAT_STAGE_1); //
	createItemStyleBlock(59, 2, t.WHEAT_STAGE_2); //
	createItemStyleBlock(59, 3, t.WHEAT_STAGE_3); //
	createItemStyleBlock(59, 4, t.WHEAT_STAGE_4); //
	createItemStyleBlock(59, 5, t.WHEAT_STAGE_5); //
	createItemStyleBlock(59, 6, t.WHEAT_STAGE_6); //
	createItemStyleBlock(59, 7, t.WHEAT_STAGE_7); //
	// --
	createBlock(60, 0, t.DIRT, t.FARMLAND_WET); // farmland
	createRotatedBlock(61, 0, t.FURNACE_FRONT_OFF, t.FURNACE_SIDE, t.FURNACE_TOP); // furnace
	createRotatedBlock(62, 0, t.FURNACE_FRONT_ON, t.FURNACE_SIDE, t.FURNACE_TOP); // burning furnace
	createSign(); // id 63 // sign post
	createDoor(64, t.DOOR_WOOD_LOWER, t.DOOR_WOOD_UPPER); // wooden door
	// -- ladders
	createSingleFaceBlock(65, 2, FACE_SOUTH, t.LADDER);
	createSingleFaceBlock(65, 3, FACE_NORTH, t.LADDER);
	createSingleFaceBlock(65, 4, FACE_EAST, t.LADDER);
	createSingleFaceBlock(65, 5, FACE_WEST, t.LADDER);
	// --
	createRails(); // id 66
	createStairs(67, t.COBBLESTONE); // cobblestone stairs
	createWallSign(); // id 68 // wall sign
	// id 69 // lever
	createSmallerBlock(70, 0, t.STONE, t.STONE, 0, 1); // stone pressure plate
	createDoor(71, t.DOOR_IRON_LOWER, t.DOOR_IRON_UPPER); // iron door
	createSmallerBlock(72, 0, t.PLANKS_OAK, t.PLANKS_OAK, 0, 1); // wooden pressure plate
	createBlock(73, 0, t.REDSTONE_ORE); // redstone ore
	createBlock(74, 0, t.REDSTONE_ORE); // glowing redstone ore
	createTorch(75, t.REDSTONE_TORCH_OFF); // redstone torch off
	createTorch(76, t.REDSTONE_TORCH_ON); // redstone torch on
	createButton(77, t.STONE); // stone button
	createSnow(); // id 78
	createIce(79, 0, t.ICE); // ice block
	createBlock(80, 0, t.SNOW); // snow block
	createCactus(); // id 81
	createBlock(82, 0, t.CLAY); // clay block
	createItemStyleBlock(83, 0, t.REEDS); // sugar cane
	createBlock(84, 0, t.NOTEBLOCK, t.JUKEBOX_TOP.rotate(1)); // jukebox
	createFence(85, 0, t.PLANKS_OAK); // oak fence
	createPumkin(86, t.PUMPKIN_FACE_OFF); // pumpkin
	createBlock(87, 0, t.NETHERRACK); // netherrack
	createBlock(88, 0, t.SOUL_SAND); // soul sand
	createBlock(89, 0, t.GLOWSTONE); // glowstone block
	createBlock(90, 0, t.PORTAL); // nether portal block
	createPumkin(91, t.PUMPKIN_FACE_ON); // jack-o-lantern
	createCake(); // id 92
	createRedstoneRepeater(93, t.REPEATER_OFF); // redstone repeater off
	createRedstoneRepeater(94, t.REPEATER_ON); // redstone repeater on
	// stained glass --
	createGlass(95, 0, t.GLASS_WHITE);
	createGlass(95, 1, t.GLASS_ORANGE);
	createGlass(95, 2, t.GLASS_MAGENTA);
	createGlass(95, 3, t.GLASS_LIGHT_BLUE);
	createGlass(95, 4, t.GLASS_YELLOW);
	createGlass(95, 5, t.GLASS_LIME);
	createGlass(95, 6, t.GLASS_PINK);
	createGlass(95, 7, t.GLASS_GRAY);
	createGlass(95, 8, t.GLASS_SILVER);
	createGlass(95, 9, t.GLASS_CYAN);
	createGlass(95, 10, t.GLASS_PURPLE);
	createGlass(95, 11, t.GLASS_BLUE);
	createGlass(95, 12, t.GLASS_BROWN);
	createGlass(95, 13, t.GLASS_GREEN);
	createGlass(95, 14, t.GLASS_RED);
	createGlass(95, 15, t.GLASS_BLACK);
	// --
	createTrapdoor(96, t.TRAPDOOR); // trapdoor
	// -- monster egg
	createBlock(97, 0, t.STONE); // stone
	createBlock(97, 1, t.COBBLESTONE); // cobblestone
	createBlock(97, 2, t.STONEBRICK); // stone brick
	// --
	// -- stone bricks
	createBlock(98, 0, t.STONEBRICK); // normal
	createBlock(98, 1, t.STONEBRICK_MOSSY); // mossy
	createBlock(98, 2, t.STONEBRICK_CRACKED); // cracked
	createBlock(98, 3, t.STONEBRICK_CARVED); // chiseled
	// --
	createHugeMushroom(99, t.MUSHROOM_BLOCK_SKIN_BROWN); // huge brown mushroom
	createHugeMushroom(100, t.MUSHROOM_BLOCK_SKIN_RED); // huge red mushroom
	createBarsPane(101, 0, t.IRON_BARS); // iron bars
	createBarsPane(102, 0, t.GLASS); // glass pane
	createBlock(103, 0, t.MELON_SIDE, t.MELON_TOP); // melon
	createStem(104); // pumpkin stem
	createStem(105); // melon stem
	createVines(); // id 106 // vines
	createFenceGate(107, t.PLANKS_OAK); // oak fence gate
	createStairs(108, t.BRICK); // brick stairs
	createStairs(109, t.STONEBRICK); // stone brick stairs
	createBlock(110, 0, t.MYCELIUM_SIDE, t.MYCELIUM_TOP); // mycelium
	// -- lily pad
	createSingleFaceBlock(111, 0, FACE_BOTTOM, t.WATERLILY.rotate(3));
	createSingleFaceBlock(111, 1, FACE_BOTTOM, t.WATERLILY.rotate(2));
	createSingleFaceBlock(111, 2, FACE_BOTTOM, t.WATERLILY.rotate(1));
	createSingleFaceBlock(111, 3, FACE_BOTTOM, t.WATERLILY);
	// --
	createBlock(112, 0, t.NETHER_BRICK); // nether brick
	createFence(113, 0, t.NETHER_BRICK); // nether brick fence
	createStairs(114, t.NETHER_BRICK); // nether brick stairs
	// -- nether wart
	createItemStyleBlock(115, 0, t.NETHER_WART_STAGE_0);
	createItemStyleBlock(115, 1, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 2, t.NETHER_WART_STAGE_1);
	createItemStyleBlock(115, 3, t.NETHER_WART_STAGE_2);
	// --
	createSmallerBlock(116, 0, t.ENCHANTING_TABLE_SIDE,
			t.ENCHANTING_TABLE_TOP, 0, texture_size * 0.75); // enchantment table
	createBrewingStand(); // id 117
	createCauldron(); // id 118 // cauldron
	createSmallerBlock(119, 0, resources.getEndportalTexture(), resources.getEndportalTexture(),
			texture_size * 0.25, texture_size * 0.75); // end portal
	createSmallerBlock(120, 0, t.ENDFRAME_SIDE, t.ENDFRAME_TOP, 0,
			texture_size * 0.8125); // end portal frame
	createBlock(121, 0, t.END_STONE); // end stone
	createDragonEgg(); // id 122
	createBlock(123, 0, t.REDSTONE_LAMP_OFF); // redstone lamp inactive
	createBlock(124, 0, t.REDSTONE_LAMP_ON); // redstone lamp active
	createSlabs(125, SlabType::WOOD, true); // double wooden slabs
	createSlabs(126, SlabType::WOOD, false); // normal wooden slabs
	createCocoas(); // id 127
	createStairs(128, t.SANDSTONE_NORMAL, t.SANDSTONE_TOP); // sandstone stairs
	createBlock(129, 0, t.EMERALD_ORE); // emerald ore
	createChest(130, resources.getEnderChest()); // ender chest
	createTripwireHook(); // tripwire hook
	createRedstoneWire(132, 0, 192, 192, 192); // tripwire
	createBlock(133, 0, t.EMERALD_BLOCK); // block of emerald
	createStairs(134, t.PLANKS_SPRUCE); // spruce wood stairs
	createStairs(135, t.PLANKS_BIRCH); // birch wood stairs
	createStairs(136, t.PLANKS_JUNGLE); // jungle wood stairs
	createCommandBlock(137, t.COMMAND_BLOCK_FRONT, t.COMMAND_BLOCK_BACK,
			t.COMMAND_BLOCK_SIDE, t.COMMAND_BLOCK_CONDITIONAL); // id 137
	createBeacon(); // beacon
	createFence(139, 0, t.COBBLESTONE); // cobblestone wall
	createFence(139, 1, t.COBBLESTONE_MOSSY); // cobblestone wall mossy
	createFlowerPot(); // id 140
	// carrots --
	createItemStyleBlock(141, 0, t.CARROTS_STAGE_0);
	createItemStyleBlock(141, 1, t.CARROTS_STAGE_0);
	createItemStyleBlock(141, 2, t.CARROTS_STAGE_1);
	createItemStyleBlock(141, 3, t.CARROTS_STAGE_1);
	createItemStyleBlock(141, 4, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 5, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 6, t.CARROTS_STAGE_2);
	createItemStyleBlock(141, 7, t.CARROTS_STAGE_3);
	// --
	// potatoes --
	createItemStyleBlock(142, 0, t.POTATOES_STAGE_0);
	createItemStyleBlock(142, 1, t.POTATOES_STAGE_0);
	createItemStyleBlock(142, 2, t.POTATOES_STAGE_1);
	createItemStyleBlock(142, 3, t.POTATOES_STAGE_1);
	createItemStyleBlock(142, 4, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 5, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 6, t.POTATOES_STAGE_2);
	createItemStyleBlock(142, 7, t.POTATOES_STAGE_3);
	// --
	createButton(143, t.PLANKS_OAK); // wooden button
	// id 144 // head
	// id 145 // anvil
	createChest(146, resources.getTrappedChest()); // trapped chest
	createDoubleChest(146, resources.getTrappedDoubleChest()); // double trapped chest
	createSmallerBlock(147, 0, t.GOLD_BLOCK, t.GOLD_BLOCK, 0, 1); // weighted pressure plate (light)
	createSmallerBlock(148, 0, t.QUARTZ_BLOCK_LINES, t.QUARTZ_BLOCK_LINES, 0, 1); // weighted pressure plate (heavy)
	createRedstoneRepeater(149, t.COMPARATOR_OFF); // redstone comparator (inactive) // TODO
	createRedstoneRepeater(150, t.COMPARATOR_ON); // redstone comparator (active) // TODO
	createSmallerBlock(151, 0, t.DAYLIGHT_DETECTOR_SIDE, t.DAYLIGHT_DETECTOR_TOP, 0, 8); // daylight sensor
	createBlock(152, 0, t.REDSTONE_BLOCK); // block of redstone
	createBlock(153, 0, t.QUARTZ_ORE); // nether quartz ore
	createHopper(); // id 154
		// block of quartz --
	createBlock(155, 0, t.QUARTZ_BLOCK_SIDE, t.QUARTZ_BLOCK_TOP);
	createBlock(155, 1, t.QUARTZ_BLOCK_CHISELED, t.QUARTZ_BLOCK_CHISELED_TOP);
	createBlock(155, 2, t.QUARTZ_BLOCK_LINES, t.QUARTZ_BLOCK_LINES_TOP);
	createBlock(155, 3, t.QUARTZ_BLOCK_LINES_TOP, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90), t.QUARTZ_BLOCK_LINES);
	createBlock(155, 4, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90), t.QUARTZ_BLOCK_LINES_TOP, t.QUARTZ_BLOCK_LINES.rotate(ROTATE_90));
	// --
	createStairs(156, t.QUARTZ_BLOCK_SIDE); // quartz stairs
	createStraightRails(157, 0, t.RAIL_ACTIVATOR); // activator rail
	createDispenserDropper(158, t.DROPPER_FRONT_HORIZONTAL); // dropper
	// stained clay --
	createBlock(159, 0, t.HARDENED_CLAY_STAINED_WHITE);
	createBlock(159, 1, t.HARDENED_CLAY_STAINED_ORANGE);
	createBlock(159, 2, t.HARDENED_CLAY_STAINED_MAGENTA);
	createBlock(159, 3, t.HARDENED_CLAY_STAINED_LIGHT_BLUE);
	createBlock(159, 4, t.HARDENED_CLAY_STAINED_YELLOW);
	createBlock(159, 5, t.HARDENED_CLAY_STAINED_LIME);
	createBlock(159, 6, t.HARDENED_CLAY_STAINED_PINK);
	createBlock(159, 7, t.HARDENED_CLAY_STAINED_GRAY);
	createBlock(159, 8, t.HARDENED_CLAY_STAINED_SILVER);
	createBlock(159, 9, t.HARDENED_CLAY_STAINED_CYAN);
	createBlock(159, 10, t.HARDENED_CLAY_STAINED_PURPLE);
	createBlock(159, 11, t.HARDENED_CLAY_STAINED_BLUE);
	createBlock(159, 12, t.HARDENED_CLAY_STAINED_BROWN);
	createBlock(159, 13, t.HARDENED_CLAY_STAINED_GREEN);
	createBlock(159, 14, t.HARDENED_CLAY_STAINED_RED);
	createBlock(159, 15, t.HARDENED_CLAY_STAINED_BLACK);
	// --
	// stained glass pane --
	createBarsPane(160, 0, t.GLASS_WHITE);
	createBarsPane(160, 1, t.GLASS_ORANGE);
	createBarsPane(160, 2, t.GLASS_MAGENTA);
	createBarsPane(160, 3, t.GLASS_LIGHT_BLUE);
	createBarsPane(160, 4, t.GLASS_YELLOW);
	createBarsPane(160, 5, t.GLASS_LIME);
	createBarsPane(160, 6, t.GLASS_PINK);
	createBarsPane(160, 7, t.GLASS_GRAY);
	createBarsPane(160, 8, t.GLASS_SILVER);
	createBarsPane(160, 9, t.GLASS_CYAN);
	createBarsPane(160, 10, t.GLASS_PURPLE);
	createBarsPane(160, 11, t.GLASS_BLUE);
	createBarsPane(160, 12, t.GLASS_BROWN);
	createBarsPane(160, 13, t.GLASS_GREEN);
	createBarsPane(160, 14, t.GLASS_RED);
	createBarsPane(160, 15, t.GLASS_BLACK);
	// --
	// id 161 acacia/dark oak leaves, see createLeaves()
	// some more wood --
	createWood(162, 0, t.LOG_ACACIA, t.LOG_ACACIA_TOP); // acacia
	createWood(162, 1, t.LOG_BIG_OAK, t.LOG_BIG_OAK_TOP); // acacia (placeholder)
	createWood(162, 2, t.LOG_ACACIA, t.LOG_ACACIA_TOP); // dark wood
	createWood(162, 3, t.LOG_BIG_OAK, t.LOG_BIG_OAK_TOP); // dark wood (placeholder)
	// --
	createStairs(163, t.PLANKS_ACACIA); // acacia wood stairs
	createStairs(164, t.PLANKS_BIG_OAK); // dark oak wood stairs
	createBlock(165, 0, t.SLIME); // slime block
	createBlock(166, 0, empty_texture); // barrier
	createTrapdoor(167, t.IRON_TRAPDOOR); // iron trapdoor
	// prismarine --
	createBlock(168, 0, t.PRISMARINE_ROUGH);
	createBlock(168, 1, t.PRISMARINE_BRICKS);
	createBlock(168, 2, t.PRISMARINE_DARK);
	// --
	createBlock(169, 0, t.SEA_LANTERN); // sea lantern
	// hay block --
	createBlock(170, 0, t.HAY_BLOCK_SIDE, t.HAY_BLOCK_TOP); // normal orientation
	createBlock(170, 4, t.HAY_BLOCK_TOP, t.HAY_BLOCK_SIDE.rotate(1), t.HAY_BLOCK_SIDE); // east-west
	createBlock(170, 8, t.HAY_BLOCK_SIDE.rotate(1), t.HAY_BLOCK_TOP, t.HAY_BLOCK_SIDE.rotate(1)); // north-south
	// --
	// carpet --
	createSmallerBlock(171, 0, t.WOOL_COLORED_WHITE, 0, 1);
	createSmallerBlock(171, 1, t.WOOL_COLORED_ORANGE, 0, 1);
	createSmallerBlock(171, 2, t.WOOL_COLORED_MAGENTA, 0, 1);
	createSmallerBlock(171, 3, t.WOOL_COLORED_LIGHT_BLUE, 0, 1);
	createSmallerBlock(171, 4, t.WOOL_COLORED_YELLOW, 0, 1);
	createSmallerBlock(171, 5, t.WOOL_COLORED_LIME, 0, 1);
	createSmallerBlock(171, 6, t.WOOL_COLORED_PINK, 0, 1);
	createSmallerBlock(171, 7, t.WOOL_COLORED_GRAY, 0, 1);
	createSmallerBlock(171, 8, t.WOOL_COLORED_SILVER, 0, 1);
	createSmallerBlock(171, 9, t.WOOL_COLORED_CYAN, 0, 1);
	createSmallerBlock(171, 10, t.WOOL_COLORED_PURPLE, 0, 1);
	createSmallerBlock(171, 11, t.WOOL_COLORED_BLUE, 0, 1);
	createSmallerBlock(171, 12, t.WOOL_COLORED_BROWN, 0, 1);
	createSmallerBlock(171, 13, t.WOOL_COLORED_GREEN, 0, 1);
	createSmallerBlock(171, 14, t.WOOL_COLORED_RED, 0, 1);
	createSmallerBlock(171, 15, t.WOOL_COLORED_BLACK, 0, 1);
	// --
	createBlock(172, 0, t.HARDENED_CLAY); // hardened clay
	createBlock(173, 0, t.COAL_BLOCK); // block of coal
	createBlock(174, 0, t.ICE_PACKED); // packed ice
	// large plants, id 175 --
	// the top texture of the sunflower is a bit modified
	RGBAImage sunflower_top = t.DOUBLE_PLANT_SUNFLOWER_TOP;
	sunflower_top.alphaBlit(t.DOUBLE_PLANT_SUNFLOWER_FRONT, 0, -texture_size * 0.25);
	createLargePlant(0, t.DOUBLE_PLANT_SUNFLOWER_BOTTOM, sunflower_top);
	createLargePlant(1, t.DOUBLE_PLANT_SYRINGA_BOTTOM, t.DOUBLE_PLANT_SYRINGA_TOP);
	createLargePlant(2, t.DOUBLE_PLANT_GRASS_BOTTOM, t.DOUBLE_PLANT_GRASS_TOP);
	createLargePlant(3, t.DOUBLE_PLANT_FERN_BOTTOM, t.DOUBLE_PLANT_FERN_TOP);
	createLargePlant(4, t.DOUBLE_PLANT_ROSE_BOTTOM, t.DOUBLE_PLANT_ROSE_TOP);
	createLargePlant(5, t.DOUBLE_PLANT_PAEONIA_BOTTOM, t.DOUBLE_PLANT_PAEONIA_TOP);
	// --
	// id 176 // standing banner
	// id 177 // wall banner
	createSmallerBlock(178, 0, t.DAYLIGHT_DETECTOR_SIDE, t.DAYLIGHT_DETECTOR_INVERTED_TOP, 0, 8); // inverted daylight sensor
	// -- red sandstone
	createBlock(179, 0, t.RED_SANDSTONE_NORMAL, t.RED_SANDSTONE_TOP); // normal
	createBlock(179, 1, t.RED_SANDSTONE_CARVED, t.RED_SANDSTONE_TOP); // chiseled
	createBlock(179, 2, t.RED_SANDSTONE_SMOOTH, t.RED_SANDSTONE_TOP); // smooth
	// --
	createStairs(180, t.RED_SANDSTONE_NORMAL, t.RED_SANDSTONE_TOP); // red sandstone stairs
	createSlabs(181, SlabType::STONE2, true); // double red sandstone slabs
	createSlabs(182, SlabType::STONE2, false); // normal red sandstone slabs
	createFenceGate(183, t.PLANKS_SPRUCE); // spruce fence gate
	createFenceGate(184, t.PLANKS_BIRCH); // birch fence gate
	createFenceGate(185, t.PLANKS_JUNGLE); // jungle fence gate
	createFenceGate(186, t.PLANKS_BIG_OAK); // dark oak fence gate
	createFenceGate(187, t.PLANKS_ACACIA); // acacia fence gate
	createFence(188, 0, t.PLANKS_SPRUCE); // spruce fence
	createFence(189, 0, t.PLANKS_BIRCH); // birch fence
	createFence(190, 0, t.PLANKS_JUNGLE); // jungle fence
	createFence(191, 0, t.PLANKS_BIG_OAK); // dark oak fence
	createFence(192, 0, t.PLANKS_ACACIA); // acacia fence
	createDoor(193, t.DOOR_SPRUCE_LOWER, t.DOOR_SPRUCE_UPPER); // spruce door
	createDoor(194, t.DOOR_BIRCH_LOWER, t.DOOR_BIRCH_UPPER); // birch door
	createDoor(195, t.DOOR_JUNGLE_LOWER, t.DOOR_JUNGLE_UPPER); // jungle door
	createDoor(196, t.DOOR_ACACIA_LOWER, t.DOOR_ACACIA_UPPER); // acacia door
	createDoor(197, t.DOOR_DARK_OAK_LOWER, t.DOOR_DARK_OAK_UPPER); // dark oak door
	createEndRod(); // id 198
	createBlock(199, 0, t.CHORUS_PLANT); // chrous plant
	// chorus flower --
	createBlock(200, 0, t.CHORUS_FLOWER);
	createBlock(200, 1, t.CHORUS_FLOWER);
	createBlock(200, 2, t.CHORUS_FLOWER);
	createBlock(200, 3, t.CHORUS_FLOWER);
	createBlock(200, 4, t.CHORUS_FLOWER);
	createBlock(200, 5, t.CHORUS_FLOWER_DEAD);
	// --
	createBlock(201, 0, t.PURPUR_BLOCK); // purpur block
	// purpur pillar --
	// TODO is the official data like this or are there also other combination? 0, 4, 8 seems odd...
	createBlock(202, 0, t.PURPUR_PILLAR, t.PURPUR_PILLAR_TOP); // vertically
	createBlock(202, 4, t.PURPUR_PILLAR_TOP, t.PURPUR_PILLAR); // east-west
	createBlock(202, 8, t.PURPUR_PILLAR_TOP, t.PURPUR_PILLAR.rotate(1)); // north-south
	// --
	createStairs(203, t.PURPUR_BLOCK); // purpur stairs
	createSlabs(204, SlabType::PURPUR, true); // purpur double slab
	createSlabs(205, SlabType::PURPUR, false); // purpur slab
	createBlock(206, 0, t.END_BRICKS); // end stone bricks
	// beetroot seeds --
	createItemStyleBlock(207, 0, t.BEETROOTS_STAGE_0);
	createItemStyleBlock(207, 2, t.BEETROOTS_STAGE_2);
	createItemStyleBlock(207, 3, t.BEETROOTS_STAGE_3);
	// --
	createSmallerBlock(208, 0, t.GRASS_PATH_SIDE, t.GRASS_PATH_TOP, 0, texture_size * 15.0 / 16.0); // grass paths
	createBlock(209, 0, resources.getEndportalTexture()); // end gateway
	createCommandBlock(210, t.REPEATING_COMMAND_BLOCK_FRONT, t.REPEATING_COMMAND_BLOCK_BACK,
			t.REPEATING_COMMAND_BLOCK_SIDE, t.REPEATING_COMMAND_BLOCK_CONDITIONAL); // id 210
	createCommandBlock(211, t.CHAIN_COMMAND_BLOCK_FRONT, t.CHAIN_COMMAND_BLOCK_BACK,
			t.CHAIN_COMMAND_BLOCK_SIDE, t.CHAIN_COMMAND_BLOCK_CONDITIONAL); // id 211
	// frosted ice --
	createIce(212, 0, t.FROSTED_ICE_0);
	createIce(212, 1, t.FROSTED_ICE_1);
	createIce(212, 2, t.FROSTED_ICE_2);
	createIce(212, 3, t.FROSTED_ICE_3);
	// --
	createBlock(213, 0, t.MAGMA); // magma
	createBlock(214, 0, t.NETHER_WART_BLOCK); // nether wart block
	createBlock(215, 0, t.RED_NETHER_BRICK); // red nether brick
	// bone block --
	createBlock(216, 0, t.BONE_BLOCK_SIDE, t.BONE_BLOCK_TOP); // vertically
	createBlock(216, 4, t.BONE_BLOCK_TOP, t.BONE_BLOCK_SIDE, t.BONE_BLOCK_SIDE); // east-west
	createBlock(216, 8, t.BONE_BLOCK_SIDE, t.BONE_BLOCK_TOP, t.BONE_BLOCK_SIDE); // north-south
	// --
	createObserver(218); // observer
	// shulker box --
    createShulkerBox(219, 0, resources.getShulkerBoxTextures()); // white
    createShulkerBox(220, 1, resources.getShulkerBoxTextures()); // orange
    createShulkerBox(221, 2, resources.getShulkerBoxTextures()); // magenta
    createShulkerBox(222, 3, resources.getShulkerBoxTextures()); // light blue
    createShulkerBox(223, 4, resources.getShulkerBoxTextures()); // yellow
    createShulkerBox(224, 5, resources.getShulkerBoxTextures()); // lime
    createShulkerBox(225, 6, resources.getShulkerBoxTextures()); // pink
    createShulkerBox(226, 7, resources.getShulkerBoxTextures()); // gray
    createShulkerBox(227, 8, resources.getShulkerBoxTextures()); // light gray
    createShulkerBox(228, 9, resources.getShulkerBoxTextures()); // cyan
    createShulkerBox(229, 10, resources.getShulkerBoxTextures()); // purple
    createShulkerBox(230, 11, resources.getShulkerBoxTextures()); // blue
    createShulkerBox(231, 12, resources.getShulkerBoxTextures()); // brown
    createShulkerBox(232, 13, resources.getShulkerBoxTextures()); // green
    createShulkerBox(233, 14, resources.getShulkerBoxTextures()); // red
    createShulkerBox(234, 15, resources.getShulkerBoxTextures()); // black
    // glazed terracotta --
    createGlazedTerracotta(235, t.GLAZED_TERRACOTTA_WHITE); // white
	createGlazedTerracotta(236, t.GLAZED_TERRACOTTA_ORANGE); // orange
	createGlazedTerracotta(237, t.GLAZED_TERRACOTTA_MAGENTA); // magenta
	createGlazedTerracotta(238, t.GLAZED_TERRACOTTA_LIGHT_BLUE); // light blue
	createGlazedTerracotta(239, t.GLAZED_TERRACOTTA_YELLOW); // yellow
	createGlazedTerracotta(240, t.GLAZED_TERRACOTTA_LIME); // lime
	createGlazedTerracotta(241, t.GLAZED_TERRACOTTA_PINK); // pink
	createGlazedTerracotta(242, t.GLAZED_TERRACOTTA_GRAY); // gray
	createGlazedTerracotta(243, t.GLAZED_TERRACOTTA_SILVER); // light gray
	createGlazedTerracotta(244, t.GLAZED_TERRACOTTA_CYAN); // cyan
	createGlazedTerracotta(245, t.GLAZED_TERRACOTTA_PURPLE); // purple
	createGlazedTerracotta(246, t.GLAZED_TERRACOTTA_BLUE); // blue
	createGlazedTerracotta(247, t.GLAZED_TERRACOTTA_BROWN); // brown
	createGlazedTerracotta(248, t.GLAZED_TERRACOTTA_GREEN); // green
	createGlazedTerracotta(249, t.GLAZED_TERRACOTTA_RED); // red
	createGlazedTerracotta(250, t.GLAZED_TERRACOTTA_BLACK); // black
	// concrete --
	createBlock(251, 0, t.CONCRETE_WHITE); // white
	createBlock(251, 1, t.CONCRETE_ORANGE); // orange
	createBlock(251, 2, t.CONCRETE_MAGENTA); // magenta
	createBlock(251, 3, t.CONCRETE_LIGHT_BLUE); // light blue
	createBlock(251, 4, t.CONCRETE_YELLOW); // yellow
	createBlock(251, 5, t.CONCRETE_LIME); // lime
	createBlock(251, 6, t.CONCRETE_PINK); // pink
	createBlock(251, 7, t.CONCRETE_GRAY); // gray
	createBlock(251, 8, t.CONCRETE_SILVER); // light gray
	createBlock(251, 9, t.CONCRETE_CYAN); // cyan
	createBlock(251, 10, t.CONCRETE_PURPLE); // purple
	createBlock(251, 11, t.CONCRETE_BLUE); // blue
	createBlock(251, 12, t.CONCRETE_BROWN); // brown
	createBlock(251, 13, t.CONCRETE_GREEN); // green
	createBlock(251, 14, t.CONCRETE_RED); // red
	createBlock(251, 15, t.CONCRETE_BLACK); // black
	// concrete powder --
	createBlock(252, 0, t.CONCRETE_POWDER_WHITE); // white
	createBlock(252, 1, t.CONCRETE_POWDER_ORANGE); // orange
	createBlock(252, 2, t.CONCRETE_POWDER_MAGENTA); // magenta
	createBlock(252, 3, t.CONCRETE_POWDER_LIGHT_BLUE); // light blue
	createBlock(252, 4, t.CONCRETE_POWDER_YELLOW); // yellow
	createBlock(252, 5, t.CONCRETE_POWDER_LIME); // lime
	createBlock(252, 6, t.CONCRETE_POWDER_PINK); // pink
	createBlock(252, 7, t.CONCRETE_POWDER_GRAY); // gray
	createBlock(252, 8, t.CONCRETE_POWDER_SILVER); // light gray
	createBlock(252, 9, t.CONCRETE_POWDER_CYAN); // cyan
	createBlock(252, 10, t.CONCRETE_POWDER_PURPLE); // purple
	createBlock(252, 11, t.CONCRETE_POWDER_BLUE); // blue
	createBlock(252, 12, t.CONCRETE_POWDER_BROWN); // brown
	createBlock(252, 13, t.CONCRETE_POWDER_GREEN); // green
	createBlock(252, 14, t.CONCRETE_POWDER_RED); // red
	createBlock(252, 15, t.CONCRETE_POWDER_BLACK); // black
	// --
	createBlock(217, 0, empty_texture); // structure void
	// structure block --
	createBlock(255, 0, t.STRUCTURE_BLOCK_SAVE);
	createBlock(255, 1, t.STRUCTURE_BLOCK_LOAD);
	createBlock(255, 2, t.STRUCTURE_BLOCK_CORNER);
	createBlock(255, 3, t.STRUCTURE_BLOCK_DATA);
	// --
}

int IsometricBlockImages::createOpaqueWater() {
	// TODO see TopdownBlockImages::createOpaqueWater()
	
	// just use the Ocean biome watercolor
	RGBAImage water = resources.getBlockTextures().WATER_STILL.colorize(0, 0.39, 0.89);

	RGBAImage opaque_water[4];
	// opaque_water[0] is water block when water texture is only on the top
	opaque_water[0].setSize(getBlockSize(), getBlockSize());
	blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
	// same, water top and south (right)
	opaque_water[1] = opaque_water[0];
	// water top and west (left)
	opaque_water[2] = opaque_water[0];
	// water top, south and west
	opaque_water[3] = opaque_water[0];

	// create sides of water blocks that don't have water neighbors on those sides
	blitFace(opaque_water[1], FACE_SOUTH, water, 0, 0, true, dleft, dright);
	blitFace(opaque_water[2], FACE_WEST, water, 0, 0, true, dleft, dright);
	blitFace(opaque_water[3], FACE_SOUTH, water, 0, 0, true, dleft, dright);
	blitFace(opaque_water[3], FACE_WEST, water, 0, 0, true, dleft, dright);

	// int last_alpha = -1;
	int water_preblit;
	for (water_preblit = 2; water_preblit < 100; water_preblit++) {
		// blit another layer of water
		blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
		blitFace(opaque_water[1], FACE_TOP, water, 0, 0, false);
		blitFace(opaque_water[2], FACE_TOP, water, 0, 0, false);
		blitFace(opaque_water[3], FACE_TOP, water, 0, 0, false);

		blitFace(opaque_water[1], FACE_SOUTH, water, 0, 0, true, dleft, dright);
		blitFace(opaque_water[2], FACE_WEST, water, 0, 0, true, dleft, dright);
		blitFace(opaque_water[3], FACE_SOUTH, water, 0, 0, true, dleft, dright);
		blitFace(opaque_water[3], FACE_WEST, water, 0, 0, true, dleft, dright);

		// then check alpha
		uint8_t min_alpha = 255;
		for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
			uint8_t alpha = rgba_alpha(opaque_water[0].getPixel(it.dest_x, it.dest_y));
			if (alpha < min_alpha)
				min_alpha = alpha;
		}

		/*
		LOG(DEBUG) << water_preblit << ": " << (int) min_alpha;
		if (last_alpha == min_alpha) {
			LOG(DEBUG) << "min alpha converges!";
			break;
		}
		*/
		
		// images are "enough" opaque
		if (min_alpha >= 250)
			break;
		//last_alpha = min_alpha;
	}

	LOG(DEBUG) << "pre-blit water (isometric): " << water_preblit;

	uint16_t id = 8;
	uint16_t data = OPAQUE_WATER;
	block_images[id | (data) << 16] = opaque_water[0];
	block_images[id | (data | OPAQUE_WATER_SOUTH) << 16] = opaque_water[1];
	block_images[id | (data | OPAQUE_WATER_WEST) << 16] = opaque_water[2];
	block_images[id | (data | OPAQUE_WATER_SOUTH | OPAQUE_WATER_WEST) << 16] = opaque_water[3];

	return water_preblit;
}

std::vector<RGBAImage> IsometricBlockImages::getExportBlocks() const {
	std::map<uint32_t, RGBAImage, block_images_comparator> blocks_sorted;
	for (auto it = block_images.begin(); it != block_images.end(); ++it) {
		uint16_t data = (it->first & 0xffff0000) >> 16;
		// ignore special variants of the blocks
		if ((data & (EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM)) == 0)
			blocks_sorted[it->first] = it->second;
	}

	std::vector<RGBAImage> blocks;
	for (auto it = blocks_sorted.begin(); it != blocks_sorted.end(); ++it)
		blocks.push_back(it->second);
	return blocks;
}

}
}
