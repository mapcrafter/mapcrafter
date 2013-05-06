/*
 * light.h
 *
 *  Created on: 05.05.2013
 *      Author: moritz
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "render/rendermodes/base.h"

namespace mapcrafter {
namespace render {

class LightingRendermode : public Rendermode {
public:
	LightingRendermode(RenderState& state);
	virtual ~LightingRendermode();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data);
	virtual void draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data);
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* LIGHT_H_ */
