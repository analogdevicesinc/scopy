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

#include "menus/plottimeaxiscontroller.hpp"

#include <QDateTimeEdit>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <timemanager.hpp>
#include <cfloat>

using namespace scopy;
using namespace datamonitor;

PlotTimeAxisController::PlotTimeAxisController(MonitorPlot *m_plot, QWidget *parent)
	: m_plot(m_plot)
	, QWidget{parent}
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	this->setLayout(layout);

	MenuSectionWidget *xAxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xAxisSection = new MenuCollapseSection(
		"X-AXIS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, xAxisContainer);

	xAxisContainer->contentLayout()->addWidget(xAxisSection);
	xAxisSection->contentLayout()->setSpacing(10);

	realTimeToggle = new MenuOnOffSwitch(tr("Use UTC date and time"), xAxisSection, false);
	livePlottingToggle = new MenuOnOffSwitch(tr("Live plotting"), xAxisContainer, false);
	livePlottingToggle->onOffswitch()->setChecked(true);

	dateEdit = new QDateEdit(QDate::currentDate(), xAxisContainer);
	dateEdit->setCalendarPopup(true);
	dateEdit->setVisible(false);
	timeEdit = new QTimeEdit(QTime::currentTime(), xAxisContainer);
	timeEdit->setDisplayFormat("hh:mm:ss");
	timeEdit->setVisible(false);

	m_xdelta = new gui::MenuSpinbox("Delta", DataMonitorUtils::getAxisDefaultMaxValue(), "s", 0, DBL_MAX, false,
					false, xAxisContainer);
	m_xdelta->setScaleRange(1, 1);
	m_xdelta->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_xdelta->setValue(DataMonitorUtils::getAxisDefaultMaxValue());

	auto &&timeTracker = TimeManager::GetInstance();

	connect(timeTracker, &TimeManager::timeout, this, [=, this]() {
		if(livePlottingToggle->onOffswitch()->isChecked()) {
			// plot using current date time as starting point
			timeEdit->setTime(QTime::currentTime());
			dateEdit->setDate(QDate::currentDate());
			updatePlotStartPoint();
		} else {
			double time = QwtDate::toDouble(QDateTime::currentDateTime());
			m_plot->updateBufferPreviewer(time);
		}
	});

	connect(realTimeToggle->onOffswitch(), &QAbstractButton::toggled, m_plot, &MonitorPlot::setIsRealTime);
	connect(livePlottingToggle->onOffswitch(), &QAbstractButton::toggled, this,
		&PlotTimeAxisController::togglePlotNow);

	connect(dateEdit, &QDateEdit::dateChanged, this, &PlotTimeAxisController::updatePlotStartPoint);
	connect(timeEdit, &QTimeEdit::timeChanged, this, &PlotTimeAxisController::updatePlotStartPoint);

	connect(m_xdelta, &gui::MenuSpinbox::valueChanged, this,
		[=, this](double value) { m_plot->updateXAxisIntervalMax(value); });

	xAxisSection->contentLayout()->addWidget(realTimeToggle);
	xAxisSection->contentLayout()->addWidget(livePlottingToggle);
	xAxisSection->contentLayout()->addWidget(dateEdit);
	xAxisSection->contentLayout()->addWidget(timeEdit);
	xAxisSection->contentLayout()->addWidget(m_xdelta);

	layout->addWidget(xAxisContainer);
}

void PlotTimeAxisController::togglePlotNow(bool toggled)
{
	dateEdit->setVisible(!toggled);
	timeEdit->setVisible(!toggled);
	updatePlotStartPoint();
}

void PlotTimeAxisController::updatePlotStartPoint()
{
	double time = QwtDate::toDouble(QDateTime(dateEdit->date(), timeEdit->time()));
	double delta = m_xdelta->value();

	m_plot->updatePlotStartingPoint(time, delta);
}

#include "moc_plottimeaxiscontroller.cpp"
