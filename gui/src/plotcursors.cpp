#include "plotcursors.h"
#include "plotaxis.h"

using namespace scopy;

PlotCursors::PlotCursors(PlotWidget *plot)
	: m_plot(plot)
	, m_tracking(false)
{
	initUI();
	connectSignals();
}

PlotCursors::~PlotCursors() {}

void PlotCursors::initUI()
{
	m_yCursors.first = new PlotAxisHandle(m_plot, m_plot->yAxis());
	m_yCursors.second = new PlotAxisHandle(m_plot, m_plot->yAxis());
	m_xCursors.first = new PlotAxisHandle(m_plot, m_plot->xAxis());
	m_xCursors.second = new PlotAxisHandle(m_plot, m_plot->xAxis());

	plotMarker1 = new QwtPlotMarker();
	plotMarker2 = new QwtPlotMarker();
	plotMarker1->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QColor(237, 28, 36),
					     QPen(QColor(255, 255, 255, 140), 2, Qt::SolidLine), QSize(5, 5)));
	plotMarker2->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QColor(237, 28, 36),
					     QPen(QColor(255, 255, 255, 140), 2, Qt::SolidLine), QSize(5, 5)));
}

void PlotCursors::connectSignals()
{
	connect(this, &PlotCursors::update, this, [=]() {
		Q_EMIT m_yCursors.first->updatePos();
		Q_EMIT m_yCursors.second->updatePos();
		Q_EMIT m_xCursors.first->updatePos();
		Q_EMIT m_xCursors.second->updatePos();
		if(m_tracking) {
			displayIntersection();
		}
	});

	connect(m_xCursors.first, &PlotAxisHandle::scalePosChanged, this, [=]() {
		if(m_tracking) {
			displayIntersection();
		}
	});
	connect(m_xCursors.second, &PlotAxisHandle::scalePosChanged, this, [=]() {
		if(m_tracking) {
			displayIntersection();
		}
	});
	connect(m_plot, &PlotWidget::channelSelected, this, [=](PlotChannel *ch) {
		PlotAxis *xAxis = m_plot->xAxis();
		PlotAxis *yAxis = m_plot->xAxis();

		if(ch != nullptr) {
			xAxis = ch->xAxis();
			yAxis = ch->yAxis();
		}
		m_yCursors.first->setAxis(yAxis);
		m_yCursors.second->setAxis(yAxis);
		m_xCursors.first->setAxis(xAxis);
		m_xCursors.second->setAxis(xAxis);
		Q_EMIT update();
	});
}

void PlotCursors::updateTracking()
{
	if(m_tracking && m_xCursors.first->handle()->isVisible()) {
		plotMarker1->attach(m_plot->plot());
	} else {
		plotMarker1->detach();
	}

	if(m_tracking && m_xCursors.second->handle()->isVisible()) {
		plotMarker2->attach(m_plot->plot());
	} else {
		plotMarker2->detach();
	}

	displayIntersection();
}

void PlotCursors::setBounded(bool leave)
{
	getY1Cursor()->handle()->setBounded(leave);
	getY2Cursor()->handle()->setBounded(leave);
	getX1Cursor()->handle()->setBounded(leave);
	getX2Cursor()->handle()->setBounded(leave);
}

PlotAxisHandle *PlotCursors::getY1Cursor() { return m_yCursors.first; }

PlotAxisHandle *PlotCursors::getY2Cursor() { return m_yCursors.second; }

PlotAxisHandle *PlotCursors::getX1Cursor() { return m_xCursors.first; }

PlotAxisHandle *PlotCursors::getX2Cursor() { return m_xCursors.second; }

void PlotCursors::setVisible(bool visible)
{
	setXVisible(visible);
	setYVisible(visible);
}

void PlotCursors::setXVisible(bool visible)
{
	m_xCursors.first->handle()->setVisible(visible);
	m_xCursors.second->handle()->setVisible(visible);
	m_xCursors.first->handle()->raise();
	m_xCursors.second->handle()->raise();
	updateTracking();
	Q_EMIT update();
}

void PlotCursors::setYVisible(bool visible)
{
	m_yCursors.first->handle()->setVisible(visible && !m_tracking);
	m_yCursors.second->handle()->setVisible(visible && !m_tracking);
	m_yCursors.first->handle()->raise();
	m_yCursors.second->handle()->raise();
	updateTracking();
	Q_EMIT update();
}

void PlotCursors::enableTracking(bool tracking)
{
	m_tracking = tracking;

	m_yCursors.first->handle()->setVisible(!tracking);
	m_yCursors.second->handle()->setVisible(!tracking);
	updateTracking();
	Q_EMIT update();
}

bool PlotCursors::tracking() const { return m_tracking; }

void PlotCursors::displayIntersection()
{
	if(m_plot->selectedChannel() == nullptr)
		return;
	QwtAxisId yaxis = m_plot->selectedChannel()->yAxis()->axisId();
	QwtAxisId xaxis = m_plot->selectedChannel()->xAxis()->axisId();
	double h1CursorPos = m_xCursors.first->getPosition();
	double h2CursorPos = m_xCursors.second->getPosition();

	plotMarker1->setAxes(xaxis, yaxis);
	plotMarker2->setAxes(xaxis, yaxis);

	auto ch = m_plot->selectedChannel();
	plotMarker1->setValue(h1CursorPos, ch->getValueAt(h1CursorPos));
	plotMarker2->setValue(h2CursorPos, ch->getValueAt(h2CursorPos));

	Q_EMIT m_yCursors.first->scalePosChanged(plotMarker1->yValue());
	Q_EMIT m_yCursors.second->scalePosChanged(plotMarker2->yValue());

	m_plot->replot();
}

void PlotCursors::setYHandlePos(HandlePos pos)
{
	m_yCursors.first->handle()->setHandlePos(pos);
	m_yCursors.second->handle()->setHandlePos(pos);
}

void PlotCursors::setXHandlePos(HandlePos pos)
{
	m_xCursors.first->handle()->setHandlePos(pos);
	m_xCursors.second->handle()->setHandlePos(pos);
}

#include "moc_plotcursors.cpp"
