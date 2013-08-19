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

#include "progress.h"

#include <iostream>
#include <ctime>

namespace mapcrafter {
namespace util {

ProgressBar::ProgressBar()
		: max(0), animated(true), start(time(NULL)), last_update(0), last_value(0),
		  last_percent(0) {
}

ProgressBar::ProgressBar(int max, bool animated)
		: max(max), animated(animated), start(time(NULL)), last_update(0), last_value(0),
		  last_percent(0) {
}

void ProgressBar::setMax(int max) {
	this->max = max;
}

int ProgressBar::getMax() const {
	return max;
}

void ProgressBar::update(int value) {
	// check when animated if we are at 100% and this is this the first time we are at 100%
	// so we show the progress bar only one time at the end
	if (!animated && (value != max || (value == max && last_value == max)))
		return;
	int now = time(NULL);
	// check if the time since the last show and the change was big enough to show a progress
	double percent = value / (double) max * 100.;
	if (last_update + 1 > now && !(last_percent != max && value == max))
		return;

	// now calculate the speed
	double speed = (double) (value - last_value) / (now - last_update);
	if (value == max)
		// at the end an average speed
		speed = (double) value / (now - start);

	if (animated)
		// go to the begin of the line
		std::cout << "\033[K";
	// show the progress bar
	std::cout << "[";
	for (int i = 0; i <= 100; i += 2) {
		if (i > percent)
			std::cout << " ";
		else if (percent - 2 < i)
			std::cout << ">";
		else
			std::cout << "=";
	}
	// and the progress, current value and speed
	char fpercent[20];
	char fspeed[20];
	sprintf(&fpercent[0], "%.2f%%", percent);
	sprintf(&fspeed[0], "%.2f", speed);
	std::cout << "] " << fpercent << " " << value << "/" << max << " " << fspeed;
	if (animated) {
		std::cout << "/s\r";
		std::cout.flush();
	} else
		std::cout << std::endl;
	// set this as last show
	last_update = now;
	last_value = value;
	last_percent = percent;
}

void ProgressBar::setAnimated(bool animated) {
	this->animated = animated;
}

bool ProgressBar::isAnimated() const {
	return animated;
}

void ProgressBar::finish() {
	update(max);
	if (animated)
		std::cout << std::endl;
}

} /* namespace util */
} /* namespace mapcrafter */
