#include "datamonitorsettings.hpp"
#include "plottimeaxiscontroller.hpp"

#include <QDateTimeEdit>
#include <QDebug>
#include <QFileDialog>
#include <QLineEdit>
#include <QScrollArea>
#include <QTimer>
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

	// XAxis settings
	PlotTimeAxisController *plotTimeAxisController = new PlotTimeAxisController(m_plot, this);
	layout->addWidget(plotTimeAxisController);

	// YAxis settings
	layout->addWidget(generateYAxisSettings(this));

	/// Curve style
	layout->addWidget(generateCurveStyleSettings(this));

	////// 7 segment settings ///////////////////
	sevenSegmentMonitorSettings = new SevenSegmentMonitorSettings(this);
	layout->addWidget(sevenSegmentMonitorSettings);

	/////// data logging /////////////////
	MenuSectionWidget *logDataContainer = new MenuSectionWidget(this);
	MenuCollapseSection *logDataSection =
		new MenuCollapseSection("Log data", MenuCollapseSection::MHCW_NONE, logDataContainer);
	logDataSection->contentLayout()->setSpacing(10);

	dataLoggingFilePath = new QLineEdit(logDataSection);
	dataLoggingFilePath->setReadOnly(true);

	connect(dataLoggingFilePath, &QLineEdit::textChanged, this, [=, this](QString path) {
		if(filename.isEmpty() && dataLoggingFilePath->isEnabled()) {
			dataLoggingFilePath->setText(tr("No file selected"));
			dataLoggingFilePath->setStyleSheet("color:red");
		} else {
			dataLoggingFilePath->setStyleSheet("color:white");
			Q_EMIT pathChanged(path);
		}
	});

	dataLoggingBrowseBtn = new QPushButton("Browse", logDataSection);
	connect(dataLoggingBrowseBtn, &QPushButton::clicked, this, &DataMonitorSettings::chooseFile);

	dataLoggingBtn = new QPushButton("Log data", logDataSection);
	connect(dataLoggingBtn, &QPushButton::clicked, this,
		[=, this]() { Q_EMIT requestDataLogging(dataLoggingFilePath->text()); });

	logDataSection->contentLayout()->addWidget(new QLabel("Choose file"));
	logDataSection->contentLayout()->addWidget(dataLoggingFilePath);
	logDataSection->contentLayout()->addWidget(dataLoggingBrowseBtn);
	logDataSection->contentLayout()->addWidget(dataLoggingBtn);

	logDataContainer->contentLayout()->addWidget(logDataSection);
	layout->addWidget(logDataContainer);

	MouseWheelWidgetGuard *mouseWheelWidgetGuard = new MouseWheelWidgetGuard(this);
	mouseWheelWidgetGuard->installEventRecursively(this);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);

	DataMonitorStyleHelper::DataMonitorSettingsStyle(this);
}

void DataMonitorSettings::updateTitle(QString title) { header->label()->setText(title); }

void DataMonitorSettings::plotYAxisMinValueUpdate(double value) { m_ymin->setValue(value); }

void DataMonitorSettings::plotYAxisMaxValueUpdate(double value) { m_ymax->setValue(value); }

QWidget *DataMonitorSettings::generateYAxisSettings(QWidget *parent)
{
	MenuSectionWidget *yaxisContainer = new MenuSectionWidget(this);
	MenuCollapseSection *yAxisSection =
		new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxisContainer);

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

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		plotYAxisController->setEnabled(!toggled);
		if(toggled) {
			plotAutoscaler->start();
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
	MenuCollapseSection *curveStyleSection =
		new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curveStylecontainer);

	gui::MenuPlotChannelCurveStyleControl *curveMneu = new gui::MenuPlotChannelCurveStyleControl(curveStyleSection);

	connect(m_plot, &MonitorPlot::monitorCurveAdded, curveMneu,
		&gui::MenuPlotChannelCurveStyleControl::addChannels);
	connect(m_plot, &MonitorPlot::monitorCurveRemoved, curveMneu,
		&gui::MenuPlotChannelCurveStyleControl::removeChannels);

	curveStyleSection->contentLayout()->addWidget(curveMneu);
	curveStylecontainer->contentLayout()->addWidget(curveStyleSection);

	return curveStylecontainer;
}

void DataMonitorSettings::chooseFile()
{
	QString selectedFilter;
	filename = QFileDialog::getSaveFileName(
		this, tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"), &selectedFilter,
		QFileDialog::Options(QFileDialog::DontUseNativeDialog));
	dataLoggingFilePath->setText(filename);
}

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
