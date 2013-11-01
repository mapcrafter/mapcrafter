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

class IProgressHandler {
public:
	virtual ~IProgressHandler() {};

	virtual int getMax() const = 0;
	virtual void setMax(int max) = 0;

	virtual int getValue() const = 0;
	virtual void setValue(int value) = 0;
};

/**
 * Shows a nice command line progress bar.
 */
class ProgressBar : public IProgressHandler {
private:
	// the maximum value of the progress
	int max;
	// the current value of the progress
	int value;

	// animated? if yes, it updates the progress bar and makes it "animated"
	// but not good if you pipe the output in a file, so you can disable it
	bool animated;

	// the time of the start of progress
	int start;
	// time of last update
	int last_update;
	// current percentage of last update
	int last_percent;
public:
	ProgressBar(int max = 0, bool animated = true);
	virtual ~ProgressBar();

	void update(int value, bool force = false);
	void finish();

	virtual int getMax() const;
	virtual void setMax(int max);

	virtual int getValue() const;
	virtual void setValue(int value);

	void setAnimated(bool animated);
	bool isAnimated() const;
};

} /* namespace util */
} /* namespace mapcrafter */
#endif /* PROGRESS_H_ */
