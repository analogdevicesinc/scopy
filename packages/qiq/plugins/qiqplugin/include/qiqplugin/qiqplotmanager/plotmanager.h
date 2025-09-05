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
#include "plotcreator.h"
#include <QObject>
#include <dockwrapperinterface.h>
#include <menuplotchannelcurvestylecontrol.h>
#include <qiqplotmanager/datamanager.h>
#include <qiqcontroller/qiqplotinfo.h>
#include <gui/plotwidget.h>

#define INPUT_PLOT_TITLE "Input Waveform"
namespace scopy::qiqplugin {

struct PlotContainer
{
	QWidget *plot;
	DockWrapperInterface *plotWrapper;
	QIQPlotInfo info;
	PlotCreatorBase *creator;
};

class PlotManager : public QObject
{
	Q_OBJECT
public:
	PlotManager(QObject *parent = nullptr);
	~PlotManager();

	void updateInputPlot(int chnlCount);
	QWidget *inputPlot() const;
	QVector<DockWrapperInterface *> plotWrappers() const;
	QStringList plotTitle() const;

Q_SIGNALS:
	void configOutput(const OutputConfig &outConfig);
	void requestNewData();
	void bufferDataReady(QVector<QVector<double>> data);
	void plotSettings(QWidget *settings);

public Q_SLOTS:
	void samplingFreqAvailable(int samplingFreq);
	void onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList);
	void onAnalysisConfig(const QString &type, const QVariantMap &config, const OutputInfo &outInfo);
	void plotSettingsRequest(const QString &plot);
	void onDataIsProcessed(int samplesOffset, int samplesCount);
	void updatePlots();

private:
	void createPlots(QList<QIQPlotInfo> &plotInfoList);
	void setupDataManager(const OutputInfo &outInfo);
	void clearPlots();
	// input plot
	void createInputPlot();
	void createInputPlotSettings();
	void removePlotChannels();
	void updateInputData();
	void addPlotChannel(const QString &label, const QColor &color);

	gui::MenuPlotChannelCurveStyleControl *m_inputCurveControl;
	PlotWidget *m_inputPlot;
	QWidget *m_inputSettings;
	DataManager *m_dataManager;
	QVector<PlotContainer> m_plotContainers;
};

} // namespace scopy::qiqplugin

#endif // PLOTMANAGER_H
