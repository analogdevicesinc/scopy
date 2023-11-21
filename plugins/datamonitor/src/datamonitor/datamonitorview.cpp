#include "datamonitorview.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorView::DataMonitorView(DataMonitorModel *dataMonitorModel, QWidget *parent)
	: QFrame{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	header = new QWidget(this);
	layout->addWidget(header);
	QHBoxLayout *headerLayout = new QHBoxLayout(header);
	header->setLayout(headerLayout);

	measuringUnit = new QLabel(dataMonitorModel->getUnitOfMeasure()->getNameAndSymbol(), header);
	title = new QLabel(dataMonitorModel->getTitle(), header);

	headerLayout->addWidget(measuringUnit, 1, Qt::AlignLeft);
	headerLayout->addWidget(title, 1, Qt::AlignRight);

	m_plot = new PlotWidget(this);
	layout->addWidget(m_plot);

	QPen ch1pen = QPen(dataMonitorModel->getColor(), 1);
	plotch = new PlotChannel(dataMonitorModel->getTitle(), ch1pen, m_plot, m_plot->xAxis(), m_plot->yAxis(), this);
	plotch->setHandle(new PlotAxisHandle(ch1pen, m_plot->yAxis(), m_plot, QwtAxis::YLeft, this));
	m_plot->addPlotAxisHandle(plotch->handle());

	m_plot->addPlotChannel(plotch);

	valueWidget = new QWidget(this);
	QHBoxLayout *valueLayout = new QHBoxLayout(valueWidget);
	valueWidget->setLayout(valueLayout);

	value = new LcdNumber();
	valueLayout->addWidget(value);
	QLabel *valueUMLabel = new QLabel(dataMonitorModel->getUnitOfMeasure()->getSymbol());
	valueLayout->addWidget(valueUMLabel);
	layout->addWidget(valueWidget);

	minValueWidget = new QWidget(this);
	QHBoxLayout *minValueLayout = new QHBoxLayout(minValueWidget);
	minValueWidget->setLayout(minValueLayout);

	QLabel *minValueLabel = new QLabel("MIN");
	minValueLayout->addWidget(minValueLabel);
	minValue = new LcdNumber();
	minValueLayout->addWidget(minValue);
	QLabel *minValueUMLabel = new QLabel(dataMonitorModel->getUnitOfMeasure()->getSymbol());
	minValueLayout->addWidget(minValueUMLabel);

	layout->addWidget(minValueWidget);

	maxValueWidget = new QWidget(this);
	QHBoxLayout *maxValueLayout = new QHBoxLayout(maxValueWidget);
	maxValueWidget->setLayout(maxValueLayout);

	QLabel *maxValueLabel = new QLabel("MAX");
	maxValueLayout->addWidget(maxValueLabel);
	maxValue = new LcdNumber();
	maxValueLayout->addWidget(maxValue);
	QLabel *maxValueUMLabel = new QLabel(dataMonitorModel->getUnitOfMeasure()->getSymbol());
	maxValueLayout->addWidget(maxValueUMLabel);

	layout->addWidget(maxValueWidget);

	updatePrecision(3);

	m_timeInterval = 1;
	m_sampleRate = 1;
	m_numSamples = 1;
}

PlotWidget *DataMonitorView::getPlot() const { return m_plot; }

QWidget *DataMonitorView::getHeader() const { return header; }

QWidget *DataMonitorView::getValueWidget() const { return value; }

QWidget *DataMonitorView::getMinValueWidget() const { return minValueWidget; }

QWidget *DataMonitorView::getMaxValueWidget() const { return maxValueWidget; }

void DataMonitorView::updateValue(double value) { this->value->display(value); }

void DataMonitorView::updateMinValue(double value)
{
	if(value == Q_INFINITY) {
		this->minValue->display(0);
	} else {
		this->minValue->display(value);
	}
}

void DataMonitorView::updateMaxValue(double value)
{
	if(value == -Q_INFINITY) {
		this->maxValue->display(0);
	} else {
		this->maxValue->display(value);
	}
}

void DataMonitorView::updatePrecision(int precision)
{
	value->setPrecision(precision);
	value->setDigitCount(4 + precision);
	value->display(value->value());

	minValue->setPrecision(precision);
	minValue->setDigitCount(4 + precision);
	minValue->display(minValue->value());

	maxValue->setPrecision(precision);
	maxValue->setDigitCount(4 + precision);
	maxValue->display(maxValue->value());
}

PlotChannel *DataMonitorView::getPlotch() const { return plotch; }

double DataMonitorView::getSampleRate() const { return m_sampleRate; }

void DataMonitorView::setSampleRate(double newSampleRate)
{
	m_sampleRate = newSampleRate;

	xdata.clear();

	xdata.reserve(newSampleRate + 2);

	plotch->yAxis()->setInterval(0, newSampleRate);

	plotMaxX = newSampleRate;

	plotch->curve()->setRawSamples(xdata.data(), ydata.data(), xdata.size());
	m_plot->replot();

	//	setNumSamples(m_sampleRate * m_timeInterval);
}

unsigned int DataMonitorView::getNumSamples() const { return m_numSamples; }

void DataMonitorView::setNumSamples(unsigned int newNumSamples)
{
	m_numSamples = newNumSamples;
	xdata.clear();
	ydata.clear();

	xdata.reserve(m_numSamples + 1);
	ydata.reserve(m_numSamples + 1);

	plotch->yAxis()->setInterval(0, m_numSamples);

	plotMaxX = m_numSamples;

	plotch->curve()->setRawSamples(xdata.data(), ydata.data(), xdata.size());
	m_plot->replot();
}

double DataMonitorView::getInterval() const { return m_timeInterval; }

void DataMonitorView::setInterval(double newInterval)
{
	m_timeInterval = newInterval;

	ydata.clear();

	plotch->xAxis()->setInterval(0, newInterval);
	plotMaxY = newInterval;
	ydata.reserve(newInterval + 2);

	plotch->curve()->setRawSamples(xdata.data(), ydata.data(), xdata.size());
	m_plot->replot();
	// setNumSamples(m_sampleRate * m_timeInterval);
}

void DataMonitorView::plotData(double newData) { xdata.push_back(newData); }

void DataMonitorView::plotData(double xValue, double yValue)
{
	xdata.push_back(xValue);
	ydata.push_back(yValue);

	if(yValue >= plotMaxY) {

		double yMin = ydata.takeFirst();
		plotMaxY = yMin + getInterval();
		plotch->yAxis()->setInterval(yMin, plotMaxY);
	}

	if(xValue >= plotMaxX) {
		double xMin = xdata.takeFirst();
		plotMaxX = xMin + getSampleRate();
		plotch->xAxis()->setInterval(xMin, plotMaxX);
	}

	plotch->curve()->setRawSamples(xdata.data(), ydata.data(), xdata.size());

	m_plot->replot();

	updateValue(yValue);
}

void DataMonitorView::togglePeakHolder(bool toggle)
{
	getMinValueWidget()->setVisible(toggle);
	getMaxValueWidget()->setVisible(toggle);
}

void DataMonitorView::togglePlot(bool toggle) { getPlot()->setVisible(toggle); }

void DataMonitorView::updateCurveStyle(Qt::PenStyle lineStyle)
{
	QPen pen(plotch->curve()->pen());
	pen.setStyle(lineStyle);
	plotch->curve()->setPen(QPen(pen));
	m_plot->replot();
}
