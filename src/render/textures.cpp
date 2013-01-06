/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "render/textures.h"

#include "util.h"

#include <iostream>
#include <map>
#include <cmath>
#include <cstdlib>

namespace mapcrafter {
namespace render {

FaceIterator::FaceIterator()
		: is_end(false), size(16), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::FaceIterator(int size)
		: is_end(false), size(size), src_x(0), src_y(0), dest_x(0), dest_y(0) {
}

FaceIterator::~FaceIterator() {
}

void FaceIterator::next() {
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

BlockTextures::BlockTextures()
		: texture_size(16), render_unknown_blocks(false), render_leaves_transparent(
		        false), max_water(99) {
}

BlockTextures::~BlockTextures() {
}

void BlockTextures::setSettings(int texture_size, bool render_unknown_blocks,
        bool render_leaves_transparent) {
	this->texture_size = texture_size;
	this->render_unknown_blocks = render_unknown_blocks;
	this->render_leaves_transparent = render_leaves_transparent;
}

/**
 * This function converts the chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadChestTexture(const Image& image, Image* textures, int texture_size) {
	if (image.getWidth() != image.getHeight())
		return false;
	// if the image is 64px wide, the chest images are 14x14
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	Image front = image.clip(size, 29 * ratio, size, size);
	front.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 6 * ratio, 3 * ratio);
	Image side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);
	Image top = image.clip(size, 0, size, size);

	// resize the chest images to texture size
	front.resizeInterpolated(texture_size, texture_size, textures[CHEST_FRONT]);
	side.resizeInterpolated(texture_size, texture_size, textures[CHEST_SIDE]);
	top.resizeInterpolated(texture_size, texture_size, textures[CHEST_TOP]);

	return true;
}

/**
 * This function converts the large chest image to usable chest textures and stores them
 * in the textures array.
 */
bool loadLargeChestTexture(const Image& image, Image* textures, int texture_size) {
	if (image.getWidth() != image.getHeight() * 2)
		return false;
	int ratio = image.getHeight() / 64;
	int size = ratio * 14;

	// note here that a whole chest is 30*ratio pixels wide, but our
	// chest textures are only 14x14 * ratio pixels, so we need to omit two rows in the middle
	// => the second image starts not at x*size, it starts at x*size+2*ratio
	Image front_left = image.clip(size, 29 * ratio, size, size);
	front_left.alphablit(image.clip(size, size, size, 4 * ratio), 0, 0);
	front_left.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), 13 * ratio,
	        3 * ratio);
	Image front_right = image.clip(2 * size + 2 * ratio, 29 * ratio, size, size);
	front_right.alphablit(image.clip(2 * size + 2 * ratio, size, size, 4 * ratio), 0, 0);
	front_right.alphablit(image.clip(ratio, ratio, 2 * ratio, 4 * ratio), -ratio,
	        3 * ratio);

	Image side = image.clip(0, 29 * ratio, size, size);
	side.alphablit(image.clip(0, size, size, 4 * ratio), 0, 0);

	Image top_left = image.clip(size, 0, size, size);
	Image top_right = image.clip(2 * size + 2 * ratio, 0, size, size);

	Image back_left = image.clip(4 * size + 2, 29 * ratio, size, size);
	back_left.alphablit(image.clip(4 * size + 2, size, size, 4 * ratio), 0, 0);
	Image back_right = image.clip(5 * size + 4, 29 * ratio, size, size);
	back_right.alphablit(image.clip(5 * size + 4, size, size, 4 * ratio), 0, 0);

	// resize the chest images to texture size
	front_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_FRONT_LEFT]);
	front_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_FRONT_RIGHT]);
	side.resizeInterpolated(texture_size, texture_size, textures[LARGECHEST_SIDE]);
	top_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_TOP_LEFT]);
	top_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_TOP_RIGHT]);
	back_left.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_BACK_LEFT]);
	back_right.resizeInterpolated(texture_size, texture_size,
	        textures[LARGECHEST_BACK_RIGHT]);

	return true;
}

bool BlockTextures::loadChests(const std::string& normal, const std::string& large,
        const std::string& ender) {
	Image img_chest, img_largechest, img_enderchest;
	if (!img_chest.readPNG(normal) || !img_largechest.readPNG(large)
	        || !img_enderchest.readPNG(ender))
		return false;

	if (!loadChestTexture(img_chest, chest, texture_size)
	        || !loadChestTexture(img_enderchest, enderchest, texture_size)
	        || !loadLargeChestTexture(img_largechest, largechest, texture_size))
		return false;
	return true;
}

bool BlockTextures::loadOther(const std::string& fire, const std::string& endportal) {
	return fire_texture.readPNG(fire) && endportal_texture.readPNG(endportal);
}

bool BlockTextures::loadBlocks(const std::string& terrain_filename) {
	Image terrain;
	if (!terrain.readPNG(terrain_filename))
		return false;
	if (terrain.getWidth() % 16 != 0 || terrain.getHeight() % 16 != 0
	        || terrain.getWidth() != terrain.getHeight())
		return false;
	splitTerrain(terrain);

	empty_texture.setSize(texture_size, texture_size);
	unknown_block.setSize(texture_size, texture_size);
	if (render_unknown_blocks)
		unknown_block.fill(rgba(255, 0, 0, 255), 0, 0, texture_size,
		        texture_size);

	loadBlocks();
	testWaterTransparency();
	return true;
}

/**
 * Comparator to sort the block int's with id and data.
 */
struct block_comparator {
	bool operator()(uint32_t b1, uint32_t b2) const {
		uint16_t id1 = b1 & 0xffff;
		uint16_t id2 = b2 & 0xffff;
		if (id1 != id2)
			return id1 < id2;
		uint16_t data1 = (b1 & 0xffff0000) >> 16;
		uint16_t data2 = (b2 & 0xffff0000) >> 16;
		return data1 < data2;
	}
};

bool BlockTextures::saveBlocks(const std::string& filename) {
	std::map<uint32_t, Image, block_comparator> blocks_sorted;
	for (std::unordered_map<uint32_t, Image>::const_iterator it = block_images.begin();
	        it != block_images.end(); ++it) {
		uint16_t data = (it->first & 0xffff0000) >> 16;
		if ((data & (EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM)) == 0)
			blocks_sorted[it->first] = it->second;
	}

	std::vector<Image> blocks;
	for (std::map<uint32_t, Image>::const_iterator it = blocks_sorted.begin();
	        it != blocks_sorted.end(); ++it)
		blocks.push_back(it->second);

	blocks.push_back(opaque_water[0]);
	blocks.push_back(opaque_water[1]);
	blocks.push_back(opaque_water[2]);
	blocks.push_back(opaque_water[3]);

	int blocksize = getBlockImageSize();
	int width = 16;
	int height = std::ceil(blocks.size() / (double) width);
	Image img(width * blocksize, height * blocksize);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int offset = y * width + x;
			if ((size_t) offset >= blocks.size())
				break;
			img.alphablit(blocks.at(offset), x * blocksize, y * blocksize);
		}
	}
	std::cout << block_images.size() << " blocks" << std::endl;
	std::cout << "all: " << blocks.size() << std::endl;

	/*
	Image terrain(texture_size * 16, texture_size * 16);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			terrain.simpleblit(getTexture(x, y), texture_size * x, texture_size * y);
		}
	}
	terrain.writePNG("test.png");
	*/

	return img.writePNG(filename);
}

#define POS(_x, _y) (x == _x && y == _y)

/**
 * Splits the terrain.png image into 16x16 images and resizes them to texture size.
 * The image resizing is done with bilinear interpolation, except the transparent leaves
 * textures, because with interpolation we would have half transparent pixels, what could
 * make the rendering process slower.
 */
void BlockTextures::splitTerrain(const Image& terrain) {
	int size = terrain.getWidth() / 16;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			Image texture = terrain.clip(x * size, y * size, size, size);
			if (POS(4, 3) || POS(4, 8) || POS(4, 12)/* || POS(1, 3)*//* <- glass */)
				texture.resizeSimple(texture_size, texture_size, textures[y * 16 + x]);
			else
				texture.resizeInterpolated(texture_size, texture_size,
				        textures[y * 16 + x]);
		}
	}
}

const Image& BlockTextures::getTexture(int x, int y) const {
	return textures[y * 16 + x];
}

/**
 * This method filters unnecessary block data, for example the leaves decay counter.
 */
uint16_t BlockTextures::filterBlockData(uint16_t id, uint16_t data) const {
	if (id == 6)
		return data & (0xff00 | 0b00000011);
	else if (id >= 8 && id <= 11) // water, lava
		return data & (0xff00 | 0b11110111);
	else if (id == 18) // leaves
		return data & (0xff00 | 0b00000011);
	else if (id == 26) // bed
		return data & (0xff00 | 0b00001011);
	else if (id == 60) // farmland
		return data & 0xff00;
	else if (id == 64 || id == 71) // doors
		return data & 0b1111110000;
	else if (id == 81 || id == 83 || id == 92) // cactus, sugar cane, cake
		return data & 0xff00;
	else if (id == 93 || id == 94) // redstone repeater
		return data & (0xff00 | 0b00000011);
	else if (id == 117) // brewing stand
		return data & 0xff00;
	else if (id == 119 || id == 120) // end portal, end portal frame
		return data & 0xff00;
	return data;
}

/**
 * Checks, if a block images has transparent pixels.
 */
bool BlockTextures::checkImageTransparency(const Image& image) const {
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (ALPHA(image.getPixel(it.dest_x, it.dest_y + texture_size/2)) < 255)
			return true;
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (ALPHA(image.getPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2))
		        < 255)
			return true;
	}
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		if (ALPHA(image.getPixel(it.dest_x, it.dest_y)) < 255)
			return true;
	}
	return false;
}

/**
 * This method adds to the block image the dark shadow edges by blitting the shadow edge
 * masks and then stores the block image with the special data.
 */
void BlockTextures::addBlockShadowEdges(uint16_t id, uint16_t data, const Image& block) {
	for (int n = 0; n <= 1; n++)
		for (int e = 0; e <= 1; e++)
			for (int b = 0; b <= 1; b++) {
				Image image = block;
				uint16_t extra_data = 0;
				if (n) {
					image.alphablit(shadow_edge_masks[0], 0, 0);
					extra_data |= EDGE_NORTH;
				}
				if (e) {
					image.alphablit(shadow_edge_masks[1], 0, 0);
					extra_data |= EDGE_EAST;
				}
				if (b) {
					image.alphablit(shadow_edge_masks[2], 0, 0);
					extra_data |= EDGE_BOTTOM;
				}
				block_images[id | ((data | extra_data) << 16)] = image;
			}
}

/**
 * Sets a block image in the block image list.
 */
void BlockTextures::setBlockImage(uint16_t id, uint16_t data, const Image& block) {
	block_images[id | (data << 16)] = block;

	// check if block contains transparency
	if (checkImageTransparency(block))
		block_transparency.insert(id | (data << 16));
	// if block is not transparent, add shadow edges
	else
		addBlockShadowEdges(id, data, block);
}

/**
 * This method is very important for the rendering performance. It preblits transparent
 * water blocks until they are nearly opaque.
 */
void BlockTextures::testWaterTransparency() {
	Image water = getTexture(13, 12);

	// opaque_water[0] is water block when water texture is only on the top
	opaque_water[0].setSize(getBlockImageSize(), getBlockImageSize());
	blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
	// same, water top and south (right)
	opaque_water[1] = opaque_water[0];
	// water top and west (left)
	opaque_water[2] = opaque_water[0];
	// water top, south and west
	opaque_water[3] = opaque_water[0];

	// now blit actual faces
	blitFace(opaque_water[1], FACE_SOUTH, water, 0, 0, false);
	blitFace(opaque_water[2], FACE_WEST, water, 0, 0, false);
	blitFace(opaque_water[3], FACE_SOUTH, water, 0, 0, false);
	blitFace(opaque_water[3], FACE_WEST, water, 0, 0, false);

	for (max_water = 2; max_water < 10; max_water++) {
		// make a copy of the first images
		Image tmp = opaque_water[0];
		// blit it over
		tmp.alphablit(tmp, 0, 0);

		// then check alpha
		uint16_t min_alpha = 255;
		for (TopFaceIterator it(texture_size); !it.end(); it.next())
			min_alpha = MIN(min_alpha, ALPHA(tmp.getPixel(it.dest_x, it.dest_y)));

		// images are "enough" opaque
		if (min_alpha == 255) {
			// do a last blit
			blitFace(opaque_water[0], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[1], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[2], FACE_TOP, water, 0, 0, false);
			blitFace(opaque_water[3], FACE_TOP, water, 0, 0, false);

			blitFace(opaque_water[1], FACE_SOUTH, water);
			blitFace(opaque_water[2], FACE_WEST, water);
			blitFace(opaque_water[3], FACE_SOUTH, water);
			blitFace(opaque_water[3], FACE_WEST, water);
			break;
		// when images are too transparent
		} else {
			// blit all images over
			for (int i = 0; i < 4; i++)
				opaque_water[i].alphablit(opaque_water[i], 0, 0);
		}
	}
}

uint32_t BlockTextures::darkenLeft(uint32_t pixel) const {
	return rgba_multiply(pixel, 0.75, 0.75, 0.75);
}

uint32_t BlockTextures::darkenRight(uint32_t pixel) const {
	return rgba_multiply(pixel, 0.6, 0.6, 0.6);
}

/**
 * This methods blits a specific face on an image.
 */
void BlockTextures::blitFace(Image& image, int face, const Image& texture, int xoff,
        int yoff, bool darken) const {
	double d = 1;
	if (darken) {
		if (face == FACE_SOUTH || face == FACE_NORTH)
			d = 0.6;
		else if (face == FACE_WEST || face == FACE_EAST)
			d = 0.75;
	}

	int size = texture.getWidth();

	if (face == FACE_BOTTOM || face == FACE_TOP) {
		if (face == FACE_BOTTOM)
			yoff += size;
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
			xoff += size;
		if (face == FACE_WEST || face == FACE_SOUTH)
			yoff += size / 2;
		for (SideFaceIterator it(size, itside); !it.end(); it.next()) {
			uint32_t pixel = texture.getPixel(it.src_x, it.src_y);
			image.blendPixel(rgba_multiply(pixel, d, d, d), it.dest_x + xoff,
			        it.dest_y + yoff);
		}
	}

}

Image BlockTextures::buildImage(const Image& left_texture, const Image& right_texture,
        const Image& upper_texture) {
	return buildImage(left_texture, right_texture, upper_texture, 0, 0, 0, 0, 0, 0);
}

Image BlockTextures::buildImage(const Image& left_texture, const Image& right_texture,
        const Image& upper_texture, int left_xoff, int left_yoff, int right_xoff,
        int right_yoff, int upper_xoff, int upper_yoff) {
	Image block(texture_size * 2, texture_size * 2);
	blitFace(block, FACE_WEST, left_texture, left_xoff, left_yoff);
	blitFace(block, FACE_SOUTH, right_texture, right_xoff, right_yoff);
	blitFace(block, FACE_TOP, upper_texture, upper_xoff, upper_yoff);
	return block;
}

Image BlockTextures::buildSmallerImage(const Image& left_texture,
        const Image& right_texture, const Image& upper_texture, int y1, int y2,
        bool move_only_top) {
	int size = y2 - y1;
	Image left = left_texture;
	Image right = right_texture;
	if (!move_only_top) {
		left = left_texture.move(0, texture_size - size);
		right = left_texture.move(0, texture_size - size);
	}
	return buildImage(left, right, upper_texture, 0, -y1, 0, -y1, 0, texture_size - y2);
}

Image BlockTextures::buildStairsSouth(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);

	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_x > texture_size / 2 ? 0 : texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_x >= texture_size / 2 || it.src_y >= texture_size / 2 - 1) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
	}
	return block;
}

Image BlockTextures::buildStairsNorth(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_x >= texture_size / 2 ? texture_size / 2 : 0;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_x <= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size : texture_size / 2;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size / 4;
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	return block;
}

Image BlockTextures::buildStairsWest(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_y > texture_size / 2 ? 0 : texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
	}

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_x <= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}
	return block;
}

Image BlockTextures::buildStairsEast(const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		int y = it.src_y > texture_size / 2 ? texture_size / 2 : 0;
		// fix to prevent a transparent gap
		if (it.src_y == texture_size / 2 && it.src_x % 2 == 0)
			y = texture_size / 2;
		if (it.src_y == texture_size / 2 - 1 && it.src_x % 2 == 0)
			y = texture_size / 2;
		block.setPixel(it.dest_x, it.dest_y + y, texture.getPixel(it.src_x, it.src_y));
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? 0 : texture_size / 2;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size / 4;
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_x >= texture_size / 2 || it.src_y >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockTextures::buildUpsideDownStairsNorth(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_TOP, texture);

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size / 2 : texture_size;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 - 4 : texture_size / 2;
		uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x < texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockTextures::buildUpsideDownStairsSouth(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_SOUTH, texture);
	blitFace(block, FACE_TOP, texture);
	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x >= texture_size / 2) {
			uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockTextures::buildUpsideDownStairsEast(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_TOP, texture);

	for (SideFaceIterator it(texture_size, SideFaceIterator::LEFT); !it.end();
	        it.next()) {
		int x = it.src_y >= texture_size / 2 ? texture_size / 2 : 0;
		int y = it.src_y >= texture_size / 2 ? texture_size / 2 - 4 : texture_size / 2;
		uint32_t pixel = darkenLeft(texture.getPixel(it.src_x, it.src_y));
		block.setPixel(it.dest_x + x, it.dest_y + y, pixel);
	}

	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x >= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

Image BlockTextures::buildUpsideDownStairsWest(const Image& texture) {
	Image block(getBlockImageSize(), getBlockImageSize());

	blitFace(block, FACE_WEST, texture);
	blitFace(block, FACE_TOP, texture);
	for (SideFaceIterator it(texture_size, SideFaceIterator::RIGHT); !it.end();
	        it.next()) {
		if (it.src_y <= texture_size / 2 || it.src_x <= texture_size / 2) {
			uint32_t pixel = darkenRight(texture.getPixel(it.src_x, it.src_y));
			block.setPixel(it.dest_x + texture_size, it.dest_y + texture_size / 2, pixel);
		}
	}

	return block;
}

void BlockTextures::buildCustomTextures() {
	shadow_edge_masks[0].setSize(getBlockImageSize(), getBlockImageSize());
	shadow_edge_masks[1].setSize(getBlockImageSize(), getBlockImageSize());
	shadow_edge_masks[2].setSize(getBlockImageSize(), getBlockImageSize());

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

void BlockTextures::createBlock(uint16_t id, uint16_t data, const Image& texture) {
	createBlock(id, data, texture, texture);
}

void BlockTextures::createBlock(uint16_t id, uint16_t data, const Image& side_texture,
        const Image& upper_texture) {
	setBlockImage(id, data, buildImage(side_texture, side_texture, upper_texture));
}

void BlockTextures::createBlock(uint16_t id, uint16_t data, const Image& left_texture,
        const Image& right_texture, const Image& upper_texture) {
	setBlockImage(id, data, buildImage(left_texture, right_texture, upper_texture));
}

void BlockTextures::createSmallerBlock(uint16_t id, uint16_t data,
        const Image& left_texture, const Image& right_texture, const Image& upper_texture,
        int y1, int y2, bool move_only_top) {
	setBlockImage(id, data,
	        buildSmallerImage(left_texture, right_texture, upper_texture, y1, y2,
	                move_only_top));
}

void BlockTextures::createSmallerBlock(uint16_t id, uint16_t data, const Image& side_face,
        const Image& upper_texture, int y1, int y2, bool move_only_top) {
	setBlockImage(id, data,
	        buildSmallerImage(side_face, side_face, upper_texture, y1, y2,
	                move_only_top));
}

void BlockTextures::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const Image& front_texture, const Image& side_texture,
        const Image& upper_texture) {
	createRotatedBlock(id, extra_data, front_texture, side_texture, side_texture,
	        upper_texture);
}

void BlockTextures::createRotatedBlock(uint16_t id, uint16_t extra_data,
        const Image& front_texture, const Image& back_texture, const Image& side_texture,
        const Image& upper_texture) {

	createBlock(id, 2 | extra_data, side_texture, back_texture,
	        upper_texture.rotate(ROTATE_270));
	createBlock(id, 3 | extra_data, side_texture, front_texture,
	        upper_texture.rotate(ROTATE_90));
	createBlock(id, 4 | extra_data, front_texture, side_texture,
	        upper_texture.rotate(ROTATE_180));
	createBlock(id, 5 | extra_data, back_texture, side_texture, upper_texture);
}

void BlockTextures::createItemStyleBlock(uint16_t id, uint16_t data,
        const Image& texture) {
	createItemStyleBlock(id, data, texture, texture);
}

void BlockTextures::createItemStyleBlock(uint16_t id, uint16_t data,
        const Image& north_south, const Image& east_west) {
	Image block(texture_size * 2, texture_size * 2);

	SideFaceIterator it(texture_size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		if (it.src_x > texture_size / 2)
			block.setPixel(texture_size / 2 + it.dest_x, texture_size / 4 + it.dest_y,
			        east_west.getPixel(it.src_x, it.src_y));
	}
	it = SideFaceIterator(texture_size, SideFaceIterator::LEFT);
	for (; !it.end(); it.next()) {
		uint32_t pixel = north_south.getPixel(it.src_x, it.src_y);
		if (ALPHA(pixel) != 0)
			block.setPixel(texture_size / 2 + it.dest_x, texture_size / 4 + it.dest_y,
			        pixel);
	}
	it = SideFaceIterator(texture_size, SideFaceIterator::RIGHT);
	for (; !it.end(); it.next()) {
		uint32_t pixel = east_west.getPixel(it.src_x, it.src_y);
		if (it.src_x <= texture_size / 2 && ALPHA(pixel) != 0)
			block.setPixel(texture_size / 2 + it.dest_x, texture_size / 4 + it.dest_y,
			        pixel);
	}
	setBlockImage(id, data, block);
}

void BlockTextures::createSingleFaceBlock(uint16_t id, uint16_t data, int face,
        const Image& texture) {
	Image block(texture_size * 2, texture_size * 2);
	blitFace(block, face, texture);
	setBlockImage(id, data, block);
}

void BlockTextures::createGrassBlock() { // id 2
	Image dirt = getTexture(2, 0);

	Image grass(dirt);
	Image grass_mask = getTexture(6, 2).colorize(0.3, 0.95, 0.3);
	grass.alphablit(grass_mask, 0, 0);

	Image upper = getTexture(8, 2).colorize(0.3, 0.95, 0.3);

	Image grass_block = buildImage(grass, grass, upper);
	setBlockImage(2, 0, grass_block);
}

void BlockTextures::createWater() { // id 8, 9
	Image water = getTexture(13, 12);
	for (int data = 0; data < 8; data++) {
		int smaller = data / 8.0 * texture_size;
		Image side_texture = water.move(0, smaller);

		Image block(getBlockImageSize(), getBlockImageSize());
		blitFace(block, FACE_WEST, side_texture, 0, 0, true);
		blitFace(block, FACE_SOUTH, side_texture, 0, 0, true);
		blitFace(block, FACE_TOP, water, 0, smaller, true);
		setBlockImage(8, data, block);
		setBlockImage(9, data, block);
	}

	for (int w = 0; w <= 1; w++)
		for (int s = 0; s <= 1; s++) {
			Image block(getBlockImageSize(), getBlockImageSize());
			uint16_t extra_data = 0;
			if (w == 1)
				blitFace(block, FACE_WEST, water);
			else
				extra_data |= FACE_WEST;

			if (s == 1)
				blitFace(block, FACE_SOUTH, water);
			else
				extra_data |= FACE_SOUTH;
			blitFace(block, FACE_TOP, water);
			setBlockImage(8, extra_data, block);
			setBlockImage(9, extra_data, block);
		}
}

void BlockTextures::createLava() { // id 10, 11
	Image lava = getTexture(13, 14);
	for (int data = 0; data < 7; data += 2) {
		int smaller = data / 8.0 * texture_size;
		Image side_texture = lava.move(0, smaller);
		Image block = buildImage(side_texture, side_texture, lava, 0, 0, 0, 0, 0,
		        smaller);
		setBlockImage(10, data, block);
		setBlockImage(11, data, block);
	}
}

void BlockTextures::createWood(uint16_t data, const Image& side) { // id 17
	Image top = getTexture(5, 1);
	createBlock(17, data | 4, top, side, side);
	createBlock(17, data | 8, side, top, side);
	createBlock(17, data, side, side, top); // old format
	createBlock(17, data | 4 | 8, side, side, top);
}

void BlockTextures::createLeaves() { // id 18
	if (render_leaves_transparent) {
		createBlock(18, 0, getTexture(4, 3).colorize(0.3 * 0.8, 1 * 0.8, 0.1 * 0.8)); // oak
		createBlock(18, 1, getTexture(4, 8).colorize(0.3 * 0.8, 1 * 0.8, 0.45 * 0.8)); // pine/spruce
		createBlock(18, 2, getTexture(4, 3).colorize(0.55 * 0.8, 0.9 * 0.8, 0.1 * 0.8)); // birch
		createBlock(18, 3, getTexture(4, 12).colorize(0.35 * 0.9, 1 * 0.9, 0.05 * 0.9)); // jungle
	} else {
		createBlock(18, 0, getTexture(5, 3).colorize(0.3, 1, 0.1)); // oak
		createBlock(18, 1, getTexture(5, 8).colorize(0.3, 1, 0.45)); // pine/spruce
		createBlock(18, 2, getTexture(5, 3).colorize(0.55, 0.9, 0.1)); // birch
		createBlock(18, 3, getTexture(5, 12).colorize(0.35, 1, 0.05)); // jungle
	}
}

void BlockTextures::createBed() { // id 26
	Image front = getTexture(5, 9);
	Image side = getTexture(6, 9);
	Image top = getTexture(6, 8);

	int yoff = texture_size / 16.0 * 9.0;
	createSmallerBlock(26, 0, side, empty_texture, top.rotate(ROTATE_90), 0, yoff, true);
	createSmallerBlock(26, 1, empty_texture, side.flip(true, false), top, 0, yoff, true);
	createSmallerBlock(26, 2, side.flip(true, false), front, top.rotate(ROTATE_270), 0,
	        yoff, true);
	createSmallerBlock(26, 3, front, side, top.flip(true, false), 0, yoff, true);

	front = getTexture(8, 9);
	side = getTexture(7, 9);
	top = getTexture(7, 8);

	createSmallerBlock(26, /*0 |*/8, side, front, top, 0, yoff, true);
	createSmallerBlock(26, 1 | 8, front, side.flip(true, false), top.rotate(ROTATE_90), 0,
	        yoff, true);
	createSmallerBlock(26, 2 | 8, side.flip(true, false), empty_texture,
	        top.flip(true, false), 0, yoff, true);
	createSmallerBlock(26, 3 | 8, empty_texture, side, top.rotate(ROTATE_270), 0, yoff,
	        true);
}

void BlockTextures::createStraightRails(uint16_t id, uint16_t extra_data,
        const Image& texture) { // id 27, 28, 66
	createSingleFaceBlock(id, 0 | extra_data, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 1 | extra_data, FACE_BOTTOM, texture);

	Image rotated_texture = texture.rotate(ROTATE_90);
	Image ascending_east(getBlockImageSize(), getBlockImageSize());
	Image ascending_west(getBlockImageSize(), getBlockImageSize());
	Image ascending_north(getBlockImageSize(), getBlockImageSize());
	Image ascending_south(getBlockImageSize(), getBlockImageSize());

	for (TopFaceIterator it(texture_size); !it.end(); it.next()) {
		ascending_east.setPixel(it.dest_x, it.dest_y + it.src_y,
		        texture.getPixel(it.src_x, it.src_y));
		ascending_east.setPixel(it.dest_x + 1, it.dest_y + it.src_y,
		        texture.getPixel(it.src_x, it.src_y));

		ascending_west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
		ascending_west.setPixel(it.dest_x, it.dest_y + (texture_size - it.src_y),
		        texture.getPixel(it.src_x, it.src_y));
	}

	ascending_north = ascending_east.flip(true, false);
	ascending_south = ascending_west.flip(true, false);

	setBlockImage(id, 2 | extra_data, ascending_east);
	setBlockImage(id, 3 | extra_data, ascending_west);
	setBlockImage(id, 4 | extra_data, ascending_north);
	setBlockImage(id, 5 | extra_data, ascending_south);
}

void BlockTextures::createPiston(uint16_t id, bool sticky) { //  id 29, 33
	Image pistonFace = sticky ? getTexture(10, 6) : getTexture(11, 6);
	Image empty_piston_face = getTexture(14, 6);
	Image side_face = getTexture(12, 6);
	Image smallerside_face = side_face.move(0, 4);
	Image bottom_face = getTexture(13, 6);
	createBlock(id, 0, side_face.rotate(ROTATE_180), bottom_face);
	createBlock(id, 1, side_face, pistonFace);
	createBlock(id, 2, side_face.rotate(ROTATE_270), bottom_face,
	        side_face.rotate(ROTATE_270));
	createBlock(id, 3, bottom_face, side_face.rotate(ROTATE_90), side_face);
	createBlock(id, 4, pistonFace, side_face.rotate(ROTATE_270),
	        side_face.rotate(ROTATE_180));
	createBlock(id, 5, side_face.rotate(ROTATE_90), pistonFace,
	        side_face.rotate(ROTATE_90));

	/*
	 buildSimpleBlock(id, 0 | 8, side_face.rotate(ROTATE_180), bottomFace);
	 buildSimpleBlock(id, 1 | 8, side_face, pistonFace);
	 buildSimpleBlock(id, 2 | 8, side_face.rotate(ROTATE_270), bottomFace, side_face.rotate(ROTATE_270));
	 buildSimpleBlock(id, 3 | 8, bottomFace, side_face.rotate(ROTATE_90), side_face);
	 buildSimpleBlock(id, 4 | 8, pistonFace, side_face.rotate(ROTATE_270), side_face.rotate(ROTATE_180));
	 buildSimpleBlock(id, 5 | 8, side_face.rotate(ROTATE_90), pistonFace, side_face.rotate(ROTATE_90));
	 */
}

void BlockTextures::createSlabs(uint16_t id, bool stone_slabs, bool double_slabs) { // id 43, 44, 125, 126
	std::map<int, Image> slab_textures;
	if (stone_slabs) {
		slab_textures[0x0] = getTexture(5, 0);
		slab_textures[0x1] = getTexture(0, 12);
		slab_textures[0x2] = getTexture(4, 0);
		slab_textures[0x3] = getTexture(0, 1);
		slab_textures[0x4] = getTexture(7, 0);
		slab_textures[0x5] = getTexture(6, 3);
	} else {
		slab_textures[0x0] = getTexture(4, 0);
		slab_textures[0x1] = getTexture(6, 12);
		slab_textures[0x2] = getTexture(6, 13);
		slab_textures[0x3] = getTexture(7, 12);
	}
	for (std::map<int, Image>::const_iterator it = slab_textures.begin();
	        it != slab_textures.end(); ++it) {
		if (double_slabs) {
			createBlock(id, it->first, it->second);
		} else {
			createSmallerBlock(id, it->first, it->second, it->second, 0,
			        texture_size / 2);
			createSmallerBlock(id, 0x8 | it->first, it->second, it->second,
			        texture_size / 2, texture_size);
		}
	}
}

void BlockTextures::createTorch(uint16_t id, const Image& texture) { // id 50, 75, 76
	createSingleFaceBlock(id, 1, FACE_WEST, texture);
	createSingleFaceBlock(id, 2, FACE_EAST, texture);
	createSingleFaceBlock(id, 3, FACE_NORTH, texture);
	createSingleFaceBlock(id, 4, FACE_SOUTH, texture);

	createItemStyleBlock(id, 5, texture);
	createItemStyleBlock(id, 6, texture);
}

void BlockTextures::createStairs(uint16_t id, const Image& texture) { // id 53, 67, 108, 109, 114, 128, 134, 135, 136
	setBlockImage(id, 0, buildStairsEast(texture));
	setBlockImage(id, 1, buildStairsWest(texture));
	setBlockImage(id, 2, buildStairsSouth(texture));
	setBlockImage(id, 3, buildStairsNorth(texture));

	setBlockImage(id, 0 | 4, buildUpsideDownStairsEast(texture));
	setBlockImage(id, 1 | 4, buildUpsideDownStairsWest(texture));
	setBlockImage(id, 2 | 4, buildUpsideDownStairsSouth(texture));
	setBlockImage(id, 3 | 4, buildUpsideDownStairsNorth(texture));
}

void BlockTextures::createChest(uint16_t id, Image* textures) { // id 54, 95, 130
	createRotatedBlock(id, 0, textures[CHEST_FRONT], textures[CHEST_SIDE],
	        textures[CHEST_TOP]);
}

void BlockTextures::createDoubleChest(uint16_t id, Image* textures) { // id 54
	// note here back and top textures are swapped
	createRotatedBlock(id, LARGECHEST_DATA_LARGE | LARGECHEST_DATA_LEFT,
	        textures[LARGECHEST_FRONT_LEFT], textures[LARGECHEST_BACK_RIGHT],
	        textures[LARGECHEST_SIDE], textures[LARGECHEST_TOP_RIGHT]);
	createRotatedBlock(id, LARGECHEST_DATA_LARGE, textures[LARGECHEST_FRONT_RIGHT],
	        textures[LARGECHEST_BACK_LEFT], textures[LARGECHEST_SIDE],
	        textures[LARGECHEST_TOP_LEFT]);
}

void BlockTextures::createDoor(uint16_t id, const Image& texture_bottom,
        const Image& texture_top) { // id 64, 71
	// TODO sometimes the texture needs to get x flipped when door is opened
	for (int top = 0; top <= 1; top++) {
		for (int flip_x = 0; flip_x <= 1; flip_x++) {
			for (int d = 0; d < 4; d++) {
				Image texture = (top ? texture_top : texture_bottom);
				if (flip_x)
					texture = texture.flip(true, false);
				Image block(getBlockImageSize(), getBlockImageSize());

				uint16_t direction = 0;
				if (d == 0) {
					direction = DOOR_NORTH;
					blitFace(block, FACE_NORTH, texture);
				} else if (d == 1) {
					direction = DOOR_SOUTH;
					blitFace(block, FACE_SOUTH, texture);
				} else if (d == 2) {
					direction = DOOR_EAST;
					blitFace(block, FACE_EAST, texture);
				} else if (d == 3) {
					direction = DOOR_WEST;
					blitFace(block, FACE_WEST, texture);
				}
				uint16_t data = (top ? DOOR_TOP : 0) | (flip_x ? DOOR_FLIP_X : 0)
				        | direction;
				setBlockImage(id, data, block);
			}
		}
	}
}

void BlockTextures::createRails() { // id 66
	Image texture = getTexture(0, 8);
	Image corner_texture = getTexture(0, 7);

	createStraightRails(66, 0, texture);
	createSingleFaceBlock(66, 6, FACE_BOTTOM, corner_texture.flip(false, true));
	createSingleFaceBlock(66, 7, FACE_BOTTOM, corner_texture);
	createSingleFaceBlock(66, 8, FACE_BOTTOM, corner_texture.flip(true, false));
	createSingleFaceBlock(66, 9, FACE_BOTTOM, corner_texture.flip(true, true));
}

void BlockTextures::createButton(uint16_t id, const Image& tex) { // id 77, 143
	Image texture = tex;
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

void BlockTextures::createSnow() { // id 78
	Image snow = getTexture(2, 4);
	for (int data = 0; data < 8; data++) {
		int smaller = texture_size - (data / 8.0 * texture_size);
		Image side_face = snow.move(0, smaller);
		Image block = buildImage(side_face, side_face, snow, 0, 0, 0, 0, 0, smaller);
		setBlockImage(78, data, block);
	}
}

void BlockTextures::createCactus() { // id 81
	Image block = buildImage(getTexture(6, 4), getTexture(6, 4), getTexture(5, 4), 2, 0,
	        -2, 0, 0, 0);
	setBlockImage(81, 0, block);
}

/**
 * Creates the texture for a fence. The texture looks like a cross, you can set if you
 * want the left or/and right connection posts.
 */
Image createFenceTexture(bool left, bool right, Image texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	Image mask(size, size);
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
void BlockTextures::createFence(uint16_t id, const Image& texture) { // id 85, 113
	Image fence_empty = createFenceTexture(false, false, texture);
	Image fence_left = createFenceTexture(true, false, texture);
	Image fence_right = createFenceTexture(false, true, texture);
	Image fence_both = createFenceTexture(true, true, texture);

	// go through all neighbor combinations
	for (uint8_t i = 0; i < 16; i++) {
		Image left = fence_empty, right = fence_empty;

		uint16_t data = i << 4;
		// special data set by the tile renderer
		bool north = (data & FACE_NORTH) == FACE_NORTH;
		bool south = (data & FACE_SOUTH) == FACE_SOUTH;
		bool east = (data & FACE_EAST) == FACE_EAST;
		bool west = (data & FACE_WEST) == FACE_WEST;

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

		createItemStyleBlock(id, data, left, right);
	}
}

void BlockTextures::createPumkin(uint16_t id, const Image& front) { // id 86, 91
	Image side = getTexture(6, 7);
	Image top = getTexture(6, 6);
	createBlock(id, 0, side, front, top);
	createBlock(id, 1, front, side, top);
	createBlock(id, 2, side, side, top);
	createBlock(id, 3, side, side, top);
	createBlock(id, 4, side, side, top);
}

void BlockTextures::createCake() { // id 92
	Image block = buildImage(getTexture(10, 7), getTexture(10, 7), getTexture(9, 7), 1, 0,
	        -1, 0, 0, 9);
	setBlockImage(92, 0, block);
}

void BlockTextures::createRedstoneRepeater(uint16_t id, const Image& texture) { // id 93, 94
	createSingleFaceBlock(id, 0, FACE_BOTTOM, texture.rotate(ROTATE_270));
	createSingleFaceBlock(id, 1, FACE_BOTTOM, texture);
	createSingleFaceBlock(id, 2, FACE_BOTTOM, texture.rotate(ROTATE_90));
	createSingleFaceBlock(id, 3, FACE_BOTTOM, texture.rotate(ROTATE_180));
}

void BlockTextures::createTrapdoor() { // id 96
	Image texture = getTexture(4, 5);
	for (uint16_t i = 0; i < 16; i++) {
		if (i & 4) {
			int data = i & 0b00000011;
			if (data == 0x0)
				createSingleFaceBlock(96, i, FACE_SOUTH, texture);
			else if (data == 0x1)
				createSingleFaceBlock(96, i, FACE_NORTH, texture);
			else if (data == 0x2)
				createSingleFaceBlock(96, i, FACE_EAST, texture);
			else if (data == 0x3)
				createSingleFaceBlock(96, i, FACE_WEST, texture);
		} else {
			if (i & 8)
				createSingleFaceBlock(96, i, FACE_TOP, texture);
			else
				createSingleFaceBlock(96, i, FACE_BOTTOM, texture);
		}
	}
}

void BlockTextures::createHugeMushroom(uint16_t id, const Image& capTexture) { // id 99, 100
	Image pores = getTexture(14, 8);
	Image stem = getTexture(13, 8);

	createBlock(id, 0, pores, pores, pores);
	createBlock(id, 1, capTexture, pores, capTexture);
	createBlock(id, 2, pores, pores, capTexture);
	createBlock(id, 3, pores, pores, capTexture);
	createBlock(id, 4, capTexture, pores, capTexture);
	createBlock(id, 5, pores, pores, capTexture);
	createBlock(id, 6, pores, pores, capTexture);
	createBlock(id, 7, capTexture, capTexture, capTexture);
	createBlock(id, 8, pores, capTexture, capTexture);
	createBlock(id, 9, pores, capTexture, capTexture);
	createBlock(id, 10, stem, stem, stem);
}

void BlockTextures::createBarsPane(uint16_t id, const Image& texture_left_right) { // id 101, 102
	Image texture_left = texture_left_right;
	Image texture_right = texture_left_right;
	texture_left.fill(0, texture_size / 2, 0, texture_size / 2, texture_size);
	texture_right.fill(0, 0, 0, texture_size / 2, texture_size);

	createItemStyleBlock(id, 0, texture_left_right);
	for (uint8_t i = 1; i < 16; i++) {
		Image left = empty_texture, right = empty_texture;

		uint16_t data = i << 4;
		bool north = (data & FACE_NORTH) == FACE_NORTH;
		bool south = (data & FACE_SOUTH) == FACE_SOUTH;
		bool east = (data & FACE_EAST) == FACE_EAST;
		bool west = (data & FACE_WEST) == FACE_WEST;

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

		createItemStyleBlock(id, data, left, right);
	}
}

void BlockTextures::createStem(uint16_t id) { // id 104, 105
	// build here only growing normal stem
	Image texture = getTexture(15, 6);

	for (int i = 0; i <= 7; i++) {
		double percentage = 1 - ((double) i / 7);
		int move = percentage * texture_size;

		if (i == 7)
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.6, 0.7, 0.01));
		else
			createItemStyleBlock(id, i, texture.move(0, move).colorize(0.3, 0.7, 0.01));
	}
}

void BlockTextures::createVines() { // id 106
	Image texture = getTexture(15, 8).colorize(0.3, 1, 0.1);

	createSingleFaceBlock(106, 0, FACE_TOP, texture);
	for (int i = 1; i < 16; i++) {
		Image block(getBlockImageSize(), getBlockImageSize());
		if (i & 1)
			blitFace(block, FACE_SOUTH, texture);
		if (i & 2)
			blitFace(block, FACE_WEST, texture);
		if (i & 4)
			blitFace(block, FACE_NORTH, texture);
		if (i & 8)
			blitFace(block, FACE_EAST, texture);
		setBlockImage(106, i, block);
	}
}

/**
 * Creates the texture for a fence gate, opened or closed.
 */
Image createFenceGateTexture(bool opened, Image texture) {
	int size = texture.getWidth();
	double ratio = (double) size / 16;

	Image mask(size, size);
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

void BlockTextures::createFenceGate() { // id 107
	Image texture = getTexture(4, 0);
	Image opened = createFenceGateTexture(true, texture);
	Image closed = createFenceGateTexture(false, texture);

	// go through states opened and closed
	for(int open = 0; open <= 1; open++) {
		Image tex = open ? opened : closed;
		Image north(texture_size * 2, texture_size * 2);
		Image east = north;
		// north and south block images are same
		// (because we ignore the direction of opened fence gates)
		blitFace(north, FACE_NORTH, tex, texture_size * 0.5, texture_size * 0.25, false);
		// also east and west
		blitFace(east, FACE_EAST, tex, -texture_size * 0.5, texture_size * 0.25, false);
		uint8_t extra = open ? 4 : 0;
		setBlockImage(107, 2 | extra, north);
		setBlockImage(107, 0 | extra, north);
		setBlockImage(107, 3 | extra, east);
		setBlockImage(107, 1 | extra, east);
	}
}

void BlockTextures::createCauldron() { // id 118
	Image side = getTexture(10, 9);
	Image water = getTexture(14, 12);

	for (int i = 0; i < 4; i++) {
		Image block(getBlockImageSize(), getBlockImageSize());
		blitFace(block, FACE_NORTH, side);
		blitFace(block, FACE_EAST, side);
		if (i == 3)
			blitFace(block, FACE_TOP, water, 0, 2);
		else if (i == 2)
			blitFace(block, FACE_TOP, water, 0, 5);
		else if (i == 1)
			blitFace(block, FACE_TOP, water, 0, 9);
		blitFace(block, FACE_SOUTH, side);
		blitFace(block, FACE_WEST, side);
		setBlockImage(118, i, block);
	}
}

void BlockTextures::createBeacon() { // id 138
	Image beacon(texture_size * 2, texture_size * 2);

	// at first create this little block in the middle
	Image beacon_texture;
	getTexture(9, 2).resizeInterpolated(texture_size * 0.75, texture_size * 0.75,
			beacon_texture);
	Image smallblock(texture_size * 2, texture_size * 2);
	blitFace(smallblock, FACE_WEST, beacon_texture);
	blitFace(smallblock, FACE_SOUTH, beacon_texture);
	blitFace(smallblock, FACE_TOP, beacon_texture);

	// then create the obsidian ground
	Image obsidian_texture = getTexture(5, 2);
	Image obsidian = buildSmallerImage(obsidian_texture, obsidian_texture,
			obsidian_texture, 0, texture_size / 4, false);

	// blit block and obsidian ground
	beacon.simpleblit(obsidian, 0, 0);
	beacon.simpleblit(smallblock, texture_size / 4, texture_size / 4);

	// then blit outside glass
	Image glass_texture = getTexture(1, 3);
	blitFace(beacon, FACE_WEST, glass_texture);
	blitFace(beacon, FACE_SOUTH, glass_texture);
	blitFace(beacon, FACE_TOP, glass_texture);

	setBlockImage(138, 0, beacon);
}

void BlockTextures::loadBlocks() {
	buildCustomTextures();
	unknown_block = buildImage(unknown_block, unknown_block, unknown_block);

	createBlock(1, 0, getTexture(1, 0)); // stone
	createGrassBlock(); // id 2
	createBlock(3, 0, getTexture(2, 0)); // dirt
	createBlock(4, 0, getTexture(0, 1)); // cobblestone
	// -- wooden planks
	createBlock(5, 0, getTexture(4, 0)); // oak
	createBlock(5, 1, getTexture(6, 12)); // pine/spruce
	createBlock(5, 2, getTexture(6, 13)); // birch
	createBlock(5, 3, getTexture(7, 12)); // jungle
	// -- saplings
	createItemStyleBlock(6, 0, getTexture(15, 0)); // oak
	createItemStyleBlock(6, 1, getTexture(15, 3)); // spruce
	createItemStyleBlock(6, 2, getTexture(15, 4)); // birch
	createItemStyleBlock(6, 3, getTexture(14, 1)); // jungle
	// --
	createBlock(7, 0, getTexture(1, 1)); // bedrock
	createWater(); // id 8, 9
	createLava(); // id 10, 11
	createBlock(12, 0, getTexture(2, 1)); // sand
	createBlock(13, 0, getTexture(3, 1)); // gravel
	createBlock(14, 0, getTexture(0, 2)); // gold ore
	createBlock(15, 0, getTexture(1, 2)); // iron ore
	createBlock(16, 0, getTexture(2, 2)); // coal ore
	// -- wood
	createWood(0, getTexture(4, 1)); // oak
	createWood(1, getTexture(4, 7)); // pine/spruce
	createWood(2, getTexture(5, 7)); // birch
	createWood(3, getTexture(9, 9)); // jungle
	createLeaves(); // id 18
	createBlock(19, 0, getTexture(0, 3)); // sponge
	createBlock(20, 0, getTexture(1, 3)); // glass
	createBlock(21, 0, getTexture(0, 10)); // lapis lazuli ore
	createBlock(22, 0, getTexture(0, 9)); // lapis lazuli block
	createRotatedBlock(23, 0, getTexture(14, 2), getTexture(13, 2), getTexture(14, 3)); // dispenser
	// -- sandstone
	createBlock(24, 0, getTexture(0, 12), getTexture(0, 11)); // normal
	createBlock(24, 1, getTexture(5, 14), getTexture(0, 11)); // chiseled
	createBlock(24, 2, getTexture(6, 14), getTexture(0, 11)); // smooth
	// --
	createBlock(25, 0, getTexture(10, 4)); // noteblock
	createBed(); // id 26 bed
	createStraightRails(27, 0, getTexture(3, 10)); // id 27 powered rail (unpowered)
	createStraightRails(27, 8, getTexture(3, 11)); // id 27 powered rail (powered)
	createStraightRails(28, 0, getTexture(3, 12)); // id 28 detector rail
	createPiston(29, true); // sticky piston
	createItemStyleBlock(30, 0, getTexture(11, 0)); // cobweb
	// -- tall grass
	createItemStyleBlock(31, 0, getTexture(7, 3)); // dead bush style
	createItemStyleBlock(31, 1, getTexture(7, 2).colorize(0.3, 0.95, 0.3)); // tall grass
	createItemStyleBlock(31, 2, getTexture(8, 3).colorize(0.3, 0.95, 0.3)); // fern
	// --
	createItemStyleBlock(32, 0, getTexture(7, 3)); // dead bush
	createPiston(33, false); // piston
	// id 34 // piston extension
	// -- wool
	createBlock(35, 0, getTexture(0, 4)); // white
	createBlock(35, 1, getTexture(2, 13)); // orange
	createBlock(35, 2, getTexture(2, 12)); // magenta
	createBlock(35, 3, getTexture(2, 11)); // light blue
	createBlock(35, 4, getTexture(2, 10)); // yellow
	createBlock(35, 5, getTexture(2, 9)); // lime
	createBlock(35, 6, getTexture(2, 8)); // pink
	createBlock(35, 7, getTexture(2, 7)); // gray
	createBlock(35, 8, getTexture(1, 14)); // light gray
	createBlock(35, 9, getTexture(1, 13)); // cyan
	createBlock(35, 10, getTexture(1, 12)); // purple
	createBlock(35, 11, getTexture(1, 11)); // blue
	createBlock(35, 12, getTexture(1, 10)); // brown
	createBlock(35, 13, getTexture(1, 9)); // green
	createBlock(35, 14, getTexture(1, 8)); // red
	createBlock(35, 15, getTexture(1, 7)); // black
	// --
	createBlock(36, 0, empty_texture); // block moved by piston aka 'block 36'
	createItemStyleBlock(37, 0, getTexture(13, 0)); // dandelion
	createItemStyleBlock(38, 0, getTexture(12, 0)); // rose
	createItemStyleBlock(39, 0, getTexture(13, 1)); // brown mushroom
	createItemStyleBlock(40, 0, getTexture(12, 1)); // red mushroom
	createBlock(41, 0, getTexture(7, 1)); // block of gold
	createBlock(42, 0, getTexture(6, 1)); // block of iron
	createSlabs(43, true, true); // double stone slabs
	createSlabs(44, true, false); // normal stone slabs
	createBlock(45, 0, getTexture(7, 0)); // bricks
	createBlock(46, 0, getTexture(8, 0), getTexture(9, 0)); // tnt
	createBlock(47, 0, getTexture(3, 2), getTexture(4, 0)); // bookshelf
	createBlock(48, 0, getTexture(4, 2)); // moss stone
	createBlock(49, 0, getTexture(5, 2)); // obsidian
	createTorch(50, getTexture(0, 5)); // torch
	createItemStyleBlock(51, 0, fire_texture); // fire
	createBlock(52, 0, getTexture(1, 4)); // monster spawner
	createStairs(53, getTexture(4, 0)); // oak wood stairs
	createChest(54, chest); // chest
	createDoubleChest(54, largechest); // chest
	// id 55 // redstone wire
	createBlock(56, 0, getTexture(2, 3)); // diamond ore
	createBlock(57, 0, getTexture(8, 1)); // block of diamond
	createBlock(58, 0, getTexture(11, 3), getTexture(12, 3), getTexture(11, 2)); // crafting table
	// -- wheat
	createItemStyleBlock(59, 0, getTexture(8, 5)); //
	createItemStyleBlock(59, 1, getTexture(9, 5)); //
	createItemStyleBlock(59, 2, getTexture(10, 5)); //
	createItemStyleBlock(59, 3, getTexture(11, 5)); //
	createItemStyleBlock(59, 4, getTexture(12, 5)); //
	createItemStyleBlock(59, 5, getTexture(13, 5)); //
	createItemStyleBlock(59, 6, getTexture(14, 5)); //
	createItemStyleBlock(59, 7, getTexture(15, 5)); //
	// --
	createBlock(60, 0, getTexture(2, 0), getTexture(6, 5)); // farmland
	createRotatedBlock(61, 0, getTexture(12, 2), getTexture(13, 2), getTexture(14, 3)); // furnace
	createRotatedBlock(62, 0, getTexture(13, 3), getTexture(13, 2), getTexture(14, 3)); // burning furnace
	// id 63 // sign post
	createDoor(64, getTexture(1, 6), getTexture(1, 5)); // wooden door
	// -- ladders
	createSingleFaceBlock(65, 2, FACE_SOUTH, getTexture(3, 5));
	createSingleFaceBlock(65, 3, FACE_NORTH, getTexture(3, 5));
	createSingleFaceBlock(65, 4, FACE_EAST, getTexture(3, 5));
	createSingleFaceBlock(65, 5, FACE_WEST, getTexture(3, 5));
	// --
	createRails(); // id 66
	createStairs(67, getTexture(0, 1)); // cobblestone stairs
	// id 68 // wall sign
	// id 69 // lever
	createSmallerBlock(70, 0, getTexture(1, 0), getTexture(1, 0), 0, 1); // stone pressure plate
	createDoor(71, getTexture(2, 6), getTexture(2, 5)); // iron door
	createSmallerBlock(72, 0, getTexture(4, 0), getTexture(4, 0), 0, 1); // wooden pressure plate
	createBlock(73, 0, getTexture(3, 3)); // redstone ore
	createBlock(74, 0, getTexture(3, 3)); // glowing redstone ore
	createTorch(75, getTexture(3, 6)); // redstone torch off
	createTorch(76, getTexture(3, 7)); // redstone torch on
	createButton(77, getTexture(0, 0)); // stone button
	createSnow(); // id 78
	createBlock(79, 0, getTexture(3, 4)); // ice
	createBlock(80, 0, getTexture(2, 4)); // snow block
	createCactus(); // id 81
	createBlock(82, 0, getTexture(8, 4)); // clay block
	createItemStyleBlock(83, 0, getTexture(9, 4)); // sugar cane
	createBlock(84, 0, getTexture(10, 4), getTexture(11, 4)); // jukebox
	createFence(85, getTexture(4, 0)); // fence
	createPumkin(86, getTexture(7, 7)); // pumpkin
	createBlock(87, 0, getTexture(7, 6)); // netherrack
	createBlock(88, 0, getTexture(8, 6)); // soul sand
	createBlock(89, 0, getTexture(9, 6)); // glowstone block
	createBlock(90, 0, getTexture(14, 0).colorize(1, 1, 1, 0.5)); // nether portal block
	createPumkin(91, getTexture(8, 7)); // jack-o-lantern
	createCake(); // id 92
	createRedstoneRepeater(93, getTexture(3, 8)); // redstone repeater off
	createRedstoneRepeater(94, getTexture(3, 9)); // redstone repeater on
	createChest(95, chest); // locked chest
	createTrapdoor(); // id 96 // trapdoor
	// -- monster egg
	createBlock(97, 0, getTexture(1, 0)); // stone
	createBlock(97, 1, getTexture(0, 1)); // cobblestone
	createBlock(97, 2, getTexture(6, 3)); // sonte brick
	// --
	// -- stone bricks
	createBlock(98, 0, getTexture(6, 3)); // normal
	createBlock(98, 1, getTexture(4, 6)); // mossy
	createBlock(98, 2, getTexture(5, 6)); // cracked
	createBlock(98, 3, getTexture(5, 13)); // chiseled
	// --
	createHugeMushroom(99, getTexture(14, 7)); // huge brown mushroom
	createHugeMushroom(100, getTexture(13, 7)); // huge red mushroom
	createBarsPane(101, getTexture(5, 5)); // iron bars
	createBarsPane(102, getTexture(1, 3)); // glass pane
	createBlock(103, 0, getTexture(8, 8), getTexture(9, 8)); // melon
	createStem(104); // pumpkin stem
	createStem(105); // melon stem
	createVines(); // id 106 // vines
	createFenceGate(); // id 107 // fence gate
	createStairs(108, getTexture(7, 0)); // brick stairs
	createStairs(109, getTexture(6, 3)); // stone brick stairs
	createBlock(110, 0, getTexture(13, 4), getTexture(14, 4)); // mycelium
	createSingleFaceBlock(111, 0, FACE_BOTTOM,
	        getTexture(12, 4).colorize(0.3, 0.95, 0.3)); // lily pad
	createBlock(112, 0, getTexture(0, 14)); // nether brick
	createFence(113, getTexture(0, 14)); // nether brick fence
	createStairs(114, getTexture(0, 14)); // nether brick stairs
	// -- nether wart
	createItemStyleBlock(115, 0, getTexture(2, 14)); //
	createItemStyleBlock(115, 1, getTexture(3, 14)); //
	createItemStyleBlock(115, 2, getTexture(3, 14)); //
	createItemStyleBlock(115, 3, getTexture(4, 14)); //
	// --
	createSmallerBlock(116, 0, getTexture(6, 11), getTexture(6, 10), 0, texture_size * 0.75, true); // enchantment table
	createItemStyleBlock(117, 0, getTexture(13, 9)); // brewing stand
	createCauldron(); // id 118 // cauldron
	createSmallerBlock(119, 0, endportal_texture, endportal_texture, 4, texture_size * 0.75, false);
	createSmallerBlock(120, 0, getTexture(15, 9), getTexture(14, 9), 0, texture_size * 0.8125, true); // end portal fram
	createBlock(121, 0, getTexture(15, 10)); // end stone
	// id 122 // dragon egg
	createBlock(123, 0, getTexture(3, 13)); // redstone lamp inactive
	createBlock(124, 0, getTexture(4, 13)); // redstone lamp active
	createSlabs(125, false, true); // wooden double slabs
	createSlabs(126, false, false); // wooden normal slabs
	// id 127 // cocoa plant
	createStairs(128, getTexture(0, 12)); // sandstone stairs
	createBlock(129, 0, getTexture(11, 10)); // emerald ore
	createChest(130, enderchest); // ender chest
	// id 131 // tripwire hook
	// id 132 // tripwire
	createBlock(133, 0, getTexture(9, 1)); // block of emerald
	createStairs(134, getTexture(6, 12)); // spruce wood stairs
	createStairs(135, getTexture(6, 13)); // birch wood stairs
	createStairs(136, getTexture(7, 12)); // jungle wood stairs
	createBlock(137, 0, getTexture(8, 11)); // command block
	createBeacon(); // beacon
	// id 139 // cobblestone wall
	// id 140 // flower pot
	// carrots --
	createItemStyleBlock(141, 0, getTexture(8, 12));
	createItemStyleBlock(141, 1, getTexture(8, 12));
	createItemStyleBlock(141, 2, getTexture(9, 12));
	createItemStyleBlock(141, 3, getTexture(9, 12));
	createItemStyleBlock(141, 4, getTexture(10, 12));
	createItemStyleBlock(141, 5, getTexture(10, 12));
	createItemStyleBlock(141, 6, getTexture(10, 12));
	createItemStyleBlock(141, 7, getTexture(11, 12));
	// --
	// potatoes --
	createItemStyleBlock(142, 0, getTexture(8, 12));
	createItemStyleBlock(142, 1, getTexture(8, 12));
	createItemStyleBlock(142, 2, getTexture(9, 12));
	createItemStyleBlock(142, 3, getTexture(9, 12));
	createItemStyleBlock(142, 4, getTexture(10, 12));
	createItemStyleBlock(142, 5, getTexture(10, 12));
	createItemStyleBlock(142, 6, getTexture(10, 12));
	createItemStyleBlock(142, 7, getTexture(12, 12));
	// --
	createButton(143, getTexture(4, 0)); // wooden button
	// id 144 // head
	// id 145 // anvil
	// id 146 // trapped chest
	// id 147 // weighted pressure plate (light)
	// id 147 // weighted pressure plate (heavy)
	// id 149 // redstone comparator (inactive)
	// id 149 // redstone comparator (active)
	// id 151 // daylight sensor
	// id 152 // block of redstone
	// id 153 // nether quartz ore
	// id 154 // hopper
}

bool BlockTextures::isBlockTransparent(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	// remove edge data
	data &= ~(EDGE_NORTH | EDGE_EAST | EDGE_BOTTOM);
	if (block_images.count(id | (data << 16)) == 0)
		return !render_unknown_blocks;
	return block_transparency.count(id | (data << 16)) != 0;
}

bool BlockTextures::hasBlock(uint16_t id, uint16_t data) const {
	return block_images.count(id | (data << 16)) != 0;
}

const Image& BlockTextures::getBlock(uint16_t id, uint16_t data) const {
	data = filterBlockData(id, data);
	if (!hasBlock(id, data))
		return unknown_block;
	return block_images.at(id | (data << 16));
}

int BlockTextures::getMaxWaterNeededOpaque() const {
	return max_water;
}

const Image& BlockTextures::getOpaqueWater(bool south, bool west) const {
	int index = ((south ? 0 : 1) | ((west ? 0 : 1) << 1));
	return opaque_water[index];
}

int BlockTextures::getBlockImageSize() const {
	return texture_size * 2;
}

int BlockTextures::getTextureSize() const {
	return texture_size;
}

int BlockTextures::getTileSize() const {
	return texture_size * 2 * 16;
}

}
}
