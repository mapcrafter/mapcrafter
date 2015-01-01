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

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <iostream>

namespace mapcrafter {
namespace util {

/**
 * Returns whether stdout is connected to a terminal.
 *
 * Internally calls isatty(...) on Linux/Unix and _isatty(...) on Windows.
 */
bool isOutTTY();

enum class TerminalColorStates {
	ENABLED,
	DISABLED,
	AUTO
};

/**
 * std::ostream manipulator to change the terminal color.
 *
 * Use it like: out << setcolor(setcolor::foreground, setcolor::red) << "Red text";
 * or better: out << setfgcolor(setcolor::red) << "Red text";
 * You can set the background color with setcolor::background/setbgcolor.
 *
 * You can reset the text color with: out << setcolor::reset;
 */
class setcolor {
public:
	setcolor(int type = 0, int color = 0);
	std::ostream& operator()(std::ostream& out) const;

	static std::ostream& reset(std::ostream& out);

	static void setEnabled(TerminalColorStates enabled);

	static const int foreground = 30;
	static const int background = 40;

	static const int black = 0;
	static const int red = 1;
	static const int green = 2;
	static const int yellow = 3;
	static const int blue = 4;
	static const int magenta = 5;
	static const int cyan = 6;
	static const int white = 7;

private:
	// type (set foreground/background) and the color code
	int type, color;

	/**
	 * Returns whether colored terminal output is enabled.
	 */
	static bool isEnabled();
	static bool enabled_initialized, enabled;
};

class setfgcolor : public setcolor {
public:
	setfgcolor(int color) : setcolor(setcolor::foreground, color) {}
};

class setbgcolor : public setcolor {
public:
	setbgcolor(int color) : setcolor(setcolor::background, color) {}
};

std::ostream& operator<<(std::ostream& out, const setcolor& color);

} /* namespace util */
} /* namespace mapcrafter */

#endif /* TERMINAL_H_ */
