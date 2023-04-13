/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AVERAGE_H
#define AVERAGE_H

#include <mutex>
#include "scopygui_export.h"

namespace scopy {


class SCOPYGUI_EXPORT SpectrumAverage {
public:
	SpectrumAverage(unsigned int data_width, unsigned int history, bool history_en);
	virtual ~SpectrumAverage();
	virtual void pushNewData(double *data) = 0;
	virtual void getAverage(double *out_data,
		unsigned int num_samples) const;
	virtual void reset() = 0;
	unsigned int dataWidth() const;
	unsigned int history() const;
	virtual void setHistory(unsigned int);
	bool historyEnabled() const;

protected:
	unsigned int m_data_width;
	unsigned int m_history_size;
	bool m_history_enabled;
	double *m_average;
};

class SCOPYGUI_EXPORT AverageHistoryOne: public SpectrumAverage
{
public:
	AverageHistoryOne(unsigned int data_width, unsigned history);
	virtual void reset();

protected:
	bool m_anyDataPushed;
};

class SCOPYGUI_EXPORT AverageHistoryN: public SpectrumAverage
{
public:
	AverageHistoryN(unsigned int data_width, unsigned int history);
	virtual ~AverageHistoryN();
	virtual void pushNewData(double *data);
	virtual void reset();

protected:
	double **m_history;
	unsigned int m_insert_index;
	unsigned int m_inserted_count;
	std::mutex m_history_mutex;

private:
	void alloc_history(unsigned int data_width, unsigned int history_size);
	void free_history();
	void setHistory(unsigned int) override;
};

class SCOPYGUI_EXPORT PeakHoldContinuous: public AverageHistoryOne
{
public:
	PeakHoldContinuous(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);
};

class SCOPYGUI_EXPORT MinHoldContinuous: public AverageHistoryOne
{
public:
	MinHoldContinuous(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);
};

class SCOPYGUI_EXPORT ExponentialRMS: public AverageHistoryOne
{
public:
	ExponentialRMS(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);
};

class SCOPYGUI_EXPORT ExponentialAverage: public AverageHistoryOne
{
public:
	ExponentialAverage(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);
};

class SCOPYGUI_EXPORT LinearRMSOne: public AverageHistoryOne
{
public:
	LinearRMSOne(unsigned int data_width, unsigned int history);
	~LinearRMSOne();
	virtual void pushNewData(double *data);

private:
	double *m_sqr_sums;
	unsigned int m_inserted_count;
};

class SCOPYGUI_EXPORT LinearAverageOne: public AverageHistoryOne
{
public:
	LinearAverageOne(unsigned int data_width, unsigned int history);
	~LinearAverageOne();
	virtual void pushNewData(double *data);

private:
	double *m_sums;
	unsigned int m_inserted_count;
};

class SCOPYGUI_EXPORT PeakHold: public AverageHistoryN
{
public:
	PeakHold(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);

private:
	double getPeakFromHistoryColumn(unsigned int col);
};

class SCOPYGUI_EXPORT MinHold: public AverageHistoryN
{
public:
	MinHold(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double *data);

private:
	double getMinFromHistoryColumn(unsigned int col);
};

class SCOPYGUI_EXPORT LinearRMS: public AverageHistoryN
{
public:
	LinearRMS(unsigned int data_width, unsigned int history);
	~LinearRMS();
	virtual void pushNewData(double *data);
	virtual void getAverage(double *out_data,
		unsigned int num_samples) const;
	virtual void reset();

private:
	double *m_sqr_sums;
};

class SCOPYGUI_EXPORT LinearAverage: public AverageHistoryN
{
public:
	LinearAverage(unsigned int data_width, unsigned int history);
	~LinearAverage();
	virtual void pushNewData(double *data);
	virtual void getAverage(double *out_data,
		unsigned int num_samples) const;
	virtual void reset();

private:
	double *m_sums;
};

} // namespace scopy

#endif // AVERAGE_H
