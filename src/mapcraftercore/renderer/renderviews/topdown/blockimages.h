/*
 * blockimages.h
 *
 *  Created on: Nov 14, 2014
 *      Author: moritz
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

	virtual int getBlockImageSize() const;

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
