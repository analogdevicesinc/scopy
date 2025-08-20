/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include "outputinfo.h"
#include <QObject>
#include <datamanager.h>
#include <iplot.h>
#include <gui/plotwidget.h>

namespace scopy::qiqplugin {

class PlotManager : public QObject
{
	Q_OBJECT
public:
	PlotManager(QObject *parent = nullptr);
	~PlotManager();

	QVector<QWidget *> getPlotW();

Q_SIGNALS:
	void configOutput(const OutputConfig &outConfig);
	void requestNewData();
	void bufferDataReady(QVector<QVector<double>> data);

public Q_SLOTS:
	void samplingFreqAvailable(int samplingFreq);
	void onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList);
	void onAnalysisConfig(const QString &type, const QVariantMap &config, const OutputInfo &outInfo);
	void onDataIsProcessed(int samplesOffset, int samplesCount);
	void updatePlots();

private:
	void updateAxis(int samples);
	PlotWidget *createPlotWidget(QIQPlotInfo plotInfo);
	void setupPlots(QList<QIQPlotInfo> plotInfoList);
	void setupDataManager(const OutputInfo &outInfo);

	int m_samplingFreq = 512;
	DataManager *m_dataManager;
	QVector<QIQPlotInfo> m_plotsInfo;
	QVector<PlotWidget *> m_plots;
};

} // namespace scopy::qiqplugin

#endif // PLOTMANAGER_H
