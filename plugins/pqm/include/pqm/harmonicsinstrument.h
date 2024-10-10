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

#ifndef HARMONICSINSTRUMENT_H
#define HARMONICSINSTRUMENT_H

#include "pluginbase/toolmenuentry.h"
#include <gui/widgets/menucombo.h>
#include <scopy-pqm_export.h>
#include <gui/plotwidget.h>
#include <gui/tooltemplate.h>
#include <gui/flexgridlayout.hpp>
#include <QTableWidget>
#include <QWidget>
#include <pqmdatalogger.h>
#include <gui/widgets/measurementlabel.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/resourcemanager.h>

#define NUMBER_OF_HARMONICS 51
#define HARMONICS_MIN_DEGREE 1
#define HARMONICS_MAX_DEGREE 50
#define MAX_CHNLS 6

namespace scopy::pqm {
class SCOPY_PQM_EXPORT HarmonicsInstrument : public QWidget, public ResourceUser
{
	Q_OBJECT
public:
	HarmonicsInstrument(ToolMenuEntry *tme, QString uri, QWidget *parent = nullptr);
	~HarmonicsInstrument();

	void showThdWidget(bool show);
public Q_SLOTS:
	void stop() override;
	void toggleHarmonics(bool en);
	void onAttrAvailable(QMap<QString, QMap<QString, QString>> attr);
Q_SIGNALS:
	void enableTool(bool en, QString toolName = "harmonics");
	void logData(PqmDataLogger::ActiveInstrument instr, const QString &filePath);
private Q_SLOTS:
	void updateTable();
	void onActiveChnlChannged(QString chnlId);
	void onSelectionChanged();

private:
	void initData();
	void initTable();
	void initPlot();
	void setupPlotChannels();
	QWidget *createThdWidget();
	QWidget *createSettingsMenu(QWidget *parent);
	QWidget *createMenuGeneralSection(QWidget *parent);
	QWidget *createMenuLogSection(QWidget *parent);
	bool selectedFromSameCol(QModelIndexList list);
	void browseFile(QLineEdit *lineEditPath);

	QString m_uri;
	bool m_running;
	QString m_harmonicsType;
	QWidget *m_thdWidget;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	QTableWidget *m_table;
	PlotWidget *m_plot;
	std::vector<double> m_xTime;
	QMap<QString, std::vector<double>> m_yValues;
	QMap<QString, MeasurementLabel *> m_labels;
	QMap<QString, PlotChannel *> m_plotChnls;
	ToolMenuEntry *m_tme;
	// keys - used for UI labels; values - context channels name
	const QMap<QString, QString> m_chnls = {{"Ia", "ia"}, {"Ib", "ib"}, {"Ic", "ic"},
						{"Ua", "ua"}, {"Ub", "ub"}, {"Uc", "uc"}};
};
} // namespace scopy::pqm
#endif // HARMONICSINSTRUMENT_H
