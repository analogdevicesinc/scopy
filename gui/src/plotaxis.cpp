#include "plotaxis.h"
#include <QwtPlotLayout>
#include <QDebug>

using namespace scopy;
PlotAxis::PlotAxis(int position, PlotWidget *p, QPen pen,QObject *parent) :
      QObject(parent), m_plotWidget(p), m_plot(p->plot()), m_position(position), m_axisId(QwtAxisId(position))
{
	m_min = -1;
	m_max = 1;
	m_divs = (isHorizontal()) ? 11.0 : 11.0;

	m_zoomer = nullptr;
	m_id = m_plotWidget->plotAxis(m_position).count();
	m_axisId = QwtAxisId(m_position, m_id);
	m_plot->setAxesCount(position, m_id + 1);

	updateAxisScale();

	// move this outside (?)
	if(isHorizontal()) {
		auto prefixFormatter = new MetricPrefixFormatter();
		m_scaleDraw = new OscScaleDraw(prefixFormatter,"s");
	}
	else  {
		auto prefixFormatter = new MetricPrefixFormatter();
		prefixFormatter->setTrimZeroes(true);
		prefixFormatter->setTwoDecimalMode(true);
		m_scaleDraw = new OscScaleDraw(prefixFormatter,"");
	}

	m_scaleDraw->setColor(pen.color());
	m_plot->setAxisScaleDraw(m_axisId,m_scaleDraw);

	m_scaleEngine = new OscScaleEngine();
	m_scaleEngine->setMajorTicksCount(m_divs);
	m_plot->setAxisScaleEngine(m_axisId, (QwtScaleEngine *)m_scaleEngine);

	m_plotWidget->addPlotAxis(this);

	setupAxisScale();
	if(isVertical()) {
		setupZoomer();
	}
	setVisible(false);

	connect(this, &PlotAxis::minChanged, this, &PlotAxis::updateAxisScale);
	connect(this, &PlotAxis::maxChanged, this, &PlotAxis::updateAxisScale);
}

void PlotAxis::setupZoomer() {
	// zoomer
	// OscPlotZoomer - need constructor -

//	m_zoomer = new QwtPlotZoomer(m_plotWidget->xAxis()->axisId(), m_axisId, m_plot->canvas(), false);

//	m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
//							  Qt::RightButton, Qt::ControlModifier);
//	m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
//							  Qt::RightButton);

//	m_zoomer->setZoomBase(false);
//	m_zoomer->setEnabled(true);

//	connect(m_zoomer,&QwtPlotZoomer::zoomed,this, [=](const QRectF &rect ) {
//		qInfo()<< rect<< m_zoomer->zoomRectIndex() << "ZoomOut";
//		m_scaleDraw->invalidateCache();
//	});  - zoom out doesn't work correctly


}

void PlotAxis::setupAxisScale() {
	QwtScaleDraw::Alignment scale =
		static_cast<QwtScaleDraw::Alignment>(m_position);
	auto scaleItem = new EdgelessPlotScaleItem(scale);

	scaleItem->scaleDraw()->setAlignment(scale);
	scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	scaleItem->setFont(m_plot->axisWidget(0)->font());

	QPalette palette = scaleItem->palette();
	palette.setBrush(QPalette::WindowText, QColor(0x6E6E6F));
	palette.setBrush(QPalette::Text, QColor(0x6E6E6F));
	scaleItem->setPalette(palette);
	scaleItem->setBorderDistance(0);
	scaleItem->attach(m_plot);
	scaleItem->setZ(200);
}

int PlotAxis::position()
{
	return m_position;
}

bool PlotAxis::isHorizontal()
{
	return (m_position == QwtAxis::XBottom || m_position == QwtAxis::XTop);
}

bool PlotAxis::isVertical()
{
	return (m_position == QwtAxis::YLeft || m_position == QwtAxis::YRight);
}

double PlotAxis::divs() const
{
	return (m_divs-1);
}

void PlotAxis::setDivs(double divs) {
	m_divs = divs;
	updateAxisScale();

}

void PlotAxis::setInterval(double min, double max) {
	setMin(min);
	setMax(max);
//	updateAxisScale();
}

void PlotAxis::setVisible(bool val)
{
	m_plot->setAxisVisible(m_axisId, val);
}

void PlotAxis::updateAxisScale() {
	m_plot->setAxisScale(m_axisId, m_min, m_max, (m_max - m_min)/m_divs); // set Divs, limits
	m_plot->replot();
}

void PlotAxis::setMin(double newMin)
{
	if (qFuzzyCompare(m_min, newMin))
		return;
	m_min = newMin;
	emit minChanged();
}

void PlotAxis::setMax(double newMax)
{
	if (qFuzzyCompare(m_max, newMax))
		return;
	m_max = newMax;
	emit maxChanged();
}

OscScaleDraw *PlotAxis::scaleDraw() const
{
	return m_scaleDraw;
}

OscScaleEngine *PlotAxis::scaleEngine() const
{
	return m_scaleEngine;
}

QwtPlotZoomer *PlotAxis::zoomer() const
{
	return m_zoomer;
}

double PlotAxis::max() const
{
	return m_max;
}

double PlotAxis::min() const
{
	return m_min;
}

const QwtAxisId &PlotAxis::axisId() const
{
	return m_axisId;
}
