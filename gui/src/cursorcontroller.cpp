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

#include "plotaxis.h"
#include <cursorcontroller.h>

using namespace scopy;

CursorController::CursorController(PlotWidget *plot, QObject *parent)
	: QObject(parent)
	, m_plot(plot)
	, xEn(true)
	, xLock(false)
	, xTrack(false)
	, yEn(true)
	, yLock(false)
	, readoutDragsEn(false)
	, m_visible(false)
{
	initUI();

	y1Cursor = plotCursors->getY1Cursor();
	y2Cursor = plotCursors->getY2Cursor();
	x1Cursor = plotCursors->getX1Cursor();
	x2Cursor = plotCursors->getX2Cursor();

	// connectSignals();
}

CursorController::~CursorController() {}

void CursorController::initUI()
{
	plotCursors = new PlotCursors(m_plot, this);
	plotCursors->setVisible(false);

	plotCursorReadouts = new PlotCursorReadouts(m_plot);
	PlotChannel *ch = m_plot->selectedChannel();
	if(ch != nullptr) {
		plotCursorReadouts->setXFormatter(ch->xAxis()->getFormatter());
		plotCursorReadouts->setYFormatter(ch->yAxis()->getFormatter());
		plotCursorReadouts->setXUnits(ch->xAxis()->getUnits());
		plotCursorReadouts->setYUnits(ch->yAxis()->getUnits());
	}
	hoverReadouts = new HoverWidget(plotCursorReadouts, m_plot->plot()->canvas(), m_plot);
	hoverReadouts->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	hoverReadouts->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverReadouts->setAnchorOffset(QPoint(-10, -10));
	hoverReadouts->setRelative(true);
}

void CursorController::connectSignals(CursorSettings *cursorSettings)
{
	// x controls
	connect(cursorSettings->getXEn(), &QAbstractButton::toggled, this, &CursorController::xEnToggled);
	connect(cursorSettings->getXLock(), &QAbstractButton::toggled, this, &CursorController::xLockToggled);
	connect(cursorSettings->getXTrack(), &QAbstractButton::toggled, plotCursors, &PlotCursors::enableTracking);

	// y controls
	connect(cursorSettings->getYEn(), &QAbstractButton::toggled, this, &CursorController::yEnToggled);
	connect(cursorSettings->getYLock(), &QAbstractButton::toggled, this, &CursorController::yLockToggled);

	// readouts controls
	connect(cursorSettings->getReadoutsDrag(), &QAbstractButton::toggled, this,
		&CursorController::readoutsDragToggled);

	//  update session in case the settings are conncted to multiple controllers
	connect(cursorSettings, &CursorSettings::sessionUpdated, this, [=]() { setVisible(isVisible()); });
	cursorSettings->updateSession();

	// cursor movement
	connect(y1Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(yLock) {
			y2Cursor->setPositionSilent(pos - yLockGap);
			plotCursorReadouts->setY2(y2Cursor->getPosition());
		}
		plotCursorReadouts->setY1(pos);
	});
	connect(y2Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(yLock) {
			y1Cursor->setPositionSilent(pos + yLockGap);
			plotCursorReadouts->setY1(y1Cursor->getPosition());
		}
		plotCursorReadouts->setY2(pos);
	});
	connect(x1Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(xLock) {
			x2Cursor->setPositionSilent(pos - LockGap);
			plotCursorReadouts->setX2(x2Cursor->getPosition());
		}
		plotCursorReadouts->setX1(pos);
	});
	connect(x2Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(xLock) {
			x1Cursor->setPositionSilent(pos + LockGap);
			plotCursorReadouts->setX1(x1Cursor->getPosition());
		}
		plotCursorReadouts->setX2(pos);
	});

	for(PlotChannel *ch : m_plot->getChannels()) {
		onAddedChannel(ch);
	}
	connect(m_plot, &PlotWidget::addedChannel, this, &CursorController::onAddedChannel);
	connect(m_plot, &PlotWidget::removedChannel, this, &CursorController::onRemovedChannel);
	connect(m_plot, &PlotWidget::channelSelected, this, [=](PlotChannel *ch) {
		PlotAxis *xAxis = m_plot->xAxis();
		PlotAxis *yAxis = m_plot->yAxis();
		if(ch != nullptr) {
			xAxis = ch->xAxis();
			yAxis = ch->yAxis();
		}

		plotCursorReadouts->setXFormatter(xAxis->getFormatter());
		plotCursorReadouts->setYFormatter(yAxis->getFormatter());
		plotCursorReadouts->setXUnits(xAxis->getUnits());
		plotCursorReadouts->setYUnits(yAxis->getUnits());
	});
}

void CursorController::xEnToggled(bool toggled)
{
	xEn = toggled;
	plotCursorReadouts->setXVisible(xEn);
	readoutsSetVisible(xEn || plotCursorReadouts->isYVisible());
	plotCursors->setXVisible(xEn);
}

void CursorController::xLockToggled(bool toggled)
{
	xLock = toggled;
	LockGap = plotCursors->getX1Cursor()->getPosition() - plotCursors->getX2Cursor()->getPosition();
}

void CursorController::xTrackToggled(bool toggled)
{
	xTrack = toggled;
	plotCursors->enableTracking(xTrack);
}

void CursorController::yEnToggled(bool toggled)
{
	yEn = toggled;
	plotCursorReadouts->setYVisible(yEn);
	readoutsSetVisible(plotCursorReadouts->isXVisible() || yEn);
	plotCursors->setYVisible(yEn);
}

void CursorController::yLockToggled(bool toggled)
{
	yLock = toggled;
	yLockGap = plotCursors->getY1Cursor()->getPosition() - plotCursors->getY2Cursor()->getPosition();
}

void CursorController::readoutsDragToggled(bool toggled)
{
	readoutDragsEn = toggled;
	hoverReadouts->setDraggable(readoutDragsEn);
}

void CursorController::onAddedChannel(PlotChannel *ch)
{
	connect(ch->xAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, &PlotCursorReadouts::setXFormatter);
	connect(ch->yAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, &PlotCursorReadouts::setYFormatter);
	connect(ch->xAxis(), &PlotAxis::unitsChanged, plotCursorReadouts, &PlotCursorReadouts::setXUnits);
	connect(ch->yAxis(), &PlotAxis::unitsChanged, plotCursorReadouts, &PlotCursorReadouts::setYUnits);
}

void CursorController::onRemovedChannel(PlotChannel *ch)
{
	disconnect(ch->xAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, nullptr);
	disconnect(ch->yAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, nullptr);
}

void CursorController::updateTracking()
{
	if(plotCursors->tracking()) {
		plotCursors->displayIntersection();
	}
}

void CursorController::syncXCursorControllers(CursorController *ctrl1, CursorController *ctrl2)
{
	ctrl2->setVisible(ctrl1->isVisible());
	ctrl2->x1Cursor->setPosition(ctrl1->x1Cursor->getPosition());
	ctrl2->x2Cursor->setPosition(ctrl1->x2Cursor->getPosition());

	// connect ctrl1 to ctrl2
	connect(ctrl1->x1Cursor, &PlotAxisHandle::scalePosChanged, ctrl2->x1Cursor, [=](double pos) {
		ctrl1->x1Cursor->blockSignals(true);
		ctrl2->x1Cursor->setPosition(pos);
		ctrl1->x1Cursor->blockSignals(false);
		ctrl2->m_plot->repaint();
	});
	connect(ctrl1->x2Cursor, &PlotAxisHandle::scalePosChanged, ctrl2->x2Cursor, [=](double pos) {
		ctrl1->x2Cursor->blockSignals(true);
		ctrl2->x2Cursor->setPosition(pos);
		ctrl1->x2Cursor->blockSignals(false);
		ctrl2->m_plot->repaint();
	});

	// connect ctrl2 to ctrl1
	connect(ctrl2->x1Cursor, &PlotAxisHandle::scalePosChanged, ctrl1->x1Cursor, [=](double pos) {
		ctrl2->x1Cursor->blockSignals(true);
		ctrl1->x1Cursor->setPosition(pos);
		ctrl2->x1Cursor->blockSignals(false);
		ctrl1->m_plot->repaint();
	});
	connect(ctrl2->x2Cursor, &PlotAxisHandle::scalePosChanged, ctrl1->x2Cursor, [=](double pos) {
		ctrl2->x2Cursor->blockSignals(true);
		ctrl1->x2Cursor->setPosition(pos);
		ctrl2->x2Cursor->blockSignals(false);
		ctrl1->m_plot->repaint();
	});
}

void CursorController::unsyncXCursorControllers(CursorController *ctrl1, CursorController *ctrl2)
{
	disconnect(ctrl1->x1Cursor, &PlotAxisHandle::scalePosChanged, ctrl2->x1Cursor, nullptr);
	disconnect(ctrl1->x2Cursor, &PlotAxisHandle::scalePosChanged, ctrl2->x2Cursor, nullptr);
	disconnect(ctrl2->x1Cursor, &PlotAxisHandle::scalePosChanged, ctrl1->x1Cursor, nullptr);
	disconnect(ctrl2->x2Cursor, &PlotAxisHandle::scalePosChanged, ctrl1->x2Cursor, nullptr);
}

bool CursorController::isVisible() { return m_visible; }

void CursorController::setVisible(bool visible)
{
	m_visible = visible;
	readoutsSetVisible(visible);
	cursorsSetVisible(visible);
	Q_EMIT visibilityChanged(visible);
}

void CursorController::readoutsSetVisible(bool visible) { hoverReadouts->setVisible(visible && (xEn || yEn)); }

void CursorController::cursorsSetVisible(bool visible)
{
	plotCursors->setXVisible(visible && xEn);
	plotCursors->setYVisible(visible && yEn);
}

PlotCursors *CursorController::getPlotCursors() { return plotCursors; }

#include "moc_cursorcontroller.cpp"
