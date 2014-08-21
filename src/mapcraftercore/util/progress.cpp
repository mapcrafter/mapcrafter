/*
 * Copyright 2012-2014 Moritz Hilscher
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

#include "progress.h"

#include "../util.h"

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <ctime>
#if defined(HAVE_SYS_IOCTL_H) && defined(HAVE_UNISTD_H)
#  include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#  include <unistd.h> // STDOUT_FILENO
#if defined(OS_WINDOWS)
#  include <windows.h>
#endif
#endif

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

MultiplexingProgressHandler::MultiplexingProgressHandler()
	: max(0), value(0) {
}

MultiplexingProgressHandler::~MultiplexingProgressHandler() {
}

void MultiplexingProgressHandler::addHandler(IProgressHandler* handler) {
	handlers.push_back(std::shared_ptr<IProgressHandler>(handler));
}

int MultiplexingProgressHandler::getMax() const {
	return max;
}

void MultiplexingProgressHandler::setMax(int max) {
	this->max = max;
	for (auto handler_it = handlers.begin(); handler_it != handlers.end(); ++handler_it)
		(*handler_it)->setMax(max);
}

int MultiplexingProgressHandler::getValue() const {
	return value;
}

void MultiplexingProgressHandler::setValue(int value) {
	this->value = value;
	for (auto handler_it = handlers.begin(); handler_it != handlers.end(); ++handler_it)
		(*handler_it)->setValue(value);
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

AbstractOutputProgressHandler::AbstractOutputProgressHandler()
	: start(std::time(nullptr)), last_update(0), last_value(0), last_percentage(0) {
}

AbstractOutputProgressHandler::~AbstractOutputProgressHandler() {
}

void AbstractOutputProgressHandler::setValue(int value) {
	int now = std::time(nullptr);
	// check whether the time since the last shown update
	// and the change was big enough to show a new update
	double percentage = value / (double) max * 100.;
	if (last_update + 1 > now && !(last_percentage != max && value == max)) {
		this->value = value;
		return;
	}

	// now calculate the average speed
	double average_speed = (double) value / (now - start);

	// eta only when we have an average speed
	int eta = -1;
	if (value != max && value != 0 && (now - start) != 0)
		eta = (max - value) / average_speed;

	// set this as last update
	last_update = now;
	last_value = value;
	last_percentage = percentage;

	this->value = value;

	// call handler
	update(percentage, average_speed, eta);
}

void AbstractOutputProgressHandler::update(double percentage, double average_speed,
		int eta) {
}

LogOutputProgressHandler::LogOutputProgressHandler()
	: last_step(0) {
}

LogOutputProgressHandler::~LogOutputProgressHandler() {
}

void LogOutputProgressHandler::update(double percentage, double average_speed,
		int eta) {
	if (percentage < last_step + 5)
		return;
	last_step = percentage;

	auto log = LOGN(INFO, "progress");
	log << std::floor(percentage) << "% complete. ";
	log << "Rendered " << value << "/" << max << " tiles ";
	log << "with average " << std::setprecision(1) << std::fixed << average_speed << "t/s.";
	if (eta != -1)
		log << " ETA " << util::format_eta(eta) << ".";
}

ProgressBar::ProgressBar()
	: last_output_len(0) {
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::update(double percentage, double average_speed, int eta) {
	// try to determine the width of the terminal
	// use 80 columns as default if we can't determine a terminal size
	int terminal_width = 80;
#ifdef TIOCGWINSZ
	struct winsize ws = {0, 0, 0, 0};
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	if (ws.ws_col != 0)
		terminal_width = ws.ws_col;
#elif defined(OS_WINDOWS)
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
		terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	}
#endif

	// create the progress stats: percentage, current/maximum value, speed, eta
	std::string stats;
	stats = createProgressStats(percentage, value, max, average_speed, eta);

	// now create the progress bar
	// with the remaining size minus one as size
	// (because the space between progress and stats)
	int progressbar_width = terminal_width - stats.size() - 1;
	std::string progressbar = createProgressBar(progressbar_width, percentage);

	// go to the begin of the line and clear it
	std::cout << "\r" << std::string(last_output_len, ' ') << "\r";

	// now show everything
	// also go back to beginning of line after it, in case there is other output
	std::cout << progressbar << " " << stats << "\r";
	std::cout.flush();

	// set this as last shown
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

void ProgressBar::finish() {
	setValue(max);
	std::cout << std::endl;
}

} /* namespace util */
} /* namespace mapcrafter */
