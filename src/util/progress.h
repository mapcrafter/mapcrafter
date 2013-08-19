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

#ifndef PROGRESS_H_
#define PROGRESS_H_

namespace mapcrafter {
namespace util {

/**
 * Shows a nice command line progress bar.
 */
class ProgressBar {
private:
	// the maximum value of the progress
	int max;
	// animated? if yes, it updates the progress bar and makes it "animated"
	// but not good if you pipe the output in a file, so you can disable it
	bool animated;

	// the time of the start of progress
	int start;
	// time of last update
	int last_update;
	// value of last update
	int last_value;
	// current percentage of last update
	int last_percent;
public:
	ProgressBar();
	ProgressBar(int max, bool animated);

	void setMax(int max);
	int getMax() const;

	void setAnimated(bool animated);
	bool isAnimated() const;

	void update(int value);
	void finish();
};

} /* namespace util */
} /* namespace mapcrafter */
#endif /* PROGRESS_H_ */
