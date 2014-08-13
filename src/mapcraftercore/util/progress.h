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

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include <memory>
#include <string>
#include <vector>

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

class MultiplexingProgressHandler : public IProgressHandler {
public:
	MultiplexingProgressHandler();
	virtual ~MultiplexingProgressHandler();

	void addHandler(IProgressHandler* handler);

	virtual int getMax() const;
	virtual void setMax(int max);

	virtual int getValue() const;
	virtual void setValue(int value);

protected:
	int max, value;

	std::vector<std::shared_ptr<IProgressHandler>> handlers;
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

class AbstractOutputProgressHandler : public DummyProgressHandler {
public:
	AbstractOutputProgressHandler();
	virtual ~AbstractOutputProgressHandler();

	virtual void setValue(int value);

	virtual void update(double percentage, double average_speed, int eta);

protected:
	// the time of the start of progress
	int start;
	// time of last update
	int last_update;
	// value of last update
	int last_value;
	// percentage of last update
	int last_percentage;
};

class LogOutputProgressHandler : public AbstractOutputProgressHandler {
public:
	LogOutputProgressHandler();
	virtual ~LogOutputProgressHandler();

	virtual void update(double percentage, double average_speed, int eta);

private:
	int last_step;
};

/**
 * Shows a nice command line progress bar.
 */
class ProgressBar : public AbstractOutputProgressHandler {
public:
	ProgressBar();
	virtual ~ProgressBar();

	virtual void update(double percentage, double average_speed, int eta);

	void setAnimated(bool animated);
	bool isAnimated() const;

	void finish();
private:
	// animated? if yes, it updates the progress bar and makes it "animated"
	// but not good if you pipe the output in a file, so you can disable it
	bool animated;

	// length of last output needed to clear the line
	int last_output_len;

	std::string createProgressBar(int width, double percentage) const;
	std::string createProgressStats(double percentage, int value, int max,
			double speed_average, int eta = -1) const;
};

} /* namespace util */
} /* namespace mapcrafter */
#endif /* PROGRESS_H_ */
