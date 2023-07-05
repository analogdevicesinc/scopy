#include "plotaxishandle.h"
#include "plotaxis.h"
#include "symbol.h"
#include "symbol_controller.h"
#include <QDebug>

using namespace scopy;
PlotAxisHandle::PlotAxisHandle(QPen pen, PlotAxis* ax, PlotWidget *p, QObject *parent)
	: QObject(parent),
	m_plotWidget(p),
	m_axis(ax),
	m_pen(pen)

{
	//QColor chnColor = color;
	m_symbolCtrl = p->symbolCtrl();
	/* Channel offset widget */
	m_chOffsetBar = new HorizBar(p->plot());
	m_symbolCtrl->attachSymbol(m_chOffsetBar);

	m_chOffsetBar->setCanLeavePlot(true);
	m_chOffsetBar->setVisible(false);
	m_chOffsetBar->setMobileAxis(ax->axisId());
	m_chOffsetBar->setPen(pen);

	m_chOffsetHdl = new RoundedHandleV(
	    QPixmap(":/gui/icons/handle_right_arrow.svg"),
	    QPixmap(":/gui/icons/handle_up_arrow.svg"),
	    QPixmap(":/gui/icons/handle_down_arrow.svg"),
	    p->leftHandlesArea(), true);

	m_chOffsetHdl->setRoundRectColor(m_pen.color());
	m_chOffsetHdl->setPen(pen);
	m_chOffsetHdl->setVisible(true);

	/* When bar position changes due to plot resizes update the handle */
	connect(p->leftHandlesArea(), &HandlesArea::sizeChanged, m_chOffsetHdl, [=](){
		m_chOffsetHdl->updatePosition();
	});

	connect(m_chOffsetBar, &HorizBar::pixelPositionChanged, this,
		[=](int pos) {
			m_chOffsetHdl->setPositionSilenty(pos);
		});

	connect(m_chOffsetHdl, &RoundedHandleV::positionChanged, this,
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

RoundedHandleV *PlotAxisHandle::offsetHdl() const
{
	return m_chOffsetHdl;
}

PlotAxis *PlotAxisHandle::axis() const
{
	return m_axis;
}
