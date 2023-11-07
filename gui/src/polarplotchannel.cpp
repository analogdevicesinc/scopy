#include "polarplotchannel.h"
#include <polarplotwidget.h>

#include <qwt_symbol.h>

using namespace scopy;

PolarPlotChannel::PolarPlotChannel(QString name, QPen pen, PolarPlotWidget *plot, QObject *parent)
	: QObject(parent)
	, m_plot(plot->plot())
	, m_pen(pen)
{
	m_data = new QwtArraySeriesData<QwtPointPolar>();

	m_curve = new QwtPolarCurve(name);
	m_curve->setStyle(QwtPolarCurve::Lines);
	m_curve->setPen(m_pen);
	m_curve->setRenderHint(QwtPolarItem::RenderAntialiased, true);

	m_curve->setItemAttribute(QwtPolarItem::Legend, true);
	m_symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(m_pen.color()), QPen(m_pen.color()), QSize(7, 7));
	m_curve->setSymbol(m_symbol);

	m_curve->setData(m_data);
}

PolarPlotChannel::~PolarPlotChannel() {}

QwtPolarCurve *PolarPlotChannel::curve() const { return m_curve; }

void PolarPlotChannel::setSamples(QVector<QwtPointPolar> samples) { m_data->setSamples(samples); }

void PolarPlotChannel::raise()
{
	detach();
	attach();
}

void PolarPlotChannel::attach() { m_curve->attach(m_plot); }

void PolarPlotChannel::detach() { m_curve->detach(); }

void PolarPlotChannel::setEnabled(bool b)
{
	if(b) {
		m_curve->attach(m_plot);
	} else {
		m_curve->detach();
	}
}

void PolarPlotChannel::enable() { setEnabled(true); }

void PolarPlotChannel::disable() { setEnabled(false); }

void PolarPlotChannel::setThickness(int thickness)
{
	QPen pen = m_curve->pen();
	pen.setWidthF(thickness);
	m_curve->setPen(pen);
	m_plot->replot();
}
