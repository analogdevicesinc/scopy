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
	m_vCursors.first = new PlotAxisHandle(m_plot, m_plot->selectedChannel()->yAxis());
	m_vCursors.second = new PlotAxisHandle(m_plot, m_plot->selectedChannel()->yAxis());
	m_hCursors.first = new PlotAxisHandle(m_plot, m_plot->selectedChannel()->xAxis());
	m_hCursors.second = new PlotAxisHandle(m_plot, m_plot->selectedChannel()->xAxis());

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
		Q_EMIT m_vCursors.first->updatePos();
		Q_EMIT m_vCursors.second->updatePos();
		Q_EMIT m_hCursors.first->updatePos();
		Q_EMIT m_hCursors.second->updatePos();
		if(m_tracking) {
			displayIntersection();
		}
	});

	connect(m_hCursors.first, &PlotAxisHandle::scalePosChanged, this, [=]() {
		if(m_tracking) {
			displayIntersection();
		}
	});
	connect(m_hCursors.second, &PlotAxisHandle::scalePosChanged, this, [=]() {
		if(m_tracking) {
			displayIntersection();
		}
	});
	connect(m_plot, &PlotWidget::channelSelected, this, [=](PlotChannel *ch) {
		m_vCursors.first->setAxis(ch->yAxis());
		m_vCursors.second->setAxis(ch->yAxis());
		m_hCursors.first->setAxis(ch->xAxis());
		m_hCursors.second->setAxis(ch->xAxis());
		Q_EMIT update();
	});
}

void PlotCursors::updateTracking()
{
	if(m_tracking && m_hCursors.first->handle()->isVisible()) {
		plotMarker1->attach(m_plot->plot());
	} else {
		plotMarker1->detach();
	}

	if(m_tracking && m_hCursors.second->handle()->isVisible()) {
		plotMarker2->attach(m_plot->plot());
	} else {
		plotMarker2->detach();
	}

	displayIntersection();
}

void PlotCursors::setBounded(bool leave)
{
	getV1Cursor()->handle()->setBounded(leave);
	getV2Cursor()->handle()->setBounded(leave);
	getH1Cursor()->handle()->setBounded(leave);
	getH2Cursor()->handle()->setBounded(leave);
}

PlotAxisHandle *PlotCursors::getV1Cursor() { return m_vCursors.first; }

PlotAxisHandle *PlotCursors::getV2Cursor() { return m_vCursors.second; }

PlotAxisHandle *PlotCursors::getH1Cursor() { return m_hCursors.first; }

PlotAxisHandle *PlotCursors::getH2Cursor() { return m_hCursors.second; }

void PlotCursors::setVisible(bool visible)
{
	horizSetVisible(visible);
	vertSetVisible(visible);
}

void PlotCursors::horizSetVisible(bool visible)
{
	m_hCursors.first->handle()->setVisible(visible);
	m_hCursors.second->handle()->setVisible(visible);
	m_hCursors.first->handle()->raise();
	m_hCursors.second->handle()->raise();
	updateTracking();
	Q_EMIT update();
}

void PlotCursors::vertSetVisible(bool visible)
{
	m_vCursors.first->handle()->setVisible(visible && !m_tracking);
	m_vCursors.second->handle()->setVisible(visible && !m_tracking);
	m_vCursors.first->handle()->raise();
	m_vCursors.second->handle()->raise();
	updateTracking();
	Q_EMIT update();
}

void PlotCursors::enableTracking(bool tracking)
{
	m_tracking = tracking;

	m_vCursors.first->handle()->setVisible(!tracking);
	m_vCursors.second->handle()->setVisible(!tracking);
	updateTracking();
	Q_EMIT update();
}

void PlotCursors::displayIntersection()
{
	QwtAxisId yaxis = m_plot->selectedChannel()->yAxis()->axisId();
	QwtAxisId xaxis = m_plot->selectedChannel()->xAxis()->axisId();
	double h1CursorPos = m_hCursors.first->getPosition();
	double h2CursorPos = m_hCursors.second->getPosition();

	plotMarker1->setAxes(xaxis, yaxis);
	plotMarker2->setAxes(xaxis, yaxis);

	plotMarker1->setValue(h1CursorPos, getHorizIntersectionAt(h1CursorPos));
	plotMarker2->setValue(h2CursorPos, getHorizIntersectionAt(h2CursorPos));

	Q_EMIT m_vCursors.first->scalePosChanged(plotMarker1->yValue());
	Q_EMIT m_vCursors.second->scalePosChanged(plotMarker2->yValue());

	m_plot->replot();
}

void PlotCursors::setVHandlePos(HandlePos pos)
{
	m_vCursors.first->handle()->setHandlePos(pos);
	m_vCursors.second->handle()->setHandlePos(pos);
}

void PlotCursors::setHHandlePos(HandlePos pos)
{
	m_hCursors.first->handle()->setHandlePos(pos);
	m_hCursors.second->handle()->setHandlePos(pos);
}

double PlotCursors::getHorizIntersectionAt(double pos)
{
	auto tmp = m_plot->selectedChannel();
	QwtSeriesData<QPointF> *curve_data = tmp->curve()->data();
	int n = curve_data->size();

	if(n == 0) {
		return -1;
	} else {
		double leftTime, rightTime, leftCustom, rightCustom;
		int rightIndex = -1;
		int leftIndex = -1;
		int left = 0;
		int right = n - 1;

		if(curve_data->sample(right).x() < pos || curve_data->sample(left).x() > pos) {
			return -1;
		}

		while(left <= right) {
			int mid = (left + right) / 2;
			double xData = curve_data->sample(mid).x();

			if(xData == pos) {
				if(mid > 0) {
					leftIndex = mid - 1;
					rightIndex = mid;
				}
				break;
			} else if(xData < pos) {
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if((leftIndex == -1 || rightIndex == -1) && left > 0) {
			leftIndex = left - 1;
			rightIndex = left;
		}
		if(leftIndex == -1 || rightIndex == -1) {
			return -1;
		}

		leftTime = curve_data->sample(leftIndex).x();
		rightTime = curve_data->sample(rightIndex).x();

		leftCustom = curve_data->sample(leftIndex).y();
		rightCustom = curve_data->sample(rightIndex).y();

		double value = (rightCustom - leftCustom) / (rightTime - leftTime) * (pos - leftTime) + leftCustom;

		return value;
	}
}

#include "moc_plotcursors.cpp"
