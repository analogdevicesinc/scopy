#include "menus/datamonitorsettings.hpp"
#include "menus/plottimeaxiscontroller.hpp"

#include <QDateTimeEdit>
#include <QDebug>
#include <QLineEdit>
#include <QScrollArea>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menuplotaxisrangecontrol.h>
#include <menuplotchannelcurvestylecontrol.h>
#include <menusectionwidget.h>
#include <mousewheelwidgetguard.h>
#include <plotautoscaler.h>
#include <plotchannel.h>
#include <timemanager.hpp>

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
	header = new EditTextMenuHeader(title, color, this);
	mainLayout->addWidget(header);

	connect(header->lineEdit(), &QLineEdit::textChanged, this,
		[=, this]() { Q_EMIT titleUpdated(header->lineEdit()->text()); });

	settingsBody = new QWidget(this);
	layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	// XAxis settings
	PlotTimeAxisController *plotTimeAxisController = new PlotTimeAxisController(m_plot, this);
	layout->addWidget(plotTimeAxisController);

	// YAxis settings
	layout->addWidget(generateYAxisSettings(this));

	/// Curve style
	layout->addWidget(generateCurveStyleSettings(this));

	////Plot style
	layout->addWidget(generatePlotUiSettings(this));

	////// 7 segment settings ///////////////////
	sevenSegmentMonitorSettings = new SevenSegmentMonitorSettings(this);
	layout->addWidget(sevenSegmentMonitorSettings);

	/////// data logging /////////////////
	dataLoggingMenu = new DataLoggingMenu(this);
	layout->addWidget(dataLoggingMenu);

	MouseWheelWidgetGuard *mouseWheelWidgetGuard = new MouseWheelWidgetGuard(this);
	mouseWheelWidgetGuard->installEventRecursively(this);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);

	DataMonitorStyleHelper::DataMonitorSettingsStyle(this);
}

void DataMonitorSettings::plotYAxisMinValueUpdate(double value) { m_ymin->setValue(value); }

void DataMonitorSettings::plotYAxisMaxValueUpdate(double value) { m_ymax->setValue(value); }

QWidget *DataMonitorSettings::generateYAxisSettings(QWidget *parent)
{
	MenuSectionWidget *yaxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yAxisSection = new MenuCollapseSection(
		"Y-AXIS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, yaxisContainer);

	yaxisContainer->contentLayout()->addWidget(yAxisSection);

	gui::MenuPlotAxisRangeControl *plotYAxisController =
		new gui::MenuPlotAxisRangeControl(m_plot->plot()->yAxis(), this);

	plotYAxisController->setMin(DataMonitorUtils::getAxisDefaultMinValue());
	plotYAxisController->setMax(DataMonitorUtils::getAxisDefaultMaxValue());

	gui::PlotAutoscaler *plotAutoscaler = new gui::PlotAutoscaler(yAxisSection);
	connect(m_plot, &MonitorPlot::monitorCurveAdded, plotAutoscaler, &gui::PlotAutoscaler::addChannels);
	connect(m_plot, &MonitorPlot::monitorCurveRemoved, plotAutoscaler, &gui::PlotAutoscaler::removeChannels);

	plotAutoscaler->setTolerance(10);

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yAxisSection, false);
	autoscale->onOffswitch()->setChecked(true);

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		plotYAxisController->setEnabled(!toggled);
		if(toggled) {
			plotAutoscaler->start();
		} else {
			plotAutoscaler->stop();
		}
	});

	auto &&timeTracker = TimeManager::GetInstance();

	connect(timeTracker, &TimeManager::toggleRunning, this, [=, this](bool toggled) {
		if(toggled) {
			if(autoscale->onOffswitch()->isChecked()) {
				plotAutoscaler->start();
			}
		} else {
			plotAutoscaler->stop();
		}
	});

	connect(plotAutoscaler, &gui::PlotAutoscaler::newMin, m_plot, &MonitorPlot::updateYAxisIntervalMin);
	connect(plotAutoscaler, &gui::PlotAutoscaler::newMax, m_plot, &MonitorPlot::updateYAxisIntervalMax);

	yAxisSection->contentLayout()->addWidget(autoscale);
	yAxisSection->contentLayout()->addWidget(plotYAxisController);

	return yaxisContainer;
}

QWidget *DataMonitorSettings::generateCurveStyleSettings(QWidget *parent)
{
	MenuSectionWidget *curveStylecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curveStyleSection = new MenuCollapseSection(
		"CURVE", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, curveStylecontainer);

	gui::MenuPlotChannelCurveStyleControl *curveMneu = new gui::MenuPlotChannelCurveStyleControl(curveStyleSection);

	connect(m_plot, &MonitorPlot::monitorCurveAdded, curveMneu,
		&gui::MenuPlotChannelCurveStyleControl::addChannels);
	connect(m_plot, &MonitorPlot::monitorCurveRemoved, curveMneu,
		&gui::MenuPlotChannelCurveStyleControl::removeChannels);

	curveStyleSection->contentLayout()->addWidget(curveMneu);
	curveStylecontainer->contentLayout()->addWidget(curveStyleSection);

	return curveStylecontainer;
}

QWidget *DataMonitorSettings::generatePlotUiSettings(QWidget *parent)
{
	MenuSectionWidget *plotStylecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *plotStyleSection =
		new MenuCollapseSection("PLOT SETTINGS", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MHW_BASEWIDGET, plotStylecontainer);

	plotStyleSection->contentLayout()->setSpacing(10);

	MenuOnOffSwitch *showYAxisLabel = new MenuOnOffSwitch(tr("Y-AXIS label"), plotStyleSection, false);
	showYAxisLabel->onOffswitch()->setChecked(true);

	connect(showYAxisLabel->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->plot()->yAxis()->setVisible(toggled); });

	MenuOnOffSwitch *showXAxisLabel = new MenuOnOffSwitch(tr("X-AXIS label"), plotStyleSection, false);
	showXAxisLabel->onOffswitch()->setChecked(true);

	connect(showXAxisLabel->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->plot()->xAxis()->setVisible(toggled); });

	MenuOnOffSwitch *showBufferPreview = new MenuOnOffSwitch(tr("Buffer Preview"), plotStyleSection, false);
	showBufferPreview->onOffswitch()->setChecked(true);

	connect(showBufferPreview->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool toggled) { m_plot->toggleBufferPreview(toggled); });

	plotStyleSection->contentLayout()->addWidget(showBufferPreview);
	plotStyleSection->contentLayout()->addWidget(showXAxisLabel);
	plotStyleSection->contentLayout()->addWidget(showYAxisLabel);

	plotStylecontainer->contentLayout()->addWidget(plotStyleSection);

	return plotStylecontainer;
}

DataLoggingMenu *DataMonitorSettings::getDataLoggingMenu() const { return dataLoggingMenu; }

SevenSegmentMonitorSettings *DataMonitorSettings::getSevenSegmentMonitorSettings() const
{
	return sevenSegmentMonitorSettings;
}

bool DataMonitorSettings::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::Wheel) {
		return true;
	}

	return QWidget::eventFilter(watched, event);
}

#include "moc_datamonitorsettings.cpp"
