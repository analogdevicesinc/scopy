#include "plotaxis.h"
#include <cursorcontroller.h>

using namespace scopy;

CursorController::CursorController(PlotWidget *plot, QObject *parent)
	: QObject(parent)
	, m_plot(plot)
	, horizEn(true)
	, horizLock(false)
	, horizTrack(false)
	, vertEn(true)
	, vertLock(false)
	, readoutDragsEn(false)
{
	initUI();

	v1Cursor = plotCursors->getV1Cursor();
	v2Cursor = plotCursors->getV2Cursor();
	h1Cursor = plotCursors->getH1Cursor();
	h2Cursor = plotCursors->getH2Cursor();

	connectSignals();
}

CursorController::~CursorController() {}

void CursorController::initUI()
{
	plotCursors = new PlotCursors(m_plot);
	plotCursors->setVisible(false);

	plotCursorReadouts = new PlotCursorReadouts(m_plot);
	PlotChannel *ch = m_plot->selectedChannel();
	plotCursorReadouts->setHorizFromatter(ch->xAxis()->getFromatter());
	plotCursorReadouts->setVertFromatter(ch->yAxis()->getFromatter());
	plotCursorReadouts->setHorizUnits(ch->xAxis()->getUnits());
	plotCursorReadouts->setVertUnits(ch->yAxis()->getUnits());

	hoverReadouts = new HoverWidget(plotCursorReadouts, m_plot->plot()->canvas(), m_plot);
	hoverReadouts->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hoverReadouts->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	hoverReadouts->setAnchorOffset(QPoint(10, 10));
	hoverReadouts->setRelative(true);

	cursorSettings = new CursorSettings(m_plot);
}

void CursorController::connectSignals()
{
	// horizontal controls
	connect(cursorSettings->getHorizEn(), &QAbstractButton::toggled, this, &CursorController::horizEnToggled);
	connect(cursorSettings->getHorizLock(), &QAbstractButton::toggled, this, &CursorController::horizLockToggled);
	connect(cursorSettings->getHorizTrack(), &QAbstractButton::toggled, plotCursors, &PlotCursors::enableTracking);

	// vertical controls
	connect(cursorSettings->getVertEn(), &QAbstractButton::toggled, this, &CursorController::vertEnToggled);
	connect(cursorSettings->getVertLock(), &QAbstractButton::toggled, this, &CursorController::vertLockToggled);

	// readouts controls
	connect(cursorSettings->getReadoutsDrag(), &QAbstractButton::toggled, this,
		&CursorController::readoutsDragToggled);

	initSession();

	// cursor movement
	connect(v1Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(vertLock) {
			v2Cursor->setPositionSilent(pos - vertLockGap);
			plotCursorReadouts->setV2(v2Cursor->getPosition());
		}
		plotCursorReadouts->setV1(pos);
	});
	connect(v2Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(vertLock) {
			v1Cursor->setPositionSilent(pos + vertLockGap);
			plotCursorReadouts->setV1(v1Cursor->getPosition());
		}
		plotCursorReadouts->setV2(pos);
	});
	connect(h1Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(horizLock) {
			h2Cursor->setPositionSilent(pos - horizLockGap);
			plotCursorReadouts->setH2(h2Cursor->getPosition());
		}
		plotCursorReadouts->setH1(pos);
	});
	connect(h2Cursor, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		if(horizLock) {
			h1Cursor->setPositionSilent(pos + horizLockGap);
			plotCursorReadouts->setH1(h1Cursor->getPosition());
		}
		plotCursorReadouts->setH2(pos);
	});

	for(PlotChannel *ch : m_plot->getChannels()) {
		onAddedChannel(ch);
	}
	connect(m_plot, &PlotWidget::addedChannel, this, &CursorController::onAddedChannel);
	connect(m_plot, &PlotWidget::removedChannel, this, &CursorController::onRemovedChannel);
	connect(m_plot, &PlotWidget::channelSelected, this, [=](PlotChannel *ch) {
		plotCursorReadouts->setHorizFromatter(ch->xAxis()->getFromatter());
		plotCursorReadouts->setVertFromatter(ch->yAxis()->getFromatter());
		plotCursorReadouts->setHorizUnits(ch->xAxis()->getUnits());
		plotCursorReadouts->setVertUnits(ch->yAxis()->getUnits());
	});
}

void CursorController::initSession()
{
	cursorSettings->getHorizEn()->toggled(horizEn);
	cursorSettings->getHorizLock()->toggled(horizLock);
	cursorSettings->getHorizTrack()->toggled(horizTrack);
	cursorSettings->getVertEn()->toggled(vertEn);
	cursorSettings->getVertLock()->toggled(vertLock);
	cursorSettings->getReadoutsDrag()->toggled(readoutDragsEn);

	setVisible(false);
}

void CursorController::horizEnToggled(bool toggled)
{
	horizEn = toggled;
	plotCursorReadouts->horizSetVisible(horizEn);
	readoutsSetVisible(horizEn || plotCursorReadouts->vertIsVisible());
	plotCursors->horizSetVisible(horizEn);
}

void CursorController::horizLockToggled(bool toggled)
{
	horizLock = toggled;
	horizLockGap = plotCursors->getH1Cursor()->getPosition() - plotCursors->getH2Cursor()->getPosition();
}

void CursorController::horizTrackToggled(bool toggled)
{
	horizTrack = toggled;
	plotCursors->enableTracking(horizTrack);
}

void CursorController::vertEnToggled(bool toggled)
{
	vertEn = toggled;
	plotCursorReadouts->vertSetVisible(vertEn);
	readoutsSetVisible(plotCursorReadouts->horizIsVisible() || vertEn);
	plotCursors->vertSetVisible(vertEn);
}

void CursorController::vertLockToggled(bool toggled)
{
	vertLock = toggled;
	vertLockGap = plotCursors->getV1Cursor()->getPosition() - plotCursors->getV2Cursor()->getPosition();
}

void CursorController::readoutsDragToggled(bool toggled)
{
	readoutDragsEn = toggled;
	hoverReadouts->setDraggable(readoutDragsEn);
}

void CursorController::onAddedChannel(PlotChannel *ch)
{
	connect(ch->xAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, &PlotCursorReadouts::setHorizFromatter);
	connect(ch->yAxis(), &PlotAxis::formatterChanged, plotCursorReadouts, &PlotCursorReadouts::setVertFromatter);
	connect(ch->xAxis(), &PlotAxis::unitsChanged, plotCursorReadouts, &PlotCursorReadouts::setHorizUnits);
	connect(ch->yAxis(), &PlotAxis::unitsChanged, plotCursorReadouts, &PlotCursorReadouts::setVertUnits);
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

void CursorController::readoutsSetVisible(bool visible) { hoverReadouts->setVisible(visible && (horizEn || vertEn)); }

void CursorController::cursorsSetVisible(bool visible)
{
	plotCursors->horizSetVisible(visible && horizEn);
	plotCursors->vertSetVisible(visible && vertEn);
}

CursorSettings *CursorController::getCursorSettings() { return cursorSettings; }

PlotCursors *CursorController::getPlotCursors() { return plotCursors; }

#include "moc_cursorcontroller.cpp"
