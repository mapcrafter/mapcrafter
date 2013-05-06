/*
 * light.cpp
 *
 *  Created on: 05.05.2013
 *      Author: moritz
 */

#include "lighting.h"

#include "render/textures.h"

#include <cmath>

namespace mapcrafter {
namespace render {

LightingRendermode::LightingRendermode(RenderState& state)
		: Rendermode(state) {
}

LightingRendermode::~LightingRendermode() {
}

bool LightingRendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	return false;
}

void LightingRendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
}

} /* namespace render */
} /* namespace mapcrafter */
