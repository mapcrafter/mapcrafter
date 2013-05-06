/*
 * base.cpp
 *
 *  Created on: 05.05.2013
 *      Author: moritz
 */

#include "base.h"

namespace mapcrafter {
namespace render {

Rendermode::Rendermode(RenderState& state)
		: state(state) {
}

Rendermode::~Rendermode() {
}

void Rendermode::start() {
}

void Rendermode::end() {
}

bool Rendermode::isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data) {
	return false;
}

void Rendermode::draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data) {
}

} /* namespace render */
} /* namespace mapcrafter */
