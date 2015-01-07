/*
 * Copyright 2012-2015 Moritz Hilscher
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

#ifndef TOPDOWN_BLOCKIMAGES_H_
#define TOPDOWN_BLOCKIMAGES_H_

#include "../../blockimages.h"

namespace mapcrafter {
namespace renderer {

class TopdownBlockImages : public AbstractBlockImages {
public:
	TopdownBlockImages();
	virtual ~TopdownBlockImages();

	virtual int getMaxWaterNeededOpaque() const;
	virtual const RGBAImage& getOpaqueWater(bool south, bool west) const;

	virtual int getBlockSize() const;

protected:
	virtual uint16_t filterBlockData(uint16_t id, uint16_t data) const;
	virtual bool checkImageTransparency(const RGBAImage& block) const;

	virtual RGBAImage createUnknownBlock() const;
	virtual RGBAImage createBiomeBlock(uint16_t id, uint16_t data, const Biome& biome_data) const;

	virtual void createBlocks();
	virtual void createBiomeBlocks();
};

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* TOPDOWN_BLOCKIMAGES_H_ */
