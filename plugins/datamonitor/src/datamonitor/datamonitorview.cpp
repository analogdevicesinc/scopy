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

	removeBtn = new QPushButton(this);

	HoverWidget *removeHover = new HoverWidget(removeBtn, m_header, this);
	removeHover->setStyleSheet("background-color: transparent; border: 0px;");
	removeHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	removeHover->setContentPos(HoverPosition::HP_TOPLEFT);
	removeHover->setVisible(true);
	removeHover->raise();

	connect(removeBtn, &QPushButton::clicked, this, &DataMonitorView::removeMonitor);

	/////////////////////////plot///////////////////
	m_monitorPlot = new MonitorPlot(this);
	layout->addWidget(m_monitorPlot);

	DataMonitorStyleHelper::DataMonitorViewStyle(this, StyleHelper::getColor("ScopyBlue"));
}

MonitorPlot *DataMonitorView::monitorPlot() const { return m_monitorPlot; }

QString DataMonitorView::getTitle() { return m_title->text(); }

void DataMonitorView::togglePlot(bool toggle) { m_monitorPlot->setVisible(toggle); }

void DataMonitorView::configureMonitor(DataMonitorModel *dataMonitorModel)
{
	setMeasureUnit(dataMonitorModel->getUnitOfMeasure());

	DataMonitorStyleHelper::DataMonitorViewStyle(this, dataMonitorModel->getColor());
}

void DataMonitorView::setMeasureUnit(UnitOfMeasurement *newMeasureUnit)
{
	m_measureUnit = newMeasureUnit;
	m_measuringUnit->setText(m_measureUnit->getNameAndSymbol());
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

bool DataMonitorView::isSelected() const { return m_isSelected; }
