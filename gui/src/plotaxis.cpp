#include "plotaxis.h"

#include <QDebug>
#include <QwtPlotLayout>
#include <qwt_scale_widget.h>
#include <edgelessplot.h>

using namespace scopy;
PlotAxis::PlotAxis(int position, PlotWidget *p, QPen pen, QObject *parent)
	: QObject(parent)
	, m_plotWidget(p)
	, m_plot(p->plot())
	, m_position(position)
	, m_axisId(QwtAxisId(position))
	, m_units("")
{
	m_min = -1;
	m_max = 1;
	m_divs = (isHorizontal()) ? 11.0 : 11.0;

	m_id = m_plotWidget->plotAxis(m_position).count();
	m_axisId = QwtAxisId(m_position, m_id);
	m_plot->setAxesCount(position, m_id + 1);

	updateAxisScale();

	m_formatter = new MetricPrefixFormatter();
	m_formatter->setTrimZeroes(true);
	m_formatter->setTwoDecimalMode(true);
	m_scaleDraw = new BasicScaleDraw(m_formatter, m_units);

	m_scaleDraw->setColor(pen.color());
	m_plot->setAxisScaleDraw(m_axisId, m_scaleDraw);

	m_scaleEngine = new OscScaleEngine();
	m_scaleEngine->setMajorTicksCount(m_divs);
	m_plot->setAxisScaleEngine(m_axisId, (QwtScaleEngine *)m_scaleEngine);

	m_plotWidget->addPlotAxis(this);

	setupAxisScale();
	setVisible(false);

	connect(this, &PlotAxis::minChanged, this, &PlotAxis::updateAxisScale);
	connect(this, &PlotAxis::maxChanged, this, &PlotAxis::updateAxisScale);
	setUnitsVisible(false);
}

PlotAxis::~PlotAxis() {}

void PlotAxis::setUnitsVisible(bool visible) { m_scaleDraw->setUnitsEnabled(visible); }

void PlotAxis::setScaleEn(bool en)
{
	if(en) {
		m_scaleItem->attach(m_plot);
	} else {
		m_scaleItem->detach();
	}
}

void PlotAxis::setupAxisScale()
{
	QwtScaleDraw::Alignment scale = static_cast<QwtScaleDraw::Alignment>(m_position);
	m_scaleItem = new EdgelessPlotScaleItem(scale);

	m_scaleItem->scaleDraw()->setAlignment(scale);
	m_scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_scaleItem->setFont(m_plot->axisWidget(0)->font());

	QPalette palette = m_scaleItem->palette();
	palette.setBrush(QPalette::WindowText, QColor(0x6E6E6F));
	palette.setBrush(QPalette::Text, QColor(0x6E6E6F));
	m_scaleItem->setPalette(palette);
	m_scaleItem->setBorderDistance(0);
	m_scaleItem->setZ(200);
}

int PlotAxis::position() { return m_position; }

bool PlotAxis::isHorizontal() { return (m_position == QwtAxis::XBottom || m_position == QwtAxis::XTop); }

bool PlotAxis::isVertical() { return (m_position == QwtAxis::YLeft || m_position == QwtAxis::YRight); }

double PlotAxis::divs() const { return (m_divs - 1); }

void PlotAxis::setFormatter(PrefixFormatter *formatter)
{
	m_formatter = formatter;
	m_scaleDraw->setFormatter(m_formatter);

	Q_EMIT formatterChanged(formatter);
}

PrefixFormatter *PlotAxis::getFormatter() { return m_formatter; }

void PlotAxis::setUnits(QString units)
{
	m_units = units;
	m_scaleDraw->setUnitType(m_units);

	Q_EMIT unitsChanged(units);
}

QString PlotAxis::getUnits() { return m_units; }

void PlotAxis::setDivs(double divs)
{
	m_divs = divs;
	updateAxisScale();
}

void PlotAxis::setInterval(double min, double max)
{
	setMin(min);
	setMax(max);
	//	updateAxisScale();
}

void PlotAxis::setVisible(bool val) { m_plot->setAxisVisible(m_axisId, val); }

void PlotAxis::updateAxisScale()
{
	m_plot->setAxisScale(m_axisId, m_min, m_max, (m_max - m_min) / m_divs); // set Divs, limits
	m_plot->replot();
	Q_EMIT axisScaleUpdated();
}

void PlotAxis::setMin(double newMin)
{
	if(qFuzzyCompare(m_min, newMin))
		return;
	m_min = newMin;
	emit minChanged(newMin);
}

void PlotAxis::setMax(double newMax)
{
	if(qFuzzyCompare(m_max, newMax))
		return;
	m_max = newMax;
	emit maxChanged(newMax);
}

BasicScaleDraw *PlotAxis::scaleDraw() const { return m_scaleDraw; }

OscScaleEngine *PlotAxis::scaleEngine() const { return m_scaleEngine; }

double PlotAxis::max() const { return m_max; }

double PlotAxis::visibleMin() const { return m_plot->axisScaleDiv(axisId()).lowerBound(); }

double PlotAxis::visibleMax() const { return m_plot->axisScaleDiv(axisId()).upperBound(); }

double PlotAxis::min() const { return m_min; }

const QwtAxisId &PlotAxis::axisId() const { return m_axisId; }

#include "moc_plotaxis.cpp"
