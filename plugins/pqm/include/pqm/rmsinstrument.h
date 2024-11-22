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

#ifndef RMSINSTRUMENT_H
#define RMSINSTRUMENT_H
#include <QWidget>
#include <gui/widgets/measurementlabel.h>
#include <measurementpanel.h>
#include <pqmdatalogger.h>
#include <scopy-pqm_export.h>
#include <gui/polarplotwidget.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/resourcemanager.h>
#include <pluginbase/toolmenuentry.h>

#define DEVICE_NAME "pqm"

namespace scopy::pqm {
class SCOPY_PQM_EXPORT RmsInstrument : public QWidget, public ResourceUser
{
	Q_OBJECT
public:
	RmsInstrument(ToolMenuEntry *tme, QString uri, QWidget *parent = nullptr);
	~RmsInstrument();

Q_SIGNALS:
	void enableTool(bool en, QString toolName = "rms");
	void logData(PqmDataLogger::ActiveInstrument instr, const QString &filePath);
public Q_SLOTS:
	void stop() override;
	void toggleRms(bool en);
	void onAttrAvailable(QMap<QString, QMap<QString, QString>> data);

private:
	void createLabels(MeasurementsPanel *mPanel, QStringList chnls, QStringList labels, QString color = "");
	void updateLabels();
	void initPlot(PolarPlotWidget *plot);
	void setupPlotChannels(PolarPlotWidget *plot, QMap<QString, QString> channels, int thickness = 5);
	void updatePlot(PolarPlotWidget *plot, QString type);
	QVector<QwtPointPolar> getPolarPlotPoints(QString chnlType);
	QWidget *createSettingsMenu(QWidget *parent);
	QWidget *createMenuLogSection(QWidget *parent);
	void browseFile(QLineEdit *lineEditPath);

	QString m_uri;
	ToolMenuEntry *m_tme;
	bool m_running;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	PolarPlotWidget *m_voltagePlot;
	PolarPlotWidget *m_currentPlot;
	QWidget *m_voltageLabelWidget;
	QWidget *m_currentLabelWidget;
	QMap<QString, QList<MeasurementLabel *>> m_labels;
	QMap<QString, QMap<QString, QString>> m_attributes;
	const QMap<QString, QMap<QString, QString>> m_chnls = {
		{"voltage", {{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}}},
		{"current", {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"}, {"In", "in"}}}};
	const QMap<QString, QString> m_attrDictionary = {{"RMS", "rms"},
							 {"Angle", "angle"},
							 {"Deviation under", "deviation_under"},
							 {"Deviation over", "deviation_over"},
							 {"Pinst", "pinst"},
							 {"Pst", "pst"},
							 {"Plt", "plt"},
							 {"U2", "u2"},
							 {"U0", "u0"},
							 {"Sneg V", "sneg_voltage"},
							 {"Spos V", "spos_voltage"},
							 {"Szro V", "szro_voltage"},
							 {"I2", "u2"},
							 {"I0", "u0"},
							 {"Sneg I", "sneg_current"},
							 {"Spos I", "spos_current"},
							 {"Szro I", "szro_current"}};
};
} // namespace scopy::pqm
#endif // RMSINSTRUMENT_H
