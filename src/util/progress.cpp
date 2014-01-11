/*
 * Copyright 2012-2014 Moritz Hilscher
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
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#include <unistd.h> // STDOUT_FILENO

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
	if (minutes < 10)
		str_minutes = "0" + str_minutes;
	std::string str_seconds = util::str(seconds) + "s";
	if (seconds < 10)
		str_seconds = "0" + str_seconds;

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
		  last_value(0), last_percentage(0), last_output_len(0) {
	setMax(max);
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::update(int value) {
	// check when animated if we are at 100% and this is this the first time we are at 100%
	// so we show the progress bar only one time at the end
	if (!animated && (value != max || (value == max && last_value == max)))
		return;
	int now = time(NULL);
	// check whether the time since the last show and the change was big enough to show a progress
	double percentage = value / (double) max * 100.;
	if (last_update + 1 > now && !(last_percentage != max && value == max))
		return;

	// now calculate the average speed
	double average_speed = (double) value / (now - start);

	// try to determine the width of the terminal
	// use 80 columns as default if we can't determine a terminal size
	int terminal_width = 80;
#ifdef TIOCGWINSZ
	struct winsize ws = {0, 0, 0, 0};
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	if (ws.ws_col != 0)
		terminal_width = ws.ws_col;
#endif

	// create the progress stats: percentage, current/maximum value, speed, eta
	std::string stats;
	// show eta only when we have an average speed
	if (value != max && value != 0 && (now - start) != 0) {
		int eta = (max - value) / average_speed;
		stats = createProgressStats(percentage, value, max, average_speed, eta);
	} else {
		stats = createProgressStats(percentage, value, max, average_speed);
	}

	// now create the progress bar
	// with the remaining size minus one as size
	// (because the space between progress and stats)
	int progressbar_width = terminal_width - stats.size() - 1;
	std::string progressbar = createProgressBar(progressbar_width, percentage);

	// go to the begin of the line and clear it
	if (animated) {
		std::cout << "\r";
		for (int i = 0; i < last_output_len; i++)
			std::cout << " ";
		std::cout << "\r";
	}

	// now show everything
	std::cout << progressbar << " " << stats;
	if (animated) {
		std::cout << "\r";
		std::cout.flush();
	} else
		std::cout << std::endl;

	// set this as last shown
	last_update = now;
	last_value = value;
	last_percentage = percentage;
	last_output_len = progressbar.size() + 1 + stats.size();
}

std::string ProgressBar::createProgressBar(int width, double percentage) const {
	// width - 2 because we need two characters for [ and ]
	width -= 2;

	std::string progressbar = "[";
	double progress_step = (double) 100 / width;
	for (int i = 0; i < width; i++) {
		double current = progress_step * i;
		if (current > percentage)
			progressbar += " ";
		else if (percentage - progress_step < current)
			progressbar += ">";
		else
			progressbar += "=";
	}
	return progressbar + "]";
}

std::string ProgressBar::createProgressStats(double percentage, int value, int max,
		double speed_average, int eta) const {
	std::string stats;
	char formatted_percent[20], formatted_speed_average[20];
	sprintf(&formatted_percent[0], "%.2f%%", percentage);
	sprintf(&formatted_speed_average[0], "%.2f", speed_average);
	stats += std::string(formatted_percent) + " ";
	stats += util::str(value) + "/" + util::str(max) + " ";
	stats += std::string(formatted_speed_average) + "/s ";

	if (eta != -1)
		stats += "ETA " + format_eta(eta);

	// add some padding to these stats
	// to prevent the progress bar changing the size all the time
	int padding = 20 - (stats.size() % 20);
	return stats + std::string(padding, ' ');
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

void ProgressBar::finish() {
	setValue(max);
	if (animated)
		std::cout << std::endl;
}

} /* namespace util */
} /* namespace mapcrafter */
