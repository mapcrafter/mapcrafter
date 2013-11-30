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

#include "other.h"

#include <iostream>
#include <cstdio>
#include <ctime>

namespace mapcrafter {
namespace util {

std::string format_eta(int eta) {
	int MINUTES = 60;
	int HOURS = 60*MINUTES;
	int DAYS = 24*HOURS;

	int days = eta / DAYS;
	eta -= days * DAYS;
	int hours = eta / HOURS;
	eta -= hours * HOURS;
	int minutes = eta / MINUTES;
	eta -= minutes * MINUTES;
	int seconds = eta;

	std::string str_days = util::str(days) + "d";
	std::string str_hours = util::str(hours) + "h";
	std::string str_minutes = util::str(minutes) + "m";
	std::string str_seconds = util::str(seconds) + "s";

	if (days > 0)
		return str_days + " " + str_hours;
	if (hours > 0)
		return str_hours + " " + str_minutes;
	if (minutes > 0)
		return str_minutes + " " + str_seconds;
	return str_seconds;
}

DummyProgressHandler::DummyProgressHandler()
	: max(0), value(0) {
}

DummyProgressHandler::~DummyProgressHandler() {
}

int DummyProgressHandler::getMax() const {
	return max;
}

void DummyProgressHandler::setMax(int max) {
	this->max = max;
}

int DummyProgressHandler::getValue() const {
	return value;
}

void DummyProgressHandler::setValue(int value) {
	this->value = value;
}

ProgressBar::ProgressBar(int max, bool animated)
		: animated(animated), start(time(NULL)), last_update(0),
		  last_value(0), last_percent(0), last_output_len(0) {
	setMax(max);
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::setAnimated(bool animated) {
	this->animated = animated;
}

bool ProgressBar::isAnimated() const {
	return animated;
}

void ProgressBar::setValue(int value) {
	update(value);
	this->value = value;
}

void ProgressBar::update(int value, bool force) {
	// check when animated if we are at 100% and this is this the first time we are at 100%
	// so we show the progress bar only one time at the end
	if (!force && !animated && (value != max || (value == max && last_value == max)))
		return;
	int now = time(NULL);
	// check if the time since the last show and the change was big enough to show a progress
	double percent = value / (double) max * 100.;
	if (!force && last_update + 1 > now && !(last_percent != max && value == max))
		return;

	// now calculate the current and average speed
	double speed = (double) (value - last_value) / (now - last_update);
	double average_speed = (double) value / (now - start);
	if (value == max)
		// at the end use the average speed
		speed = average_speed;

	// show the progress bar
	std::string output = "[";
	for (int i = 0; i <= 100; i += 2) {
		if (i > percent)
			output += " ";
		else if (percent - 2 < i)
			output += ">";
		else
			output += "=";
	}
	// and the progress, current value and speed
	char fpercent[20];
	char fspeed[20];
	sprintf(&fpercent[0], "%.2f%%", percent);
	sprintf(&fspeed[0], "%.2f", speed);
	output += "] ";
	output += std::string(fpercent) + " ";
	output += util::str(value) + "/" + util::str(max) + " ";
	output += std::string(fspeed) + "/s ";

	// show ETA, but not at the end
	if (value != max) {
		int eta = (max - value) / average_speed;
		output += "ETA " + format_eta(eta);
	}

	// go to the begin of the line and clear it
	if (animated) {
		std::cout << "\r";
		for (int i = 0; i < last_output_len; i++)
			std::cout << " ";
		std::cout << "\r";
	}

	std::cout << output;
	if (animated) {
		std::cout.flush();
	} else
		std::cout << std::endl;
	// set this as last shown
	last_update = now;
	last_value = value;
	last_percent = percent;
	last_output_len = output.size();
}

void ProgressBar::finish() {
	update(max);
	if (animated)
		std::cout << std::endl;
}

} /* namespace util */
} /* namespace mapcrafter */
