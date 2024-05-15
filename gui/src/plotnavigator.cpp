#include "plotnavigator.hpp"
#include "plotmagnifier.hpp"
#include "plotzoomer.hpp"
#include "qevent.h"

#include <QPushButton>
#include <hoverwidget.h>
#include <plotaxis.h>
#include <stylehelper.h>

using namespace scopy;

Navigator::Navigator()
{
	history = new QStack<QRectF>();
	lastOperation = navigationType::None;
}

Navigator::~Navigator()
{
	delete history;
	if(magnifier)
		magnifier->deleteLater();
	if(zoomer)
		zoomer->deleteLater();
}

PlotNavigator::PlotNavigator(PlotWidget *plotWidget, QSet<PlotChannel *> *channels)
	: QObject(plotWidget->plot())
	, m_plotWidget(plotWidget)
	, m_plot(plotWidget->plot())
	, m_axes(new QSet<QwtAxisId>())
	, m_channels(channels)
	, m_navigators(new QSet<Navigator *>())
	, m_en(true)
	, m_historyEn(true)
	, m_autoBase(true)
	, m_zoomerEn(true)
	, m_magnifierEn(true)
	, m_resetOnNewBase(true)
	, m_zoomerXModifier(Qt::ShiftModifier)
	, m_zoomerYModifier(Qt::ControlModifier)
	, m_zoomerXYModifier(Qt::NoModifier)
	, m_magnifierPanModifier(Qt::ShiftModifier)
	, m_magnifierZoomModifier(Qt::NoModifier)
	, m_resetButton(nullptr)
{
	init();
}

void PlotNavigator::init()
{
	for(PlotChannel *ch : *m_channels) {
		m_axes->insert(ch->xAxis()->axisId());
		m_axes->insert(ch->yAxis()->axisId());
	}

	initResetButton();
	setResetButtonEn(true);

	initNavigators();
	connect(this, &PlotNavigator::undo, this, &PlotNavigator::onUndo);
	connect(this, &PlotNavigator::reset, this, &PlotNavigator::onReset);

	setEnabled(m_en);
	setHistoryEn(m_historyEn);
}

void PlotNavigator::initNavigators()
{
	// this zoomer is only used for drawing the zoom selection rectangle
	m_visibleZoomer = new PlotZoomer(m_plot);
	m_visibleZoomer->setBlockZoomEn(true);
	m_visibleZoomer->setBlockZoomResetEn(true);
	m_visibleZoomer->setEnabled(false);
	m_visibleZoomer->setMinimumZoom(0);

	for(QwtAxisId axisId : *m_axes) {
		addNavigators(axisId);
	}
}

void PlotNavigator::initResetButton()
{
	m_resetButton = new QPushButton(m_plot->canvas());
	QIcon icon(QPixmap(":/gui/icons/search_crossed.svg"));
	m_resetButton->setFlat(true);
	StyleHelper::TransparentWidget(m_resetButton);
	m_resetButton->setIcon(icon);

	connect(m_resetButton, &QPushButton::clicked, this, [=]() { Q_EMIT reset(); });
	connect(this, &PlotNavigator::rectChanged, this, [=]() { m_resetButton->setVisible(isZoomed()); });

	m_resetHover = new HoverWidget(m_resetButton, m_plot->canvas(), m_plot->canvas());
	m_resetHover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	m_resetHover->setContentPos(HoverPosition::HP_TOPLEFT);
	m_resetHover->setAnchorOffset(QPoint(-6, -6));
	m_resetHover->show();
}

QWidget *PlotNavigator::canvas() { return m_plot->canvas(); }

bool PlotNavigator::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);

		if(mouseEvent->button() == Qt::MouseButton::RightButton) {
			if(m_historyEn) {
				Q_EMIT undo();
			} else {
				Q_EMIT reset();
			}
		}
	} else if(event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

		if(keyEvent->modifiers().testFlag(m_zoomerXModifier)) {
			setZoomerXAxesEn(true);
			setZoomerYAxesEn(false);
		} else if(keyEvent->modifiers().testFlag(m_zoomerYModifier)) {
			setZoomerXAxesEn(false);
			setZoomerYAxesEn(true);
		} else if(keyEvent->modifiers().testFlag(m_zoomerXYModifier)) {
			setZoomerXAxesEn(true);
			setZoomerYAxesEn(true);
		}
	} else if(event->type() == QEvent::KeyRelease) {
		QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

		if(m_zoomerXYModifier ==
		   Qt::NoModifier) { // we can't catch NoModifier event so it assumes it's always enabled
			setZoomerXAxesEn(true);
			setZoomerYAxesEn(true);
		} else {
			if(keyEvent->modifiers().testFlag(m_zoomerXModifier) ||
			   keyEvent->modifiers().testFlag(m_zoomerYModifier) ||
			   keyEvent->modifiers().testFlag(m_zoomerXYModifier)) {
				setZoomerXAxesEn(false);
				setZoomerYAxesEn(false);
			}
		}
	}

	return QObject::eventFilter(object, event);
}

QSet<Navigator *> *PlotNavigator::navigators() { return m_navigators; }

void PlotNavigator::addRectToHistory(Navigator *nav, const QRectF &rect, navigationType type)
{
	if(m_historyEn) {
		if(nav->lastOperation == type && (type == navigationType::Magnify || type == navigationType::Pan)) {
			nav->history->pop();
		}
		nav->history->push(rect);
		nav->lastOperation = type;
	}
}

void PlotNavigator::addNavigators(QwtAxisId axisId)
{
	Navigator *nav = new Navigator();
	PlotMagnifier *magnifier = createMagnifier(axisId);
	PlotZoomer *zoomer = createZoomer(axisId);

	nav->magnifier = magnifier;
	nav->zoomer = zoomer;
	m_navigators->insert(nav);

	connect(magnifier, &PlotMagnifier::zoomedRect, this, [=](const QRectF &rect) {
		addRectToHistory(nav, rect, navigationType::Magnify);
		Q_EMIT rectChanged(rect, navigationType::Magnify);
	});
	connect(magnifier, &PlotMagnifier::pannedRect, this, [=](const QRectF &rect) {
		addRectToHistory(nav, rect, navigationType::Pan);
		Q_EMIT rectChanged(rect, navigationType::Pan);
	});
	connect(zoomer, &PlotZoomer::zoomed, this, [=](const QRectF &rect) {
		addRectToHistory(nav, rect, navigationType::Zoom);
		Q_EMIT rectChanged(rect, navigationType::Zoom);
	});

	connect(magnifier, &PlotMagnifier::reset, this,
		[=]() { Q_EMIT rectChanged(zoomer->zoomBase(), navigationType::None); });
	connect(zoomer, &PlotZoomer::reset, this,
		[=]() { Q_EMIT rectChanged(zoomer->zoomBase(), navigationType::None); });

	connect(m_plotWidget->plotAxisFromId(axisId), &PlotAxis::axisScaleUpdated, this, [=]() {
		if(m_autoBase) {
			setBaseRect(axisId);
			if(m_resetOnNewBase)
				Q_EMIT reset();
		}
	});
}

void PlotNavigator::removeNavigators(QwtAxisId axisId)
{
	for(Navigator *nav : *m_navigators) {
		if(nav->magnifier->getXAxis() == axisId || nav->magnifier->getYAxis() == axisId) {
			delete nav;
			m_navigators->remove(nav);
		}
	}
}

void PlotNavigator::onReset()
{
	for(Navigator *nav : *m_navigators) {
		nav->history->clear();
		nav->lastOperation = navigationType::None;
	}
}

void PlotNavigator::onUndo()
{
	bool doReset = true;
	for(Navigator *nav : *m_navigators) {
		if(nav->history->length() > 1) {
			nav->history->pop();
			nav->zoomer->silentZoom(nav->history->top());
			doReset = false;
			Q_EMIT rectChanged(nav->history->top(), navigationType::None);
		}
	}

	if(doReset) {
		Q_EMIT reset();
	}
}

PlotMagnifier *PlotNavigator::createMagnifier(QwtAxisId axisId)
{
	PlotMagnifier *magnifier = new PlotMagnifier(m_plot);
	connect(this, &PlotNavigator::reset, magnifier, &PlotMagnifier::reset);

	if(axisId.isXAxis()) {
		magnifier->setXAxis(axisId);
		magnifier->setYAxisEn(false);
		magnifier->setEnabled(isXAxesEn() && isMagnifierEn());
	}
	if(axisId.isYAxis()) {
		magnifier->setYAxis(axisId);
		magnifier->setXAxisEn(false);
		magnifier->setEnabled(isYAxesEn() && isMagnifierEn());
	}
	magnifier->setBlockZoomResetEn(true);
	magnifier->setBounded(isBounded());
	magnifier->setPanModifier(m_magnifierPanModifier);
	magnifier->setZoomModifier(m_magnifierZoomModifier);

	return magnifier;
}

PlotZoomer *PlotNavigator::createZoomer(QwtAxisId axisId)
{
	PlotZoomer *zoomer = new PlotZoomer(m_plot);
	connect(this, &PlotNavigator::reset, zoomer, &PlotZoomer::reset);

	if(axisId.isXAxis()) {
		zoomer->setXAxis(axisId);
		zoomer->setYAxisEn(false);
		zoomer->setEnabled(isXAxesEn() && isZoomerEn());
	}
	if(axisId.isYAxis()) {
		zoomer->setYAxis(axisId);
		zoomer->setXAxisEn(false);
		zoomer->setEnabled(isYAxesEn() && isZoomerEn());
	}
	zoomer->setBlockZoomResetEn(true);
	zoomer->setDrawZoomRectEn(false);
	zoomer->setBounded(isBounded());
	zoomer->setMinimumZoom(0);

	return zoomer;
}

PlotNavigator::~PlotNavigator() {}

QSet<QwtAxisId> *PlotNavigator::axes() { return m_axes; }

QSet<PlotChannel *> *PlotNavigator::channels() { return m_channels; }

void PlotNavigator::addChannel(PlotChannel *channel)
{
	m_channels->insert(channel);

	// insert axes if there aren't other channels already using them
	QwtAxisId axisId = channel->xAxis()->axisId();
	if(!m_axes->contains(axisId)) {
		m_axes->insert(axisId);
		addNavigators(axisId);
		setBaseRect(axisId);
	}

	axisId = channel->yAxis()->axisId();
	if(!m_axes->contains(axisId)) {
		m_axes->insert(axisId);
		addNavigators(axisId);
		setBaseRect(axisId);
	}

	m_visibleZoomer->setEnabled(isZoomerEn());
}

void PlotNavigator::removeChannel(PlotChannel *channel)
{
	m_channels->remove(channel);

	// remove axes if there aren't other channels using them
	QwtAxisId xAxis = channel->xAxis()->axisId();
	bool xFound = false;
	QwtAxisId yAxis = channel->yAxis()->axisId();
	bool yFound = false;

	for(PlotChannel *ch : *m_channels) {
		if(ch->xAxis()->axisId() == xAxis) {
			xFound = true;
		}
		if(ch->yAxis()->axisId() == yAxis) {
			yFound = true;
		}
	}

	/*for(Navigator *nav : *m_navigators) {
		if((xFound && nav->magnifier->getXAxis() == xAxis) || (yFound && nav->magnifier->getYAxis() == yAxis)) {
			m_navigators->remove(nav);
			delete nav;
		}
	}*/

	if(m_channels->empty()) {
		m_visibleZoomer->setEnabled(false);
	}
}

bool PlotNavigator::isZoomed()
{
	bool zoomed = false;
	for(Navigator *nav : *m_navigators) {
		if((nav->zoomer->isEnabled() && nav->zoomer->isZoomed()) ||
		   (nav->magnifier->isEnabled() && nav->magnifier->isZoomed()) &&
			   nav->zoomer->getCurrentRect() != nav->zoomer->zoomBase()) {
			zoomed = true;
			break;
		}
	}

	return zoomed;
}

void PlotNavigator::setZoomerXAxesEn(bool en)
{
	m_visibleZoomer->setXAxisEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->zoomer->isXAxisEn()) {
			nav->zoomer->setEnabled(en);
		}
	}
}

void PlotNavigator::setZoomerYAxesEn(bool en)
{
	m_visibleZoomer->setYAxisEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->zoomer->isYAxisEn()) {
			nav->zoomer->setEnabled(en);
		}
	}
}

void PlotNavigator::setXAxesEn(bool en)
{
	setZoomerXAxesEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->magnifier->isXAxisEn()) {
			nav->magnifier->setEnabled(en);
		}
	}
}

bool PlotNavigator::isXAxesEn() { return m_visibleZoomer->isXAxisEn(); }

void PlotNavigator::setYAxesEn(bool en)
{
	setZoomerYAxesEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->magnifier->isYAxisEn()) {
			nav->magnifier->setEnabled(en);
		}
	}
}

bool PlotNavigator::isYAxesEn() { return m_visibleZoomer->isYAxisEn(); }

bool PlotNavigator::isEnabled() { return m_en; }

void PlotNavigator::setEnabled(bool en)
{
	m_en = en;
	if(m_en) {
		canvas()->installEventFilter(this);
	} else {
		canvas()->removeEventFilter(this);
	}

	setMagnifierEn(en);
	setZoomerEn(en);
}

void PlotNavigator::setBaseRect(const QRectF &rect)
{
	for(Navigator *nav : *m_navigators) {
		nav->magnifier->setBaseRect(rect);
		nav->zoomer->setBaseRect(rect);
	}
}

void PlotNavigator::setBaseRect(QwtAxisId axisId)
{
	for(Navigator *nav : *m_navigators) {
		if(nav->zoomer->getXAxis() == axisId || nav->zoomer->getYAxis() == axisId) {
			nav->magnifier->setBaseRect();
			nav->zoomer->setBaseRect();
		}
	}
}

void PlotNavigator::setBaseRect()
{
	for(Navigator *nav : *m_navigators) {
		nav->magnifier->setBaseRect();
		nav->zoomer->setBaseRect();
	}
}

void PlotNavigator::setBounded(bool bounded)
{
	m_visibleZoomer->setBounded(bounded);
	for(Navigator *nav : *m_navigators) {
		nav->magnifier->setBounded(bounded);
		nav->zoomer->setBounded(bounded);
	}
}

bool PlotNavigator::isBounded() { return m_visibleZoomer->isBoundEn(); }

void PlotNavigator::setHistoryEn(bool en) { m_historyEn = en; }

bool PlotNavigator::isHistoryEn() { return m_historyEn; }

void PlotNavigator::setMagnifierEn(bool en)
{
	m_magnifierEn = en;
	for(Navigator *nav : *m_navigators) {
		PlotMagnifier *mag = nav->magnifier;
		mag->setEnabled(en && ((mag->isXAxisEn() && isXAxesEn()) || (mag->isYAxisEn() && isYAxesEn())));
	}
}

bool PlotNavigator::isMagnifierEn() { return m_magnifierEn; }

void PlotNavigator::setZoomerEn(bool en)
{
	m_zoomerEn = en;
	m_visibleZoomer->setEnabled(en);
	for(Navigator *nav : *m_navigators) {
		PlotZoomer *zoom = nav->zoomer;
		zoom->setEnabled(en && ((zoom->isXAxisEn() && isXAxesEn()) || (zoom->isYAxisEn() && isYAxesEn())));
	}
}

void PlotNavigator::setMagnifierPanModifier(Qt::KeyboardModifier modifier)
{
	m_magnifierPanModifier = modifier;
	for(Navigator *nav : *m_navigators) {
		nav->magnifier->setPanModifier(modifier);
	}
}

void PlotNavigator::setMagnifierZoomModifier(Qt::KeyboardModifier modifier)
{
	m_magnifierZoomModifier = modifier;
	for(Navigator *nav : *m_navigators) {
		nav->magnifier->setZoomModifier(modifier);
	}
}

Qt::KeyboardModifier PlotNavigator::getMagnifierPanModifier() { return m_magnifierPanModifier; }

Qt::KeyboardModifier PlotNavigator::getMagnifierZoomModifier() { return m_magnifierZoomModifier; }

void PlotNavigator::setZoomerXModifier(Qt::KeyboardModifier modifier) { m_zoomerXModifier = modifier; }

void PlotNavigator::setZoomerYModifier(Qt::KeyboardModifier modifier) { m_zoomerYModifier = modifier; }

void PlotNavigator::setZoomerXYModifier(Qt::KeyboardModifier modifier) { m_zoomerXYModifier = modifier; }

Qt::KeyboardModifier PlotNavigator::getZoomerXModifier() { return m_zoomerXModifier; }

Qt::KeyboardModifier PlotNavigator::getZoomerYModifier() { return m_zoomerYModifier; }

Qt::KeyboardModifier PlotNavigator::getZoomerXYModifier() { return m_zoomerXYModifier; }

bool PlotNavigator::isZoomerEn() { return m_zoomerEn; }

void PlotNavigator::setAutoBaseEn(bool en) { m_autoBase = en; }

bool PlotNavigator::isAutoBaseEn() { return m_autoBase; }

void PlotNavigator::setResetOnNewBase(bool en) { m_resetOnNewBase = en; }

bool PlotNavigator::getResetOnNewBase() { return m_resetOnNewBase; }

void PlotNavigator::syncNavigators(PlotNavigator *pNav1, Navigator *nav1, PlotNavigator *pNav2, Navigator *nav2)
{
	// connect nav1 to nav2
	connect(nav1->zoomer, &PlotZoomer::zoomed, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Zoom);
	});
	connect(nav1->magnifier, &PlotMagnifier::zoomedRect, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Magnify);
	});
	connect(nav1->magnifier, &PlotMagnifier::pannedRect, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Pan);
	});
	connect(pNav1, &PlotNavigator::reset, nav2->magnifier, &PlotMagnifier::reset);
	connect(pNav1, &PlotNavigator::reset, nav2->zoomer, &PlotZoomer::reset);

	// connect nav2 to nav1
	connect(nav2->zoomer, &PlotZoomer::zoomed, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Zoom);
	});
	connect(nav2->magnifier, &PlotMagnifier::zoomedRect, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Magnify);
	});
	connect(nav2->magnifier, &PlotMagnifier::pannedRect, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Pan);
	});
	connect(pNav2, &PlotNavigator::reset, nav1->magnifier, &PlotMagnifier::reset);
	connect(pNav2, &PlotNavigator::reset, nav1->zoomer, &PlotZoomer::reset);
}

void PlotNavigator::syncPlotNavigators(PlotNavigator *pNav1, PlotNavigator *pNav2, QSet<QwtAxisId> *axes)
{
	connect(pNav1, &PlotNavigator::undo, pNav2, &PlotNavigator::onUndo);
	connect(pNav1, &PlotNavigator::reset, pNav2, &PlotNavigator::onReset);

	connect(pNav2, &PlotNavigator::undo, pNav1, &PlotNavigator::onUndo);
	connect(pNav2, &PlotNavigator::reset, pNav1, &PlotNavigator::onReset);

	for(Navigator *nav1 : *pNav1->navigators()) {
		if((nav1->magnifier->isXAxisEn() && axes->contains(nav1->magnifier->getXAxis())) ||
		   (nav1->magnifier->isYAxisEn() && axes->contains(nav1->magnifier->getYAxis()))) {
			for(Navigator *nav2 : *pNav2->navigators()) {
				if((nav2->magnifier->isXAxisEn() && axes->contains(nav2->magnifier->getXAxis())) ||
				   (nav2->magnifier->isYAxisEn() && axes->contains(nav2->magnifier->getYAxis()))) {
					syncNavigators(pNav1, nav1, pNav2, nav2);
				}
			}
		}
	}
}

void PlotNavigator::setResetButtonEn(bool en) { m_resetHover->setVisible(en); }

#include "moc_plotnavigator.cpp"
