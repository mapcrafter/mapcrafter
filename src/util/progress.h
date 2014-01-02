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

#include <string>

namespace mapcrafter {
namespace util {

std::string format_eta(int eta);

/**
 * A basic interface for a progress handler.
 */
class IProgressHandler {
public:
	virtual ~IProgressHandler() {};

	virtual int getMax() const = 0;
	virtual void setMax(int max) = 0;

	virtual int getValue() const = 0;
	virtual void setValue(int value) = 0;
};

/**
 * A dummy progress handler. Implements progress handler interface and allows setting
 * and getting the progress values.
 */
class DummyProgressHandler : public IProgressHandler {
public:
	DummyProgressHandler();
	virtual ~DummyProgressHandler();

	virtual int getMax() const;
	virtual void setMax(int max);

	virtual int getValue() const;
	virtual void setValue(int value);
protected:
	// the maximum and current value of the progress
	int max, value;
};

/**
 * Shows a nice command line progress bar.
 */
class ProgressBar : public DummyProgressHandler {
public:
	ProgressBar(int max = 0, bool animated = true);
	virtual ~ProgressBar();

	void setAnimated(bool animated);
	bool isAnimated() const;

	virtual void setValue(int value);
	void finish();
private:
	// animated? if yes, it updates the progress bar and makes it "animated"
	// but not good if you pipe the output in a file, so you can disable it
	bool animated;

	// the time of the start of progress
	int start;
	// time of last update
	int last_update;
	// value of last update
	int last_value;
	// percentage of last update
	int last_percentage;
	// length of last output needed to clear the line
	int last_output_len;

	void update(int value);

	std::string createProgressBar(int width, double percentage) const;
	std::string createProgressStats(double percentage, int value, int max,
			double speed, double speed_average, int eta = -1) const;
};

} /* namespace util */
} /* namespace mapcrafter */
#endif /* PROGRESS_H_ */
