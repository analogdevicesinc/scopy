#include "plotnavigator.hpp"
#include "mouseplotmagnifier.hpp"
#include "boundedplotzoomer.hpp"
#include "qevent.h"

#include <plotaxis.h>

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
{
	init();
}

void PlotNavigator::init()
{
	for(PlotChannel *ch : *m_channels) {
		m_axes->insert(ch->xAxis()->axisId());
		m_axes->insert(ch->yAxis()->axisId());
	}

	initNavigators();
	connect(this, &PlotNavigator::undo, this, &PlotNavigator::onUndo);
	connect(this, &PlotNavigator::reset, this, &PlotNavigator::onReset);

	setEnabled(m_en);
	setHistoryEn(m_historyEn);
}

void PlotNavigator::initNavigators()
{
	// this zoomer is only used for drawing the zoom selection rectangle
	m_visibleZoomer = new BoundedPlotZoomer(m_plot);
	m_visibleZoomer->setBlockZoomEn(true);
	m_visibleZoomer->setBlockZoomResetEn(true);
	m_visibleZoomer->setEnabled(false);

	for(QwtAxisId axisId : *m_axes) {
		addNavigators(axisId);
	}
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
	MousePlotMagnifier *magnifier = createMagnifier(axisId);
	BoundedPlotZoomer *zoomer = createZoomer(axisId);
	zoomer->setDrawZoomRectEn(false);

	nav->magnifier = magnifier;
	nav->zoomer = zoomer;
	m_navigators->insert(nav);

	connect(magnifier, &MousePlotMagnifier::zoomedRect, this,
		[=](const QRectF &rect) { addRectToHistory(nav, rect, navigationType::Magnify); });
	connect(magnifier, &MousePlotMagnifier::pannedRect, this,
		[=](const QRectF &rect) { addRectToHistory(nav, rect, navigationType::Pan); });
	connect(zoomer, &BoundedPlotZoomer::zoomed, this,
		[=](const QRectF &rect) { addRectToHistory(nav, rect, navigationType::Zoom); });

	if(m_plotWidget) {
		connect(m_plotWidget->plotAxisFromId(axisId), &PlotAxis::axisScaleUpdated, this, [=]() {
			if(m_autoBase) {
				setBaseRect();
				onReset();
			}
		});
	}
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
		}
	}

	if(doReset) {
		Q_EMIT reset();
	}
}

MousePlotMagnifier *PlotNavigator::createMagnifier(QwtAxisId axisId)
{
	MousePlotMagnifier *magnifier = new MousePlotMagnifier(m_plot);
	connect(this, &PlotNavigator::reset, magnifier, &MousePlotMagnifier::reset);

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

	return magnifier;
}

BoundedPlotZoomer *PlotNavigator::createZoomer(QwtAxisId axisId)
{
	BoundedPlotZoomer *zoomer = new BoundedPlotZoomer(m_plot);
	connect(this, &PlotNavigator::reset, zoomer, &BoundedPlotZoomer::reset);

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
	}

	axisId = channel->yAxis()->axisId();
	if(!m_axes->contains(axisId)) {
		m_axes->insert(axisId);
		addNavigators(axisId);
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

	for(Navigator *nav : *m_navigators) {
		if((xFound && nav->magnifier->getXAxis() == xAxis) || (yFound && nav->magnifier->getYAxis() == yAxis)) {
			delete nav;
			m_navigators->remove(nav);
		}
	}

	if(m_channels->empty()) {
		m_visibleZoomer->setEnabled(false);
	}
}

void PlotNavigator::setXAxesEn(bool en)
{
	m_visibleZoomer->setXAxisEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->magnifier->isXAxisEn()) {
			nav->magnifier->setEnabled(en);
		}
		if(nav->zoomer->isXAxisEn()) {
			nav->zoomer->setEnabled(en);
		}
	}
}

bool PlotNavigator::isXAxesEn() { return m_visibleZoomer->isXAxisEn(); }

void PlotNavigator::setYAxesEn(bool en)
{
	m_visibleZoomer->setYAxisEn(en);
	for(Navigator *nav : *m_navigators) {
		if(nav->magnifier->isYAxisEn()) {
			nav->magnifier->setEnabled(en);
		}
		if(nav->zoomer->isYAxisEn()) {
			nav->zoomer->setEnabled(en);
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
		MousePlotMagnifier *mag = nav->magnifier;
		mag->setEnabled(en && ((mag->isXAxisEn() && isXAxesEn()) || (mag->isYAxisEn() && isYAxesEn())));
	}
}

bool PlotNavigator::isMagnifierEn() { return m_magnifierEn; }

void PlotNavigator::setZoomerEn(bool en)
{
	m_zoomerEn = en;
	m_visibleZoomer->setEnabled(en);
	for(Navigator *nav : *m_navigators) {
		BoundedPlotZoomer *zoom = nav->zoomer;
		zoom->setEnabled(en && ((zoom->isXAxisEn() && isXAxesEn()) || (zoom->isYAxisEn() && isYAxesEn())));
	}
}

bool PlotNavigator::isZoomerEn() { return m_zoomerEn; }

void PlotNavigator::setAutoBaseEn(bool en) { m_autoBase = en; }

bool PlotNavigator::isAutoBaseEn() { return m_autoBase; }

void PlotNavigator::syncNavigators(PlotNavigator *pNav1, Navigator *nav1, PlotNavigator *pNav2, Navigator *nav2)
{
	// connect nav1 to nav2
	connect(nav1->zoomer, &BoundedPlotZoomer::zoomed, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Zoom);
	});
	connect(nav1->magnifier, &MousePlotMagnifier::zoomedRect, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Magnify);
	});
	connect(nav1->magnifier, &MousePlotMagnifier::pannedRect, pNav2, [=](const QRectF &rect) {
		nav2->zoomer->silentZoom(rect);
		pNav2->addRectToHistory(nav2, rect, navigationType::Pan);
	});
	connect(pNav1, &PlotNavigator::reset, nav2->magnifier, &MousePlotMagnifier::reset);
	connect(pNav1, &PlotNavigator::reset, nav2->zoomer, &BoundedPlotZoomer::reset);

	// connect nav2 to nav1
	connect(nav2->zoomer, &BoundedPlotZoomer::zoomed, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Zoom);
	});
	connect(nav2->magnifier, &MousePlotMagnifier::zoomedRect, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Magnify);
	});
	connect(nav2->magnifier, &MousePlotMagnifier::pannedRect, pNav1, [=](const QRectF &rect) {
		nav1->zoomer->silentZoom(rect);
		pNav1->addRectToHistory(nav1, rect, navigationType::Pan);
	});
	connect(pNav2, &PlotNavigator::reset, nav1->magnifier, &MousePlotMagnifier::reset);
	connect(pNav2, &PlotNavigator::reset, nav1->zoomer, &BoundedPlotZoomer::reset);
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
