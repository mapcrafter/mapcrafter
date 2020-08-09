/*
 * Copyright 2012-2018 Moritz Hilscher
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

#include "blockstate.h"

#include "../util.h"

#include <cassert>

namespace mapcrafter {
namespace mc {

BlockState::BlockState(std::string name)
	: name(name) {
	updateVariantDescription();
}

std::string BlockState::getName() const {
	return name;
}

const std::map<std::string, std::string>& BlockState::getProperties() const {
	return properties;
}

bool BlockState::hasProperty(std::string key) const {
	return properties.count(key);
}

std::string BlockState::getProperty(std::string key, std::string default_value) const {
	if (!properties.count(key)) {
		return default_value;
	}
	return properties.at(key);
}

void BlockState::setProperty(std::string key, std::string value) {
	properties[key] = value;
	updateVariantDescription();
}

const std::string BlockState::getVariantDescription() const {
	return variant_description;
}

bool BlockState::operator<(const BlockState& other) const {
	return variant_description < other.variant_description;
}

BlockState BlockState::parse(std::string name, std::string variant_description) {
	mc::BlockState block(name);
	block.properties = util::parseProperties(variant_description);
	block.updateVariantDescription();
	return block;
}

void BlockState::updateVariantDescription() {
	variant_description = "";
	for (auto it = properties.begin(); it != properties.end(); ++it) {
		variant_description += it->first + "=" + it->second + ",";
	}
}

BlockStateRegistry::BlockStateRegistry()
	: unknown_block("mapcrafter:unknown") {
}

uint16_t BlockStateRegistry::getBlockID(const BlockState& block) {
	std::lock_guard<std::mutex> guard(mutex);

	// first check if that block name is known
	auto it = block_lookup.find(block.getName());
	if (it == block_lookup.end()) {
		// block name unknown -> insert block
		uint16_t id = block_states.size();
		block_lookup[block.getName()][block.getVariantDescription()] = id;
		block_states.push_back(block);
		return id;
	}

	// block name is known -> just search for the variant now
	auto it2 = it->second.find(block.getVariantDescription());
	if (it2 == it->second.end()) {
		// variant not found -> insert block
		uint16_t id = block_states.size();
		it->second[block.getVariantDescription()] = id;
		block_states.push_back(block);
		return id;
	}
	return it2->second;
}

const BlockState& BlockStateRegistry::getBlockState(uint16_t id) const {
	if (id >= block_states.size()) {
		assert(false);
		return unknown_block;
	}
	return block_states.at(id);
}

void BlockStateRegistry::addKnownProperty(std::string block, std::string property) {
	known_properties[block].insert(property);
}

bool BlockStateRegistry::isKnownProperty(std::string block, std::string property) const {
	auto it = known_properties.find(block);
	if (it == known_properties.end()) {
		return false;
	}
	return it->second.count(property);
}

}
}

