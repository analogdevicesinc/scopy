/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef INCREMENTSTRATEGY_H
#define INCREMENTSTRATEGY_H

#include <plot_utils.hpp>
#include <scopy-gui_export.h>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT IncrementStrategy
{
public:
	virtual ~IncrementStrategy() {}
	virtual double increment(double val) = 0;
	virtual double decrement(double val) = 0;
	virtual void setScale(double scale) = 0;
};

class SCOPY_GUI_EXPORT IncrementStrategy125 : public IncrementStrategy
{
public:
	NumberSeries m_steps;

	IncrementStrategy125()
		: m_steps(1e-9, 1e9, 10)
	{}
	~IncrementStrategy125() {}
	virtual double increment(double val) override { return m_steps.getNumberAfter(val); }
	virtual double decrement(double val) override { return m_steps.getNumberBefore(val); }

	double m_scale;
	void setScale(double scale) override { m_scale = scale; }
};

class SCOPY_GUI_EXPORT IncrementStrategyPower2 : public IncrementStrategy
{
public:
	QList<double> m_steps;
	IncrementStrategyPower2()
	{
		for(int i = 30; i >= 0; i--) {
			m_steps.append(-(1 << i));
		}
		for(int i = 0; i < 31; i++) {
			m_steps.append(1 << i);
		}
	}
	~IncrementStrategyPower2() {}
	virtual double increment(double val) override
	{
		int i = 0;
		val = val + 1;
		while(val > m_steps[i]) {
			i++;
		}
		return m_steps[i];
	}
	virtual double decrement(double val) override
	{
		int i = m_steps.count() - 1;
		val = val - 1;
		while(val < m_steps[i]) {
			i--;
		}
		return m_steps[i];
	}
	double m_scale;

	void setScale(double scale) override { m_scale = scale; }
};
class SCOPY_GUI_EXPORT IncrementStrategyFixed : public IncrementStrategy
{
public:
	IncrementStrategyFixed(double k = 1)
	{
		m_k = k;
		m_scale = 1;
	}
	~IncrementStrategyFixed() {}
	virtual double increment(double val) override
	{
		val = val + m_k * m_scale;
		return val;
	}
	virtual double decrement(double val) override
	{
		val = val - m_k * m_scale;
		return val;
	}
	void setK(double val) { m_k = val; }
	double k() { return m_k; }

private:
	double m_k;
	double m_scale;

	void setScale(double scale) override { m_scale = scale; }
};

class SCOPY_GUI_EXPORT IncrementStrategyTest : public IncrementStrategy
{
public:
	IncrementStrategyTest() { m_scale = 1; }
	~IncrementStrategyTest() {}
	virtual double increment(double val) override
	{
		val = val + m_scale;
		return val;
	}
	virtual double decrement(double val) override
	{
		val = val - m_scale;
		return val;
	}

private:
	double m_scale;

	void setScale(double scale) override { m_scale = scale; }
};
} // namespace gui
} // namespace scopy
#endif // INCREMENTSTRATEGY_H
