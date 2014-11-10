/*
 * Copyright 2012-2014 Moritz Hilscher
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

#ifndef BLOCKIMAGES_H_
#define BLOCKIMAGES_H_

#include "biomes.h"
#include "blocktextures.h"
#include "image.h"

#include <string>
#include <cstdint>

namespace mapcrafter {
namespace renderer {

class BlockImageTextureResources {
public:
	BlockImageTextureResources();
	~BlockImageTextureResources();

	void setTextureSize(int texture_size);

	bool loadChests(const std::string& normal, const std::string& normal_double,
			const std::string& ender,
			const std::string& trapped, const std::string& trapped_double);
	bool loadColors(const std::string& foliagecolor, const std::string& grasscolor);
	bool loadOther(const std::string& endportal);
	bool loadBlocks(const std::string& block_dir);
	bool loadAll(const std::string& textures_dir);
	bool saveBlocks(const std::string& filename);

	const BlockTextures& getBlockTextures() const;
	const RGBAImage& getEndportalTexture() const;

	const RGBAImage* getNormalChest() const;
	const RGBAImage* getNormalDoubleChest() const;
	const RGBAImage* getEnderChest() const;
	const RGBAImage* getTrappedChest() const;
	const RGBAImage* getTrappedDoubleChest() const;

	const RGBAImage& getFoliageColors() const;
	const RGBAImage& getGrassColors() const;

	static const int CHEST_FRONT = 0;
	static const int CHEST_SIDE = 1;
	static const int CHEST_TOP = 2;

	static const int LARGECHEST_FRONT_LEFT = 0;
	static const int LARGECHEST_FRONT_RIGHT = 1;
	static const int LARGECHEST_SIDE = 2;
	static const int LARGECHEST_TOP_LEFT = 3;
	static const int LARGECHEST_TOP_RIGHT = 4;
	static const int LARGECHEST_BACK_LEFT = 5;
	static const int LARGECHEST_BACK_RIGHT = 6;

private:
	int texture_size;

	BlockTextures textures;
	RGBAImage empty_texture;
	RGBAImage endportal_texture;

	RGBAImage chest_normal[3], chest_normal_double[7];
	RGBAImage chest_ender[3];
	RGBAImage chest_trapped[3], chest_trapped_double[7];

	RGBAImage foliagecolors, grasscolors;
};

class BlockImages {
public:
	virtual ~BlockImages();

	virtual void setSettings(int texture_size, int rotation, bool render_unknown_blocks,
			bool render_leaves_transparent, const std::string& rendermode) = 0;

	virtual bool loadAll(const std::string& textures_dir) = 0;
	virtual bool saveBlocks(const std::string& filename) = 0;

	virtual bool isBlockTransparent(uint16_t id, uint16_t data) const = 0;
	virtual bool hasBlock(uint16_t id, uint16_t) const = 0;
	virtual const RGBAImage& getBlock(uint16_t id, uint16_t data) const = 0;
	virtual RGBAImage getBiomeDependBlock(uint16_t id, uint16_t data, const Biome& biome) const = 0;

	virtual int getMaxWaterNeededOpaque() const = 0;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const = 0;

	virtual int getBlockImageSize() const = 0;
	virtual int getTextureSize() const = 0;
	virtual int getTileSize() const = 0;
};

}
}

#endif /* BLOCKIMAGES_H_ */
