/*
 * base.h
 *
 *  Created on: 05.05.2013
 *      Author: moritz
 */

#ifndef BASE_H_
#define BASE_H_

#include "mc/pos.h"
#include "mc/cache.h"

#include "render/image.h"
#include "render/render.h"

namespace mapcrafter {
namespace render {

/**
 * A simple interface to implement different rendermodes.
 */
class Rendermode {
protected:
	RenderState& state;
public:
	Rendermode(RenderState& state);
	virtual ~Rendermode();

	// is called when the tile renderer starts rendering a tile
	virtual void start();
	// is called when the tile renderer finished rendering a tile
	virtual void end();

	// is called to allow the rendermode to hide specific blocks
	virtual bool isHidden(const mc::BlockPos& pos, uint16_t id, uint8_t data);
	// is called to allow the rendermode to change a block image
	virtual void draw(Image& image, const mc::BlockPos& pos, uint16_t id, uint8_t data);
};

} /* namespace render */
} /* namespace mapcrafter */

#include "render/rendermodes/lighting.h"

#endif /* BASE_H_ */
