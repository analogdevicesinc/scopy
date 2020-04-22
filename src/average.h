/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef AVERAGE_H
#define AVERAGE_H

namespace adiscope {

class SpectrumAverage
{
public:
	SpectrumAverage(unsigned int data_width, unsigned int history);
	virtual ~SpectrumAverage();
	virtual void pushNewData(double* data) = 0;
	virtual void getAverage(double* out_data, unsigned int num_samples) const;
	virtual void reset() = 0;
	unsigned int dataWidth() const;
	unsigned int history() const;

protected:
	unsigned int m_data_width;
	unsigned int m_history_size;
	double* m_average;
};

class AverageHistoryOne : public SpectrumAverage
{
public:
	AverageHistoryOne(unsigned int data_width, unsigned history);
	virtual void reset();

protected:
	bool m_anyDataPushed;
};

class AverageHistoryN : public SpectrumAverage
{
public:
	AverageHistoryN(unsigned int data_width, unsigned int history);
	virtual ~AverageHistoryN();
	virtual void pushNewData(double* data);
	virtual void reset();

protected:
	double** m_history;
	unsigned int m_insert_index;
	unsigned int m_inserted_count;

private:
	void alloc_history(unsigned int data_width, unsigned int history_size);
	void free_history();
};

class PeakHoldContinuous : public AverageHistoryOne
{
public:
	PeakHoldContinuous(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);
};

class MinHoldContinuous : public AverageHistoryOne
{
public:
	MinHoldContinuous(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);
};

class ExponentialRMS : public AverageHistoryOne
{
public:
	ExponentialRMS(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);
};

class ExponentialAverage : public AverageHistoryOne
{
public:
	ExponentialAverage(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);
};

class PeakHold : public AverageHistoryN
{
public:
	PeakHold(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);

private:
	double getPeakFromHistoryColumn(unsigned int col);
};

class MinHold : public AverageHistoryN
{
public:
	MinHold(unsigned int data_width, unsigned int history);
	virtual void pushNewData(double* data);

private:
	double getMinFromHistoryColumn(unsigned int col);
};

class LinearRMS : public AverageHistoryN
{
public:
	LinearRMS(unsigned int data_width, unsigned int history);
	~LinearRMS();
	virtual void pushNewData(double* data);
	virtual void getAverage(double* out_data, unsigned int num_samples) const;
	virtual void reset();

private:
	double* m_sqr_sums;
};

class LinearAverage : public AverageHistoryN
{
public:
	LinearAverage(unsigned int data_width, unsigned int history);
	~LinearAverage();
	virtual void pushNewData(double* data);
	virtual void getAverage(double* out_data, unsigned int num_samples) const;
	virtual void reset();

private:
	double* m_sums;
};

} // namespace adiscope

#endif // AVERAGE_H
