/*
 * Copyright 2012 Moritz Hilscher
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

#include "util.h"

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iomanip>

namespace mapcrafter {

bool isBigEndian() {
	int test = 0xff00;
	return *((char*) &test) == 0xff;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start = 0;
	while ((start = str.find(from, start)) != std::string::npos) {
		str.replace(start, from.length(), to);
		start += to.length();
	}
}

bool copyFile(const fs::path& from, const fs::path& to) {
	std::ifstream in(from.c_str());
	std::ofstream out(to.c_str());

	if (!in || !out)
		return false;

	out << in.rdbuf();
	if (!out)
		return false;
	in.close();
	out.close();
	return true;
}

bool copyDirectory(const fs::path& from, const fs::path& to) {
	if (!fs::exists(from) || !fs::is_directory(from))
		return false;
	if (!fs::exists(to) && !fs::create_directories(to))
		return false;
	fs::directory_iterator end;
	for (fs::directory_iterator it(from); it != end; ++it) {
		if (fs::is_regular_file(*it)) {
			if (!copyFile(*it, to / it->path().filename()))
				return false;
		} else if (fs::is_directory(*it)) {
			if (!copyDirectory(*it, to / it->path().filename()))
				return false;
		}
	}
	return true;
}

ProgressBar::ProgressBar()
		: max(0), start(time(NULL)), last_update(0), last_value(0), last_percent(0) {
}

ProgressBar::ProgressBar(int max)
		: max(max), start(time(NULL)), last_update(0), last_value(0), last_percent(0) {
}

void ProgressBar::setMax(int max) {
	this->max = max;
}

int ProgressBar::getMax() const {
	return max;
}

void ProgressBar::update(int value) {
	int now = time(NULL);
	double percent = value / (double) max * 100.;
	if (last_update + 1 > now && !(last_percent != max && value == max))
		return;

	double speed = (double) (value - last_value) / (now - last_update);
	if(value == max)
		speed = (double) value / (now - start);

	std::cout << "\033[K" << "[";
	for (int i = 0; i <= 100; i += 2) {
		if (i > percent)
			std::cout << " ";
		else if (percent - 2 < i)
			std::cout << ">";
		else
			std::cout << "=";
	}
	char fpercent[20];
	char fspeed[20];
	sprintf(&fpercent[0], "%.2f%%", percent);
	sprintf(&fspeed[0], "%.2f", speed);
	std::cout << "] " << std::setprecision(4) << fpercent << " " << value << "/" << max
	        << " " << fspeed << "/s\r";
	std::cout.flush();
	last_update = now;
	last_value = value;
	last_percent = percent;
}

void ProgressBar::finish() {
	update(max);
	std::cout << std::endl;
}

}
