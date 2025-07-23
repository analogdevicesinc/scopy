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

#ifndef PLOTCREATOR_H
#define PLOTCREATOR_H

#include "menuplotchannelcurvestylecontrol.h"
#include <QObject>
#include <plotautoscaler.h>
#include <controller/extprocplotinfo.h>
#include <gui/plotwidget.h>

using namespace scopy::gui;
namespace scopy::extprocplugin {

class PlotCreatorBase : public QObject
{
	Q_OBJECT
public:
	explicit PlotCreatorBase(QObject *parent = nullptr)
		: QObject(parent)
	{}
	virtual ~PlotCreatorBase() = default;

	virtual QWidget *settingsMenu() = 0;
	virtual void updatePlot(const QMap<QString, QVector<double>> &data) = 0;
	virtual QString plotType() const = 0;
	virtual void requestSettings(const QString &title) = 0;
	virtual void dataManagerEntries(const QStringList &entries) = 0;

	virtual void enableChannelAdd(bool en) = 0;
	virtual QWidget *plot() const = 0;
	virtual ExtProcPlotInfo plotInfo() const { return m_plotInfo; };

	void setPlotInfo(const ExtProcPlotInfo &newPlotInfo) { m_plotInfo = newPlotInfo; };

protected:
	ExtProcPlotInfo m_plotInfo;
};

class StandardPlotCreator : public PlotCreatorBase
{
	Q_OBJECT
public:
	explicit StandardPlotCreator(const ExtProcPlotInfo &plotInfo, QObject *parent = nullptr);

	QWidget *plot() const override;
	QWidget *settingsMenu() override;
	void updatePlot(const QMap<QString, QVector<double>> &data) override;
	QString plotType() const override { return "plotWidget"; }
	void enableChannelAdd(bool en) override;

Q_SIGNALS:
	void requestSettings(const QString &title) override;
	void dataManagerEntries(const QStringList &entries) override;
	void plotChnlsUpdated(const QStringList &chnls);

private Q_SLOTS:
	void onNewMin(double min);
	void onNewMax(double max);

private:
	void init(const ExtProcPlotInfo &plotInfo);
	void setupPlotChannels();
	void configurePlotAxis();
	void applyPlotFlags();

	void createPlotSettings();
	QWidget *createYAxisSection();
	QWidget *createDataManagerSection();
	QWidget *createGeneralSettingsSection();

	void updatePlotChannels(const ExtProcPlotInfo &plotInfo);
	void clearPlotChannels();

	void enableTabCloseBtn(bool en);
	void addTabCloseBtn(int tabIndex);
	void updateChnlsTab();
	void updateOnDmEntries(QComboBox *xCombo, QComboBox *yCombo, int chnlIdx);
	QWidget *createTabEntryW(QTabWidget *parent, int chnlIdx);
	void initPlotChnlsTab(QWidget *parent = nullptr);
	QStringList getChnlsList();

	void autoscaleX();
	void autoscaleY();
	void addPlotChannel();
	void rmPlotChannel(int chnlIdx);

	bool m_first = true;
	QStringList m_dmEntries;
	PlotWidget *m_plotWidget{nullptr};
	QWidget *m_plotSettings{nullptr};
	QTabWidget *m_chnlsTab{nullptr};
	PlotAutoscaler *m_plotAutoscaler{nullptr};
	MenuPlotChannelCurveStyleControl *m_curveControl;
};

class PlotCreatorFactory
{
public:
	static PlotCreatorBase *createPlotCreator(const ExtProcPlotInfo &plotInfo, QObject *parent = nullptr);

private:
	static QString determinePlotType(const ExtProcPlotInfo &plotInfo);
};

} // namespace scopy::extprocplugin

#endif // PLOTCREATOR_H
