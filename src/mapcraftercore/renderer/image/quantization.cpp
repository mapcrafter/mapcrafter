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

#include "quantization.h"

#include <set>
#include <queue>

namespace mapcrafter {
namespace renderer {

Octree::Octree(Octree* parent, int level)
	: parent(parent), level(level), reference(0), red(0), green(0), blue(0), alpha(0), color_id(-1) {
	for (int i = 0; i < 16; i++)
		children[i] = nullptr;
}

Octree::~Octree() {
	for (int i = 0; i < 16; i++)
		if (children[i])
			delete children[i];
}

Octree* Octree::getParent() {
	return parent;
}

const Octree* Octree::getParent() const {
	return parent;
}

int Octree::getLevel() const {
	return level;
}

bool Octree::isRoot() const {
	return parent == nullptr;
}

bool Octree::isLeaf() const {
	for (int i = 0; i < 16; i++)
		if (children[i])
			return false;
	return true;
}

bool Octree::hasChildren(int index) const {
	assert(index >= 0 && index < 16);
	return children[index] != nullptr;
}

int Octree::getChildrenCount() const {
	int count = 0;
	for (int i = 0; i < 16; i++)
		if (children[i])
			count++;
	return count;
}

Octree* Octree::getChildren(int index) {
	assert(index >= 0 && index < 16);
	if (!children[index])
		children[index] = new Octree(this, level + 1);
	return children[index];
}

const Octree* Octree::getChildren(int index) const {
	assert(index >= 0 && index < 16);
	if (!children[index])
		return nullptr;
	return children[index];
}

bool Octree::hasColor() const {
	return reference > 0;
}

RGBAPixel Octree::getColor() const {
	assert(hasColor());
	return rgba(red / reference, green / reference, blue / reference, alpha / reference);
}

int Octree::getCount() const {
	return reference;
}

void Octree::setColor(RGBAPixel color) {
	reference++;
	red += rgba_red(color);
	green += rgba_green(color);
	blue += rgba_blue(color);
	alpha += rgba_alpha(color);
}

void Octree::reduceToParent() {
	assert(isLeaf());
	assert(!isRoot());
	
	parent->reference += reference;
	parent->red += red;
	parent->green += green;
	parent->blue += blue;
	parent->alpha += alpha;
	
	for (int i = 0; i < 16; i++) {
		if (parent->children[i] == this) {
			parent->children[i] = nullptr;
			break;
		}
	}
}

void Octree::setColorID(int color_id) {
	this->color_id = color_id;
}

int Octree::getColorID() const {
	assert(color_id != -1);
	return color_id;
}

void Octree::updateParents() {
	Octree* node = parent;
	while (node) {
		node->subtree_colors.push_back(std::make_pair(color_id, getColor()));
		node = node->getParent();
	}
}

namespace {

int nth_bit(int x, int n) {
	return (x >> n) & 1;
}

}

Octree* Octree::findOrCreateNode(Octree* octree, RGBAPixel color) {
	assert(octree != nullptr);

	uint8_t red = rgba_red(color);
	uint8_t green = rgba_green(color);
	uint8_t blue = rgba_blue(color);
	uint8_t alpha = rgba_alpha(color);

	Octree* node = octree;
	for (int i = 7; i >= 8 - OCTREE_COLOR_BITS; i--) {
		int index = (nth_bit(red, i) << 3) | (nth_bit(green, i) << 2) | nth_bit(blue, i) << 1 | nth_bit(alpha, i);
		node = node->getChildren(index);
		assert(node != nullptr);
	}
	return node;
}

int Octree::findNearestColor(const Octree* octree, RGBAPixel color) {
	assert(octree != nullptr);

	uint8_t red = rgba_red(color);
	uint8_t green = rgba_green(color);
	uint8_t blue = rgba_blue(color);
	uint8_t alpha = rgba_alpha(color);

	const Octree* node = octree;
	for (int i = 7; i >= 8 - OCTREE_COLOR_BITS; i--) {
		if (node->hasColor())
			break;
		int index = (nth_bit(red, i) << 3) | (nth_bit(green, i) << 2) | nth_bit(blue, i) << 1 | nth_bit(alpha, i);
		if (node->hasChildren(index))
			node = node->getChildren(index);
		else
			break;
	}

	if (node->hasColor())
		return node->getColorID();
	auto& colors = node->subtree_colors;
	int min_distance = -1;
	int best_color = -1;
	for (auto it = colors.begin(); it != colors.end(); ++it) {
		int distance = rgba_distance2(color, it->second);
		if (best_color == -1 || distance < min_distance) {
			min_distance = distance;
			best_color = it->first;
		}
	}
	// this shouldn't happen if all the colors are cached
	assert(best_color != -1);
	return best_color;
}

SubPalette::SubPalette(const std::vector<RGBAPixel>& palette_colors)
	: initialized(false), palette_colors(palette_colors) {
}

int SubPalette::getNearestColor(const RGBAPixel& color) {
	if (!initialized)
		initialize(color);

	int min_distance = 256 * 256 * 4;
	int best_color = -1;
	for (size_t j = 0; j < colors.size(); j++) {
		int i = colors[j];
		int distance = rgba_distance2(palette_colors[i], color);
		if (distance < min_distance) {
			min_distance = distance;
			best_color = i;
		}
		if (distance == 0)
			return i;
	}

	assert(best_color != -1);
	return best_color;
}

void SubPalette::initialize(const RGBAPixel& c) {
	RGBAPixel center = rgba(
		(OctreePalette2::BIN_FOR_COLOR(rgba_red(c)) * 256 + 128) / OctreePalette2::BINS,
		(OctreePalette2::BIN_FOR_COLOR(rgba_green(c)) * 256 + 128) / OctreePalette2::BINS,
		(OctreePalette2::BIN_FOR_COLOR(rgba_blue(c)) * 256 + 128) / OctreePalette2::BINS,
		(OctreePalette2::BIN_FOR_COLOR(rgba_alpha(c)) * 256 + 128) / OctreePalette2::BINS
	);

	int nearest = 256 * 256 * 4;
	for (size_t i = 0; i < palette_colors.size(); i++) {
		int distance = rgba_distance2(palette_colors[i], center);
		if (distance < nearest)
			nearest = distance;
		if (nearest == 0)
			break;
	}

	double tmp = sqrt(nearest) + 2 * sqrt(2) * (128 / OctreePalette2::BINS);
	int nearest_dist = (tmp * tmp) + 1;
	for (size_t i = 0; i < palette_colors.size(); i++) {
		int distance = rgba_distance2(palette_colors[i], center);
		if (distance <= nearest_dist)
			colors.push_back(i);
	}

	initialized = true;
}

OctreePalette::OctreePalette(const std::vector<RGBAPixel>& colors)
	: colors(colors) {
	// add each color to the octree, assign a palette index and update parents
	for (size_t i = 0; i < colors.size(); i++) {
		RGBAPixel color = colors[i];
		Octree* node = Octree::findOrCreateNode(&octree, color);
		node->setColor(color);
		node->setColorID(i);
		node->updateParents();
	}
}

OctreePalette::~OctreePalette() {
}

const std::vector<RGBAPixel>& OctreePalette::getColors() const {
	return colors;
}

int OctreePalette::getNearestColor(const RGBAPixel& color) {
	// now just automagically find the best color
	return Octree::findNearestColor(&octree, color);
}	

OctreePalette2::OctreePalette2(const std::vector<RGBAPixel>& colors)
	: colors(colors) {
	sub_palettes.resize(OctreePalette2::BINS_ALL, nullptr);
}

OctreePalette2::~OctreePalette2() {
	for (size_t i = 0; i < sub_palettes.size(); i++) {
		if (sub_palettes[i] != nullptr)
			delete sub_palettes[i];
	}
}

const std::vector<RGBAPixel>& OctreePalette2::getColors() const {
	return colors;
}

int OctreePalette2::getNearestColor(const RGBAPixel& color) {
	// find the belonging sub palette for this color and ask it for the nearest color
	int bins = OctreePalette2::BINS;
	size_t index = OctreePalette2::BIN_FOR_COLOR(rgba_red(color));
	index += bins * OctreePalette2::BIN_FOR_COLOR(rgba_green(color));
	index += bins * bins * OctreePalette2::BIN_FOR_COLOR(rgba_blue(color));
	index += bins * bins * bins * OctreePalette2::BIN_FOR_COLOR(rgba_alpha(color));
	assert(index < sub_palettes.size());
	if (sub_palettes[index] == nullptr)
		sub_palettes[index] = new SubPalette(colors);
	return sub_palettes[index]->getNearestColor(color);
}

namespace {

struct NodeComparator {
	bool operator()(const Octree* node1, const Octree* node2) const {
		// reduce nodes on higher levels first
		if (node1->getLevel() != node2->getLevel())
			return node1->getLevel() < node2->getLevel();
		// reduce nodes with fewer colors first
		if (node1->getCount() != node2->getCount())
			return node1->getCount() > node2->getCount();
		return node1 < node2;
	};
};

}

/**
 * Simple octree color quantization: Similar to http://rosettacode.org/wiki/Color_quantization#C
 */
void octreeColorQuantize(const RGBAImage& image, size_t max_colors,
		std::vector<RGBAPixel>& colors, Octree** octree) {
	assert(max_colors > 0);

	// have an octree with the colors as leaves
	Octree* internal_octree = new Octree();
	// and a priority queue of leaves to be processed
	// the order of leaves is very important, see NodeComparator
	std::priority_queue<Octree*, std::vector<Octree*>, NodeComparator> queue;

	// insert the colors into the octree
	for (int x = 0; x < image.getWidth(); x++) {
		for (int y = 0; y < image.getHeight(); y++) {
			RGBAPixel color = image.pixel(x, y);
			Octree* node = Octree::findOrCreateNode(internal_octree, color);
			node->setColor(color);
			// add the leaf only once to the queue
			if (node->getCount() == 1)
				queue.push(node);
		}
	}

	// now: reduce the leaves until we have less colors than maximum
	while (queue.size() > max_colors) {
		Octree* node = queue.top();
		assert(node->isLeaf());
		queue.pop();
		
		// add the color value of the leaf to the parent
		node->reduceToParent();
		Octree* parent = node->getParent();
		// delete the leaf (leaf is automatically removed from parent in reduceToParent())
		delete node;

		// add parent to queue if it is a leaf now
		if (parent->isLeaf())
			queue.push(parent);
	}

	// gather the quantized colors
	while (queue.size()) {
		Octree* node = queue.top();
		assert(node->isLeaf());
		node->setColorID(colors.size());
		colors.push_back(node->getColor());
		queue.pop();
	}

	if (octree != nullptr)
		*octree = internal_octree;
	else
		delete internal_octree;
}

}
}
