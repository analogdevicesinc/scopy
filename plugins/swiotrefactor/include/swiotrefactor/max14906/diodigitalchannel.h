/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef SCOPY_DIODIGITALCHANNEL_H
#define SCOPY_DIODIGITALCHANNEL_H

#include "scopy-swiotrefactor_export.h"
#include <mutex>
#include <QWidget>
#include <gui/lcdNumber.hpp>
#include <gui/smallOnOffSwitch.h>
#include <gui/plotwidget.h>
#include <gui/generic_menu.hpp>
#include <gui/customSwitch.h>
#include <gui/widgets/menucombo.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT DioDigitalChannel : public QWidget
{
	Q_OBJECT
public:
	explicit DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent = nullptr);
	~DioDigitalChannel() override;

	void updateTimeScale(double newMax);
	void addDataSample(double value);
	const QStringList getConfigModes() const;
	void setConfigModes(QStringList &configModes);
	const QString &getSelectedConfigMode() const;
	void setSelectedConfigMode(const QString &selectedConfigMode);
	void resetPlot();

private:
	void connectSignalsAndSlots();

	QString m_deviceName;
	QString m_deviceType;

	QStringList m_configModes;
	QString m_selectedConfigMode;

	friend class DioDigitalChannelController;
	std::mutex m_mutex;

	// plot
	PlotWidget *m_plot;
	QVector<double> m_yValues;
	QVector<double> m_xTime;
	PlotChannel *m_plotCh;
	int m_timeSpan = 10;

	// bottom container
	MenuCombo *m_configModesCombo;
	MenuCombo *m_currentLimitsCombo;
	LcdNumber *m_lcdNumber;
	SmallOnOffSwitch *m_valueSwitch;
	// top container
	QLabel *m_channelName;
	QLabel *m_channelType;

	void initData();
	void initPlot();
	QWidget *createBottomContainer(QWidget *parent);
	QWidget *createTopContainer(QWidget *parent);
Q_SIGNALS:
	void outputValueChanged(bool value);
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_DIODIGITALCHANNEL_H
