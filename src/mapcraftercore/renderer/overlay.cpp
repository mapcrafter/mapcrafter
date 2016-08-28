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

#include "overlay.h"

#include "overlays/areaoverlay.h"
#include "overlays/heightoverlay.h"
#include "overlays/lighting.h"
#include "overlays/lightleveloverlay.h"
#include "overlays/slimeoverlay.h"
#include "overlays/spawnoverlay.h"
#include "image.h"
#include "../config/mapcrafterconfig.h"
#include "../config/configsections/map.h"
#include "../config/configsections/overlay.h"
#include "../config/configsections/world.h"

#include <iostream>

namespace mapcrafter {
namespace renderer {

const OverlayRendererType OverlayRenderer::TYPE = OverlayRendererType::DUMMY;

TintingOverlayRenderer::TintingOverlayRenderer()
	: high_contrast(true) {
}

TintingOverlayRenderer::~TintingOverlayRenderer() {
}

void TintingOverlayRenderer::setHighContrast(bool high_contrast) {
	this->high_contrast = high_contrast;
}

void TintingOverlayRenderer::tintBlock(RGBAImage& image, RGBAPixel color) const {
	/*
	if (high_contrast) {
		// do the high contrast mode magic
		auto overlay = getRecolor(color);
		for (int y = 0; y < image.getWidth(); y++) {
			for (int x = 0; x < image.getHeight(); x++) {
				RGBAPixel& pixel = image.pixel(x, y);
				if (pixel != 0)
					pixel = rgba_add_clamp(pixel, overlay);
			}
		}
	} else {
		// otherwise just simple alphablending
		for (int y = 0; y < image.getWidth(); y++) {
			for (int x = 0; x < image.getHeight(); x++) {
				RGBAPixel& pixel = image.pixel(x, y);
				if (pixel != 0)
					blend(pixel, color);
			}
		}
	}
	*/

	image.fill(color);
}

std::tuple<int, int, int> TintingOverlayRenderer::getRecolor(RGBAPixel color) const {
	// get luminance of recolor:
	// "10*r + 3*g + b" should actually be "3*r + 10*g + b"
	// it was a typo, but doesn't look bad either
	int luminance = (10 * rgba_red(color) + 3 * rgba_green(color) + rgba_blue(color)) / 14;

	float alpha_factor = 3; // 3 is similar to alpha=85
	// something like that would be possible too, but overlays won't look exactly like
	// overlays with that alpha value, so don't use it for now
	// alpha_factor = (float) 255.0 / rgba_alpha(color);

	// try to do luminance-neutral additive/subtractive color
	// instead of alpha blending (for better contrast)
	// so first subtract luminance from each component
	int nr = (rgba_red(color) - luminance) / alpha_factor;
	int ng = (rgba_green(color) - luminance) / alpha_factor;
	int nb = (rgba_blue(color) - luminance) / alpha_factor;
	return std::make_tuple(nr, ng, nb);
}

const OverlayRendererType TintingOverlayRenderer::TYPE = OverlayRendererType::TINTING;

Overlay* createOverlay(const config::WorldSection& world_config,
		const config::MapSection& map_config,
		std::shared_ptr<config::OverlaySection> overlay_config, int rotation) {
	config::OverlayType type = overlay_config->getType();
	std::string id = overlay_config->getID();
	std::string name = overlay_config->getName();

	/*
	if (type == config::OverlayType::LIGHTING) {
		return new LightingOverlay(id, name, overlay_config.isDay(),
				overlay_config.getLightingIntensity(),
				overlay_config.getLightingWaterIntensity(),
				world_config.getDimension() == mc::Dimension::END);
	} else if (type == config::OverlayType::SLIME) {
		return new SlimeOverlay(world_config.getInputDir(), rotation);
	} else if (type == config::OverlayType::SPAWN) {
		return new SpawnOverlay(overlay_config.isDay());
	} else {
		// may not happen
		assert(false);
	}
	*/
	if (type == config::OverlayType::AREA) {
		return new AreaOverlay(overlay_config);
	} else if (type == config::OverlayType::HEIGHT) {
		return new HeightOverlay(overlay_config);
	} else if (type == config::OverlayType::LIGHTING) {
		bool simulate_sun_light = false;
		simulate_sun_light = simulate_sun_light || world_config.getDimension() == mc::Dimension::END;
		simulate_sun_light = simulate_sun_light || map_config.getBlockHandler() == BlockHandlerType::CAVE;
		return new LightingOverlay(overlay_config, simulate_sun_light);
	} else if (type == config::OverlayType::LIGHTLEVEL) {
		return new LightLevelOverlay(overlay_config);
	} else if (type == config::OverlayType::SLIME) {
		return new SlimeOverlay(overlay_config, world_config.getInputDir(), rotation);
	} else if (type == config::OverlayType::SPAWN) {
		return new SpawnOverlay(overlay_config);
	} else {
		// may not happen
		assert(false);
	}
	return nullptr;
}

std::vector<std::shared_ptr<Overlay>> createOverlays(
		const config::WorldSection& world_config, const config::MapSection& map_config,
		const std::map<std::string, std::shared_ptr<config::OverlaySection>>& overlays_config,
		int rotation) {
	std::vector<std::shared_ptr<Overlay>> overlays;
	
	auto overlay_types = map_config.getOverlays();
	for (auto it = overlay_types.begin(); it != overlay_types.end(); ++it) {
		std::shared_ptr<config::OverlaySection> overlay_config = overlays_config.at(*it);
		Overlay* overlay = createOverlay(world_config, map_config,
				overlay_config, rotation);
		overlays.push_back(std::shared_ptr<Overlay>(overlay));
	}

	return overlays;
}

}
}

