#include "datamonitorsettings.hpp"

#include <QDebug>
#include <QScrollArea>
#include <QTimer>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <plotchannel.h>

using namespace scopy;
using namespace datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS, "DataMonitorSettings")

DataMonitorSettings::DataMonitorSettings(MonitorPlot *m_plot, QWidget *parent)
	: m_plot(m_plot)
	, QWidget{parent}
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

	MenuSectionWidget *removeMonitorSection = new MenuSectionWidget(this);
	layout->addWidget(removeMonitorSection);

	deleteMonitor = new QPushButton("Delete monitor view");
	removeMonitorSection->contentLayout()->addWidget(deleteMonitor);
	connect(deleteMonitor, &QPushButton::clicked, this, &DataMonitorSettings::removeMonitor);

	MenuSectionWidget *togglePlotContainer = new MenuSectionWidget(this);

	plotSwitch = new MenuOnOffSwitch("Toggle Plot ", togglePlotContainer, false);
	plotSwitch->onOffswitch()->setChecked(true);

	togglePlotContainer->contentLayout()->addWidget(plotSwitch);
	layout->addWidget(togglePlotContainer);

	connect(plotSwitch->onOffswitch(), &QAbstractButton::toggled, this, &DataMonitorSettings::togglePlot);

	// XAxis settings
	layout->addWidget(generateXAxisSettings(this));
	// YAxis settings
	layout->addWidget(generateYAxisSettings(this));

	layout->addWidget(createCurveMenu(this));

	DataMonitorStyleHelper::DataMonitorSettingsStyle(this);
}

void DataMonitorSettings::plotToggle(bool toggled) { plotSwitch->onOffswitch()->setChecked(toggled); }

void DataMonitorSettings::addMonitorsList(QMap<QString, DataMonitorModel *> *monitorList)
{

	MenuSectionWidget *selectMonitorsWidgets = new MenuSectionWidget(this);

	layout->addWidget(selectMonitorsWidgets);

	selectMonitorsWidgets->contentLayout()->addWidget(new QLabel("Available monitors: "));

	monitorsGroup = new QButtonGroup(this);

	foreach(QString monitor, monitorList->keys()) {
		addMonitor(monitor, monitorList->value(monitor)->getColor());
	}

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

void DataMonitorSettings::addMonitor(QString monitor, QColor monitorColor)
{
	auto dev = monitor.split(":")[0];

	if(!deviceMap.contains(dev)) {
		generateDeviceSection(dev);
	}

	MenuControlButton *monitorChannel = new MenuControlButton(deviceMap.value(dev));
	deviceMap.value(dev)->contentLayout()->addWidget(monitorChannel);
	monitorChannel->setName(monitor);
	monitorChannel->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	monitorChannel->setOpenMenuChecksThis(true);
	monitorChannel->setColor(monitorColor);
	monitorChannel->button()->setVisible(false);
	monitorChannel->setCheckable(false);

	monitorsGroup->addButton(monitorChannel);

	// apply hover to the buttons based on the color they have
	// ?? hover as option for MenuControlButton
	monitorChannel->setStyleSheet(QString(":hover{ background-color: %1 ; }").arg(monitorColor.name()));

	connect(monitorChannel, &MenuControlButton::clicked, monitorChannel->checkBox(), &QCheckBox::toggle);

	connect(monitorChannel->checkBox(), &QCheckBox::toggled, this,
		[=](bool toggled) { Q_EMIT monitorToggled(toggled, monitor); });
}

void DataMonitorSettings::generateDeviceSection(QString device)
{
	MenuSectionWidget *devMontirosContainer = new MenuSectionWidget(this);

	MenuCollapseSection *devMonitorsSection =
		new MenuCollapseSection(device, MenuCollapseSection::MHCW_NONE, devMontirosContainer);
	devMontirosContainer->contentLayout()->addWidget(devMonitorsSection);
	layout->addWidget(devMontirosContainer);

	devMonitorsSection->header()->setChecked(false);

	deviceMap.insert(device, devMonitorsSection);
}

void DataMonitorSettings::updateTitle(QString title) { header->label()->setText(title); }

void DataMonitorSettings::plotYAxisMinValueUpdate(double value) { m_ymin->setValue(value); }

void DataMonitorSettings::plotYAxisMaxValueUpdate(double value) { m_ymax->setValue(value); }

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
	MenuSectionWidget *yaxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yAxisSection =
		new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxisContainer);

	yaxisContainer->contentLayout()->addWidget(yAxisSection);

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yAxisSection, false);

	QTimer *m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(1000);

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

	m_ymin->setValue(DataMonitorUtils::getAxisDefaultMinValue());

	m_ymax = new PositionSpinButton(
		{
			{"UM", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"YMax", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, yMinMax);

	m_ymax->setValue(DataMonitorUtils::getAxisDefaultMaxValue());

	yMinMaxLayout->addWidget(m_ymin);
	yMinMaxLayout->addWidget(m_ymax);

	yAxisSection->contentLayout()->addWidget(yMinMax);

	connect(m_autoScaleTimer, &QTimer::timeout, this, [=]() {
		m_plot->plotYAxisAutoscale();
		plotYAxisMinValueUpdate(m_plot->getYAxisIntervalMin());
		plotYAxisMaxValueUpdate(m_plot->getYAxisIntervalMax());
	});

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, yMinMax, [=](bool toggled) {
		plotYAxisAutoscale(toggled);
		yMinMax->setEnabled(!toggled);
		if(toggled) {
			m_autoScaleTimer->start();
		} else {
			m_autoScaleTimer->stop();
		}
	});

	connect(m_ymin, &PositionSpinButton::valueChanged, this, [=](double value) {
		m_plot->updateYAxisIntervalMin(value);
		m_ymax->setMinValue(value);
	});
	connect(m_ymax, &PositionSpinButton::valueChanged, this, [=](double value) {
		m_plot->updateYAxisIntervalMax(value);
		m_ymin->setMaxValue(value);
	});

	return yaxisContainer;
}

QWidget *DataMonitorSettings::generateXAxisSettings(QWidget *parent)
{
	MenuSectionWidget *xAxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xAxisSection =
		new MenuCollapseSection("X-AXIS", MenuCollapseSection::MHCW_NONE, xAxisContainer);

	xAxisContainer->contentLayout()->addWidget(xAxisSection);

	MenuOnOffSwitch *realTimeToggle = new MenuOnOffSwitch(tr("Real Time"), xAxisSection, false);
	xAxisSection->contentLayout()->addWidget(realTimeToggle);

	connect(realTimeToggle->onOffswitch(), &QAbstractButton::toggled, m_plot, &MonitorPlot::setIsRealTime);

	// X-MIN-MAX
	QWidget *xMinMax = new QWidget(xAxisSection);
	QHBoxLayout *xMinMaxLayout = new QHBoxLayout(xMinMax);
	xMinMaxLayout->setMargin(0);
	xMinMaxLayout->setSpacing(10);
	xMinMax->setLayout(xMinMaxLayout);

	// TODO replace UM with the unit of measure of main monitor

	m_xmin = new PositionSpinButton(
		{
			{"s", 1},
			{"min", 60},
		},
		"XMin", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, xMinMax);

	m_xmin->setValue(DataMonitorUtils::getAxisDefaultMinValue());

	m_xmax = new PositionSpinButton(
		{
			{"s", 1},
			{"min", 60},
		},
		"XMax", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, xMinMax);

	m_xmax->setValue(DataMonitorUtils::getAxisDefaultMaxValue());

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	xAxisSection->contentLayout()->addWidget(xMinMax);

	connect(m_xmin, &PositionSpinButton::valueChanged, this, [=](double value) {
		m_plot->updateXAxisIntervalMin(value);
		m_xmax->setMinValue(value);
	});
	connect(m_xmax, &PositionSpinButton::valueChanged, this, [=](double value) {
		m_plot->updateXAxisIntervalMax(value);
		m_xmin->setMaxValue(value);
	});

	return xAxisContainer;
}

QWidget *DataMonitorSettings::createCurveMenu(QWidget *parent)
{
	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	QWidget *curveSettings = new QWidget(curve);
	QHBoxLayout *curveSettingsLay = new QHBoxLayout(curveSettings);
	curveSettingsLay->setMargin(0);
	curveSettingsLay->setSpacing(10);
	curveSettings->setLayout(curveSettingsLay);

	MenuCombo *cbThicknessW = new MenuCombo("Thickness", curve);
	auto cbThickness = cbThicknessW->combo();
	cbThickness->addItem("1");
	cbThickness->addItem("2");
	cbThickness->addItem("3");
	cbThickness->addItem("4");
	cbThickness->addItem("5");

	connect(cbThickness, qOverload<int>(&QComboBox::currentIndexChanged), m_plot,
		[=](int idx) { m_plot->changeCurveThickness(cbThickness->itemText(idx).toFloat()); });

	MenuCombo *cbStyleW = new MenuCombo("Style", curve);
	auto cbStyle = cbStyleW->combo();
	cbStyle->addItem("Lines", PlotChannel::PCS_LINES);
	cbStyle->addItem("Dots", PlotChannel::PCS_DOTS);
	cbStyle->addItem("Steps", PlotChannel::PCS_STEPS);
	cbStyle->addItem("Sticks", PlotChannel::PCS_STICKS);
	cbStyle->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	StyleHelper::MenuComboBox(cbStyle, "cbStyle");

	connect(cbStyle, qOverload<int>(&QComboBox::currentIndexChanged), m_plot, &MonitorPlot::changeCurveStyle);

	curveSettingsLay->addWidget(cbThicknessW);
	curveSettingsLay->addWidget(cbStyleW);
	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}
