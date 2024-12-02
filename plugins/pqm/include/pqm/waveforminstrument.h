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

#ifndef WAVEFORMINSTRUMENT_H
#define WAVEFORMINSTRUMENT_H

#include "scopy-pqm_export.h"

#include <QWidget>
#include <menucombo.h>
#include <menuspinbox.h>
#include <plottingstrategy.h>
#include <pqmdatalogger.h>
#include <gui/spinbox_a.hpp>
#include <gui/plotwidget.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/resourcemanager.h>
#include <pluginbase/toolmenuentry.h>

#define ROLLING_MODE "rolling"
#define TRIGGER_MODE "trigger"

namespace scopy::pqm {
class SCOPY_PQM_EXPORT WaveformInstrument : public QWidget, public ResourceUser
{
	Q_OBJECT
public:
	WaveformInstrument(ToolMenuEntry *tme, QString uri, QWidget *parent = nullptr);
	~WaveformInstrument();

	void showOneBuffer(bool hasFwVers);
public Q_SLOTS:
	void stop() override;
	void toggleWaveform(bool en);
	void onBufferDataAvailable(QMap<QString, QVector<double>> data);
Q_SIGNALS:
	void enableTool(bool en, QString toolName = "waveform");
	void logData(PqmDataLogger::ActiveInstrument instr, const QString &filePath);

private Q_SLOTS:
	void onTriggeredChnlChanged(QString triggeredChnl);
	void onRollingSwitch(bool checked);

private:
	void initData();
	void initPlot(PlotWidget *plot, QString unitType, int yMin = -650, int yMax = 650);
	void setupChannels(PlotWidget *plot, QMap<QString, QString> chnls);
	QWidget *createSettMenu(QWidget *parent);
	QWidget *createMenuLogSection(QWidget *parent);
	QWidget *createMenuPlotSection(QWidget *parent);

	void updateXData(int dataSize);
	void plotData(QMap<QString, QVector<double>> chnlsData);
	void deletePlottingStrategy();
	void createTriggeredStrategy(QString triggerChnl);
	void browseFile(QLineEdit *lineEditPath);

	bool m_running;
	PlotWidget *m_voltagePlot;
	PlotWidget *m_currentPlot;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	GearBtn *m_settBtn;
	gui::MenuSpinbox *m_timespanSpin;
	MenuCombo *m_triggeredBy;

	QMap<QString, PlotChannel *> m_plotChnls;
	QVector<double> m_xTime;
	PlottingStrategy *m_plottingStrategy = nullptr;

	ToolMenuEntry *m_tme;
	QString m_uri;

	const double m_plotSampleRate = 5120;
	const QMap<QString, QMap<QString, QString>> m_chnls = {
		{"voltage", {{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}}},
		{"current", {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"}, {"In", "in"}}}};
};
} // namespace scopy::pqm

#endif // WAVEFORMINSTRUMENT_H
