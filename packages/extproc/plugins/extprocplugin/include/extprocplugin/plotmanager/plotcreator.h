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

#include <QObject>
#include <controller/extprocplotinfo.h>
#include <gui/plotwidget.h>

namespace scopy::extprocplugin {

class PlotCreatorBase : public QObject
{
	Q_OBJECT
public:
	explicit PlotCreatorBase(QObject *parent = nullptr)
		: QObject(parent)
	{}
	virtual ~PlotCreatorBase() = default;

	virtual QWidget *createPlot(const ExtProcPlotInfo &plotInfo) = 0;
	virtual QWidget *settingsMenu() = 0;
	virtual void updatePlot(QWidget *plot, const ExtProcPlotInfo &plotInfo,
				const QMap<QString, QVector<double>> &data) = 0;
	virtual QString plotType() const = 0;
	virtual void requestSettings(const QString &title) = 0;
};

class StandardPlotCreator : public PlotCreatorBase
{
	Q_OBJECT
public:
	explicit StandardPlotCreator(QObject *parent = nullptr);

	QWidget *createPlot(const ExtProcPlotInfo &plotInfo) override;
	QWidget *settingsMenu() override;
	void updatePlot(QWidget *plot, const ExtProcPlotInfo &plotInfo,
			const QMap<QString, QVector<double>> &data) override;
	QString plotType() const override { return "plotWidget"; }

Q_SIGNALS:
	void requestSettings(const QString &title) override;

private:
	void setupPlotChannels(PlotWidget *plot, const ExtProcPlotInfo &plotInfo);
	void configurePlotAxis(PlotWidget *plot, const ExtProcPlotInfo &plotInfo);
	void applyPlotFlags(PlotWidget *plot, const ExtProcPlotInfo &plotInfo);

	void createPlotSettings(PlotWidget *plot, const ExtProcPlotInfo &plotInfo);
	void updatePlotChannels(PlotWidget *plot, const ExtProcPlotInfo &plotInfo);
	void clearPlotChannels(PlotWidget *plot);

	QWidget *m_plotSettings;
};

class PlotCreatorFactory
{
public:
	static PlotCreatorBase *createPlotCreator(const ExtProcPlotInfo &plotInfo);

private:
	static QString determinePlotType(const ExtProcPlotInfo &plotInfo);
};

} // namespace scopy::extprocplugin

#endif // PLOTCREATOR_H
