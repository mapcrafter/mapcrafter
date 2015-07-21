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

#ifndef IMAGE_QUANTIZATION_H_
#define IMAGE_QUANTIZATION_H_

#include "palette.h"
#include "../image.h"
#include "../../util.h"

#include <vector>

namespace mapcrafter {
namespace renderer {

class Octree;

// number of significant bits to use of the color components
// determines the count of leaves in the octree
// -> more bits, more leaves, more memory/time needed
const int COLOR_BITS = 5;

/**
 * Represents an octree (actually a hextree) which is used for color quantization.
 *
 * Have a look at this: 
 *  - http://www.cubic.org/docs/octree.htm
 *  - http://rosettacode.org/wiki/Color_quantization#C
 * 
 * Octrees are cool! \o/
 */
class Octree {
public:
	/**
	 * Constructor.
	 */
	Octree(Octree* parent = nullptr, int level = 0);

	/**
	 * Yeah, destructor.
	 */
	~Octree();

	/**
	 * Returns the parent of this node.
	 */
	Octree* getParent();

	/**
	 * Returns the (const) parent of this node.
	 */
	const Octree* getParent() const;

	/**
	 * Returns the level of the node (distance to root node).
	 */
	int getLevel() const;

	/**
	 * Returns whether this node is the root of the tree.
	 */
	bool isRoot() const;

	/**
	 * Returns whether this node is a leaf.
	 */
	bool isLeaf() const;

	/**
	 * Returns whether this node has a specific children.
	 */
	bool hasChildren(int index) const;

	/**
	 * Returns the count of children of this node.
	 */
	int getChildrenCount() const;

	/**
	 * Returns the index'd children. Creates it if it doesn't exist.
	 */
	Octree* getChildren(int index);

	/**
	 * Returns the (const) index'd children. Returns nullptr if it doesn't exist.
	 */
	const Octree* getChildren(int index) const;

	/**
	 * Returns whether this node has a color (= reference > 0).
	 */
	bool hasColor() const;

	/**
	 * Returns the color of the node (as average of the red, green and blue values).
	 */
	RGBAPixel getColor() const;

	/**
	 * Returns how many color this node represents.
	 */
	int getCount() const;

	/**
	 * Adds a color to this node.
	 */
	void setColor(RGBAPixel color);

	/**
	 * Reduces the colors of this node to the parent node and automatically removes the
	 * node from the parent. You have to delete the node after that on your own.
	 */
	void reduceToParent();

	/**
	 * Returns the color palette index of the color associated with this node (if any).
	 */
	int getColorID() const;

	/**
	 * Sets the color palette index.
	 */
	void setColorID(int color_id);

	/**
	 * Adds this color to the subtree colors array of all parent nodes. Set the color
	 * index BEFORE you call this method.
	 */
	void updateParents();

	/**
	 * Returns the leaf node which should represent a specific color. It traverses to
	 * the leaf node and creates all nodes on that path if they don't exist.
	 */
	static Octree* findOrCreateNode(Octree* octree, RGBAPixel color);

	/**
	 * Finds the index of the color (from the color palette) which is the nearest to
	 * a specified color.
	 *
	 * This only works if the nodes have cached the available colors in their subtrees,
	 * e.g. you have to create an octree, insert your palette colors, give them IDs,
	 * and call the updateParents-method on the nodes of the palette colors.
	 */
	static int findNearestColor(const Octree* octree, RGBAPixel color);

protected:
	// parent and children of this node
	Octree* parent;
	Octree* children[16];
	int level;

	// how many colors this node represents
	// only leaves or reduced nodes have a reference != 0
	int reference;
	// sum of represented colors -> average is color of this node
	int red, green, blue, alpha;
	
	
	// index of the belonging color in the color palette (if any)
	int color_id;
	// TODO link with color palette?
	// array of palette colors (color index, color) in subtrees of this node
	std::vector<std::pair<int, RGBAPixel>> subtree_colors;
};

/**
 * Implements a color palette with fast (almost-)nearest-color-access using a octree.
 *
 * "almost" because every node stores which colors are available in the subtrees.
 * When you reach a node where the children where you want to go to doesn't exist, it
 * searches in this list for the nearest color. That's not exactly 100% accurate
 * somtimes, but good enough.
 */
class OctreePalette : public Palette {
public:
	/**
	 * Constructor. Takes an array of palette colors.
	 */
	OctreePalette(const std::vector<RGBAPixel>& colors);

	virtual ~OctreePalette();

	virtual const std::vector<RGBAPixel>& getColors() const;
	virtual int getNearestColor(const RGBAPixel& color) const;

protected:
	// available colors
	std::vector<RGBAPixel> colors;
	// octree used to efficiently find nearest colors
	Octree octree;
};

/**
 * Quantizes the colors of a given image to max_colors >= colors. Stores the palette
 * colors in the supplied vector and also the used octree to quantize the colors in the
 * octree pointer pointer if you need it.
 */
void octreeColorQuantize(const RGBAImage& image, size_t max_colors,
		std::vector<RGBAPixel>& colors, Octree** octree = nullptr);

}
}

#endif /* IMAGE_QUANTIZATION_H_ */

