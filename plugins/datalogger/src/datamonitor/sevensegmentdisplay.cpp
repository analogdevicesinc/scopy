#include "datamonitor/sevensegmentdisplay.hpp"

#include <QLabel>
#include <QScrollArea>
#include <datamonitorstylehelper.hpp>

using namespace scopy::datamonitor;

SevenSegmentDisplay::SevenSegmentDisplay(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	QWidget *mainContainer = new QWidget(this);
	QVBoxLayout *mainContainerLayout = new QVBoxLayout(this);
	mainContainerLayout->setMargin(0);
	mainContainerLayout->setSpacing(10);
	mainContainer->setLayout(mainContainerLayout);

	QWidget *widgetBody = new QWidget(this);
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	widgetBody->setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(mainContainer);
	mainLayout->addWidget(scrollArea);

	mainContainerLayout->addWidget(widgetBody);
	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	mainContainerLayout->addItem(spacer);

	monitorList = new QMap<QString, SevenSegmentMonitor *>();
}

SevenSegmentDisplay::~SevenSegmentDisplay() {}

void SevenSegmentDisplay::generateSegment(DataMonitorModel *model)
{
	if(!monitorList->contains(model->getName())) {
		SevenSegmentMonitor *monitor = new SevenSegmentMonitor(model, this);
		layout->addWidget(monitor);
		monitorList->insert(model->getName(), monitor);
	}
}

void SevenSegmentDisplay::removeSegment(QString monitor)
{
	if(monitorList->contains(monitor)) {
		delete monitorList->value(monitor);
		monitorList->remove(monitor);
	}
}

void SevenSegmentDisplay::updatePrecision(int precision)
{
	foreach(auto monitor, monitorList->keys()) {
		monitorList->value(monitor)->updatePrecision(precision);
	}
}

void SevenSegmentDisplay::togglePeakHolder(bool toggle)
{
	foreach(auto monitor, monitorList->keys()) {
		monitorList->value(monitor)->togglePeakHolder(toggle);
	}
}
