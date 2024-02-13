#include "datamonitorview.hpp"

#include <datamonitorstylehelper.hpp>
#include "dynamicWidget.h"

using namespace scopy;
using namespace datamonitor;

DataMonitorView::DataMonitorView(QFrame *parent)
	: QFrame{parent}
{
	installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout(this);
	this->setLayout(layout);

	//////////////////////////header////////////////////////
	m_header = new QWidget(this);
	layout->addWidget(m_header);
	QHBoxLayout *headerLayout = new QHBoxLayout(m_header);
	m_header->setLayout(headerLayout);
	m_measuringUnit = new QLabel("", m_header);
	m_title = new QLineEdit("DataMonitor", m_header);
	connect(m_title, &QLineEdit::textChanged, this, &DataMonitorView::titleChanged);

	headerLayout->addWidget(m_measuringUnit, 1, Qt::AlignLeft);
	headerLayout->addWidget(m_title, 1, Qt::AlignRight);

	/////////////////////////plot///////////////////
	m_monitorPlot = new MonitorPlot(this);
	layout->addWidget(m_monitorPlot);

	//////////////////////last read value
	m_valueWidget = new QWidget(this);
	QHBoxLayout *valueLayout = new QHBoxLayout(m_valueWidget);
	valueLayout->setAlignment(Qt::AlignRight);
	m_valueWidget->setLayout(valueLayout);
	m_value = new LcdNumber();
	valueLayout->addWidget(m_value);
	m_valueUMLabel = new QLabel("");
	valueLayout->addWidget(m_valueUMLabel);
	layout->addWidget(m_valueWidget);

	//////////////////////min value///////////////////
	m_minValueWidget = new QWidget(this);
	QHBoxLayout *minValueLayout = new QHBoxLayout(m_minValueWidget);
	minValueLayout->setAlignment(Qt::AlignRight);
	m_minValueWidget->setLayout(minValueLayout);

	QLabel *m_minValueLabel = new QLabel("MIN");
	minValueLayout->addWidget(m_minValueLabel);
	m_minValue = new LcdNumber();
	minValueLayout->addWidget(m_minValue);
	m_minValueUMLabel = new QLabel("");
	minValueLayout->addWidget(m_minValueUMLabel);

	layout->addWidget(m_minValueWidget);

	//////////////////////max value///////////////////
	m_maxValueWidget = new QWidget(this);
	QHBoxLayout *maxValueLayout = new QHBoxLayout(m_maxValueWidget);
	maxValueLayout->setAlignment(Qt::AlignRight);
	m_maxValueWidget->setLayout(maxValueLayout);

	QLabel *m_maxValueLabel = new QLabel("MAX");
	maxValueLayout->addWidget(m_maxValueLabel);
	m_maxValue = new LcdNumber();
	maxValueLayout->addWidget(m_maxValue);
	m_maxValueUMLabel = new QLabel("");
	maxValueLayout->addWidget(m_maxValueUMLabel);

	layout->addWidget(m_maxValueWidget);

	updatePrecision(3);
	DataMonitorStyleHelper::DataMonitorViewStyle(this, StyleHelper::getColor("ScopyBlue"));
}

MonitorPlot *DataMonitorView::monitorPlot() const { return m_monitorPlot; }

QString DataMonitorView::getTitle() { return m_title->text(); }

void DataMonitorView::updateValue(double time, double value) { m_value->display(value); }

void DataMonitorView::updateMinValue(double value)
{
	if(value == Q_INFINITY) {
		this->m_minValue->display(0);
	} else {
		this->m_minValue->display(value);
	}
}

void DataMonitorView::updateMaxValue(double value)
{
	if(value == -Q_INFINITY) {
		this->m_maxValue->display(0);
	} else {
		this->m_maxValue->display(value);
	}
}

void DataMonitorView::updatePrecision(int precision)
{
	m_value->setPrecision(precision);
	m_value->setDigitCount(4 + precision);
	m_value->display(m_value->value());

	m_minValue->setPrecision(precision);
	m_minValue->setDigitCount(4 + precision);
	m_minValue->display(m_minValue->value());

	m_maxValue->setPrecision(precision);
	m_maxValue->setDigitCount(4 + precision);
	m_maxValue->display(m_maxValue->value());
}

void DataMonitorView::togglePeakHolder(bool toggle)
{
	m_minValueWidget->setVisible(toggle);
	m_maxValueWidget->setVisible(toggle);
}

void DataMonitorView::togglePlot(bool toggle) { m_monitorPlot->setVisible(toggle); }

void DataMonitorView::configureMonitor(DataMonitorModel *dataMonitorModel)
{
	m_mainMonitor = dataMonitorModel->getName();
	setMeasureUnit(dataMonitorModel->getUnitOfMeasure());

	if(m_mainMonitor != "") {
		disconnect(valueUpdateConnection);
		disconnect(minValueUpdateConnection);
		disconnect(maxValueUpdateConnection);

		m_monitorPlot->setMainMonitor(dataMonitorModel->getName());
	}

	valueUpdateConnection =
		connect(dataMonitorModel, &DataMonitorModel::valueUpdated, this, &DataMonitorView::updateValue);

	minValueUpdateConnection =
		connect(dataMonitorModel, &DataMonitorModel::minValueUpdated, this, &DataMonitorView::updateMinValue);

	maxValueUpdateConnection =
		connect(dataMonitorModel, &DataMonitorModel::maxValueUpdated, this, &DataMonitorView::updateMaxValue);

	// if any values are available update current values based on available values
	// when data aquisition is off will still be able to see last read value of the monitor
	if(!dataMonitorModel->getValues()->isEmpty()) {
		updateValue(dataMonitorModel->getValues()->last().first, dataMonitorModel->getValues()->last().second);
		updateMinValue(dataMonitorModel->minValue());
		updateMaxValue(dataMonitorModel->maxValue());
	}

	DataMonitorStyleHelper::DataMonitorViewStyle(this, dataMonitorModel->getColor());
}

void DataMonitorView::setMeasureUnit(UnitOfMeasurement *newMeasureUnit)
{
	m_measureUnit = newMeasureUnit;
	m_measuringUnit->setText(m_measureUnit->getNameAndSymbol());
	m_valueUMLabel->setText(m_measureUnit->getSymbol());
	m_minValueUMLabel->setText(m_measureUnit->getSymbol());
	m_maxValueUMLabel->setText(m_measureUnit->getSymbol());
}

void DataMonitorView::resetView()
{

	disconnect(valueUpdateConnection);
	disconnect(minValueUpdateConnection);
	disconnect(maxValueUpdateConnection);

	m_mainMonitor  = "" ;
	m_monitorPlot->setMainMonitor(m_mainMonitor);

	updateMinValue(Q_INFINITY);
	updateMaxValue(-Q_INFINITY);
	updateValue(0,0);
	DataMonitorStyleHelper::DataMonitorViewStyle(this, StyleHelper::getColor("ScopyBlue"));
}

void DataMonitorView::toggleSelected()
{
	m_isSelected = !m_isSelected;
	scopy::setDynamicProperty(this, "is_selected", m_isSelected);
}

bool DataMonitorView::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress) {
		Q_EMIT widgetClicked();
	}

	return QWidget::eventFilter(watched, event);
}

bool DataMonitorView::isSelected() const
{
	return m_isSelected;
}
