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


