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

namespace mapcrafter {
namespace renderer {

Octree::Octree(Octree* parent)
	: parent(parent), reference(0), red(0), green(0), blue(0), color_id(-1) {
	for (int i = 0; i < 8; i++)
		children[i] = nullptr;
}

Octree::~Octree() {
	for (int i = 0; i < 8; i++)
		if (children[i])
			delete children[i];
}

Octree* Octree::getParent() {
	return parent;
}

const Octree* Octree::getParent() const {
	return parent;
}

bool Octree::isLeaf() const {
	for (int i = 0; i < 8; i++)
		if (children[i])
			return false;
	return true;
}

bool Octree::hasChildren(int index) const {
	assert(index >= 0 && index < 8);
	return children[index] != nullptr;
}

int Octree::getChildrenCount() const {
	int count = 0;
	for (int i = 0; i < 8; i++)
		if (children[i])
			count++;
	return count;
}

Octree* Octree::getChildren(int index) {
	assert(index >= 0 && index < 8);
	if (!children[index])
		children[index] = new Octree(this);
	return children[index];
}

const Octree* Octree::getChildren(int index) const {
	assert(index >= 0 && index < 8);
	if (!children[index])
		return nullptr;
	return children[index];
}

bool Octree::hasColor() const {
	return reference > 0;
}

RGBAPixel Octree::getColor() const {
	assert(hasColor());
	// TODO how to handle alpha values?
	return rgba(red / reference, green / reference, blue / reference, 255);
}

void Octree::setColor(RGBAPixel color) {
	// TODO make sure we are setting the right color?
	reference++;
	red += rgba_red(color);
	green += rgba_green(color);
	blue += rgba_blue(color);
}

void Octree::reduceColor() {
	assert(!isLeaf());
	reference = red = green = blue = 0;
	for (int i = 0; i < 8; i++) {
		if (!children[i])
			continue;
		assert(children[i]->hasColor());
		reference += children[i]->reference;
		red += children[i]->red;
		green += children[i]->green;
		blue += children[i]->blue;
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

	Octree* node = octree;
	for (int i = 7; i >= 0; i--) {
		int index = (nth_bit(red, i) << 2) | (nth_bit(green, i) << 1) | nth_bit(blue, i);
		node = node->getChildren(index);
		assert(node != nullptr);
	}
	return node;
}

const Octree* Octree::findNearestNode(const Octree* octree, RGBAPixel color) {
	assert(octree != nullptr);

	uint8_t red = rgba_red(color);
	uint8_t green = rgba_green(color);
	uint8_t blue = rgba_blue(color);

	const Octree* node = octree;
	for (int i = 7; i >= 0; i--) {
		if (node->hasColor())
			return node;
		int index = (nth_bit(red, i) << 2) | (nth_bit(green, i) << 1) | nth_bit(blue, i);
		node = node->getChildren(index);
		if (node == nullptr)
			return nullptr;
	}
	return node;
}

namespace {

int colorDistanceSquare(RGBAPixel color1, RGBAPixel color2) {
	return std::pow(rgba_red(color1) - rgba_red(color2), 2)
			+ std::pow(rgba_green(color1) - rgba_green(color2), 2)
			+ std::pow(rgba_blue(color1) - rgba_blue(color2), 2);
}

}

int Octree::findNearestColor(const Octree* octree, RGBAPixel color) {
	assert(octree != nullptr);

	uint8_t red = rgba_red(color);
	uint8_t green = rgba_green(color);
	uint8_t blue = rgba_blue(color);

	const Octree* node = octree;
	for (int i = 7; i >= 0; i--) {
		if (node->hasColor())
			break;
		int index = (nth_bit(red, i) << 2) | (nth_bit(green, i) << 1) | nth_bit(blue, i);
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
		int distance = colorDistanceSquare(color, it->second);
		if (best_color == -1 || distance < min_distance) {
			min_distance = distance;
			best_color = it->first;
		}
	}
	return best_color;
}

OctreePalette::OctreePalette(const std::vector<RGBAPixel>& colors)
	: colors(colors) {
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

int OctreePalette::getNearestColor(const RGBAPixel& color) const {
	return Octree::findNearestColor(&octree, color);
}

void octreeColorQuantize(const RGBAImage& image, size_t max_colors, std::vector<RGBAPixel>& colors, Octree** octree) {
	Octree* internal_octree = new Octree();

	// add colors to octree
	std::set<Octree*> parents, next_parents;
	for (int x = 0; x < image.getWidth(); x++) {
		for (int y = 0; y < image.getHeight(); y++) {
			RGBAPixel color = image.pixel(x, y);
			Octree* node = Octree::findOrCreateNode(internal_octree, color);
			node->setColor(color);
			next_parents.insert(node->getParent());
		}
	}

	// color reduce parents of leaves, layer by layer
	// TODO this is still the simple version, improve it
	// - better way of managening leaves with their parents
	// - parents sorted by their children reference sums?
	size_t leaves_count = 0;
	for (int level = 7; level >= 0; level--) {
		parents = next_parents;
		next_parents.clear();

		leaves_count = 0;
		for (auto parent_it = parents.begin(); parent_it != parents.end(); ++parent_it)
			leaves_count += (*parent_it)->getChildrenCount();

		while (parents.size() > 0 && leaves_count > max_colors) {
			Octree* parent = *parents.begin();
			assert(!parent->isLeaf());
			parents.erase(parents.begin());
			next_parents.insert(parent->getParent());
			leaves_count -= parent->getChildrenCount() - 1;
			parent->reduceColor();
		}

		if (leaves_count <= max_colors)
			break;
	}

	// gather reduced colors
	for (auto parent_it = parents.begin(); parent_it != parents.end(); ++parent_it) {
		for (int i = 0; i < 8; i++)
			if ((*parent_it)->hasChildren(i)) {
				Octree* node = (*parent_it)->getChildren(i);
				node->setColorID(colors.size());
				colors.push_back(node->getColor());
			}
	}
	for (auto parent_it = next_parents.begin(); parent_it != next_parents.end(); ++parent_it) {
		for (int i = 0; i < 8; i++) {
			if ((*parent_it)->hasChildren(i)) {
				Octree* children = (*parent_it)->getChildren(i);
				if (!parents.count(children)) {
					children->setColorID(colors.size());
					colors.push_back(children->getColor());
				}
			}
		}
	}

	if (octree != nullptr)
		*octree = internal_octree;
	else
		delete internal_octree;
}

}
}
