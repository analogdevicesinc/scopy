#include "plotchannel.h"
#include <QPen>
#include "plotwidget.h"
#include "plotaxis.h"

using namespace scopy;

PlotChannel::PlotChannel(QString name, QPen pen, PlotWidget *plot, PlotAxis *xAxis, PlotAxis *yAxis, QObject *parent)
	: QObject(parent),
	m_plotWidget(plot),
	m_plot(m_plotWidget->plot()),
	m_xAxis(xAxis),
	m_yAxis(yAxis),
	m_handle(nullptr)
{
	m_curve = new QwtPlotCurve(name);
	m_curve->setAxes(m_xAxis->axisId(), m_yAxis->axisId());
	m_curve->setStyle( QwtPlotCurve::Lines );
	m_curve->setPen(pen);
	m_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
	m_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );

	m_curve->setItemAttribute(QwtPlotItem::Legend, true);
	symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(pen.color()),
					  QPen(pen.color()), QSize(7,7));
	m_curve->setSymbol(symbol);
	// curvefitter (?)

}

PlotChannel::~PlotChannel()
{
}

QwtPlotCurve *PlotChannel::curve() const
{
	return m_curve;
}

void PlotChannel::setEnabled(bool b)
{
	if(b)
		m_curve->attach(m_plot);
	else
		m_curve->detach();
}

void PlotChannel::enable()
{
	setEnabled(true);
}

void PlotChannel::disable()
{
	setEnabled(false);
}

void PlotChannel::setThickness(int thickness)
{
	QPen pen = m_curve->pen();
	pen.setWidthF(thickness);
	m_curve->setPen(pen);
	m_plot->replot();
}

void PlotChannel::setStyle(int style)
{

	m_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
	m_curve->setCurveAttribute(QwtPlotCurve::Fitted, false);

	switch(style) {
	case PCS_LINES:
		m_curve->setStyle(QwtPlotCurve::Lines);
		break;
	case PCS_DOTS:
		m_curve->setStyle(QwtPlotCurve::Dots);
		break;
	case PCS_STEPS:
		m_curve->setStyle(QwtPlotCurve::Steps);
		break;
	case PCS_STICKS:
		m_curve->setStyle(QwtPlotCurve::Sticks);
		break;
	case PCS_SMOOTH:
		m_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
		m_curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
		m_curve->setStyle(QwtPlotCurve::Lines);
		break;
	default:
		m_curve->setStyle(QwtPlotCurve::Lines);
		break;
	}
	m_plot->replot();
}

PlotAxisHandle *PlotChannel::handle() const
{
	return m_handle;
}

void PlotChannel::setHandle(PlotAxisHandle *newHandle)
{
	m_handle = newHandle;
}

void PlotChannel::raise()
{
	detach();
	attach();
	if(m_handle->handle()) {
		m_handle->handle()->raise();
	}
}

void PlotChannel::attach()
{
	m_curve->attach(m_plot);
}

void PlotChannel::detach()
{
	m_curve->detach();
}

PlotAxis *PlotChannel::yAxis() const
{
	return m_yAxis;
}

PlotAxis *PlotChannel::xAxis() const
{
	return m_xAxis;
}


