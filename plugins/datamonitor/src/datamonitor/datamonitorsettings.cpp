#include "datamonitorsettings.hpp"

#include <QScrollArea>
#include <datamonitorstylehelper.hpp>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <plotchannel.h>

using namespace scopy;
using namespace datamonitor;

DataMonitorSettings::DataMonitorSettings(QWidget *parent)
	: QWidget{parent}
{
	mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);
}

DataMonitorSettings::~DataMonitorSettings() {}

void DataMonitorSettings::init(QString title, QColor color)
{
	header = new MenuHeaderWidget(title, QPen(color), this);
	mainLayout->addWidget(header);

	settingsBody = new QWidget(this);
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	MenuSectionWidget *plotSection = new MenuSectionWidget(this);
	layout->addWidget(plotSection);

	plotSwitch = new MenuOnOffSwitch("Plot", plotSection, false);
	plotSwitch->onOffswitch()->setChecked(true);
	plotSection->contentLayout()->addWidget(plotSwitch);

	plotSize = new MenuComboWidget("Plot Size (s)", plotSection);
	plotSize->combo()->addItem(QString("1"));
	plotSize->combo()->addItem(QString("10"));
	plotSize->combo()->addItem(QString("60"));
	plotSize->combo()->setCurrentIndex(1);

	connect(plotSize->combo(), &QComboBox::currentTextChanged, this,
		[=](QString newTimeValue) { Q_EMIT changeTimePeriod(newTimeValue.toInt()); });

	plotSection->contentLayout()->addWidget(plotSize);

	plotStyle = new MenuComboWidget("Plot Style", plotSection);
	plotStyle->combo()->addItem("Lines", PlotChannel::PCS_LINES);
	plotStyle->combo()->addItem("Dots", PlotChannel::PCS_DOTS);
	plotStyle->combo()->addItem("Steps", PlotChannel::PCS_STEPS);
	plotStyle->combo()->addItem("Sticks", PlotChannel::PCS_STICKS);
	plotStyle->combo()->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	plotStyle->combo()->setCurrentIndex(0);

	plotSection->contentLayout()->addWidget(plotStyle);

	MenuSectionWidget *peakHolderSection = new MenuSectionWidget(this);
	peakHolderSection->contentLayout()->setSpacing(10);
	layout->addWidget(peakHolderSection);

	peakHolderSwitch = new MenuOnOffSwitch("Peak Holder ", peakHolderSection, false);
	peakHolderSwitch->onOffswitch()->setChecked(true);
	peakHolderReset = new QPushButton("Reset", peakHolderSection);

	peakHolderSection->contentLayout()->addWidget(peakHolderSwitch);
	peakHolderSection->contentLayout()->addWidget(peakHolderReset);

	connect(plotSwitch->onOffswitch(), &QAbstractButton::toggled, this, &DataMonitorSettings::togglePlot);
	connect(peakHolderReset, &QPushButton::clicked, this, &DataMonitorSettings::resetPeakHolder);
	connect(peakHolderSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		&DataMonitorSettings::togglePeakHolder);
	connect(plotSize->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&DataMonitorSettings::plotSizeIndexChanged);
	connect(plotStyle->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&DataMonitorSettings::changeLineStyle);

	MenuSectionWidget *removeMonitorSection = new MenuSectionWidget(this);
	layout->addWidget(removeMonitorSection);

	deleteMonitor = new QPushButton("Remove monitor");
	removeMonitorSection->contentLayout()->addWidget(deleteMonitor);
	connect(deleteMonitor, &QPushButton::clicked, this, &DataMonitorSettings::removeMonitor);

	DataMonitorStyleHelper::DataMonitorSettingsStyle(this);
}

void DataMonitorSettings::peakHolderToggle(bool toggled) { peakHolderSwitch->onOffswitch()->setChecked(toggled); }

void DataMonitorSettings::plotToggle(bool toggled) { plotSwitch->onOffswitch()->setChecked(toggled); }

void DataMonitorSettings::peakHolderResetClicked() { peakHolderReset->click(); }

void DataMonitorSettings::changeLineStyle(int index) { Q_EMIT lineStyleIndexChanged(index); }

void DataMonitorSettings::addMonitorsList(QList<QString> monitorList)
{
	monitorsCheckboxList = new QList<QPair<QString, QCheckBox *>>();
	activeMonitors = new QList<QString>;

	MenuSectionWidget *selectMonitorsWidgets = new MenuSectionWidget(this);

	mainMonitorCombo = new MenuComboWidget("Main Monitor", selectMonitorsWidgets);
	selectMonitorsWidgets->contentLayout()->addWidget(mainMonitorCombo);

	layout->addWidget(selectMonitorsWidgets);

	selectMonitorsWidgets->contentLayout()->addWidget(new QLabel("Toggle monitors: "));

	for(int i = 0; i < monitorList.length(); i++) {
		addMonitor(monitorList.at(i));
	}

	mainMonitorCombo->combo()->setCurrentIndex(0);

	connect(mainMonitorCombo->combo(), &QComboBox::currentTextChanged, this,
		&DataMonitorSettings::mainMonitorChanged);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

void DataMonitorSettings::addMonitor(QString monitor)
{
	auto dev = monitor.split(":")[0];

	if(!deviceMap.contains(dev)) {
		// TODO move to function
		CollapsableMenuControlButton *devMontirosCtrl = new CollapsableMenuControlButton(this);
		devMontirosCtrl->getControlBtn()->setName(dev);
		devMontirosCtrl->getControlBtn()->setOpenMenuChecksThis(true);
		devMontirosCtrl->getControlBtn()->setDoubleClickToOpenMenu(true);
		devMontirosCtrl->getControlBtn()->button()->setVisible(false);
		devMontirosCtrl->getControlBtn()->checkBox()->setChecked(false);

		layout->addWidget(devMontirosCtrl);

		MenuSectionWidget *activeMonitorsWidget = new MenuSectionWidget(devMontirosCtrl);

		activeMonitorsWidget->setVisible(false);
		activeMonitorsWidget->contentLayout()->setSpacing(4);

		connect(devMontirosCtrl->getControlBtn()->checkBox(), &QCheckBox::toggled, activeMonitorsWidget,
			&QWidget::setVisible);

		devMontirosCtrl->add(activeMonitorsWidget);

		deviceMap.insert(dev, activeMonitorsWidget);
	}

	QCheckBox *select = new QCheckBox(deviceMap.value(dev));
	deviceMap.value(dev)->contentLayout()->addWidget(select);
	select->setText(monitor);

	monitorsCheckboxList->push_back(qMakePair(monitor, select));

	connect(select, &QCheckBox::toggled, this, [=](bool toggled) {
		if(toggled) {
			activeMonitors->append(monitor);
		} else {
			activeMonitors->removeAt(activeMonitors->indexOf(monitor));
		}
		if(mainMonitorCombo->combo()->count() > 0) {
			mainMonitorCombo->combo()->blockSignals(true);
		}

		mainMonitorCombo->combo()->clear();
		mainMonitorCombo->combo()->addItems(*activeMonitors);
		mainMonitorCombo->combo()->blockSignals(false);

		Q_EMIT monitorToggled(toggled, QString(monitor));
	});
}

void DataMonitorSettings::updateTitle(QString title) { header->label()->setText(title); }

void DataMonitorSettings::updateMainMonitor(QString monitorTitle)
{
	mainMonitorCombo->combo()->setCurrentText(monitorTitle);
}

QList<QString> *DataMonitorSettings::getActiveMonitors() { return activeMonitors; }

Qt::PenStyle DataMonitorSettings::lineStyleFromIdx(int idx)
{
	switch(idx) {
	case 0:
		return Qt::PenStyle::SolidLine;
	case 1:
		return Qt::PenStyle::DotLine;
	case 2:
		return Qt::PenStyle::DashLine;
	case 3:
		return Qt::PenStyle::DashDotLine;
	case 4:
		return Qt::PenStyle::DashDotDotLine;
	}
	return Qt::PenStyle::SolidLine;
}
