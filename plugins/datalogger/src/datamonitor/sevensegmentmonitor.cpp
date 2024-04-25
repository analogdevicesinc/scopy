#include "datamonitor/sevensegmentmonitor.hpp"
#include <datamonitorstylehelper.hpp>

using namespace scopy::datamonitor;

SevenSegmentMonitor::SevenSegmentMonitor(DataMonitorModel *model, QWidget *parent)
	: m_model(model)
	, QFrame{parent}
{
	layout = new QHBoxLayout(this);
	layout->setMargin(10);
	layout->setSpacing(10);
	setLayout(layout);

	QVBoxLayout *header = new QVBoxLayout();
	header->setMargin(10);
	header->setSpacing(10);

	name = new QLabel(model->getName(), this);
	unitOfMeasurement = new QLabel(model->getUnitOfMeasure()->getNameAndSymbol(), this);

	header->addWidget(name, Qt::AlignLeft);
	header->addWidget(unitOfMeasurement);
	header->setAlignment(unitOfMeasurement, Qt::AlignLeft);

	lcdNumber = new LcdNumber(this);

	minMaxLayout = new QVBoxLayout();
	minMaxLayout->setMargin(0);
	minMaxLayout->setSpacing(10);

	QHBoxLayout *minLayout = new QHBoxLayout();
	lcdNumberMin = new LcdNumber(this);

	minLabel = new QLabel("MIN:");
	minLayout->addWidget(minLabel, 1);
	minLayout->addWidget(lcdNumberMin, 3);

	QHBoxLayout *maxLayout = new QHBoxLayout();
	lcdNumberMax = new LcdNumber(this);

	maxLabel = new QLabel("MAX:");
	maxLayout->addWidget(maxLabel, 1);
	maxLayout->addWidget(lcdNumberMax, 3);

	connect(model, &DataMonitorModel::valueUpdated, this,
		[=, this](double time, double value) { lcdNumber->display(value); });

	connect(model, &DataMonitorModel::minValueUpdated, this,
		[=, this](double value) { lcdNumberMin->display(value); });

	connect(model, &DataMonitorModel::maxValueUpdated, this,
		[=, this](double value) { lcdNumberMax->display(value); });

	minMaxLayout->addLayout(minLayout);
	minMaxLayout->addLayout(maxLayout);

	layout->addLayout(header, 1);
	layout->addWidget(lcdNumber, 4);
	layout->addLayout(minMaxLayout, 1);

	updatePrecision(DataMonitorUtils::getDefaultPrecision());

	DataMonitorStyleHelper::SevenSegmentMonitorsStyle(this, model->getColor().name());
}

void SevenSegmentMonitor::togglePeakHolder(bool toggle)
{
	minLabel->setVisible(toggle);
	lcdNumberMin->setVisible(toggle);

	maxLabel->setVisible(toggle);
	lcdNumberMax->setVisible(toggle);
}

void SevenSegmentMonitor::updatePrecision(int precision)
{
	lcdNumber->setPrecision(precision);
	lcdNumber->setDigitCount(precision + 4);
	lcdNumber->display(m_model->getLastReadValue().second);

	lcdNumberMin->setPrecision(precision);
	lcdNumberMin->setDigitCount(precision + 4);
	lcdNumberMin->display(m_model->minValue());

	lcdNumberMax->setPrecision(precision);
	lcdNumberMax->setDigitCount(precision + 4);
	lcdNumberMax->display(m_model->maxValue());
}
