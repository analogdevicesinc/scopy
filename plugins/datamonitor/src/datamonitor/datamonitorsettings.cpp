#include "datamonitorsettings.hpp"

#include <QDebug>
#include <QScrollArea>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <plotchannel.h>

using namespace scopy;
using namespace datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS, "DataMonitorSettings")

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

	// YAxis settings
	QWidget *yaxismenu = generateYAxisSettings(this);
	layout->addWidget(yaxismenu);

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

void DataMonitorSettings::changeLineStyle(int index)
{
	Q_EMIT lineStyleIndexChanged(mainMonitorCombo->combo()->currentText(), index);
}

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

void DataMonitorSettings::updateMainMonitor(QString monitorName)
{
	mainMonitorCombo->combo()->setCurrentText(monitorName);
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

QWidget *DataMonitorSettings::generateYAxisSettings(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yAxisSection =
		new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	yaxiscontainer->contentLayout()->addWidget(yAxisSection);

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yAxisSection, false);

	yAxisSection->contentLayout()->addWidget(autoscale);

	// Y-MIN-MAX
	QWidget *yMinMax = new QWidget(yAxisSection);
	QHBoxLayout *yMinMaxLayout = new QHBoxLayout(yMinMax);
	yMinMaxLayout->setMargin(0);
	yMinMaxLayout->setSpacing(10);
	yMinMax->setLayout(yMinMaxLayout);

	// TODO replace UM with the unit of measure of main monitor

	m_ymin = new PositionSpinButton(
		{
			{"UM", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"YMin", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, yMinMax);

	m_ymin->setValue(DataMonitorUtils::getYAxisDefaultMinValue());

	m_ymax = new PositionSpinButton(
		{
			{"UM", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"YMax", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, yMinMax);

	m_ymax->setValue(DataMonitorUtils::getYAxisDefaultMaxValue());

	yMinMaxLayout->addWidget(m_ymin);
	yMinMaxLayout->addWidget(m_ymax);

	yAxisSection->contentLayout()->addWidget(yMinMax);

	MenuComboWidget *plotStyle = new MenuComboWidget("Curve Style", yAxisSection);
	plotStyle->combo()->addItem("Lines", PlotChannel::PCS_LINES);
	plotStyle->combo()->addItem("Dots", PlotChannel::PCS_DOTS);
	plotStyle->combo()->addItem("Steps", PlotChannel::PCS_STEPS);
	plotStyle->combo()->addItem("Sticks", PlotChannel::PCS_STICKS);
	plotStyle->combo()->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	plotStyle->combo()->setCurrentIndex(0);

	yAxisSection->contentLayout()->addWidget(plotStyle);

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, yMinMax, [=](bool toggled) {
		plotYAxisAutoscaleToggled(mainMonitorCombo->combo()->currentText(), toggled);
		yMinMax->setEnabled(!toggled);
	});

	connect(m_ymin, &PositionSpinButton::valueChanged, this, [=](double value) {
		Q_EMIT plotYAxisMinValueChange(mainMonitorCombo->combo()->currentText(), value);
		m_ymax->setMinValue(value);
	});
	connect(m_ymax, &PositionSpinButton::valueChanged, this, [=](double value) {
		Q_EMIT plotYAxisMaxValueChange(mainMonitorCombo->combo()->currentText(), value);
		m_ymin->setMaxValue(value);
	});

	connect(plotStyle->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&DataMonitorSettings::changeLineStyle);

	// create value checked for min and max so min < max and max > min

	return yaxiscontainer;
}
