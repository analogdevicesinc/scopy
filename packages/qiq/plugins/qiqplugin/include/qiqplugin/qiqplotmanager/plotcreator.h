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
#include <qiqcontroller/qiqplotinfo.h>
#include <gui/plotwidget.h>

namespace scopy::qiqplugin {

class PlotCreatorBase : public QObject
{
	Q_OBJECT
public:
	explicit PlotCreatorBase(QObject *parent = nullptr)
		: QObject(parent)
	{}
	virtual ~PlotCreatorBase() = default;

	virtual QWidget *createPlot(const QIQPlotInfo &plotInfo) = 0;
	virtual QWidget *settingsMenu() = 0;
	virtual void updatePlot(QWidget *plot, const QIQPlotInfo &plotInfo,
				const QMap<QString, QVector<double>> &data) = 0;
	virtual QString plotType() const = 0;
};

class StandardPlotCreator : public PlotCreatorBase
{
	Q_OBJECT
public:
	explicit StandardPlotCreator(QObject *parent = nullptr);

	QWidget *createPlot(const QIQPlotInfo &plotInfo) override;
	QWidget *settingsMenu() override;
	void updatePlot(QWidget *plot, const QIQPlotInfo &plotInfo,
			const QMap<QString, QVector<double>> &data) override;
	QString plotType() const override { return "plotWidget"; }

private:
	void setupPlotChannels(PlotWidget *plot, const QIQPlotInfo &plotInfo);
	void configurePlotAxis(PlotWidget *plot, const QIQPlotInfo &plotInfo);
	void applyPlotFlags(PlotWidget *plot, const QIQPlotInfo &plotInfo);

	void createPlotSettings(PlotWidget *plot, const QIQPlotInfo &plotInfo);
	void updatePlotChannels(PlotWidget *plot, const QIQPlotInfo &plotInfo);
	void clearPlotChannels(PlotWidget *plot);

	QWidget *m_plotSettings;
};

class PlotCreatorFactory
{
public:
	static PlotCreatorBase *createPlotCreator(const QIQPlotInfo &plotInfo);

private:
	static QString determinePlotType(const QIQPlotInfo &plotInfo);
};

} // namespace scopy::qiqplugin

#endif // PLOTCREATOR_H
