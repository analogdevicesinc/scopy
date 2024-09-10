#include "plotchannel.h"
#include "plotaxis.h"
#include <QPen>
#include <QwtText>

using namespace scopy;

PlotChannel::PlotChannel(QString name, QPen pen, PlotAxis *xAxis, PlotAxis *yAxis, QObject *parent)
	: QObject(parent)
	, m_xAxis(xAxis)
	, m_yAxis(yAxis)
	, m_handle(nullptr)
	, m_pen(pen)
	, m_name(name)
	, m_style(0)
	, m_thickness(1)

{}

void PlotChannel::init()
{
	m_curve = new QwtPlotCurve(m_name);
	m_curve->setAxes(m_xAxis->axisId(), m_yAxis->axisId());
	m_curve->setStyle(QwtPlotCurve::Lines);
	m_curve->setPen(m_pen);
	m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	m_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);

	m_curve->setItemAttribute(QwtPlotItem::Legend, true);
	symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(m_pen.color()), QPen(m_pen.color()), QSize(7, 7));
	m_curve->setSymbol(symbol);
	// curvefitter (?)
}

void PlotChannel::deinit() { delete m_curve; }

PlotChannel::~PlotChannel() {}

QwtPlotCurve *PlotChannel::curve() const { return m_curve; }

void PlotChannel::setEnabled(bool b)
{
	if(b)
		Q_EMIT attachCurve(m_curve);
	else
		m_curve->detach();
}

void PlotChannel::enable() { setEnabled(true); }

void PlotChannel::disable() { setEnabled(false); }

void PlotChannel::setThicknessInternal(int thickness)
{
	QPen pen = m_curve->pen();
	pen.setWidthF(thickness);
	m_curve->setPen(pen);
	Q_EMIT doReplot();
}

void PlotChannel::setStyleInternal(int style)
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
	Q_EMIT doReplot();
}

void PlotChannel::setXAxis(PlotAxis *newXAxis)
{
	m_xAxis = newXAxis;
	curve()->setXAxis(m_xAxis->axisId());
}

void PlotChannel::setYAxis(PlotAxis *newYAxis)
{
	m_yAxis = newYAxis;
	curve()->setYAxis(m_yAxis->axisId());
}

QString PlotChannel::name() const { return m_name; }

QList<QwtPlotMarker *> PlotChannel::markers() { return m_markers; }

QwtPlotMarker *PlotChannel::buildMarker(QString str, QwtSymbol::Style shape, double x, double y)
{
	QwtPlotMarker *m = new QwtPlotMarker();

	QBrush brush;
	brush.setColor(m_pen.color());
	brush.setStyle(Qt::SolidPattern);

	m->setSymbol(new QwtSymbol(shape, brush, m_pen, QSize(12, 12)));
	m->setLinePen(m_pen);
	QwtText txt(str);
	txt.setColor(m_pen.color());
	m->setLabel(txt);
	m->setXAxis(m_xAxis->axisId());
	m->setYAxis(m_yAxis->axisId());

	m->setXValue(x);
	m->setYValue(y);
	Q_EMIT attachCurve(m_curve);
	return m;
}

void PlotChannel::addMarker(QwtPlotMarker *m) { m_markers.append(m); }

void PlotChannel::setSamples(const float *xData, const float *yData, size_t size, bool copy)
{
	if(copy) {
		curve()->setSamples(xData, yData, size);
	} else {
		curve()->setRawSamples(xData, yData, size);
	}
	Q_EMIT newData(xData, yData, size, copy);
}

void PlotChannel::clearMarkers()
{
	for(auto *m : m_markers) {
		m->detach();
		delete m;
	}
	m_markers.clear();
}

void PlotChannel::removeMarker(QwtPlotMarker *m)
{
	m->detach();
	m_markers.removeAll(m);
	delete(m);
}

PlotAxisHandle *PlotChannel::handle() const { return m_handle; }

void PlotChannel::setHandle(PlotAxisHandle *newHandle) { m_handle = newHandle; }

void PlotChannel::raise()
{
	detach();
	attach();
	if(m_handle && m_handle->handle()) {
		m_handle->handle()->raise();
	}
}

void PlotChannel::attach() { Q_EMIT attachCurve(m_curve); }

void PlotChannel::detach() { m_curve->detach(); }

PlotAxis *PlotChannel::yAxis() const { return m_yAxis; }

PlotAxis *PlotChannel::xAxis() const { return m_xAxis; }

int PlotChannel::thickness() const { return m_thickness; }

void PlotChannel::setThickness(int newThickness)
{
	if(m_thickness == newThickness)
		return;
	m_thickness = newThickness;
	setThicknessInternal(newThickness);
	Q_EMIT thicknessChanged();
}

int PlotChannel::style() const { return m_style; }

void PlotChannel::setStyle(int newStyle)
{
	if(m_style == newStyle)
		return;
	m_style = newStyle;
	setStyleInternal(newStyle);
	Q_EMIT styleChanged();
}
#include "moc_plotchannel.cpp"
