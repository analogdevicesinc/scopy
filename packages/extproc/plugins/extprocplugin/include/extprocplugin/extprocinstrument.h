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

#ifndef EXTPROCINSTRUMENT_H
#define EXTPROCINSTRUMENT_H

#include "scopy-extprocplugin_export.h"
#include <QWidget>
#include <inputconfig.h>
#include <outputinfo.h>
#include <outputconfig.h>
#include <plotmanager/plotmanager.h>
#include <plotmanager/datareader.h>
#include "dataprocessingservice.h"
#include <settingsmenu.h>
#include <toolbuttons.h>
#include <QProcess>
#include <measurementpanel.h>
#include <tooltemplate.h>
#include <dockableareainterface.h>
#include <controller/runresults.h>
#include <pluginbase/toolmenuentry.h>

namespace scopy::extprocplugin {
class SCOPY_EXTPROCPLUGIN_EXPORT ExtProcInstrument : public QWidget
{
	Q_OBJECT
public:
	ExtProcInstrument(ToolMenuEntry *tme, QWidget *parent = nullptr);
	~ExtProcInstrument();

	void setAvailableChannels(QMap<QString, QList<ChannelInfo>> channels);
Q_SIGNALS:
	void bufferParamsChanged(const BufferParams &params);
	void requestAnalysisInfo(const QString &type);
	void outputConfigured(const OutputConfig &outConfig);
	void analysisConfigChanged(const QString &type, const QVariantMap &config);
	void runPressed(bool en);
	void requestNewData();

public Q_SLOTS:
	void onAnalysisTypes(const QStringList &types);
	void onInputFormatChanged(const InputConfig &inConfig);
	void onOutputConfig(const OutputConfig &outConfig);
	void onRunResponse(const RunResults &runResults);
	void onAnalysisInfo(const QString &type, const QVariantMap &params, const OutputInfo &outputInfo,
			    const QList<ExtProcPlotInfo> plotInfoList, QStringList measurements);
	void onAnalysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo);
	void tmeToggled(bool checked);
	void onProcessFinished(int exitCode);
	void onBufferDataReady(QVector<QVector<float>> &inputData);
	// void onProcessDataCompleted(const RunResults &result);

private:
	void addPlots();
	void setupConnections();
	void clearMeasurementLabels();
	void configureOutput();
	void enableAcquisition();
	void updateMeasurements(const QVariantMap &measurements);
	void fillMeasurementsPanel(const QStringList &measurements);
	void setupDataReader(const OutputInfo &outInfo);
	QPushButton *createMenuButton(const QString &name, QWidget *parent = nullptr);

	QMap<QString, MeasurementLabel *> m_labels;
	MeasurementsPanel *m_panel;
	ToolMenuEntry *m_tme;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	PlotManager *m_plotManager;
	DataReader *m_dataReader;
	DataProcessingService *m_dataProcessingService;
	SettingsMenu *m_settings;

	DockableAreaInterface *m_dockableArea;

	bool m_inputFormatConfigured = false;
	bool m_outputConfigured = false;

	const QString MEASURE_PANEL_ID = "measure";
};
} // namespace scopy::extprocplugin
#endif // EXTPROCINSTRUMENT_H
