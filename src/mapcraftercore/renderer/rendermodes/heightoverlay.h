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

#ifndef RENDERMODES_HEIGHTOVERLAY_H_
#define RENDERMODES_HEIGHTOVERLAY_H_

#include "../overlay.h"
#include "../rendermode.h"
#include "../../config/configsection.h"
#include "../../util.h"

#include <tuple>

namespace mapcrafter {

namespace config {
class HeightColor;
}

namespace renderer {

class HeightOverlay : public TintingOverlay<config::HeightOverlaySection> {
public:
	HeightOverlay(std::shared_ptr<config::ConfigSection> overlay_config);

protected:
	void initializeColors(const std::vector<config::HeightColor>& base_colors, int default_opacity);

	virtual RGBAPixel getBlockColor(const mc::BlockPos& pos, const mc::BlockPos& pos_for, uint16_t id, uint16_t data);

	std::vector<RGBAPixel> colors;
};

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* RENDERMODES_HEIGHTOVERLAY_H_ */
