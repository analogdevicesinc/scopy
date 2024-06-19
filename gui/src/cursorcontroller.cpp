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
{
	initUI();

	y1Cursor = plotCursors->getY1Cursor();
	y2Cursor = plotCursors->getY2Cursor();
	x1Cursor = plotCursors->getX1Cursor();
	x2Cursor = plotCursors->getX2Cursor();

	connectSignals();
}

CursorController::~CursorController() {}

void CursorController::initUI()
{
	plotCursors = new PlotCursors(m_plot);
	plotCursors->setVisible(false);

	plotCursorReadouts = new PlotCursorReadouts(m_plot);
	PlotChannel *ch = m_plot->selectedChannel();
	plotCursorReadouts->setXFormatter(ch->xAxis()->getFormatter());
	plotCursorReadouts->setYFormatter(ch->yAxis()->getFormatter());
	plotCursorReadouts->setXUnits(ch->xAxis()->getUnits());
	plotCursorReadouts->setYUnits(ch->yAxis()->getUnits());

	hoverReadouts = new HoverWidget(plotCursorReadouts, m_plot->plot()->canvas(), m_plot);
	hoverReadouts->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hoverReadouts->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	hoverReadouts->setAnchorOffset(QPoint(10, 10));
	hoverReadouts->setRelative(true);

	cursorSettings = new CursorSettings(m_plot);
}

void CursorController::connectSignals()
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

	initSession();

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
		plotCursorReadouts->setXFormatter(ch->xAxis()->getFormatter());
		plotCursorReadouts->setYFormatter(ch->yAxis()->getFormatter());
		plotCursorReadouts->setXUnits(ch->xAxis()->getUnits());
		plotCursorReadouts->setYUnits(ch->yAxis()->getUnits());
	});
}

void CursorController::initSession()
{
	cursorSettings->getXEn()->toggled(xEn);
	cursorSettings->getXLock()->toggled(xLock);
	cursorSettings->getXTrack()->toggled(xTrack);
	cursorSettings->getYEn()->toggled(yEn);
	cursorSettings->getYLock()->toggled(yLock);
	cursorSettings->getReadoutsDrag()->toggled(readoutDragsEn);

	setVisible(false);
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

void CursorController::setVisible(bool visible)
{
	readoutsSetVisible(visible);
	cursorsSetVisible(visible);
}

void CursorController::readoutsSetVisible(bool visible) { hoverReadouts->setVisible(visible && (xEn || yEn)); }

void CursorController::cursorsSetVisible(bool visible)
{
	plotCursors->setXVisible(visible && xEn);
	plotCursors->setYVisible(visible && yEn);
}

CursorSettings *CursorController::getCursorSettings() { return cursorSettings; }

PlotCursors *CursorController::getPlotCursors() { return plotCursors; }

#include "moc_cursorcontroller.cpp"
