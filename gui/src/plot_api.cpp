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

#include "plot_api.h"
#include "plotcomponent.h"
#include "plotwidget.h"
#include "plotchannel.h"
#include "plotaxis.h"
#include "cursorcontroller.h"
#include "plotcursors.h"
#include "plotaxishandle.h"

#include <QPdfWriter>
#include <QPainter>
#include <QPageLayout>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PLOT_API, "PlotAPI")

using namespace scopy;

PlotAPI::PlotAPI(PlotComponent *component, PlotWidget *plot, CursorController *cursor, QObject *parent)
	: ApiObject(parent)
	, m_component(component)
	, m_plot(plot)
	, m_cursor(cursor)
{}

PlotAPI::~PlotAPI() {}

// ==================== TITLE & LABELS ====================

QString PlotAPI::getTitle() { return m_component ? m_component->name() : QString(); }

void PlotAPI::setTitle(const QString &title)
{
	if(m_component)
		m_component->setName(title);
}

bool PlotAPI::isLabelsEnabled() { return m_plot ? m_plot->showXAxisLabels() : false; }

void PlotAPI::setLabelsEnabled(bool enabled)
{
	if(m_component)
		m_component->showPlotLabels(enabled);
}

// ==================== CHANNELS ====================

QStringList PlotAPI::getChannels()
{
	QStringList channels;
	if(!m_plot)
		return channels;
	for(PlotChannel *ch : m_plot->getChannels()) {
		channels.append(ch->name());
	}
	return channels;
}

PlotChannel *PlotAPI::findChannel(const QString &name)
{
	if(!m_plot)
		return nullptr;
	for(PlotChannel *ch : m_plot->getChannels()) {
		if(ch->name() == name)
			return ch;
	}
	return nullptr;
}

// ==================== CURVE STYLE ====================

int PlotAPI::getChannelThickness(const QString &channel)
{
	PlotChannel *ch = findChannel(channel);
	return ch ? ch->thickness() : 0;
}

void PlotAPI::setChannelThickness(const QString &channel, int thickness)
{
	PlotChannel *ch = findChannel(channel);
	if(ch)
		ch->setThickness(thickness);
}

int PlotAPI::getChannelStyle(const QString &channel)
{
	PlotChannel *ch = findChannel(channel);
	return ch ? ch->style() : 0;
}

void PlotAPI::setChannelStyle(const QString &channel, int style)
{
	PlotChannel *ch = findChannel(channel);
	if(ch)
		ch->setStyle(style);
}

// ==================== X-AXIS RANGE ====================

double PlotAPI::getXMin() { return m_plot && m_plot->xAxis() ? m_plot->xAxis()->min() : 0.0; }

double PlotAPI::getXMax() { return m_plot && m_plot->xAxis() ? m_plot->xAxis()->max() : 0.0; }

void PlotAPI::setXMin(double min)
{
	if(m_plot && m_plot->xAxis())
		m_plot->xAxis()->setMin(min);
}

void PlotAPI::setXMax(double max)
{
	if(m_plot && m_plot->xAxis())
		m_plot->xAxis()->setMax(max);
}

// ==================== Y-AXIS RANGE ====================

double PlotAPI::getYMin() { return m_plot && m_plot->yAxis() ? m_plot->yAxis()->min() : 0.0; }

double PlotAPI::getYMax() { return m_plot && m_plot->yAxis() ? m_plot->yAxis()->max() : 0.0; }

void PlotAPI::setYMin(double min)
{
	if(m_plot && m_plot->yAxis())
		m_plot->yAxis()->setMin(min);
}

void PlotAPI::setYMax(double max)
{
	if(m_plot && m_plot->yAxis())
		m_plot->yAxis()->setMax(max);
}

// ==================== CURSORS ====================

bool PlotAPI::isCursorsVisible() { return m_cursor ? m_cursor->isVisible() : false; }

void PlotAPI::setCursorsVisible(bool visible)
{
	if(m_cursor)
		m_cursor->setVisible(visible);
}

void PlotAPI::setXCursorsEnabled(bool enabled)
{
	if(m_cursor)
		m_cursor->xEnToggled(enabled);
}

void PlotAPI::setYCursorsEnabled(bool enabled)
{
	if(m_cursor)
		m_cursor->yEnToggled(enabled);
}

void PlotAPI::setXCursorsLocked(bool locked)
{
	if(m_cursor)
		m_cursor->xLockToggled(locked);
}

void PlotAPI::setYCursorsLocked(bool locked)
{
	if(m_cursor)
		m_cursor->yLockToggled(locked);
}

void PlotAPI::setTrackingEnabled(bool enabled)
{
	if(m_cursor)
		m_cursor->xTrackToggled(enabled);
}

double PlotAPI::getX1CursorPosition()
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return 0.0;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getX1Cursor();
	return h ? h->getPosition() : 0.0;
}

double PlotAPI::getX2CursorPosition()
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return 0.0;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getX2Cursor();
	return h ? h->getPosition() : 0.0;
}

void PlotAPI::setX1CursorPosition(double pos)
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getX1Cursor();
	if(h)
		h->setPosition(pos);
}

void PlotAPI::setX2CursorPosition(double pos)
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getX2Cursor();
	if(h)
		h->setPosition(pos);
}

double PlotAPI::getY1CursorPosition()
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return 0.0;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getY1Cursor();
	return h ? h->getPosition() : 0.0;
}

double PlotAPI::getY2CursorPosition()
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return 0.0;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getY2Cursor();
	return h ? h->getPosition() : 0.0;
}

void PlotAPI::setY1CursorPosition(double pos)
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getY1Cursor();
	if(h)
		h->setPosition(pos);
}

void PlotAPI::setY2CursorPosition(double pos)
{
	if(!m_cursor || !m_cursor->getPlotCursors())
		return;
	PlotAxisHandle *h = m_cursor->getPlotCursors()->getY2Cursor();
	if(h)
		h->setPosition(pos);
}

// ==================== PRINT/EXPORT ====================

bool PlotAPI::printPlot(const QString &filePath)
{
	if(!m_plot)
		return false;

	QPdfWriter writer(filePath);
	writer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Landscape, QMarginsF(10, 10, 10, 10)));
	writer.setResolution(150);

	QPainter painter(&writer);
	if(!painter.isActive()) {
		qWarning(CAT_PLOT_API) << "Failed to open PDF file:" << filePath;
		return false;
	}

	m_plot->printPlot(&painter);
	painter.end();
	return true;
}

#include "moc_plot_api.cpp"
