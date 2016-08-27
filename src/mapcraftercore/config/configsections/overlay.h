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

#ifndef SECTIONS_OVERLAY_H_
#define SECTIONS_OVERLAY_H_

#include "../configsection.h"
#include "../validation.h"
#include "../../mc/worldcrop.h"
#include "../../renderer/overlay.h"
#include "../../util.h"

#include <iosfwd>
#include <string>

namespace mapcrafter {
namespace config {

enum class OverlayType {
	AREA,
	HEIGHT,
	LIGHTING,
	LIGHTLEVEL,
	SLIME,
	SPAWN,
};

std::ostream& operator<<(std::ostream& out, OverlayType overlay);

class OverlaySection : public ConfigSection {
public:
	OverlaySection();
	virtual ~OverlaySection();

	virtual std::string getPrettyName() const;
	virtual void dump(std::ostream& out) const;

	std::string getID() const;
	std::string getName() const;
	OverlayType getType() const;
	bool isBase() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	Field<std::string> name;
	Field<bool> base;

	Field<bool> day;
	Field<double> lighting_intensity, lighting_water_intensity;
};

class DummyOverlaySection : public OverlaySection {
};

class AreaOverlaySection : public OverlaySection {
public:
	AreaOverlaySection();

	virtual void dump(std::ostream& out) const;

	const util::Color& getColor() const;
	const mc::Area& getArea() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	mc::Area area;

	Field<util::Color> color;
	Field<int> min_y, max_y;
	Field<int> min_x, max_x, min_z, max_z;
	Field<int> center_x, center_z, radius;
};

struct HeightColor {
	int y;
	util::Color color;
};

std::ostream& operator<<(std::ostream& out, const HeightColor& color);

class HeightOverlaySection : public OverlaySection {
public:
	virtual void dump(std::ostream& out) const;

	int getDefaultOpacity() const;
	const std::vector<HeightColor> getColors() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);
	virtual void postParse(const INIConfigSection& section,
			ValidationList& validation);

private:
	Field<int> default_opacity;
	Field<std::string> colors;
	std::vector<HeightColor> colors_vector;
};

class LightingOverlaySection : public OverlaySection {
public:
	virtual void dump(std::ostream& out) const;

	bool isDay() const;
	double getIntensity() const;
	double getWaterIntensity() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);

private:
	Field<bool> day;
	Field<double> intensity, water_intensity;
};

class LightLevelOverlaySection : public OverlaySection {
public:
	virtual void dump(std::ostream& out) const;

	bool isDay() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);

private:
	Field<bool> day;
};

class SlimeOverlaySection : public OverlaySection {
public:
	virtual void dump(std::ostream& out) const;

	util::Color getColor() const;
	int getOpacity() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);

private:
	Field<util::Color> color;
	Field<int> opacity;
};

class SpawnOverlaySection : public OverlaySection {
public:
	virtual void dump(std::ostream& out) const;

	bool isDay() const;
	util::Color getColor() const;

protected:
	virtual void preParse(const INIConfigSection& section,
			ValidationList& validation);
	virtual bool parseField(const std::string key, const std::string value,
			ValidationList& validation);

private:
	Field<bool> day;
	Field<util::Color> color;
};

} /* namespace config */
} /* namespace mapcrafter */

#endif /* SECTIONS_OVERLAY_H_ */
