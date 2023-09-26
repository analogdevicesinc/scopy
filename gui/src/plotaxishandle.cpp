#include "plotaxishandle.h"
#include "plotaxis.h"
#include "symbol.h"
#include "symbol_controller.h"
#include <QDebug>

using namespace scopy;
PlotAxisHandle::PlotAxisHandle(QPen pen, PlotAxis* ax, PlotWidget *p, int position, QObject *parent)
	: QObject(parent),
	m_plotWidget(p),
	m_axis(ax),
	m_pen(pen)
{
	VertHandlesArea *area;
	bool left = false;
	if(position == QwtAxis::YLeft) {
		area = p->leftHandlesArea();
		left = true;
	} else {
		area = p->rightHandlesArea();
	}

	//QColor chnColor = color;
	m_symbolCtrl = p->symbolCtrl();
	/* Channel offset widget */
	m_chOffsetBar = new HorizBar(p->plot());
	m_symbolCtrl->attachSymbol(m_chOffsetBar);

	m_chOffsetBar->setCanLeavePlot(true);
	m_chOffsetBar->setVisible(false);
	m_chOffsetBar->setMobileAxis(ax->axisId());
	m_chOffsetBar->setPen(pen);

	m_handle = new RoundedHandleV(
	    (left) ? QPixmap(":/gui/icons/handle_right_arrow.svg")
		   : QPixmap(":/gui/icons/handle_left_arrow.svg"),
	    QPixmap(":/gui/icons/handle_up_arrow.svg"),
	    QPixmap(":/gui/icons/handle_down_arrow.svg"),
	    area, left);

	m_handle->setRoundRectColor(m_pen.color());
	m_handle->setPen(pen);
	m_handle->setVisible(true);

	/* When bar position changes due to plot resizes update the handle */
	connect(area, &HandlesArea::sizeChanged, m_handle, [=](){
		m_handle->updatePosition();
	});

	connect(m_chOffsetBar, &HorizBar::pixelPositionChanged, this,
		[=](int pos) {
			m_handle->setPositionSilenty(pos);
		});

	connect(m_handle, &RoundedHandleV::positionChanged, this,
		[=](int pos) {
			QwtScaleMap yMap = p->plot()->canvasMap(ax->axisId());
			double offset = yMap.invTransform(pos);			
			double min = ax->min() - offset;
			double max = ax->max() - offset;
			ax->setInterval(min, max);
			p->replot();
		}
		);

}

PlotAxisHandle::~PlotAxisHandle()
{	
}

RoundedHandleV *PlotAxisHandle::handle() const
{
	return m_handle;
}

PlotAxis *PlotAxisHandle::axis() const
{
	return m_axis;
}
