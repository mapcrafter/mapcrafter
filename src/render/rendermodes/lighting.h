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

#ifndef LIGHT_H_
#define LIGHT_H_

#include "render/rendermodes/base.h"

namespace mapcrafter {
namespace render {

class LightingRendermode : public Rendermode {
private:
	bool day;
public:
	LightingRendermode(RenderState& state, bool day);
	virtual ~LightingRendermode();

	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data);
	virtual void draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data);
};

} /* namespace render */
} /* namespace mapcrafter */
#endif /* LIGHT_H_ */
