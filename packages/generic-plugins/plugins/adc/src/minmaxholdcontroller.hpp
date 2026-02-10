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

#ifndef MINMAXHOLDCONTROLLER_HPP
#define MINMAXHOLDCONTROLLER_HPP

#include <QObject>
#include <plotchannel.h>
#include <vector>

namespace scopy {
namespace adc {

class FFTPlotComponentChannel;

class MinMaxHoldController : public QObject
{
	Q_OBJECT
public:
	MinMaxHoldController(FFTPlotComponentChannel *ch, QObject *parent);
	~MinMaxHoldController();

	void setEnabled(bool enabled);
	bool enabled() const;

	void setMinEnabled(bool enabled);
	void setMaxEnabled(bool enabled);
	bool minEnabled() const;
	bool maxEnabled() const;

	void setMainChannelHidden(bool hidden);

	void resetMin();
	void resetMax();

	void update(const float *xData, const float *yData, size_t size);

	void onChannelEnabled();
	void onChannelDisabled();
	void onYAxisChanged(PlotAxis *yAxis);

	PlotChannel *minChannel() const;
	PlotChannel *maxChannel() const;

Q_SIGNALS:
	void enabledChanged(bool enabled);

private:
	void createChannels();
	void updateVisibility();
	void updateMinHold(const float *yData, size_t size);
	void updateMaxHold(const float *yData, size_t size);

	FFTPlotComponentChannel *m_ch;

	bool m_enabled = false;
	bool m_minEnabled = false;
	bool m_maxEnabled = false;
	bool m_mainChannelHidden = false;

	std::vector<float> m_xData;
	std::vector<float> m_minData;
	std::vector<float> m_maxData;

	PlotChannel *m_minCh = nullptr;
	PlotChannel *m_maxCh = nullptr;
};

} // namespace adc
} // namespace scopy

#endif // MINMAXHOLDCONTROLLER_HPP
