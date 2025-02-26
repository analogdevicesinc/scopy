/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "harmoniccalibration.h"
#include "qtconcurrentrun.h"
#include "style.h"
#include "style_properties.h"

#include <widgets/horizontalspinbox.h>
#include <stylehelper.h>
#include <admtstylehelper.h>

using namespace scopy;
using namespace scopy::admt;

static int acquisitionUITimerRate = 500; // In ms
static int acquisitionSampleRate = 20; 
static int acquisitionGraphSampleRate = 100;
static int motorWaitVelocityReachedSampleRate = 50;

static int calibrationUITimerRate = 500;
static int utilityUITimerRate = 1000;

static int continuousCalibrationSampleRate = 10000000; // In nanoseconds
static int deviceStatusMonitorRate = 500;
static int motorPositionMonitorRate = 500;

static int bufferSize = 1;

static int cycleCount = 11;
static int samplesPerCycle = 256;
static int totalSamplesCount = cycleCount * samplesPerCycle;
static bool isStartMotor = false;
static bool isPostCalibration = false;
static bool isCalculatedCoeff = false;
static bool isAngleDisplayFormat = false;
static bool resetToZero = true;
static bool hasMTDiagnostics = false;
static bool isMotorRotationClockwise = true;
static double fast_motor_rpm = 300;

static double motorFullStepAngle = 0.9; // TODO input as configuration
static double microStepResolution = 256; // TODO input as configuration
static int motorfCLK = 12500000; // 12.5 Mhz, TODO input as configuration
static double motorAccelTime = 1; // In seconds

static double motorFullStepPerRevolution = 360 / motorFullStepAngle;
static double motorMicrostepPerRevolution = motorFullStepPerRevolution * microStepResolution;
static double motorTimeUnit = static_cast<double>(1 << 24) / motorfCLK; // t = 2^24/16Mhz

static int acquisitionDisplayLength = 200;
static QVector<double> acquisitionAngleList, acquisitionABSAngleList, acquisitionTurnCountList, acquisitionTmp0List,
					   acquisitionTmp1List, acquisitionSineList, acquisitionCosineList, acquisitionRadiusList,
					   graphDataList, graphPostDataList;

static const QColor scopyBlueColor = Style::getColor(json::theme::interactive_primary_idle);
static const QColor sineColor = QColor("#85e94c");
static const QColor cosineColor = QColor("#91e6cf");
static const QColor faultLEDColor = QColor("#c81a28");
static const QColor gpioLEDColor = Style::getColor(json::theme::interactive_secondary_idle);
static const QColor statusLEDColor = QColor("#2e9e6f");

static const QPen scopyBluePen(scopyBlueColor);
static const QPen channel0Pen(Style::getAttribute(json::global::ch0));
static const QPen channel1Pen(Style::getAttribute(json::global::ch1));
static const QPen channel2Pen(Style::getAttribute(json::global::ch2));
static const QPen channel3Pen(Style::getAttribute(json::global::ch3));
static const QPen channel4Pen(Style::getAttribute(json::global::ch4));
static const QPen channel5Pen(Style::getAttribute(json::global::ch5));
static const QPen channel6Pen(Style::getAttribute(json::global::ch6));
static const QPen channel7Pen(Style::getAttribute(json::global::ch7));
static const QPen sinePen(sineColor);
static const QPen cosinePen(cosineColor);

static map<string, string> deviceRegisterMap;
static map<string, int> GENERALRegisterMap;
static map<string, bool> DIGIOENRegisterMap;
static map<string, bool> FAULTRegisterMap;
static map<string, bool> DIAG1RegisterMap;
static map<string, double> DIAG2RegisterMap;
static map<string, double> DIAG1AFERegisterMap;

static QString deviceName = "";
static QString deviceType = "";
static bool is5V = false;

static double H1_MAG_ANGLE, H2_MAG_ANGLE, H3_MAG_ANGLE, H8_MAG_ANGLE, H1_PHASE_ANGLE, H2_PHASE_ANGLE, H3_PHASE_ANGLE, H8_PHASE_ANGLE;
static uint32_t H1_MAG_HEX, H2_MAG_HEX, H3_MAG_HEX, H8_MAG_HEX, H1_PHASE_HEX, H2_PHASE_HEX, H3_PHASE_HEX, H8_PHASE_HEX;

static int acquisitionGraphYMin = 0;
static int acquisitionGraphYMax = 360;

static bool deviceStatusFault = false;

static bool isAcquisitionTab = false;
static bool isCalibrationTab = false;
static bool isUtilityTab = false;

static bool isStartAcquisition = false;

static bool isDeviceStatusMonitor = false;
static bool isMotorPositionMonitor = false;
static bool isMotorVelocityCheck = false;
static bool isMotorVelocityReached = false;
static bool isResetMotorToZero = false;

static int readMotorDebounce = 50; // In ms
static int calibrationMode = 0;

static int globalSpacingSmall = Style::getDimension(json::global::unit_0_5);
static int globalSpacingMedium = Style::getDimension(json::global::unit_1);

static map<AcquisitionDataKey, bool> acquisitionDataMap = {
    {RADIUS, false},
    {ANGLE, false},
    {TURNCOUNT, false},
    {ABSANGLE, false},
    {SINE, false},
    {COSINE, false},
    {SECANGLI, false},
    {SECANGLQ, false},
    {ANGLESEC, false},
    {DIAG1, false},
    {DIAG2, false},
    {TMP0, false},
    {TMP1, false},
    {CNVCNT, false},
    {SCRADIUS, false},
    {SPIFAULT, false}
};

HarmonicCalibration::HarmonicCalibration(ADMTController *m_admtController, bool isDebug, QWidget *parent)
	: QWidget(parent)
	, isDebug(isDebug)
	, m_admtController(m_admtController)
{
	ADMTStyleHelper::GetInstance()->initColorMap();
	readDeviceProperties();
	initializeADMT();
	initializeMotor();
	readSequence();

	rotationChannelName = m_admtController->getChannelId(ADMTController::Channel::ROTATION);
	angleChannelName = m_admtController->getChannelId(ADMTController::Channel::ANGLE);
	countChannelName = m_admtController->getChannelId(ADMTController::Channel::COUNT);
	temperatureChannelName = m_admtController->getChannelId(ADMTController::Channel::TEMPERATURE);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *lay = new QHBoxLayout(this);
	tabWidget = new QTabWidget(this);

	setLayout(lay);
    lay->setMargin(0);
	lay->insertWidget(1, tabWidget);
	tabWidget->setStyleSheet("");
	tabWidget->tabBar()->setStyleSheet("");
	Style::setStyle(tabWidget, style::properties::admt::tabWidget);
	Style::setStyle(tabWidget->tabBar(), style::properties::admt::tabBar);

	tabWidget->addTab(createAcquisitionWidget(), "Acquisition");
	tabWidget->addTab(createCalibrationWidget(), "Calibration");
	tabWidget->addTab(createUtilityWidget(), "Utility");
	tabWidget->addTab(createRegistersWidget(), "Registers");

	connect(tabWidget, &QTabWidget::currentChanged, [=](int index){
		tabWidget->setCurrentIndex(index);

		if(index == 0 || index == 1)
		{
			startDeviceStatusMonitor();
			startCurrentMotorPositionMonitor();
		}
		else{
			stopDeviceStatusMonitor();
			stopCurrentMotorPositionMonitor();
		}

		if(index == 0) // Acquisition Tab
		{ 
			startAcquisitionUITask();
			readSequence();
			updateSequenceWidget();
			updateAcquisitionMotorRPM();
			updateAcquisitionMotorRotationDirection();
		}
		else
		{
			stopAcquisitionUITask();
			stop();
		}

		if(index == 1) // Calibration Tab
		{ 
			startCalibrationUITask();
			updateCalibrationMotorRPM();
			updateCalibrationMotorRotationDirection();
		}
		else 
		{ 
			stopCalibrationUITask();
		}

		if(index == 2) // Utility Tab
		{ 
			readSequence();
			toggleFaultRegisterMode(GENERALRegisterMap.at("Sequence Type"));
			toggleMTDiagnostics(GENERALRegisterMap.at("Sequence Type"));
			toggleUtilityTask(true);
		}	
		else 
		{
			toggleUtilityTask(false);
		}

		if(index == 3) // Registers Tab
		{ 
			readSequence();
			toggleRegisters(GENERALRegisterMap.at("Sequence Type"));
		}
	});

	connect(this, &HarmonicCalibration::updateFaultStatusSignal, this, &HarmonicCalibration::updateFaultStatus);
	connect(this, &HarmonicCalibration::motorPositionChanged, this, &HarmonicCalibration::updateMotorPosition);

	connect(this, &HarmonicCalibration::calibrationLogWriteSignal, this, &HarmonicCalibration::calibrationLogWrite);
	connect(this, &HarmonicCalibration::commandLogWriteSignal, this, &HarmonicCalibration::commandLogWrite);
	connect(this, &HarmonicCalibration::DIGIORegisterChanged, this, &HarmonicCalibration::updateDIGIOUI);
	connect(this, &HarmonicCalibration::FaultRegisterChanged, this, &HarmonicCalibration::updateFaultRegisterUI);
	connect(this, &HarmonicCalibration::DIAG1RegisterChanged, this, &HarmonicCalibration::updateMTDiagnosticRegisterUI);
	connect(this, &HarmonicCalibration::DIAG2RegisterChanged, this, &HarmonicCalibration::updateMTDiagnosticsUI);

	startAcquisitionUITask();
	startDeviceStatusMonitor();
	startCurrentMotorPositionMonitor();
}

HarmonicCalibration::~HarmonicCalibration() 
{
	requestDisconnect();
}

ToolTemplate* HarmonicCalibration::createAcquisitionWidget()
{
	tool = new ToolTemplate(this);
	openLastMenuButton = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuButtonGroup = dynamic_cast<OpenLastMenuBtn *>(openLastMenuButton)->getButtonGroup();

    settingsButton = new GearBtn(this);
    runButton = new RunBtn(this);

	QPushButton *resetGMRButton = new QPushButton(this);
	resetGMRButton->setText("GMR Reset");
	Style::setStyle(resetGMRButton, style::properties::button::singleButton);
	connect(resetGMRButton, &QPushButton::clicked, this, &HarmonicCalibration::GMRReset);

	rightMenuButtonGroup->addButton(settingsButton);

	#pragma region Raw Data Widget
	QScrollArea *rawDataScroll = new QScrollArea(this);
	rawDataScroll->setWidgetResizable(true);
	QWidget *rawDataWidget = new QWidget(rawDataScroll);
	rawDataScroll->setWidget(rawDataWidget);
	QVBoxLayout *rawDataLayout = new QVBoxLayout(rawDataWidget);
	rawDataLayout->setMargin(0);
	rawDataWidget->setLayout(rawDataLayout);

	MenuSectionWidget *rotationWidget = new MenuSectionWidget(rawDataWidget);
	MenuSectionWidget *angleWidget = new MenuSectionWidget(rawDataWidget);
	MenuSectionWidget *countWidget = new MenuSectionWidget(rawDataWidget);
	MenuSectionWidget *tempWidget = new MenuSectionWidget(rawDataWidget);
	Style::setStyle(rotationWidget, style::properties::widget::basicComponent);
	Style::setStyle(angleWidget, style::properties::widget::basicComponent);
	Style::setStyle(countWidget, style::properties::widget::basicComponent);
	Style::setStyle(tempWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *rotationSection = new MenuCollapseSection("ABS Angle", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, rotationWidget);
	MenuCollapseSection *angleSection = new MenuCollapseSection("Angle", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, angleWidget);
	MenuCollapseSection *countSection = new MenuCollapseSection("Count", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, countWidget);
	MenuCollapseSection *tempSection = new MenuCollapseSection("Temp 0", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, tempWidget);
	rotationSection->contentLayout()->setSpacing(globalSpacingSmall);
	angleSection->contentLayout()->setSpacing(globalSpacingSmall);
	countSection->contentLayout()->setSpacing(globalSpacingSmall);
	tempSection->contentLayout()->setSpacing(globalSpacingSmall);

	rotationWidget->contentLayout()->addWidget(rotationSection);
	angleWidget->contentLayout()->addWidget(angleSection);
	countWidget->contentLayout()->addWidget(countSection);
	tempWidget->contentLayout()->addWidget(tempSection);

	rotationValueLabel = new QLabel("--.--°", rotationSection);
	angleValueLabel = new QLabel("--.--°", angleSection);
	countValueLabel = new QLabel("--", countSection);
	tempValueLabel = new QLabel("--.-- °C", tempSection);

	rotationSection->contentLayout()->addWidget(rotationValueLabel);
	angleSection->contentLayout()->addWidget(angleValueLabel);
	countSection->contentLayout()->addWidget(countValueLabel);
	tempSection->contentLayout()->addWidget(tempValueLabel);

	#pragma region Acquisition Motor Control Section Widget
	MenuSectionWidget *motorControlSectionWidget = new MenuSectionWidget(rawDataWidget);
	Style::setStyle(motorControlSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *motorControlCollapseSection = new MenuCollapseSection("Motor Control", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, motorControlSectionWidget);
	motorControlSectionWidget->contentLayout()->addWidget(motorControlCollapseSection);

	QWidget *motorRPMWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorRPMLayout = new QVBoxLayout(motorRPMWidget);
	motorRPMWidget->setLayout(motorRPMLayout);
	motorRPMLayout->setMargin(0);
	motorRPMLayout->setSpacing(0);
	QLabel *motorRPMLabel = new QLabel("Motor RPM", motorRPMWidget);
	acquisitionMotorRPMLineEdit = new QLineEdit(QString::number(motor_rpm), motorRPMWidget);
	connectLineEditToRPM(acquisitionMotorRPMLineEdit, motor_rpm);
	motorRPMLayout->addWidget(motorRPMLabel);
	motorRPMLayout->addWidget(acquisitionMotorRPMLineEdit);

	QWidget *currentPositionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *currentPositionLayout = new QVBoxLayout(currentPositionWidget);
	currentPositionWidget->setLayout(currentPositionLayout);
	currentPositionLayout->setMargin(0);
	currentPositionLayout->setSpacing(0);
	QLabel *currentPositionLabel = new QLabel("Current Position", currentPositionWidget);
	acquisitionMotorCurrentPositionLineEdit = new QLineEdit("--.--", currentPositionWidget);
	acquisitionMotorCurrentPositionLineEdit->setReadOnly(true);
	connectLineEditToDouble(acquisitionMotorCurrentPositionLineEdit, current_pos);
	currentPositionLayout->addWidget(currentPositionLabel);
	currentPositionLayout->addWidget(acquisitionMotorCurrentPositionLineEdit);

	QWidget *targetPositionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *targetPositionLayout = new QVBoxLayout(targetPositionWidget);
	targetPositionWidget->setLayout(targetPositionLayout);
	targetPositionLayout->setMargin(0);
	targetPositionLayout->setSpacing(0);
	QLabel *targetPositionLabel = new QLabel("Target Position", targetPositionWidget);
	motorTargetPositionLineEdit = new QLineEdit(QString::number(target_pos), targetPositionWidget);
	connectLineEditToNumberWrite(motorTargetPositionLineEdit, target_pos, ADMTController::MotorAttribute::TARGET_POS);
	targetPositionLayout->addWidget(targetPositionLabel);
	targetPositionLayout->addWidget(motorTargetPositionLineEdit);

	QWidget *motorDirectionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorDirectionLayout = new QVBoxLayout(motorDirectionWidget);
	motorDirectionWidget->setLayout(motorDirectionLayout);
	motorDirectionLayout->setMargin(0);
	motorDirectionLayout->setSpacing(0);
	QLabel *motorDirectionLabel = new QLabel("Rotation Direction", motorDirectionWidget);
	acquisitionMotorDirectionSwitch = new CustomSwitch("CW", "CCW", motorControlSectionWidget);
	acquisitionMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
	connect(acquisitionMotorDirectionSwitch, &CustomSwitch::toggled, this, [=](bool b){
		isMotorRotationClockwise = b;
	});
	motorDirectionLayout->addWidget(motorDirectionLabel);
	motorDirectionLayout->addWidget(acquisitionMotorDirectionSwitch);

	QPushButton *continuousRotationButton = new QPushButton("Continuous Rotation", motorControlSectionWidget);
	StyleHelper::BasicButton(continuousRotationButton);
	connect(continuousRotationButton, &QPushButton::clicked, this, &HarmonicCalibration::moveMotorContinuous);

	QPushButton *stopMotorButton = new QPushButton("Stop Motor", motorControlSectionWidget);
	StyleHelper::BasicButton(stopMotorButton);
	connect(stopMotorButton, &QPushButton::clicked, this, &HarmonicCalibration::stopMotor);

	motorControlCollapseSection->contentLayout()->setSpacing(globalSpacingSmall);
	motorControlCollapseSection->contentLayout()->addWidget(motorRPMWidget);
	motorControlCollapseSection->contentLayout()->addWidget(currentPositionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(targetPositionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(motorDirectionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(continuousRotationButton);
	motorControlCollapseSection->contentLayout()->addWidget(stopMotorButton);
	#pragma endregion

	rawDataLayout->setSpacing(globalSpacingSmall);
	rawDataLayout->addWidget(angleWidget);
	rawDataLayout->addWidget(rotationWidget);
	rawDataLayout->addWidget(countWidget);
	rawDataLayout->addWidget(tempWidget);
	rawDataLayout->addWidget(motorControlSectionWidget);
	rawDataLayout->addStretch();
	#pragma endregion

	#pragma region Acquisition Graph Section Widget
	MenuSectionWidget *acquisitionGraphSectionWidget = new MenuSectionWidget(this);
	Style::setStyle(acquisitionGraphSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *acquisitionGraphCollapseSection = new MenuCollapseSection("Captured Data", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, acquisitionGraphSectionWidget);
	acquisitionGraphSectionWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	acquisitionGraphSectionWidget->contentLayout()->addWidget(acquisitionGraphCollapseSection);
	acquisitionGraphCollapseSection->contentLayout()->setSpacing(8);

	acquisitionGraphPlotWidget = new PlotWidget();
	acquisitionGraphPlotWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	acquisitionXPlotAxis = new PlotAxis(QwtAxis::XBottom, acquisitionGraphPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	acquisitionXPlotAxis->setInterval(0, acquisitionDisplayLength);
	acquisitionYPlotAxis = new PlotAxis(QwtAxis::YLeft, acquisitionGraphPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	acquisitionYPlotAxis->setInterval(0, 360);

	acquisitionAnglePlotChannel = new PlotChannel("Angle", channel0Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionABSAnglePlotChannel = new PlotChannel("ABS Angle", channel1Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionTurnCountPlotChannel = new PlotChannel("Turn Count", channel2Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionTmp0PlotChannel = new PlotChannel("TMP 0", channel3Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionTmp1PlotChannel = new PlotChannel("TMP 1", channel4Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSinePlotChannel = new PlotChannel("Sine", sinePen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionCosinePlotChannel = new PlotChannel("Cosine", cosinePen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionRadiusPlotChannel = new PlotChannel("Radius", channel5Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSecAnglQPlotChannel = new PlotChannel("SECANGLQ", channel6Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSecAnglIPlotChannel = new PlotChannel("SECANGLI", channel7Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);

	acquisitionGraphPlotWidget->addPlotChannel(acquisitionAnglePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionABSAnglePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionTurnCountPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionTmp0PlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionTmp1PlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSinePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionCosinePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionRadiusPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSecAnglQPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSecAnglIPlotChannel);
	acquisitionAnglePlotChannel->setEnabled(true);
	acquisitionABSAnglePlotChannel->setEnabled(true);
	acquisitionTurnCountPlotChannel->setEnabled(true);
	acquisitionTmp0PlotChannel->setEnabled(true);
	acquisitionTmp1PlotChannel->setEnabled(true);
	acquisitionSinePlotChannel->setEnabled(true);
	acquisitionCosinePlotChannel->setEnabled(true);
	acquisitionRadiusPlotChannel->setEnabled(true);
	acquisitionSecAnglQPlotChannel->setEnabled(true);
	acquisitionSecAnglIPlotChannel->setEnabled(true);
	acquisitionGraphPlotWidget->selectChannel(acquisitionAnglePlotChannel);

	acquisitionGraphPlotWidget->setShowXAxisLabels(true);
	acquisitionGraphPlotWidget->setShowYAxisLabels(true);
	acquisitionGraphPlotWidget->showAxisLabels();
	acquisitionGraphPlotWidget->replot();

	#pragma region Channel Selection
	QWidget *acquisitionGraphChannelWidget = new QWidget(acquisitionGraphSectionWidget);
	QGridLayout *acquisitionGraphChannelGridLayout = new QGridLayout(acquisitionGraphChannelWidget);
	acquisitionGraphChannelGridLayout->setContentsMargins(16, 8, 8, 16);
	acquisitionGraphChannelGridLayout->setSpacing(8);

	QCheckBox *angleCheckBox = new QCheckBox("Angle", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(angleCheckBox, channel0Pen.color());
	connectCheckBoxToAcquisitionGraph(angleCheckBox, acquisitionAnglePlotChannel, ANGLE);
	angleCheckBox->setChecked(true);

	QCheckBox *absAngleCheckBox = new QCheckBox("ABS Angle", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(absAngleCheckBox, channel1Pen.color());
	connectCheckBoxToAcquisitionGraph(absAngleCheckBox, acquisitionABSAnglePlotChannel, ABSANGLE);

	QCheckBox *temp0CheckBox = new QCheckBox("Temp 0", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(temp0CheckBox, channel3Pen.color());
	connectCheckBoxToAcquisitionGraph(temp0CheckBox, acquisitionTmp0PlotChannel, TMP0);

	QCheckBox *sineCheckBox = new QCheckBox("Sine", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(sineCheckBox, sineColor);
	connectCheckBoxToAcquisitionGraph(sineCheckBox, acquisitionSinePlotChannel, SINE);

	QCheckBox *cosineCheckBox = new QCheckBox("Cosine", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(cosineCheckBox, cosineColor);
	connectCheckBoxToAcquisitionGraph(cosineCheckBox, acquisitionCosinePlotChannel, COSINE);

	QCheckBox *radiusCheckBox = new QCheckBox("Radius", acquisitionGraphChannelWidget);
	ADMTStyleHelper::ColoredSquareCheckbox(radiusCheckBox, channel5Pen.color());
	connectCheckBoxToAcquisitionGraph(radiusCheckBox, acquisitionRadiusPlotChannel, RADIUS);

	if(GENERALRegisterMap.at("Sequence Type") == 0) // Sequence Mode 1
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 1);
	}
	else if(GENERALRegisterMap.at("Sequence Type") == 1) // Sequence Mode 2
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 1);
	}
	#pragma endregion

	acquisitionGraphCollapseSection->contentLayout()->addWidget(acquisitionGraphPlotWidget);
	acquisitionGraphCollapseSection->contentLayout()->addWidget(acquisitionGraphChannelWidget);
	#pragma endregion

	#pragma region General Setting
	QScrollArea *generalSettingScroll = new QScrollArea(this);
	generalSettingScroll->setWidgetResizable(true);
	QWidget *generalSettingWidget = new QWidget(generalSettingScroll);
	generalSettingScroll->setWidget(generalSettingWidget);
	QVBoxLayout *generalSettingLayout = new QVBoxLayout(generalSettingWidget);
	generalSettingLayout->setMargin(0);
	generalSettingWidget->setLayout(generalSettingLayout);

	header = new MenuHeaderWidget(deviceName + " " + deviceType, scopyBluePen, this);
	Style::setStyle(header, style::properties::widget::basicComponent);

	// General Setting Widget
	MenuSectionWidget *generalWidget = new MenuSectionWidget(generalSettingWidget);
	Style::setStyle(generalWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *generalSection = new MenuCollapseSection("Data Acquisition", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, generalWidget);
	generalSection->header()->toggle();
	generalSection->contentLayout()->setSpacing(globalSpacingSmall);
	generalWidget->contentLayout()->addWidget(generalSection);

	// Graph Update Interval
	QWidget *graphUpdateIntervalWidget = new QWidget(generalSection);
	QVBoxLayout *graphUpdateIntervalLayout = new QVBoxLayout(graphUpdateIntervalWidget);
	graphUpdateIntervalWidget->setLayout(graphUpdateIntervalLayout);
	graphUpdateIntervalLayout->setMargin(0);
	graphUpdateIntervalLayout->setSpacing(0);
	QLabel *graphUpdateIntervalLabel = new QLabel(graphUpdateIntervalWidget);
	graphUpdateIntervalLabel->setText("Graph Update Interval (ms)");
	graphUpdateIntervalLineEdit = new QLineEdit(graphUpdateIntervalWidget);
	graphUpdateIntervalLineEdit->setText(QString::number(acquisitionGraphSampleRate));
	connectLineEditToNumber(graphUpdateIntervalLineEdit, acquisitionGraphSampleRate, 1, 5000);
	graphUpdateIntervalLayout->addWidget(graphUpdateIntervalLabel);
	graphUpdateIntervalLayout->addWidget(graphUpdateIntervalLineEdit);

	// Data Sample Size
	QWidget *displayLengthWidget = new QWidget(generalSection);
	QVBoxLayout *displayLengthLayout = new QVBoxLayout(displayLengthWidget);
	displayLengthWidget->setLayout(displayLengthLayout);
	displayLengthLayout->setMargin(0);
	displayLengthLayout->setSpacing(0);
	QLabel *displayLengthLabel = new QLabel("Display Length", displayLengthWidget);
	displayLengthLineEdit = new QLineEdit(displayLengthWidget);
	displayLengthLineEdit->setText(QString::number(acquisitionDisplayLength));
	connectLineEditToNumber(displayLengthLineEdit, acquisitionDisplayLength, 1, 2048);
	displayLengthLayout->addWidget(displayLengthLabel);
	displayLengthLayout->addWidget(displayLengthLineEdit);

	QPushButton *resetYAxisButton = new QPushButton("Reset Y-Axis Scale", generalSection);
	StyleHelper::BasicButton(resetYAxisButton);
	connect(resetYAxisButton, &QPushButton::clicked, this, &HarmonicCalibration::resetAcquisitionYAxisScale);

	generalSection->contentLayout()->addWidget(graphUpdateIntervalWidget);
	generalSection->contentLayout()->addWidget(displayLengthWidget);
	generalSection->contentLayout()->addWidget(resetYAxisButton);

	MenuSectionWidget *sequenceWidget = new MenuSectionWidget(generalSettingWidget);
	Style::setStyle(sequenceWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *sequenceSection = new MenuCollapseSection("Sequence", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, sequenceWidget);
	sequenceWidget->contentLayout()->addWidget(sequenceSection);
	sequenceSection->contentLayout()->setSpacing(globalSpacingSmall);

	sequenceTypeMenuCombo = new MenuCombo("Sequence Type", sequenceSection);
	QComboBox *sequenceTypeComboBox = sequenceTypeMenuCombo->combo();
	sequenceTypeComboBox->addItem("Mode 1", QVariant(0));
	if(deviceType.toStdString() == "Automotive")
	{
		sequenceTypeComboBox->addItem("Mode 2", QVariant(1));
	}

	conversionTypeMenuCombo = new MenuCombo("Conversion Type", sequenceSection);
	QComboBox *conversionTypeComboBox = conversionTypeMenuCombo->combo();
	conversionTypeComboBox->addItem("Continuous conversions", QVariant(0));
	conversionTypeComboBox->addItem("One-shot conversion", QVariant(1));

	convertSynchronizationMenuCombo = new MenuCombo("Convert Synchronization", sequenceSection);
	QComboBox *convertSynchronizationComboBox = convertSynchronizationMenuCombo->combo();
	convertSynchronizationComboBox->addItem("Enabled", QVariant(1));
	convertSynchronizationComboBox->addItem("Disabled", QVariant(0));

	angleFilterMenuCombo = new MenuCombo("Angle Filter", sequenceSection);
	QComboBox *angleFilterComboBox = angleFilterMenuCombo->combo();
	angleFilterComboBox->addItem("Enabled", QVariant(1));
	angleFilterComboBox->addItem("Disabled", QVariant(0));

	eighthHarmonicMenuCombo = new MenuCombo("8th Harmonic", sequenceSection);
	QComboBox *eighthHarmonicComboBox = eighthHarmonicMenuCombo->combo();
	eighthHarmonicComboBox->addItem("User-Supplied Values", QVariant(1));
	eighthHarmonicComboBox->addItem("ADI Factory Values", QVariant(0));

	updateSequenceWidget();

	applySequenceButton = new QPushButton("Apply", sequenceSection);
	StyleHelper::BasicButton(applySequenceButton);
	connect(applySequenceButton, &QPushButton::clicked, this, &HarmonicCalibration::applySequenceAndUpdate);

	sequenceSection->contentLayout()->addWidget(sequenceTypeMenuCombo);
	sequenceSection->contentLayout()->addWidget(conversionTypeMenuCombo);
	sequenceSection->contentLayout()->addWidget(convertSynchronizationMenuCombo);
	sequenceSection->contentLayout()->addWidget(angleFilterMenuCombo);
	sequenceSection->contentLayout()->addWidget(eighthHarmonicMenuCombo);
	sequenceSection->contentLayout()->addWidget(applySequenceButton);

	#pragma region Device Status Widget
	MenuSectionWidget *acquisitionDeviceStatusWidget = new MenuSectionWidget(generalSettingWidget);
	Style::setStyle(acquisitionDeviceStatusWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *acquisitionDeviceStatusSection = new MenuCollapseSection("Device Status", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, generalWidget);
	acquisitionDeviceStatusSection->contentLayout()->setSpacing(globalSpacingSmall);
	acquisitionDeviceStatusWidget->contentLayout()->addWidget(acquisitionDeviceStatusSection);

	acquisitionFaultRegisterLEDWidget = createStatusLEDWidget("Fault Register", "status", false, acquisitionDeviceStatusSection);
	acquisitionDeviceStatusSection->contentLayout()->addWidget(acquisitionFaultRegisterLEDWidget);

	if(deviceType == "Automotive" && GENERALRegisterMap.at("Sequence Type") == 1) // Automotive & Sequence Mode 2
	{
		QCheckBox *acquisitionSPICRCLEDWidget = createStatusLEDWidget("SPI CRC", "status", false, acquisitionDeviceStatusSection);
		QCheckBox *acquisitionSPIFlagLEDWidget = createStatusLEDWidget("SPI Flag", "status", false, acquisitionDeviceStatusSection);
		acquisitionDeviceStatusSection->contentLayout()->addWidget(acquisitionSPICRCLEDWidget);
		acquisitionDeviceStatusSection->contentLayout()->addWidget(acquisitionSPIFlagLEDWidget);
	}
	#pragma endregion

	generalSettingLayout->setSpacing(globalSpacingSmall);
	generalSettingLayout->addWidget(acquisitionDeviceStatusWidget);
	generalSettingLayout->addWidget(header);
	generalSettingLayout->addWidget(sequenceWidget);
	generalSettingLayout->addWidget(generalWidget);
	generalSettingLayout->addStretch();
	#pragma endregion

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(false);
    tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);
    tool->addWidgetToTopContainerMenuControlHelper(openLastMenuButton, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);
    tool->addWidgetToTopContainerHelper(resetGMRButton, TTA_RIGHT);
    tool->addWidgetToTopContainerHelper(runButton, TTA_RIGHT);
	tool->leftStack()->add("rawDataScroll", rawDataScroll);
	tool->rightStack()->add("generalSettingScroll", generalSettingScroll);
	tool->addWidgetToCentralContainerHelper(acquisitionGraphSectionWidget);

	connect(runButton, &QPushButton::toggled, this, &HarmonicCalibration::setRunning);
	connect(this, &HarmonicCalibration::runningChanged, this, &HarmonicCalibration::run);
	connect(this, &HarmonicCalibration::runningChanged, runButton, &QAbstractButton::setChecked);
	
	return tool;
}

ToolTemplate* HarmonicCalibration::createCalibrationWidget()
{
	ToolTemplate *tool = new ToolTemplate(this);

	#pragma region Calibration Data Graph Widget
	QWidget *calibrationDataGraphWidget = new QWidget();
	QGridLayout *calibrationDataGraphLayout = new QGridLayout(calibrationDataGraphWidget);
	calibrationDataGraphWidget->setLayout(calibrationDataGraphLayout);
	calibrationDataGraphLayout->setMargin(0);
	calibrationDataGraphLayout->setSpacing(5);

	MenuSectionWidget *calibrationDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	Style::setStyle(calibrationDataGraphSectionWidget, style::properties::widget::basicComponent);
	calibrationDataGraphTabWidget = new QTabWidget(calibrationDataGraphSectionWidget);
	calibrationDataGraphTabWidget->tabBar()->setStyleSheet("QTabBar::tab { width: 176px; }");
	calibrationDataGraphSectionWidget->contentLayout()->setSpacing(8);
	calibrationDataGraphSectionWidget->contentLayout()->addWidget(calibrationDataGraphTabWidget);

	#pragma region Calibration Samples
	QWidget *calibrationSamplesWidget = new QWidget(calibrationDataGraphTabWidget);
	Style::setStyle(calibrationSamplesWidget, style::properties::widget::basicBackground);
	QVBoxLayout *calibrationSamplesLayout = new QVBoxLayout(calibrationSamplesWidget);
	calibrationSamplesWidget->setLayout(calibrationSamplesLayout);
	calibrationSamplesLayout->setMargin(0);
	calibrationSamplesLayout->setSpacing(0);

	calibrationRawDataPlotWidget = new PlotWidget();
	calibrationRawDataPlotWidget->setContentsMargins(10, 10, 10, 6);

	calibrationRawDataXPlotAxis = new PlotAxis(QwtAxis::XBottom, calibrationRawDataPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	calibrationRawDataXPlotAxis->setMin(0);
	calibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, calibrationRawDataPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	calibrationRawDataYPlotAxis->setInterval(0, 360);	

	calibrationRawDataPlotChannel = new PlotChannel("Samples", scopyBluePen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationSineDataPlotChannel = new PlotChannel("Sine", sinePen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationCosineDataPlotChannel = new PlotChannel("Cosine", cosinePen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);

	calibrationRawDataPlotWidget->addPlotChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationSineDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationCosineDataPlotChannel);
	calibrationSineDataPlotChannel->setEnabled(true);
	calibrationCosineDataPlotChannel->setEnabled(true);
	calibrationRawDataPlotChannel->setEnabled(true);
	calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);

	calibrationRawDataPlotWidget->setShowXAxisLabels(true);
	calibrationRawDataPlotWidget->setShowYAxisLabels(true);
	calibrationRawDataPlotWidget->showAxisLabels();
	calibrationRawDataPlotWidget->replot();

	QWidget *calibrationDataGraphChannelsWidget = new QWidget(calibrationDataGraphTabWidget);
	ADMTStyleHelper::UIBackgroundStyle(calibrationDataGraphChannelsWidget);
	QHBoxLayout *calibrationDataGraphChannelsLayout = new QHBoxLayout(calibrationDataGraphChannelsWidget);
	calibrationDataGraphChannelsWidget->setLayout(calibrationDataGraphChannelsLayout);
	calibrationDataGraphChannelsLayout->setContentsMargins(20, 13, 20, 5);
	calibrationDataGraphChannelsLayout->setSpacing(20);
	
	MenuControlButton *toggleAngleButton = createChannelToggleWidget("Angle", scopyBlueColor, calibrationDataGraphChannelsWidget);
	MenuControlButton *toggleSineButton = createChannelToggleWidget("Sine", sineColor, calibrationDataGraphChannelsWidget);
	MenuControlButton *toggleCosineButton = createChannelToggleWidget("Cosine", cosineColor, calibrationDataGraphChannelsWidget);

	calibrationDataGraphChannelsLayout->addWidget(toggleAngleButton);
	calibrationDataGraphChannelsLayout->addWidget(toggleSineButton);
	calibrationDataGraphChannelsLayout->addWidget(toggleCosineButton);
	calibrationDataGraphChannelsLayout->addStretch();

	calibrationSamplesLayout->addWidget(calibrationRawDataPlotWidget);
	calibrationSamplesLayout->addWidget(calibrationDataGraphChannelsWidget);
	#pragma endregion

	#pragma region Post Calibration Samples
	QWidget *postCalibrationSamplesWidget = new QWidget(calibrationDataGraphTabWidget);
	Style::setStyle(postCalibrationSamplesWidget, style::properties::widget::basicBackground);
	QVBoxLayout *postCalibrationSamplesLayout = new QVBoxLayout(postCalibrationSamplesWidget);
	postCalibrationSamplesWidget->setLayout(postCalibrationSamplesLayout);
	postCalibrationSamplesLayout->setMargin(0);
	postCalibrationSamplesLayout->setSpacing(0);

	postCalibrationRawDataPlotWidget = new PlotWidget();
	postCalibrationRawDataPlotWidget->setContentsMargins(10, 10, 10, 6);

	postCalibrationRawDataXPlotAxis = new PlotAxis(QwtAxis::XBottom, postCalibrationRawDataPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	postCalibrationRawDataXPlotAxis->setMin(0);
	postCalibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, postCalibrationRawDataPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	postCalibrationRawDataYPlotAxis->setInterval(0, 360);

	postCalibrationRawDataPlotChannel = new PlotChannel("Samples", scopyBluePen, postCalibrationRawDataXPlotAxis, postCalibrationRawDataYPlotAxis);
	postCalibrationSineDataPlotChannel = new PlotChannel("Sine", sinePen, postCalibrationRawDataXPlotAxis, postCalibrationRawDataYPlotAxis);
	postCalibrationCosineDataPlotChannel = new PlotChannel("Cosine", cosinePen, postCalibrationRawDataXPlotAxis, postCalibrationRawDataYPlotAxis);

	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationRawDataPlotChannel);
	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationSineDataPlotChannel);
	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationCosineDataPlotChannel);

	postCalibrationSineDataPlotChannel->setEnabled(true);
	postCalibrationCosineDataPlotChannel->setEnabled(true);
	postCalibrationRawDataPlotChannel->setEnabled(true);
	postCalibrationRawDataPlotWidget->selectChannel(postCalibrationRawDataPlotChannel);
	
	postCalibrationRawDataPlotWidget->setShowXAxisLabels(true);
	postCalibrationRawDataPlotWidget->setShowYAxisLabels(true);
	postCalibrationRawDataPlotWidget->showAxisLabels();
	postCalibrationRawDataPlotWidget->replot();

	QWidget *postCalibrationDataGraphChannelsWidget = new QWidget(calibrationDataGraphTabWidget);
	QHBoxLayout *postCalibrationDataGraphChannelsLayout = new QHBoxLayout(postCalibrationDataGraphChannelsWidget);
	ADMTStyleHelper::GraphChannelStyle(postCalibrationDataGraphChannelsWidget, postCalibrationDataGraphChannelsLayout);

	MenuControlButton *togglePostAngleButton = createChannelToggleWidget("Angle", scopyBlueColor, postCalibrationDataGraphChannelsWidget);
	MenuControlButton *togglePostSineButton = createChannelToggleWidget("Sine", sineColor, postCalibrationDataGraphChannelsWidget);
	MenuControlButton *togglePostCosineButton = createChannelToggleWidget("Cosine", cosineColor, postCalibrationDataGraphChannelsWidget);

	postCalibrationDataGraphChannelsLayout->addWidget(togglePostAngleButton);
	postCalibrationDataGraphChannelsLayout->addWidget(togglePostSineButton);
	postCalibrationDataGraphChannelsLayout->addWidget(togglePostCosineButton);
	postCalibrationDataGraphChannelsLayout->addStretch();

	postCalibrationSamplesLayout->addWidget(postCalibrationRawDataPlotWidget);
	postCalibrationSamplesLayout->addWidget(postCalibrationDataGraphChannelsWidget);
	#pragma endregion

	calibrationDataGraphTabWidget->addTab(calibrationSamplesWidget, "Calibration Samples");
	calibrationDataGraphTabWidget->addTab(postCalibrationSamplesWidget, "Post Calibration Samples");

	MenuSectionWidget *resultDataSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	Style::setStyle(resultDataSectionWidget, style::properties::widget::basicComponent);
	resultDataTabWidget = new QTabWidget(resultDataSectionWidget);
	resultDataTabWidget->tabBar()->setStyleSheet("QTabBar::tab { width: 160px; }");
	resultDataSectionWidget->contentLayout()->setSpacing(8);
	resultDataSectionWidget->contentLayout()->addWidget(resultDataTabWidget);

	QColor magnitudeColor = Style::getColor(json::global::ch0);
	QColor phaseColor = Style::getColor(json::global::ch1);
	QPen magnitudePen = QPen(magnitudeColor);
	QPen phasePen = QPen(phaseColor);

	#pragma region Angle Error Widget
	QWidget *angleErrorWidget = new QWidget();
	Style::setStyle(angleErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *angleErrorLayout = new QVBoxLayout(angleErrorWidget);
	angleErrorWidget->setLayout(angleErrorLayout);
	angleErrorLayout->setMargin(0);
	angleErrorLayout->setSpacing(0);

	angleErrorPlotWidget = new PlotWidget();
	angleErrorPlotWidget->setContentsMargins(10, 10, 10, 6);
	angleErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, angleErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	angleErrorXPlotAxis->setMin(0);
	angleErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, angleErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	angleErrorYPlotAxis->setInterval(-4, 4);

	angleErrorPlotChannel = new PlotChannel("Angle Error", scopyBluePen, angleErrorXPlotAxis, angleErrorYPlotAxis);
	
	angleErrorPlotWidget->addPlotChannel(angleErrorPlotChannel);
	angleErrorPlotChannel->setEnabled(true);
	angleErrorPlotWidget->selectChannel(angleErrorPlotChannel);
	
	angleErrorPlotWidget->setShowXAxisLabels(true);
	angleErrorPlotWidget->setShowYAxisLabels(true);
	angleErrorPlotWidget->showAxisLabels();
	angleErrorPlotWidget->replot();

	angleErrorLayout->addWidget(angleErrorPlotWidget);
	#pragma endregion

	#pragma region FFT Angle Error Widget
	QWidget *FFTAngleErrorWidget = new QWidget();
	Style::setStyle(FFTAngleErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *FFTAngleErrorLayout = new QVBoxLayout(FFTAngleErrorWidget);
	FFTAngleErrorWidget->setLayout(FFTAngleErrorLayout);
	FFTAngleErrorLayout->setMargin(0);
	FFTAngleErrorLayout->setSpacing(0);

	FFTAngleErrorPlotWidget = new PlotWidget();
	FFTAngleErrorPlotWidget->setContentsMargins(10, 10, 10, 6);

	FFTAngleErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, FFTAngleErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	FFTAngleErrorXPlotAxis->setMin(0);
	FFTAngleErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, FFTAngleErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	FFTAngleErrorYPlotAxis->setInterval(-4, 4);

	FFTAngleErrorMagnitudeChannel = new PlotChannel("FFT Angle Error Magnitude", magnitudePen, FFTAngleErrorXPlotAxis, FFTAngleErrorYPlotAxis);
	FFTAngleErrorPhaseChannel = new PlotChannel("FFT Angle Error Phase", phasePen, FFTAngleErrorXPlotAxis, FFTAngleErrorYPlotAxis);
	FFTAngleErrorPlotWidget->addPlotChannel(FFTAngleErrorMagnitudeChannel);
	FFTAngleErrorPlotWidget->addPlotChannel(FFTAngleErrorPhaseChannel);

	FFTAngleErrorPhaseChannel->setEnabled(true);
	FFTAngleErrorMagnitudeChannel->setEnabled(true);
	FFTAngleErrorPlotWidget->selectChannel(FFTAngleErrorMagnitudeChannel);
	
	FFTAngleErrorPlotWidget->setShowXAxisLabels(true);
	FFTAngleErrorPlotWidget->setShowYAxisLabels(true);
	FFTAngleErrorPlotWidget->showAxisLabels();
	FFTAngleErrorPlotWidget->replot();

	QWidget *FFTAngleErrorChannelsWidget = new QWidget();
	QHBoxLayout *FFTAngleErrorChannelsLayout = new QHBoxLayout(FFTAngleErrorChannelsWidget);
	ADMTStyleHelper::GraphChannelStyle(FFTAngleErrorChannelsWidget, FFTAngleErrorChannelsLayout);

	MenuControlButton *toggleFFTAngleErrorMagnitudeButton = createChannelToggleWidget("Magnitude", magnitudeColor, FFTAngleErrorChannelsWidget);
	MenuControlButton *toggleFFTAngleErrorPhaseButton = createChannelToggleWidget("Phase", phaseColor, FFTAngleErrorChannelsWidget);

	FFTAngleErrorChannelsLayout->addWidget(toggleFFTAngleErrorMagnitudeButton);
	FFTAngleErrorChannelsLayout->addWidget(toggleFFTAngleErrorPhaseButton);
	FFTAngleErrorChannelsLayout->addStretch();

	FFTAngleErrorLayout->addWidget(FFTAngleErrorPlotWidget);
	FFTAngleErrorLayout->addWidget(FFTAngleErrorChannelsWidget);
	#pragma endregion

	#pragma region Corrected Error Widget
	QWidget *correctedErrorWidget = new QWidget();
	Style::setStyle(correctedErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *correctedErrorLayout = new QVBoxLayout(correctedErrorWidget);
	correctedErrorWidget->setLayout(correctedErrorLayout);
	correctedErrorLayout->setMargin(0);
	correctedErrorLayout->setSpacing(0);

	correctedErrorPlotWidget = new PlotWidget();
	correctedErrorPlotWidget->setContentsMargins(10, 10, 10, 6);

	correctedErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, correctedErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	correctedErrorXPlotAxis->setMin(0);
	correctedErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, correctedErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	correctedErrorYPlotAxis->setInterval(-4, 4);

	correctedErrorPlotChannel = new PlotChannel("Corrected Error", scopyBluePen, correctedErrorXPlotAxis, correctedErrorYPlotAxis);
	correctedErrorPlotWidget->addPlotChannel(correctedErrorPlotChannel);

	correctedErrorPlotChannel->setEnabled(true);
	correctedErrorPlotWidget->selectChannel(correctedErrorPlotChannel);
	
	correctedErrorPlotWidget->setShowXAxisLabels(true);
	correctedErrorPlotWidget->setShowYAxisLabels(true);
	correctedErrorPlotWidget->showAxisLabels();
	correctedErrorPlotWidget->replot();

	correctedErrorLayout->addWidget(correctedErrorPlotWidget);
	#pragma endregion

	#pragma region FFT Corrected Error Widget
	QWidget *FFTCorrectedErrorWidget = new QWidget();
	Style::setStyle(FFTCorrectedErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *FFTCorrectedErrorLayout = new QVBoxLayout(FFTCorrectedErrorWidget);
	FFTCorrectedErrorWidget->setLayout(FFTCorrectedErrorLayout);
	FFTCorrectedErrorLayout->setMargin(0);
	FFTCorrectedErrorLayout->setSpacing(0);

	FFTCorrectedErrorPlotWidget = new PlotWidget();
	FFTCorrectedErrorPlotWidget->setContentsMargins(10, 10, 10, 6);

	FFTCorrectedErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, FFTCorrectedErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	FFTCorrectedErrorXPlotAxis->setMin(0);
	FFTCorrectedErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, FFTCorrectedErrorPlotWidget, QPen(Style::getColor(json::theme::interactive_primary_idle)));
	FFTCorrectedErrorYPlotAxis->setInterval(-4, 4);

	FFTCorrectedErrorMagnitudeChannel = new PlotChannel("FFT Corrected Error Magnitude", magnitudePen, FFTCorrectedErrorXPlotAxis, FFTCorrectedErrorYPlotAxis);
	FFTCorrectedErrorPhaseChannel = new PlotChannel("FFT Corrected Error Phase", phasePen, FFTCorrectedErrorXPlotAxis, FFTCorrectedErrorYPlotAxis);
	FFTCorrectedErrorPlotWidget->addPlotChannel(FFTCorrectedErrorMagnitudeChannel);
	FFTCorrectedErrorPlotWidget->addPlotChannel(FFTCorrectedErrorPhaseChannel);

	FFTCorrectedErrorPhaseChannel->setEnabled(true);
	FFTCorrectedErrorMagnitudeChannel->setEnabled(true);
	FFTCorrectedErrorPlotWidget->selectChannel(FFTCorrectedErrorMagnitudeChannel);
	
	FFTCorrectedErrorPlotWidget->setShowXAxisLabels(true);
	FFTCorrectedErrorPlotWidget->setShowYAxisLabels(true);
	FFTCorrectedErrorPlotWidget->showAxisLabels();
	FFTCorrectedErrorPlotWidget->replot();

	QWidget *FFTCorrectedErrorChannelsWidget = new QWidget();
	QHBoxLayout *FFTCorrectedErrorChannelsLayout = new QHBoxLayout(FFTCorrectedErrorChannelsWidget);
	ADMTStyleHelper::GraphChannelStyle(FFTCorrectedErrorChannelsWidget, FFTCorrectedErrorChannelsLayout);

	MenuControlButton *toggleFFTCorrectedErrorMagnitudeButton = createChannelToggleWidget("Magnitude", magnitudeColor, FFTCorrectedErrorChannelsWidget);
	MenuControlButton *toggleFFTCorrectedErrorPhaseButton = createChannelToggleWidget("Phase", phaseColor, FFTCorrectedErrorChannelsWidget);

	FFTCorrectedErrorChannelsLayout->addWidget(toggleFFTCorrectedErrorMagnitudeButton);
	FFTCorrectedErrorChannelsLayout->addWidget(toggleFFTCorrectedErrorPhaseButton);
	FFTCorrectedErrorChannelsLayout->addStretch();

	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorPlotWidget);
	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorChannelsWidget);
	#pragma endregion

	resultDataTabWidget->addTab(angleErrorWidget, "Angle Error");
	resultDataTabWidget->addTab(FFTAngleErrorWidget, "FFT Angle Error");
	resultDataTabWidget->addTab(correctedErrorWidget, "Corrected Error");
	resultDataTabWidget->addTab(FFTCorrectedErrorWidget, "FFT Corrected Error");

	calibrationDataGraphLayout->addWidget(calibrationDataGraphSectionWidget, 0, 0);
	calibrationDataGraphLayout->addWidget(resultDataSectionWidget, 1, 0);

	calibrationDataGraphLayout->setColumnStretch(0, 1);
	calibrationDataGraphLayout->setRowStretch(0, 1);
	calibrationDataGraphLayout->setRowStretch(1, 1);
	#pragma endregion
	
	#pragma region Calibration Settings Widget
	QWidget *calibrationSettingsGroupWidget = new QWidget();
	QVBoxLayout *calibrationSettingsGroupLayout = new QVBoxLayout(calibrationSettingsGroupWidget);
	calibrationSettingsGroupWidget->setLayout(calibrationSettingsGroupLayout);
	calibrationSettingsGroupLayout->setMargin(0);
	calibrationSettingsGroupLayout->setSpacing(8);

	#pragma region Device Status Widget
	MenuSectionWidget *calibrationDeviceStatusWidget = new MenuSectionWidget(calibrationSettingsGroupWidget);
	Style::setStyle(calibrationDeviceStatusWidget, style::properties::widget::basicComponent);
	calibrationDeviceStatusWidget->contentLayout()->setSpacing(8);
	MenuCollapseSection *calibrationDeviceStatusSection = new MenuCollapseSection("Device Status", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationSettingsGroupWidget);
	calibrationDeviceStatusSection->contentLayout()->setSpacing(8);
	calibrationDeviceStatusWidget->contentLayout()->addWidget(calibrationDeviceStatusSection);

	calibrationFaultRegisterLEDWidget = createStatusLEDWidget("Fault Register", "status", false, calibrationDeviceStatusSection);
	calibrationDeviceStatusSection->contentLayout()->addWidget(calibrationFaultRegisterLEDWidget);

	if(deviceType == "Automotive" && GENERALRegisterMap.at("Sequence Type") == 1) // Automotive & Sequence Mode 2
	{
		QCheckBox *calibrationSPICRCLEDWidget = createStatusLEDWidget("SPI CRC", "status", false, calibrationDeviceStatusSection);
		QCheckBox *calibrationSPIFlagLEDWidget = createStatusLEDWidget("SPI Flag", "status", false, calibrationDeviceStatusSection);
		calibrationDeviceStatusSection->contentLayout()->addWidget(calibrationSPICRCLEDWidget);
		calibrationDeviceStatusSection->contentLayout()->addWidget(calibrationSPIFlagLEDWidget);
	}
	#pragma endregion

	#pragma region Acquire Calibration Samples Button
	calibrationStartMotorButton = new QPushButton(" Acquire Samples", calibrationSettingsGroupWidget);
	Style::setStyle(calibrationStartMotorButton, style::properties::admt::fullWidthRunButton);
	calibrationStartMotorButton->setCheckable(true);
	calibrationStartMotorButton->setChecked(false);
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play.svg", Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
						 "/icons/play_stop.svg",
					 Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::On);
	calibrationStartMotorButton->setIcon(icon1);

	connect(calibrationStartMotorButton, &QPushButton::toggled, this, [=](bool toggled) { 
		calibrationStartMotorButton->setText(toggled ? " Stop Acquisition" : " Acquire Samples"); 
		isStartMotor = toggled;
		if(toggled){
			isPostCalibration = false;
			startCalibration();			
		}
		else{
			stopCalibration();
		}
	});
	#pragma endregion

	#pragma region Start Calibration Button
	calibrateDataButton = new QPushButton(" Calibrate", calibrationSettingsGroupWidget);
	Style::setStyle(calibrateDataButton, style::properties::admt::fullWidthRunButton);
	calibrateDataButton->setCheckable(true);
	calibrateDataButton->setChecked(false);
	calibrateDataButton->setEnabled(false);
	calibrateDataButton->setIcon(icon1);

	connect(calibrateDataButton, &QPushButton::toggled, this, [=](bool toggled) {
		calibrateDataButton->setText(toggled ? " Stop Calibration" : " Calibrate");
		if(toggled) postCalibrateData();
		else stopCalibration();
	});
	#pragma endregion

	#pragma region Reset Calibration Button
	clearCalibrateDataButton = new QPushButton("Reset Calibration", calibrationSettingsGroupWidget);
	StyleHelper::BasicButton(clearCalibrateDataButton);
	QIcon resetIcon;
	// resetIcon.addPixmap(Util::ChangeSVGColor(":/gui/icons/refresh.svg", "white", 1), QIcon::Normal, QIcon::Off);
	resetIcon.addPixmap(Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)), 
			QIcon::Normal, QIcon::Off);
	clearCalibrateDataButton->setIcon(resetIcon);
	clearCalibrateDataButton->setIconSize(QSize(26, 26));
	#pragma endregion

	QScrollArea *calibrationSettingsScrollArea = new QScrollArea();
	QWidget *calibrationSettingsWidget = new QWidget(calibrationSettingsScrollArea);
	QVBoxLayout *calibrationSettingsLayout = new QVBoxLayout(calibrationSettingsWidget);
	calibrationSettingsScrollArea->setWidgetResizable(true);
	calibrationSettingsScrollArea->setWidget(calibrationSettingsWidget);
	calibrationSettingsWidget->setFixedWidth(260);
	calibrationSettingsWidget->setLayout(calibrationSettingsLayout);

	calibrationSettingsGroupLayout->addWidget(calibrationDeviceStatusWidget);
	calibrationSettingsGroupLayout->addWidget(calibrationStartMotorButton);
	calibrationSettingsGroupLayout->addWidget(calibrateDataButton);
	calibrationSettingsGroupLayout->addWidget(clearCalibrateDataButton);
	calibrationSettingsGroupLayout->addWidget(calibrationSettingsScrollArea);

	#pragma region Calibration Coefficient Section Widget
	MenuSectionWidget *calibrationCoeffSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationCoeffSectionWidget, style::properties::widget::basicComponent);
	QLabel *calibrationDisplayFormatLabel = new QLabel(calibrationCoeffSectionWidget);
	calibrationDisplayFormatLabel->setText("Display Format");
	Style::setStyle(calibrationDisplayFormatLabel, style::properties::label::menuMedium);
	QLabel *calibrationCalculatedCoeffLabel = new QLabel(calibrationCoeffSectionWidget);
	calibrationCalculatedCoeffLabel->setText("Calculated Coefficients");
	Style::setStyle(calibrationCalculatedCoeffLabel, style::properties::label::menuMedium);

	calibrationDisplayFormatSwitch = new CustomSwitch();
	calibrationDisplayFormatSwitch->setOffText("Hex");
	calibrationDisplayFormatSwitch->setOnText("Angle");
	calibrationDisplayFormatSwitch->setProperty("bigBtn", true);

	QWidget *calibrationCalculatedCoeffWidget = new QWidget(calibrationCoeffSectionWidget);
	QGridLayout *calibrationCalculatedCoeffLayout = new QGridLayout(calibrationCalculatedCoeffWidget);

	calibrationCalculatedCoeffWidget->setLayout(calibrationCalculatedCoeffLayout);
	calibrationCalculatedCoeffLayout->setMargin(0);
	calibrationCalculatedCoeffLayout->setVerticalSpacing(4);
	Style::setStyle(calibrationCalculatedCoeffWidget, style::properties::widget::basicBackground); 

	QWidget *h1RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h1RowLayout = new QHBoxLayout(h1RowContainer);
	QLabel *calibrationH1Label = new QLabel("H1", calibrationCalculatedCoeffWidget);
	calibrationH1MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH1PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	ADMTStyleHelper::CalculatedCoeffWidgetRowStyle(h1RowContainer, h1RowLayout, calibrationH1Label, calibrationH1MagLabel, calibrationH1PhaseLabel);

	QWidget *h2RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h2RowLayout = new QHBoxLayout(h2RowContainer);
	QLabel *calibrationH2Label = new QLabel("H2", calibrationCalculatedCoeffWidget);
	calibrationH2MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH2PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	ADMTStyleHelper::CalculatedCoeffWidgetRowStyle(h2RowContainer, h2RowLayout, calibrationH2Label, calibrationH2MagLabel, calibrationH2PhaseLabel);

	QWidget *h3RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h3RowLayout = new QHBoxLayout(h3RowContainer);
	QLabel *calibrationH3Label = new QLabel("H3", calibrationCalculatedCoeffWidget);
	calibrationH3MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH3PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	ADMTStyleHelper::CalculatedCoeffWidgetRowStyle(h3RowContainer, h3RowLayout, calibrationH3Label, calibrationH3MagLabel, calibrationH3PhaseLabel);

	QWidget *h8RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h8RowLayout = new QHBoxLayout(h8RowContainer);
	QLabel *calibrationH8Label = new QLabel("H8", calibrationCalculatedCoeffWidget);
	calibrationH8MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH8PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	ADMTStyleHelper::CalculatedCoeffWidgetRowStyle(h8RowContainer, h8RowLayout, calibrationH8Label, calibrationH8MagLabel, calibrationH8PhaseLabel);

	calibrationCalculatedCoeffLayout->addWidget(h1RowContainer, 0, 0);
	calibrationCalculatedCoeffLayout->addWidget(h2RowContainer, 1, 0);
	calibrationCalculatedCoeffLayout->addWidget(h3RowContainer, 2, 0);
	calibrationCalculatedCoeffLayout->addWidget(h8RowContainer, 3, 0);

	calibrationCoeffSectionWidget->contentLayout()->setSpacing(8);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatSwitch);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffWidget);
	#pragma endregion

	#pragma region Calibration Dataset Configuration
	MenuSectionWidget *calibrationDatasetConfigSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationDatasetConfigSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *calibrationDatasetConfigCollapseSection = new MenuCollapseSection("Dataset Configuration", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationDatasetConfigSectionWidget);
	calibrationDatasetConfigSectionWidget->contentLayout()->setSpacing(8);
	calibrationDatasetConfigSectionWidget->contentLayout()->addWidget(calibrationDatasetConfigCollapseSection);

	#pragma region Continuous Calibration Toggle
	calibrationModeMenuCombo = new MenuCombo("Calibration Mode", calibrationDatasetConfigCollapseSection);
	auto calibrationModeCombo = calibrationModeMenuCombo->combo();
	calibrationModeCombo->addItem("Continuous", QVariant(0));
	calibrationModeCombo->addItem("One Shot", QVariant(1));
	connectMenuComboToNumber(calibrationModeMenuCombo, calibrationMode);
	#pragma endregion

	QWidget *calibrationCycleCountWidget = new QWidget(calibrationDatasetConfigCollapseSection);
	QVBoxLayout *calibrationCycleCountLayout = new QVBoxLayout(calibrationCycleCountWidget);
	calibrationCycleCountWidget->setLayout(calibrationCycleCountLayout);
	calibrationCycleCountLayout->setMargin(0);
	calibrationCycleCountLayout->setSpacing(0);
	QLabel *calibrationCycleCountLabel = new QLabel("Cycle Count", calibrationCycleCountWidget);
	QLineEdit *calibrationCycleCountLineEdit = new QLineEdit(calibrationCycleCountWidget);
	calibrationCycleCountLineEdit->setText(QString::number(cycleCount));
	connectLineEditToNumber(calibrationCycleCountLineEdit, cycleCount, 1, 1000);
	calibrationCycleCountLayout->addWidget(calibrationCycleCountLabel);
	calibrationCycleCountLayout->addWidget(calibrationCycleCountLineEdit);

	QWidget *calibrationSamplesPerCycleWidget = new QWidget(calibrationDatasetConfigCollapseSection);
	QVBoxLayout *calibrationSamplesPerCycleLayout = new QVBoxLayout(calibrationSamplesPerCycleWidget);
	calibrationSamplesPerCycleWidget->setLayout(calibrationSamplesPerCycleLayout);
	calibrationSamplesPerCycleLayout->setMargin(0);
	calibrationSamplesPerCycleLayout->setSpacing(0);
	QLabel *calibrationSamplesPerCycleLabel = new QLabel("Samples Per Cycle", calibrationSamplesPerCycleWidget);
	QLineEdit *calibrationSamplesPerCycleLineEdit = new QLineEdit(calibrationSamplesPerCycleWidget);
	calibrationSamplesPerCycleLineEdit->setText(QString::number(samplesPerCycle));
	connectLineEditToNumber(calibrationSamplesPerCycleLineEdit, samplesPerCycle, 1, 5000);
	calibrationSamplesPerCycleLayout->addWidget(calibrationSamplesPerCycleLabel);
	calibrationSamplesPerCycleLayout->addWidget(calibrationSamplesPerCycleLineEdit);

	calibrationDatasetConfigCollapseSection->contentLayout()->setSpacing(8);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationModeMenuCombo);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountWidget);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationSamplesPerCycleWidget);

	#pragma endregion

	#pragma region Calibration Data Section Widget
	MenuSectionWidget *calibrationDataSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationDataSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *calibrationDataCollapseSection = new MenuCollapseSection("Calibration Data", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationDataSectionWidget);
	calibrationDataSectionWidget->contentLayout()->setSpacing(8);
	calibrationDataSectionWidget->contentLayout()->addWidget(calibrationDataCollapseSection);

	QPushButton *importSamplesButton = new QPushButton("Import Samples", calibrationDataCollapseSection);
	QPushButton *extractDataButton = new QPushButton("Save to CSV", calibrationDataCollapseSection);
	StyleHelper::BasicButton(importSamplesButton);
	StyleHelper::BasicButton(extractDataButton);

	calibrationDataCollapseSection->contentLayout()->setSpacing(8);
	calibrationDataCollapseSection->contentLayout()->addWidget(importSamplesButton);
	calibrationDataCollapseSection->contentLayout()->addWidget(extractDataButton);
	#pragma endregion

	#pragma region Motor Control Section Widget
	MenuSectionWidget *motorControlSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(motorControlSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *motorControlCollapseSection = new MenuCollapseSection("Motor Control", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, motorControlSectionWidget);
	motorControlSectionWidget->contentLayout()->setSpacing(8);
	motorControlSectionWidget->contentLayout()->addWidget(motorControlCollapseSection);

	QWidget *motorRPMWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorRPMLayout = new QVBoxLayout(motorRPMWidget);
	motorRPMWidget->setLayout(motorRPMLayout);
	motorRPMLayout->setMargin(0);
	motorRPMLayout->setSpacing(0);
	QLabel *motorRPMLabel = new QLabel("Motor RPM", motorRPMWidget);
	calibrationMotorRPMLineEdit = new QLineEdit(QString::number(motor_rpm), motorRPMWidget);
	connectLineEditToRPM(calibrationMotorRPMLineEdit, motor_rpm);
	motorRPMLayout->addWidget(motorRPMLabel);
	motorRPMLayout->addWidget(calibrationMotorRPMLineEdit);

	QWidget *currentPositionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *currentPositionLayout = new QVBoxLayout(currentPositionWidget);
	currentPositionWidget->setLayout(currentPositionLayout);
	currentPositionLayout->setMargin(0);
	currentPositionLayout->setSpacing(0);
	QLabel *currentPositionLabel = new QLabel("Current Position", currentPositionWidget);
	calibrationMotorCurrentPositionLineEdit = new QLineEdit("--.--", currentPositionWidget);
	calibrationMotorCurrentPositionLineEdit->setReadOnly(true);
	connectLineEditToDouble(calibrationMotorCurrentPositionLineEdit, current_pos);
	currentPositionLayout->addWidget(currentPositionLabel);
	currentPositionLayout->addWidget(calibrationMotorCurrentPositionLineEdit);

	QWidget *targetPositionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *targetPositionLayout = new QVBoxLayout(targetPositionWidget);
	targetPositionWidget->setLayout(targetPositionLayout);
	targetPositionLayout->setMargin(0);
	targetPositionLayout->setSpacing(0);
	QLabel *targetPositionLabel = new QLabel("Target Position", targetPositionWidget);
	motorTargetPositionLineEdit = new QLineEdit(QString::number(target_pos), targetPositionWidget);
	connectLineEditToNumberWrite(motorTargetPositionLineEdit, target_pos, ADMTController::MotorAttribute::TARGET_POS);
	targetPositionLayout->addWidget(targetPositionLabel);
	targetPositionLayout->addWidget(motorTargetPositionLineEdit);

	QWidget *motorDirectionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorDirectionLayout = new QVBoxLayout(motorDirectionWidget);
	motorDirectionWidget->setLayout(motorDirectionLayout);
	motorDirectionLayout->setMargin(0);
	motorDirectionLayout->setSpacing(0);
	QLabel *motorDirectionLabel = new QLabel("Rotation Direction", motorDirectionWidget);
	calibrationMotorDirectionSwitch = new CustomSwitch("CW", "CCW", motorControlSectionWidget);
	calibrationMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
	connect(calibrationMotorDirectionSwitch, &CustomSwitch::toggled, this, [=](bool b){
		isMotorRotationClockwise = b;
	});
	motorDirectionLayout->addWidget(motorDirectionLabel);
	motorDirectionLayout->addWidget(calibrationMotorDirectionSwitch);

	QPushButton *continuousRotationButton = new QPushButton("Continuous Rotation", motorControlSectionWidget);
	StyleHelper::BasicButton(continuousRotationButton);
	connect(continuousRotationButton, &QPushButton::clicked, this, &HarmonicCalibration::moveMotorContinuous);

	QPushButton *stopMotorButton = new QPushButton("Stop Motor", motorControlSectionWidget);
	StyleHelper::BasicButton(stopMotorButton);
	connect(stopMotorButton, &QPushButton::clicked, this, &HarmonicCalibration::stopMotor);

	motorControlCollapseSection->contentLayout()->setSpacing(8);
	motorControlCollapseSection->contentLayout()->addWidget(motorRPMWidget);
	motorControlCollapseSection->contentLayout()->addWidget(currentPositionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(targetPositionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(motorDirectionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(continuousRotationButton);
	motorControlCollapseSection->contentLayout()->addWidget(stopMotorButton);
	#pragma endregion

	#pragma region Logs Section Widget
	MenuSectionWidget *logsSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(logsSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *logsCollapseSection = new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, logsSectionWidget);
	logsCollapseSection->header()->toggle();
	logsSectionWidget->contentLayout()->setSpacing(8);
	logsSectionWidget->contentLayout()->addWidget(logsCollapseSection);

	logsPlainTextEdit = new QPlainTextEdit(logsSectionWidget);
	logsPlainTextEdit->setReadOnly(true);
	logsPlainTextEdit->setFixedHeight(320);
	logsPlainTextEdit->setStyleSheet("QPlainTextEdit { border: none; }");

	logsCollapseSection->contentLayout()->setSpacing(8);
	logsCollapseSection->contentLayout()->addWidget(logsPlainTextEdit);
	#pragma endregion

	calibrationSettingsLayout->setMargin(0);
	calibrationSettingsLayout->addWidget(calibrationDatasetConfigSectionWidget);
	calibrationSettingsLayout->addWidget(motorControlSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationCoeffSectionWidget);
	// calibrationSettingsLayout->addWidget(motorConfigurationSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationDataSectionWidget);
	calibrationSettingsLayout->addWidget(logsSectionWidget);
	calibrationSettingsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	#pragma endregion

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(false);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(false);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(false);
	tool->setLeftContainerWidth(270);
	tool->setRightContainerWidth(270);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->addWidgetToCentralContainerHelper(calibrationDataGraphWidget);
	tool->rightStack()->add("calibrationSettingsGroupWidget", calibrationSettingsGroupWidget);

	connect(extractDataButton, &QPushButton::clicked, this, &HarmonicCalibration::extractCalibrationData);
	connect(importSamplesButton, &QPushButton::clicked, this, &HarmonicCalibration::importCalibrationData);
	connect(clearCalibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::resetAllCalibrationState);
	connect(toggleAngleButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleSineButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		calibrationRawDataPlotWidget->selectChannel(calibrationSineDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleCosineButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		calibrationRawDataPlotWidget->selectChannel(calibrationCosineDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostAngleButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationRawDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostSineButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationSineDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostCosineButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationCosineDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleFFTAngleErrorMagnitudeButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		FFTAngleErrorPlotWidget->selectChannel(FFTAngleErrorMagnitudeChannel);
		FFTAngleErrorPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleFFTAngleErrorPhaseButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		FFTAngleErrorPlotWidget->selectChannel(FFTAngleErrorPhaseChannel);
		FFTAngleErrorPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleFFTCorrectedErrorMagnitudeButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		FFTCorrectedErrorPlotWidget->selectChannel(FFTCorrectedErrorMagnitudeChannel);
		FFTCorrectedErrorPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleFFTCorrectedErrorPhaseButton->checkBox(), &QCheckBox::toggled, this, [=](bool b){
		FFTCorrectedErrorPlotWidget->selectChannel(FFTCorrectedErrorPhaseChannel);
		FFTCorrectedErrorPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(calibrationDisplayFormatSwitch, &CustomSwitch::toggled, this, [=](bool b){
		isAngleDisplayFormat = b;
		displayCalculatedCoeff();
	});

	connect(&m_calibrationWaitVelocityWatcher, &QFutureWatcher<void>::finished, this, [=]() {
		if(isMotorVelocityReached)
		{
			startCalibrationStreamThread();
		}
		else
		{
			startCurrentMotorPositionMonitor();
			startDeviceStatusMonitor();
		}
	});

	connect(&m_resetMotorToZeroWatcher, &QFutureWatcher<void>::finished, this, [=]() {
		startWaitForVelocityReachedThread(1);
	});

	connect(&m_calibrationStreamWatcher, &QFutureWatcher<void>::started, [=]() {
		QThread *thread = QThread::currentThread();
		thread->setPriority(QThread::TimeCriticalPriority);
	});

	connect(&m_calibrationStreamWatcher, &QFutureWatcher<void>::finished, this, [=]() {
		stopMotor();
		isStartMotor = false;
		toggleTabSwitching(true);
		toggleCalibrationControls(true);
		
		QString log = "Calibration Stream Timing: \n";
		for(auto& value : m_admtController->streamBufferedIntervals) { log += QString::number(value) + "\n"; }
		Q_EMIT calibrationLogWriteSignal(log);
		
		if(isPostCalibration)
		{
			graphPostDataList = m_admtController->streamBufferedValues;
			postCalibrationRawDataPlotChannel->curve()->setSamples(graphPostDataList);
			postCalibrationRawDataPlotWidget->replot();
			if(static_cast<int>(graphPostDataList.size()) == totalSamplesCount) 
			{
				computeSineCosineOfAngles(graphPostDataList);
				m_admtController->postcalibrate(vector<double>(graphPostDataList.begin(), graphPostDataList.end()), cycleCount, samplesPerCycle, !isMotorRotationClockwise);
				populateCorrectedAngleErrorGraphs();
				isPostCalibration = false;
				isStartMotor = false;
				resetToZero = true;
				toggleCalibrationButtonState(4);
			}
		}
		else
		{
			graphDataList = m_admtController->streamBufferedValues;
			calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
			calibrationRawDataPlotWidget->replot();
			if(static_cast<int>(graphDataList.size()) >= totalSamplesCount) 
			{
				computeSineCosineOfAngles(graphDataList);
				calibrationLogWrite(m_admtController->calibrate(vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount, samplesPerCycle, !isMotorRotationClockwise));
				populateAngleErrorGraphs();
				calculateHarmonicValues();
				toggleCalibrationButtonState(2);
			}
			else
			{
				resetToZero = true;
				toggleCalibrationButtonState(0);
			}
		}

		startCurrentMotorPositionMonitor();
		startDeviceStatusMonitor();
	});



	return tool;
}

ToolTemplate* HarmonicCalibration::createRegistersWidget()
{
	ToolTemplate *tool = new ToolTemplate(this);

	QScrollArea *registerScrollArea = new QScrollArea();
	QWidget *registerWidget = new QWidget(registerScrollArea);
	registerWidget->setContentsMargins(0, 0, Style::getDimension(json::global::unit_0_5), 0);
	QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
	registerScrollArea->setWidgetResizable(true);
	registerScrollArea->setWidget(registerWidget);
	registerWidget->setLayout(registerLayout);
	registerLayout->setMargin(0);
	registerLayout->setSpacing(globalSpacingSmall);

	MenuCollapseSection *registerConfigurationCollapseSection = new MenuCollapseSection("Configuration", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerConfigurationGridWidget = new QWidget(registerWidget);
	QGridLayout *registerConfigurationGridLayout = new QGridLayout(registerConfigurationGridWidget);
	registerConfigurationGridWidget->setLayout(registerConfigurationGridLayout);
	registerConfigurationGridLayout->setMargin(0);
	registerConfigurationGridLayout->setSpacing(globalSpacingSmall);
	registerConfigurationCollapseSection->contentLayout()->addWidget(registerConfigurationGridWidget);

	MenuCollapseSection *registerSensorDataCollapseSection = new MenuCollapseSection("Sensor Data", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerSensorDataGridWidget = new QWidget(registerWidget);
	QGridLayout *registerSensorDataGridLayout = new QGridLayout(registerSensorDataGridWidget);
	registerSensorDataGridWidget->setLayout(registerSensorDataGridLayout);
	registerSensorDataGridLayout->setMargin(0);
	registerSensorDataGridLayout->setSpacing(globalSpacingSmall);
	registerSensorDataCollapseSection->contentLayout()->addWidget(registerSensorDataGridWidget);

	MenuCollapseSection *registerDeviceIDCollapseSection = new MenuCollapseSection("Device ID", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerDeviceIDGridWidget = new QWidget(registerWidget);
	QGridLayout *registerDeviceIDGridLayout = new QGridLayout(registerDeviceIDGridWidget);
	registerDeviceIDGridWidget->setLayout(registerDeviceIDGridLayout);
	registerDeviceIDGridLayout->setMargin(0);
	registerDeviceIDGridLayout->setSpacing(globalSpacingSmall);
	registerDeviceIDCollapseSection->contentLayout()->addWidget(registerDeviceIDGridWidget);

	MenuCollapseSection *registerHarmonicsCollapseSection = new MenuCollapseSection("Harmonics", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerHarmonicsGridWidget = new QWidget(registerWidget);
	QGridLayout *registerHarmonicsGridLayout = new QGridLayout(registerHarmonicsGridWidget);
	registerHarmonicsGridWidget->setLayout(registerHarmonicsGridLayout);
	registerHarmonicsGridLayout->setMargin(0);
	registerHarmonicsGridLayout->setSpacing(globalSpacingSmall);
	registerHarmonicsCollapseSection->contentLayout()->addWidget(registerHarmonicsGridWidget);

	cnvPageRegisterBlock = new RegisterBlockWidget("CNVPAGE", "Convert Start and Page Select", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::CNVPAGE), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	digIORegisterBlock = new RegisterBlockWidget("DIGIO", "Digital Input Output", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIO), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIO), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	faultRegisterBlock = new RegisterBlockWidget("FAULT", "Fault Register", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::FAULT), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	generalRegisterBlock = new RegisterBlockWidget("GENERAL", "General Device Configuration", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	digIOEnRegisterBlock = new RegisterBlockWidget("DIGIOEN", "Enable Digital Input/Outputs", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	angleCkRegisterBlock = new RegisterBlockWidget("ANGLECK", "Primary vs Secondary Angle Check", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ANGLECK), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ANGLECK), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	eccDcdeRegisterBlock = new RegisterBlockWidget("ECCDCDE", "Error Correction Codes", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ECCDCDE), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ECCDCDE), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	eccDisRegisterBlock = new RegisterBlockWidget("ECCDIS", "Error Correction Code disable", m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ECCDIS), m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ECCDIS), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);

	absAngleRegisterBlock = new RegisterBlockWidget("ABSANGLE", "Absolute Angle", m_admtController->getSensorRegister(ADMTController::SensorRegister::ABSANGLE), m_admtController->getSensorPage(ADMTController::SensorRegister::ABSANGLE), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	angleRegisterBlock = new RegisterBlockWidget("ANGLE", "Angle Register", m_admtController->getSensorRegister(ADMTController::SensorRegister::ANGLEREG), m_admtController->getSensorPage(ADMTController::SensorRegister::ANGLEREG), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	angleSecRegisterBlock = new RegisterBlockWidget("ANGLESEC", "Secondary Angle", m_admtController->getSensorRegister(ADMTController::SensorRegister::ANGLESEC), m_admtController->getSensorPage(ADMTController::SensorRegister::ANGLESEC), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	sineRegisterBlock = new RegisterBlockWidget("SINE", "Sine Measurement", m_admtController->getSensorRegister(ADMTController::SensorRegister::SINE), m_admtController->getSensorPage(ADMTController::SensorRegister::SINE), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	cosineRegisterBlock = new RegisterBlockWidget("COSINE", "Cosine Measurement", m_admtController->getSensorRegister(ADMTController::SensorRegister::COSINE), m_admtController->getSensorPage(ADMTController::SensorRegister::COSINE), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	secAnglIRegisterBlock = new RegisterBlockWidget("SECANGLI", "In-phase secondary angle measurement", m_admtController->getSensorRegister(ADMTController::SensorRegister::SECANGLI), m_admtController->getSensorPage(ADMTController::SensorRegister::SECANGLI), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	secAnglQRegisterBlock = new RegisterBlockWidget("SECANGLQ", "Quadrature phase secondary angle measurement", m_admtController->getSensorRegister(ADMTController::SensorRegister::SECANGLQ), m_admtController->getSensorPage(ADMTController::SensorRegister::SECANGLQ), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	radiusRegisterBlock = new RegisterBlockWidget("RADIUS", "Angle measurement radius", m_admtController->getSensorRegister(ADMTController::SensorRegister::RADIUS), m_admtController->getSensorPage(ADMTController::SensorRegister::RADIUS), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	diag1RegisterBlock = new RegisterBlockWidget("DIAG1", "State of the MT reference resistors and AFE fixed input voltage", m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG1), m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG1), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	diag2RegisterBlock = new RegisterBlockWidget("DIAG2", "Measurements of two diagnostics resistors of fixed value", m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG2), m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG2), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	tmp0RegisterBlock = new RegisterBlockWidget("TMP0", "Primary Temperature Sensor", m_admtController->getSensorRegister(ADMTController::SensorRegister::TMP0), m_admtController->getSensorPage(ADMTController::SensorRegister::TMP0), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	tmp1RegisterBlock = new RegisterBlockWidget("TMP1", "Secondary Temperature Sensor", m_admtController->getSensorRegister(ADMTController::SensorRegister::TMP1), m_admtController->getSensorPage(ADMTController::SensorRegister::TMP1), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	cnvCntRegisterBlock = new RegisterBlockWidget("CNVCNT", "Conversion Count", m_admtController->getSensorRegister(ADMTController::SensorRegister::CNVCNT), m_admtController->getSensorPage(ADMTController::SensorRegister::CNVCNT), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	uniqID0RegisterBlock = new RegisterBlockWidget("UNIQID0", "Unique ID Register 0", m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID0), m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID0), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID1RegisterBlock = new RegisterBlockWidget("UNIQID1", "Unique ID Register 1", m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID1), m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID1), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID2RegisterBlock = new RegisterBlockWidget("UNIQID2", "Unique ID Register 2", m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID2), m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID2), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID3RegisterBlock = new RegisterBlockWidget("UNIQID3", "Product, voltage supply. ASIL and ASIC revision identifiers", m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID3), m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID3), RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	h1MagRegisterBlock = new RegisterBlockWidget("H1MAG", "1st Harmonic error magnitude", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1MAG), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h1PhRegisterBlock = new RegisterBlockWidget("H1PH", "1st Harmonic error phase", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1PH), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h2MagRegisterBlock = new RegisterBlockWidget("H2MAG", "2nd Harmonic error magnitude", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H2MAG), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h2PhRegisterBlock = new RegisterBlockWidget("H2PH", "2nd Harmonic error phase", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H2PH), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h3MagRegisterBlock = new RegisterBlockWidget("H3MAG", "3rd Harmonic error magnitude", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H3MAG), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h3PhRegisterBlock = new RegisterBlockWidget("H3PH", "3rd Harmonic error phase", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H3PH), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h8MagRegisterBlock = new RegisterBlockWidget("H8MAG", "8th Harmonic error magnitude", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H8MAG), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h8PhRegisterBlock = new RegisterBlockWidget("H8PH", "8th Harmonic error phase", m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H8PH), RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	
	registerConfigurationGridLayout->addWidget(cnvPageRegisterBlock, 0, 0);
	registerConfigurationGridLayout->addWidget(digIORegisterBlock, 0, 1);
	registerConfigurationGridLayout->addWidget(faultRegisterBlock, 0, 2);
	registerConfigurationGridLayout->addWidget(generalRegisterBlock, 0, 3);
	registerConfigurationGridLayout->addWidget(digIOEnRegisterBlock, 0, 4);
	registerConfigurationGridLayout->addWidget(eccDcdeRegisterBlock, 1, 0);
	registerConfigurationGridLayout->addWidget(eccDisRegisterBlock, 1, 1);
	registerConfigurationGridLayout->addWidget(angleCkRegisterBlock, 1, 2);

	registerSensorDataGridLayout->addWidget(absAngleRegisterBlock, 0, 0);
	registerSensorDataGridLayout->addWidget(angleRegisterBlock, 0, 1);
	registerSensorDataGridLayout->addWidget(sineRegisterBlock, 0, 2);
	registerSensorDataGridLayout->addWidget(cosineRegisterBlock, 0, 3);
	registerSensorDataGridLayout->addWidget(cnvCntRegisterBlock, 0, 4);
	registerSensorDataGridLayout->addWidget(tmp0RegisterBlock, 1, 0);
	registerSensorDataGridLayout->addWidget(tmp1RegisterBlock, 1, 1);
	registerSensorDataGridLayout->addWidget(diag1RegisterBlock, 1, 2);
	registerSensorDataGridLayout->addWidget(diag2RegisterBlock, 1, 3);
	registerSensorDataGridLayout->addWidget(radiusRegisterBlock, 1, 4);
	registerSensorDataGridLayout->addWidget(angleSecRegisterBlock, 2, 0);
	registerSensorDataGridLayout->addWidget(secAnglIRegisterBlock, 2, 1);
	registerSensorDataGridLayout->addWidget(secAnglQRegisterBlock, 2, 2);

	registerDeviceIDGridLayout->addWidget(uniqID0RegisterBlock, 0, 0);
	registerDeviceIDGridLayout->addWidget(uniqID1RegisterBlock, 0, 1);
	registerDeviceIDGridLayout->addWidget(uniqID2RegisterBlock, 0, 2);
	registerDeviceIDGridLayout->addWidget(uniqID3RegisterBlock, 0, 3);
	QSpacerItem *registerDeviceSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
	registerDeviceIDGridLayout->addItem(registerDeviceSpacer, 0, 4);

	registerHarmonicsGridLayout->addWidget(h1MagRegisterBlock, 0, 0);
	registerHarmonicsGridLayout->addWidget(h1PhRegisterBlock, 0, 1);
	registerHarmonicsGridLayout->addWidget(h2MagRegisterBlock, 0, 2);
	registerHarmonicsGridLayout->addWidget(h2PhRegisterBlock, 0, 3);
	registerHarmonicsGridLayout->addWidget(h3MagRegisterBlock, 0, 4);
	registerHarmonicsGridLayout->addWidget(h3PhRegisterBlock, 1, 0);
	registerHarmonicsGridLayout->addWidget(h8MagRegisterBlock, 1, 1);
	registerHarmonicsGridLayout->addWidget(h8PhRegisterBlock, 1, 2);
	
	for(int c=0; c < registerConfigurationGridLayout->columnCount(); ++c) registerConfigurationGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerDeviceIDGridLayout->columnCount(); ++c) registerDeviceIDGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerHarmonicsGridLayout->columnCount(); ++c) registerHarmonicsGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerSensorDataGridLayout->columnCount(); ++c) registerSensorDataGridLayout->setColumnStretch(c,1);

	QWidget *registerActionsWidget = new QWidget(registerWidget);
	QHBoxLayout *registerActionsLayout = new QHBoxLayout(registerActionsWidget);
	registerActionsWidget->setLayout(registerActionsLayout);
	registerActionsLayout->setMargin(0);
	registerActionsLayout->setSpacing(globalSpacingSmall);
	readAllRegistersButton = new QPushButton("Read All Registers", registerWidget);
	StyleHelper::BasicButton(readAllRegistersButton);
	readAllRegistersButton->setFixedWidth(270);
	connect(readAllRegistersButton, &QPushButton::clicked, this, &HarmonicCalibration::readAllRegisters);
	registerActionsLayout->addWidget(readAllRegistersButton);

	registerLayout->addWidget(registerConfigurationCollapseSection);
	registerLayout->addWidget(registerSensorDataCollapseSection);
	registerLayout->addWidget(registerDeviceIDCollapseSection);
	registerLayout->addWidget(registerHarmonicsCollapseSection);
	registerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(false);
	tool->rightContainer()->setVisible(false);
	tool->bottomContainer()->setVisible(false);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->addWidgetToTopContainerHelper(registerActionsWidget, ToolTemplateAlignment::TTA_LEFT);
	tool->addWidgetToCentralContainerHelper(registerScrollArea);

	tool->layout()->setMargin(0);
	tool->layout()->setContentsMargins(globalSpacingSmall, globalSpacingSmall, Style::getDimension(json::global::unit_0_5), globalSpacingSmall);

	connectRegisterBlockToRegistry(cnvPageRegisterBlock);
	connectRegisterBlockToRegistry(digIORegisterBlock);
	connectRegisterBlockToRegistry(faultRegisterBlock);
	connectRegisterBlockToRegistry(generalRegisterBlock);
	connectRegisterBlockToRegistry(digIOEnRegisterBlock);
	connectRegisterBlockToRegistry(angleCkRegisterBlock);
	connectRegisterBlockToRegistry(eccDcdeRegisterBlock);
	connectRegisterBlockToRegistry(eccDisRegisterBlock);

	connectRegisterBlockToRegistry(absAngleRegisterBlock);
	connectRegisterBlockToRegistry(angleRegisterBlock);
	connectRegisterBlockToRegistry(angleSecRegisterBlock);
	connectRegisterBlockToRegistry(sineRegisterBlock);
	connectRegisterBlockToRegistry(cosineRegisterBlock);
	connectRegisterBlockToRegistry(secAnglIRegisterBlock);
	connectRegisterBlockToRegistry(secAnglQRegisterBlock);
	connectRegisterBlockToRegistry(radiusRegisterBlock);
	connectRegisterBlockToRegistry(diag1RegisterBlock);
	connectRegisterBlockToRegistry(diag2RegisterBlock);
	connectRegisterBlockToRegistry(tmp0RegisterBlock);
	connectRegisterBlockToRegistry(tmp1RegisterBlock);
	connectRegisterBlockToRegistry(cnvCntRegisterBlock);

	connectRegisterBlockToRegistry(uniqID0RegisterBlock);
	connectRegisterBlockToRegistry(uniqID1RegisterBlock);
	connectRegisterBlockToRegistry(uniqID2RegisterBlock);
	connectRegisterBlockToRegistry(uniqID3RegisterBlock);

	connectRegisterBlockToRegistry(h1MagRegisterBlock);
	connectRegisterBlockToRegistry(h1PhRegisterBlock);
	connectRegisterBlockToRegistry(h2MagRegisterBlock);
	connectRegisterBlockToRegistry(h2PhRegisterBlock);
	connectRegisterBlockToRegistry(h3MagRegisterBlock);
	connectRegisterBlockToRegistry(h3PhRegisterBlock);
	connectRegisterBlockToRegistry(h8MagRegisterBlock);
	connectRegisterBlockToRegistry(h8PhRegisterBlock);

	return tool;
}

ToolTemplate* HarmonicCalibration::createUtilityWidget()
{
	ToolTemplate *tool = new ToolTemplate(this);

	#pragma region Left Utility Widget
	QWidget *leftUtilityWidget = new QWidget(this);
	QVBoxLayout *leftUtilityLayout = new QVBoxLayout(leftUtilityWidget);
	leftUtilityWidget->setLayout(leftUtilityLayout);
	leftUtilityLayout->setMargin(0);
	leftUtilityLayout->setSpacing(8);
	#pragma region Command Log Widget
	MenuSectionWidget *commandLogSectionWidget = new MenuSectionWidget(leftUtilityWidget);
	Style::setStyle(commandLogSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *commandLogCollapseSection = new MenuCollapseSection("Command Log", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, commandLogSectionWidget);
	commandLogSectionWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	commandLogSectionWidget->contentLayout()->addWidget(commandLogCollapseSection);
	commandLogCollapseSection->contentLayout()->setSpacing(8);

	commandLogPlainTextEdit = new QPlainTextEdit(commandLogSectionWidget);
	commandLogPlainTextEdit->setReadOnly(true);
	commandLogPlainTextEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	commandLogPlainTextEdit->setStyleSheet("QPlainTextEdit { border: none; }");

	clearCommandLogButton = new QPushButton("Clear Command Logs", commandLogSectionWidget);
	StyleHelper::BasicButton(clearCommandLogButton);
	connect(clearCommandLogButton, &QPushButton::clicked, this, &HarmonicCalibration::clearCommandLog);

	commandLogCollapseSection->contentLayout()->addWidget(commandLogPlainTextEdit);
	commandLogCollapseSection->contentLayout()->addWidget(clearCommandLogButton);

	leftUtilityLayout->addWidget(commandLogSectionWidget, 1);
	leftUtilityLayout->addStretch();
	#pragma endregion
	#pragma endregion

	#pragma region Center Utility Widget
	QWidget *centerUtilityWidget = new QWidget(this);
	QHBoxLayout *centerUtilityLayout = new QHBoxLayout(centerUtilityWidget);
	centerUtilityWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	centerUtilityWidget->setContentsMargins(2, 0, 2, 0);
	centerUtilityWidget->setLayout(centerUtilityLayout);
	centerUtilityLayout->setMargin(0);
	centerUtilityLayout->setSpacing(8);

	QScrollArea *DIGIOScrollArea = new QScrollArea(centerUtilityWidget);
	QWidget *DIGIOWidget = new QWidget(DIGIOScrollArea);
	DIGIOScrollArea->setWidget(DIGIOWidget);
	DIGIOScrollArea->setWidgetResizable(true);
	QVBoxLayout *DIGIOLayout = new QVBoxLayout(DIGIOWidget);
	DIGIOWidget->setLayout(DIGIOLayout);
	DIGIOLayout->setMargin(0);
	DIGIOLayout->setSpacing(8);

	#pragma region DIGIO Monitor
	MenuSectionWidget *DIGIOMonitorSectionWidget = new MenuSectionWidget(DIGIOWidget);
	Style::setStyle(DIGIOMonitorSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *DIGIOMonitorCollapseSection = new MenuCollapseSection("DIGIO Monitor", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, DIGIOMonitorSectionWidget);
	DIGIOMonitorSectionWidget->contentLayout()->addWidget(DIGIOMonitorCollapseSection);
	DIGIOMonitorCollapseSection->contentLayout()->setSpacing(8);

	DIGIOBusyStatusLED = createStatusLEDWidget("BUSY (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOCNVStatusLED = createStatusLEDWidget("CNV (Input)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOSENTStatusLED = createStatusLEDWidget("SENT (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOACALCStatusLED = createStatusLEDWidget("ACALC (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOFaultStatusLED = createStatusLEDWidget("FAULT (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOBootloaderStatusLED = createStatusLEDWidget("BOOTLOADER (Output)", "digio", false, DIGIOMonitorCollapseSection);

	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOBusyStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOCNVStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOSENTStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOACALCStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOFaultStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOBootloaderStatusLED);
	#pragma endregion

	#pragma region DIGIO Control
	MenuSectionWidget *DIGIOControlSectionWidget = new MenuSectionWidget(DIGIOWidget);
	Style::setStyle(DIGIOControlSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *DIGIOControlCollapseSection = new MenuCollapseSection("DIGIO Control", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, DIGIOControlSectionWidget);
	DIGIOControlCollapseSection->contentLayout()->setSpacing(8);
	DIGIOControlSectionWidget->contentLayout()->addWidget(DIGIOControlCollapseSection);

	QWidget *DIGIOControlGridWidget = new QWidget(DIGIOControlCollapseSection);
	QGridLayout *DIGIOControlGridLayout = new QGridLayout(DIGIOControlGridWidget);
	DIGIOControlGridWidget->setLayout(DIGIOControlGridLayout);
	DIGIOControlGridLayout->setMargin(0);
	DIGIOControlGridLayout->setSpacing(8);

	QLabel *DIGIO0Label = new QLabel("DIGIO0", DIGIOControlGridWidget);
	QLabel *DIGIO1Label = new QLabel("DIGIO1", DIGIOControlGridWidget);
	QLabel *DIGIO2Label = new QLabel("DIGIO2", DIGIOControlGridWidget);
	QLabel *DIGIO3Label = new QLabel("DIGIO3", DIGIOControlGridWidget);
	QLabel *DIGIO4Label = new QLabel("DIGIO4", DIGIOControlGridWidget);
	QLabel *DIGIO5Label = new QLabel("DIGIO5", DIGIOControlGridWidget);
	QLabel *DIGIOFunctionLabel = new QLabel("DIGIO Function", DIGIOControlGridWidget);
	QLabel *GPIOModeLabel = new QLabel("GPIO Mode", DIGIOControlGridWidget);

	DIGIO0ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO0ENToggleSwitch, "Output", "Input");
	connect(DIGIO0ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO0EN", value);
	});

	DIGIO0FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO0FNCToggleSwitch, "GPIO0", "BUSY");
	connect(DIGIO0FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("BUSY", value);
	});

	DIGIO1ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO1ENToggleSwitch, "Output", "Input");
	connect(DIGIO1ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO1EN", value);
	});

	DIGIO1FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO1FNCToggleSwitch, "GPIO1", "CNV");
	connect(DIGIO1FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("CNV", value);
	});

	DIGIO2ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO2ENToggleSwitch, "Output", "Input");
	connect(DIGIO2ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO2EN", value);
	});

	DIGIO2FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO2FNCToggleSwitch, "GPIO2", "SENT");
	connect(DIGIO2FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("SENT", value);
	});

	DIGIO3ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO3ENToggleSwitch, "Output", "Input");
	connect(DIGIO3ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO3EN", value);
	});

	DIGIO3FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO3FNCToggleSwitch, "GPIO3", "ACALC");
	connect(DIGIO3FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("ACALC", value);
	});

	DIGIO4ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO4ENToggleSwitch, "Output", "Input");
	connect(DIGIO4ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO4EN", value);
	});

	DIGIO4FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO4FNCToggleSwitch, "GPIO4", "FAULT");
	connect(DIGIO4FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("FAULT", value);
	});

	DIGIO5ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO5ENToggleSwitch, "Output", "Input");
	connect(DIGIO5ENToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("DIGIO5EN", value);
	});

	DIGIO5FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO5FNCToggleSwitch, "GPIO5", "BOOT");
	connect(DIGIO5FNCToggleSwitch, &CustomSwitch::clicked, [=](bool value){
		toggleDIGIOEN("BOOTLOAD", value);
	});

	QPushButton *DIGIOResetButton = new QPushButton("Reset DIGIO", DIGIOControlGridWidget);
	DIGIOResetButton->setFixedWidth(100);
	StyleHelper::BasicButton(DIGIOResetButton);
	connect(DIGIOResetButton, &QPushButton::clicked, [=]{
		toggleUtilityTask(false);
		resetDIGIO();
		toggleUtilityTask(true);
	});

	DIGIOControlGridLayout->addWidget(DIGIOFunctionLabel, 0, 1);
	DIGIOControlGridLayout->addWidget(GPIOModeLabel, 0, 2);

	DIGIOControlGridLayout->addWidget(DIGIO0Label, 1, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO0FNCToggleSwitch, 1, 1);
	DIGIOControlGridLayout->addWidget(DIGIO0ENToggleSwitch, 1, 2);

	DIGIOControlGridLayout->addWidget(DIGIO1Label, 2, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO1FNCToggleSwitch, 2, 1);
	DIGIOControlGridLayout->addWidget(DIGIO1ENToggleSwitch, 2, 2);

	DIGIOControlGridLayout->addWidget(DIGIO2Label, 3, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO2FNCToggleSwitch, 3, 1);
	DIGIOControlGridLayout->addWidget(DIGIO2ENToggleSwitch, 3, 2);

	DIGIOControlGridLayout->addWidget(DIGIO3Label, 4, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO3FNCToggleSwitch, 4, 1);
	DIGIOControlGridLayout->addWidget(DIGIO3ENToggleSwitch, 4, 2);

	DIGIOControlGridLayout->addWidget(DIGIO4Label, 5, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO4FNCToggleSwitch, 5, 1);
	DIGIOControlGridLayout->addWidget(DIGIO4ENToggleSwitch, 5, 2);

	DIGIOControlGridLayout->addWidget(DIGIO5Label, 6, 0, Qt::AlignLeft);
	DIGIOControlGridLayout->addWidget(DIGIO5FNCToggleSwitch, 6, 1);
	DIGIOControlGridLayout->addWidget(DIGIO5ENToggleSwitch, 6, 2);

	DIGIOControlGridLayout->setColumnStretch(0, 1);

	DIGIOControlCollapseSection->contentLayout()->addWidget(DIGIOControlGridWidget);
	DIGIOControlCollapseSection->contentLayout()->addWidget(DIGIOResetButton);

	if(GENERALRegisterMap.at("Sequence Type") == 0)
	{
		DIGIO2FNCToggleSwitch->setVisible(false);
		DIGIO4FNCToggleSwitch->setVisible(false);
	}

	#pragma endregion

	DIGIOLayout->addWidget(DIGIOMonitorSectionWidget);
	DIGIOLayout->addWidget(DIGIOControlSectionWidget);
	DIGIOLayout->addStretch();

	#pragma region MTDIAG1 Widget
	MenuSectionWidget *MTDIAG1SectionWidget = new MenuSectionWidget(centerUtilityWidget);
	Style::setStyle(MTDIAG1SectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *MTDIAG1CollapseSection = new MenuCollapseSection("MT Diagnostic Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, MTDIAG1SectionWidget);
	MTDIAG1SectionWidget->contentLayout()->addWidget(MTDIAG1CollapseSection);
	MTDIAG1CollapseSection->contentLayout()->setSpacing(8);

	R0StatusLED = createStatusLEDWidget("R0", "mtdiag", false, MTDIAG1SectionWidget);
	R1StatusLED = createStatusLEDWidget("R1", "mtdiag", false, MTDIAG1SectionWidget);
	R2StatusLED = createStatusLEDWidget("R2", "mtdiag", false, MTDIAG1SectionWidget);
	R3StatusLED = createStatusLEDWidget("R3", "mtdiag", false, MTDIAG1SectionWidget);
	R4StatusLED = createStatusLEDWidget("R4", "mtdiag", false, MTDIAG1SectionWidget);
	R5StatusLED = createStatusLEDWidget("R5", "mtdiag", false, MTDIAG1SectionWidget);
	R6StatusLED = createStatusLEDWidget("R6", "mtdiag", false, MTDIAG1SectionWidget);
	R7StatusLED = createStatusLEDWidget("R7", "mtdiag", false, MTDIAG1SectionWidget);

	MTDIAG1CollapseSection->contentLayout()->addWidget(R0StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R1StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R2StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R3StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R4StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R5StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R6StatusLED);
	MTDIAG1CollapseSection->contentLayout()->addWidget(R7StatusLED);
	#pragma endregion

	#pragma region MT Diagnostics
	MTDiagnosticsScrollArea = new QScrollArea(centerUtilityWidget);
	QWidget *MTDiagnosticsWidget = new QWidget(MTDiagnosticsScrollArea);
	MTDiagnosticsScrollArea->setWidget(MTDiagnosticsWidget);
	MTDiagnosticsScrollArea->setWidgetResizable(true);
	QVBoxLayout *MTDiagnosticsLayout = new QVBoxLayout(MTDiagnosticsWidget);
	MTDiagnosticsWidget->setLayout(MTDiagnosticsLayout);
	MTDiagnosticsLayout->setMargin(0);
	MTDiagnosticsLayout->setSpacing(8);

	MenuSectionWidget *MTDiagnosticsSectionWidget = new MenuSectionWidget(centerUtilityWidget);
	Style::setStyle(MTDiagnosticsSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *MTDiagnosticsCollapseSection = new MenuCollapseSection("MT Diagnostics", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, MTDiagnosticsSectionWidget);
	MTDiagnosticsSectionWidget->contentLayout()->addWidget(MTDiagnosticsCollapseSection);
	MTDiagnosticsCollapseSection->contentLayout()->setSpacing(8);

	QLabel *AFEDIAG0Label = new QLabel("AFEDIAG0 (%)");
	Style::setStyle(AFEDIAG0Label, style::properties::label::menuSmall);
	QLabel *AFEDIAG1Label = new QLabel("AFEDIAG1 (%)");
	Style::setStyle(AFEDIAG1Label, style::properties::label::menuSmall);
	QLabel *AFEDIAG2Label = new QLabel("AFEDIAG2 (V)");
	Style::setStyle(AFEDIAG2Label, style::properties::label::menuSmall);

	AFEDIAG0LineEdit = new QLineEdit(MTDiagnosticsSectionWidget);
	AFEDIAG1LineEdit = new QLineEdit(MTDiagnosticsSectionWidget);
	AFEDIAG2LineEdit = new QLineEdit(MTDiagnosticsSectionWidget);
	AFEDIAG0LineEdit->setReadOnly(true);
	AFEDIAG1LineEdit->setReadOnly(true);
	AFEDIAG2LineEdit->setReadOnly(true);
	connectLineEditToNumber(AFEDIAG0LineEdit, afeDiag0, "V");
	connectLineEditToNumber(AFEDIAG1LineEdit, afeDiag1, "V");
	connectLineEditToNumber(AFEDIAG2LineEdit, afeDiag2, "V");

	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG0Label);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG0LineEdit);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG1Label);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG1LineEdit);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG2Label);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG2LineEdit);

	MTDiagnosticsLayout->addWidget(MTDiagnosticsSectionWidget);
	MTDiagnosticsLayout->addWidget(MTDIAG1SectionWidget);
	MTDiagnosticsLayout->addStretch();
	#pragma endregion

	centerUtilityLayout->addWidget(DIGIOScrollArea);
	centerUtilityLayout->addWidget(MTDiagnosticsScrollArea);

	#pragma endregion

	#pragma region Right Utility Widget
	QScrollArea *rightUtilityScrollArea = new QScrollArea(this);
	QWidget *rightUtilityWidget = new QWidget(rightUtilityScrollArea);
	rightUtilityScrollArea->setWidget(rightUtilityWidget);
	rightUtilityScrollArea->setWidgetResizable(true);
	QVBoxLayout *rightUtilityLayout = new QVBoxLayout(rightUtilityWidget);
	rightUtilityWidget->setLayout(rightUtilityLayout);
	rightUtilityLayout->setMargin(0);
	rightUtilityLayout->setSpacing(8);

	MenuSectionWidget *faultRegisterSectionWidget = new MenuSectionWidget(rightUtilityWidget);
	Style::setStyle(faultRegisterSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *faultRegisterCollapseSection = new MenuCollapseSection("Fault Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, faultRegisterSectionWidget);
	faultRegisterSectionWidget->contentLayout()->addWidget(faultRegisterCollapseSection);
	faultRegisterCollapseSection->contentLayout()->setSpacing(8);
	
	VDDUnderVoltageStatusLED = createStatusLEDWidget("VDD Under Voltage", "fault", false, faultRegisterCollapseSection);
	VDDOverVoltageStatusLED = createStatusLEDWidget("VDD Over Voltage", "fault", false, faultRegisterCollapseSection);
	VDRIVEUnderVoltageStatusLED = createStatusLEDWidget("VDRIVE Under Voltage", "fault", false, faultRegisterCollapseSection);
	VDRIVEOverVoltageStatusLED = createStatusLEDWidget("VDRIVE Over Voltage", "fault", false, faultRegisterCollapseSection);
	AFEDIAGStatusLED = createStatusLEDWidget("AFEDIAG", "fault", false, faultRegisterCollapseSection);
	NVMCRCFaultStatusLED = createStatusLEDWidget("NVM CRC Fault", "fault", false, faultRegisterCollapseSection);
	ECCDoubleBitErrorStatusLED = createStatusLEDWidget("ECC Double Bit Error", "fault", false, faultRegisterCollapseSection);
	OscillatorDriftStatusLED = createStatusLEDWidget("Oscillator Drift", "fault", false, faultRegisterCollapseSection);
	CountSensorFalseStateStatusLED = createStatusLEDWidget("Count Sensor False State", "fault", false, faultRegisterCollapseSection);
	AngleCrossCheckStatusLED = createStatusLEDWidget("Angle Cross Check", "fault", false, faultRegisterCollapseSection);
	TurnCountSensorLevelsStatusLED = createStatusLEDWidget("Turn Count Sensor Levels", "fault", false, faultRegisterCollapseSection);
	MTDIAGStatusLED = createStatusLEDWidget("MTDIAG", "fault", false, faultRegisterCollapseSection);
	TurnCounterCrossCheckStatusLED = createStatusLEDWidget("Turn Counter Cross Check", "fault", false, faultRegisterCollapseSection);
	RadiusCheckStatusLED = createStatusLEDWidget("Radius Check", "fault", false, faultRegisterCollapseSection);
	SequencerWatchdogStatusLED = createStatusLEDWidget("Sequencer Watchdog", "fault", false, faultRegisterCollapseSection);

	faultRegisterCollapseSection->contentLayout()->addWidget(VDDUnderVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(VDDOverVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(VDRIVEUnderVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(VDRIVEOverVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(AFEDIAGStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(NVMCRCFaultStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(ECCDoubleBitErrorStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(OscillatorDriftStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(CountSensorFalseStateStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(AngleCrossCheckStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(TurnCountSensorLevelsStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(MTDIAGStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(TurnCounterCrossCheckStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(RadiusCheckStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(SequencerWatchdogStatusLED);

	rightUtilityLayout->addWidget(faultRegisterSectionWidget);
	rightUtilityLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	#pragma endregion
	
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(false);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(false);
	tool->setLeftContainerWidth(240);
	tool->setRightContainerWidth(224);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->leftStack()->addWidget(leftUtilityWidget);
	tool->addWidgetToCentralContainerHelper(centerUtilityWidget);
	tool->rightStack()->addWidget(rightUtilityScrollArea);

	return tool;
}

void HarmonicCalibration::readDeviceProperties()
{
	uint32_t *uniqId3RegisterValue = new uint32_t;
    uint32_t *cnvPageRegValue = new uint32_t;
    uint32_t page = m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID3);
    uint32_t cnvPageAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	bool success = false;

    if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, page) != -1){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, cnvPageRegValue) != -1){
			if(*cnvPageRegValue == page){
                if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID3), uniqId3RegisterValue) != -1){
                    deviceRegisterMap = m_admtController->getUNIQID3RegisterMapping(static_cast<uint16_t>(*uniqId3RegisterValue));

                    if(deviceRegisterMap.at("Supply ID") == "5V") { is5V = true; }
                    else if(deviceRegisterMap.at("Supply ID") == "3.3V") { is5V = false; }
                    else { is5V = false; }

                    deviceName = QString::fromStdString(deviceRegisterMap.at("Product ID"));
                    
                    if(deviceRegisterMap.at("ASIL ID") == "ASIL QM") { deviceType = QString::fromStdString("Industrial"); }
                    else if(deviceRegisterMap.at("ASIL ID") == "ASIL B") { deviceType = QString::fromStdString("Automotive"); }
                    else { deviceType = QString::fromStdString(deviceRegisterMap.at("ASIL ID")); }

					success = true;
                }
			}
		}
	}

	if(!success) { StatusBarManager::pushMessage("Failed to read device properties"); }
}

void HarmonicCalibration::initializeADMT()
{
	bool success = resetDIGIO();
	success = resetGENERAL();

	if(!success){ StatusBarManager::pushMessage("Failed initialize ADMT"); }
}

bool HarmonicCalibration::readSequence(){
	uint32_t *generalRegValue = new uint32_t;
	uint32_t generalRegisterAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL);
	uint32_t generalRegisterPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL);

	bool success = false;

	if(changeCNVPage(generalRegisterPage)){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), generalRegisterAddress, generalRegValue) != -1){
			if(*generalRegValue != UINT32_MAX){
				GENERALRegisterMap = m_admtController->getGeneralRegisterBitMapping(static_cast<uint16_t>(*generalRegValue));
				success = true;
			}
		}
	}

	return success;
}

bool HarmonicCalibration::writeSequence(const map<string, int> &settings)
{
	uint32_t *generalRegValue = new uint32_t;
	uint32_t generalRegisterAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL);

	bool success = false;

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), generalRegisterAddress, generalRegValue) != -1){

		uint32_t newGeneralRegValue = m_admtController->setGeneralRegisterBitMapping(*generalRegValue, settings);
		uint32_t generalRegisterPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL);

		if(changeCNVPage(generalRegisterPage)){
			if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), generalRegisterAddress, newGeneralRegValue) != -1){
				if(readSequence()){
					if(settings.at("Convert Synchronization") == GENERALRegisterMap.at("Convert Synchronization") &&
						settings.at("Angle Filter") == GENERALRegisterMap.at("Angle Filter") &&
						settings.at("8th Harmonic") == GENERALRegisterMap.at("8th Harmonic") &&
						settings.at("Sequence Type") == GENERALRegisterMap.at("Sequence Type") &&
						settings.at("Conversion Type") == GENERALRegisterMap.at("Conversion Type"))
					{
						success = true;
					}
				}
			}
		}
	}

	return success;
}

void HarmonicCalibration::applySequence(){
	toggleWidget(applySequenceButton, false);
	applySequenceButton->setText("Writing...");
	QTimer::singleShot(1000, this, [this](){
		this->toggleWidget(applySequenceButton, true);
		applySequenceButton->setText("Apply");
	});
	uint32_t *generalRegValue = new uint32_t;
	uint32_t generalRegisterAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL);
	map<string, int> settings;

	bool success = false;

	settings["Sequence Type"] = qvariant_cast<int>(sequenceTypeMenuCombo->combo()->currentData()); // sequenceType;
	settings["Conversion Type"] = qvariant_cast<int>(conversionTypeMenuCombo->combo()->currentData()); // conversionType;
	settings["Convert Synchronization"] = qvariant_cast<int>(convertSynchronizationMenuCombo->combo()->currentData()); // convertSync;
	settings["Angle Filter"] = qvariant_cast<int>(angleFilterMenuCombo->combo()->currentData()); // angleFilter;
	settings["8th Harmonic"] = qvariant_cast<int>(eighthHarmonicMenuCombo->combo()->currentData()); // eighthHarmonic;

	success = writeSequence(settings);
	if(!success) StatusBarManager::pushMessage("Failed to apply sequence settings"); 
	else StatusBarManager::pushMessage("Sequence settings applied successfully"); 
}

bool HarmonicCalibration::changeCNVPage(uint32_t page){
	uint32_t *cnvPageRegValue = new uint32_t;
	uint32_t cnvPageAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, page) != -1){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, cnvPageRegValue) != -1){
			if(*cnvPageRegValue == page){
				return true; 
			}
		}
	}

	return false;
}

void HarmonicCalibration::initializeMotor()
{
	motor_rpm = 60;
	rotate_vmax = convertRPStoVMAX(convertRPMtoRPS(motor_rpm));
	writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
	writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
	
	amax = convertAccelTimetoAMAX(motorAccelTime);
	writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
	readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);

	dmax = 3000;
	writeMotorAttributeValue(ADMTController::MotorAttribute::DMAX, dmax);
	readMotorAttributeValue(ADMTController::MotorAttribute::DMAX, dmax);

	ramp_mode = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);
	readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);

	target_pos = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);

	current_pos = 0;
	readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
}

void HarmonicCalibration::startDeviceStatusMonitor()
{
	if(!m_deviceStatusThread.isRunning())
	{
		isDeviceStatusMonitor = true;
		m_deviceStatusThread = QtConcurrent::run(this, &HarmonicCalibration::getDeviceFaultStatus, deviceStatusMonitorRate);
		m_deviceStatusWatcher.setFuture(m_deviceStatusThread);
	}
}

void HarmonicCalibration::stopDeviceStatusMonitor()
{
	isDeviceStatusMonitor = false;
	if(m_deviceStatusThread.isRunning())
	{
		m_deviceStatusThread.cancel();
		m_deviceStatusWatcher.waitForFinished();
	}
}

void HarmonicCalibration::getDeviceFaultStatus(int sampleRate)
{
	uint32_t *readValue = new uint32_t;
	bool registerFault = false;
	while(isDeviceStatusMonitor)
	{
		if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT), 0) == 0)
		{
			if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT), readValue) == 0)
			{
				registerFault = m_admtController->checkRegisterFault(static_cast<uint16_t>(*readValue), GENERALRegisterMap.at("Sequence Type") == 0 ? true : false);
				Q_EMIT updateFaultStatusSignal(registerFault);
			}
			else
			{
				Q_EMIT updateFaultStatusSignal(true);
			}
		}
		else
		{
			Q_EMIT updateFaultStatusSignal(true);
		}

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::requestDisconnect()
{
	stopAcquisition();

	stopAcquisitionUITask();
	stopCalibrationUITask();
	stopUtilityTask();

	stopCalibrationStreamThread();

	stopDeviceStatusMonitor();
	stopCurrentMotorPositionMonitor();
}

void HarmonicCalibration::startCurrentMotorPositionMonitor()
{
	if(!m_currentMotorPositionThread.isRunning())
	{
		isMotorPositionMonitor = true;
		m_currentMotorPositionThread = QtConcurrent::run(this, &HarmonicCalibration::currentMotorPositionTask, motorPositionMonitorRate);
		m_currentMotorPositionWatcher.setFuture(m_currentMotorPositionThread);
	}
}

void HarmonicCalibration::stopCurrentMotorPositionMonitor()
{
	isMotorPositionMonitor = false;
	if(m_currentMotorPositionThread.isRunning())
	{
		m_currentMotorPositionThread.cancel();
		m_currentMotorPositionWatcher.waitForFinished();
	}
}

void HarmonicCalibration::currentMotorPositionTask(int sampleRate)
{
	double motorPosition = 0;
	while(isMotorPositionMonitor)
	{
		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, motorPosition);

		Q_EMIT motorPositionChanged(motorPosition);

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::updateMotorPosition(double position)
{
	current_pos = position;
	if(isAcquisitionTab) updateLineEditValue(acquisitionMotorCurrentPositionLineEdit, current_pos);
	else if(isCalibrationTab) updateLineEditValue(calibrationMotorCurrentPositionLineEdit, current_pos);
}

bool HarmonicCalibration::resetGENERAL()
{
	bool success = false;

	uint32_t resetValue = 0x0000;
	if(deviceRegisterMap.at("ASIL ID") == "ASIL QM") { resetValue = 0x1231; } // Industrial or ASIL QM
	else if(deviceRegisterMap.at("ASIL ID") == "ASIL B") { resetValue = 0x1200; } // Automotive or ASIL B

	if(resetValue != 0x0000){
		if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											  m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL), resetValue) == 0) { success = true; }
	}

	return success;
}

#pragma region Acquisition Methods
bool HarmonicCalibration::updateChannelValues(){
	bool success = false;
	rotation = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), rotationChannelName, bufferSize);
	if(rotation == static_cast<double>(UINT64_MAX)) { return false; }
	angle = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), angleChannelName, bufferSize);
	if(angle == static_cast<double>(UINT64_MAX)) { return false; }
	updateCountValue();
	if(count == static_cast<double>(UINT64_MAX)) { return false; }
	temp = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), temperatureChannelName, bufferSize);
	if(temp == static_cast<double>(UINT64_MAX)) { return false; }
	return success = true;
}

void HarmonicCalibration::updateCountValue(){
	uint32_t *absAngleRegValue = new uint32_t;
	bool success = false;
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE), 0x0000) == 0){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getSensorRegister(ADMTController::SensorRegister::ABSANGLE), absAngleRegValue) == 0){
			count = m_admtController->getAbsAngleTurnCount(static_cast<uint16_t>(*absAngleRegValue));
			success = true;
		}
	}
	if(!success){ count = static_cast<double>(UINT64_MAX); }
}

void HarmonicCalibration::updateLineEditValues(){
	if(rotation == static_cast<double>(UINT64_MAX)) { rotationValueLabel->setText("N/A"); }
	else { rotationValueLabel->setText(QString::number(rotation) + "°"); }
	if(angle == static_cast<double>(UINT64_MAX)) { angleValueLabel->setText("N/A"); }
	else { angleValueLabel->setText(QString::number(angle) + "°"); }
	if(count == static_cast<double>(UINT64_MAX)) { countValueLabel->setText("N/A"); }
	else { countValueLabel->setText(QString::number(count)); }
	if(temp == static_cast<double>(UINT64_MAX)) { tempValueLabel->setText("N/A"); }
	else { tempValueLabel->setText(QString::number(temp) + " °C"); }
}

void HarmonicCalibration::startAcquisition()
{
	isStartAcquisition = true;
	acquisitionXPlotAxis->setInterval(0, acquisitionDisplayLength);

    m_acquisitionDataThread = QtConcurrent::run(this, &HarmonicCalibration::getAcquisitionSamples, acquisitionSampleRate);
	m_acquisitionDataWatcher.setFuture(m_acquisitionDataThread);
	m_acquisitionGraphThread = QtConcurrent::run(this, &HarmonicCalibration::acquisitionPlotTask, acquisitionGraphSampleRate);
	m_acquisitionGraphWatcher.setFuture(m_acquisitionGraphThread);
}

void HarmonicCalibration::stopAcquisition()
{
	isStartAcquisition = false;
	if(m_acquisitionDataThread.isRunning())
	{
		m_acquisitionDataThread.cancel();
		m_acquisitionDataWatcher.waitForFinished();
	}
	if(m_acquisitionGraphThread.isRunning())
	{
		m_acquisitionGraphThread.cancel();
		m_acquisitionGraphWatcher.waitForFinished();
	}

	stopCurrentMotorPositionMonitor();
}

void HarmonicCalibration::updateFaultStatus(bool status)
{
	bool isChanged = (deviceStatusFault != status);
	if(isChanged)
	{
		deviceStatusFault = status;
		acquisitionFaultRegisterLEDWidget->setChecked(deviceStatusFault);
		calibrationFaultRegisterLEDWidget->setChecked(deviceStatusFault);
	}
}

void HarmonicCalibration::updateAcquisitionMotorRPM()
{
	acquisitionMotorRPMLineEdit->setText(QString::number(motor_rpm));
}

void HarmonicCalibration::updateAcquisitionMotorRotationDirection()
{
	acquisitionMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
}

void HarmonicCalibration::getAcquisitionSamples(int sampleRate)
{
	while(isStartAcquisition)
	{
		if(!updateChannelValues()) { break; }

		if(acquisitionDataMap.at(ANGLE) == false && acquisitionAngleList.size() > 0) acquisitionAngleList.clear();
		if(acquisitionDataMap.at(ABSANGLE) == false && acquisitionABSAngleList.size() > 0) acquisitionABSAngleList.clear();
		if(acquisitionDataMap.at(TURNCOUNT) == false && acquisitionTurnCountList.size() > 0) acquisitionTurnCountList.clear();
		if(acquisitionDataMap.at(TMP0) == false && acquisitionTmp0List.size() > 0) acquisitionTmp0List.clear();
		if(acquisitionDataMap.at(SINE) == false && acquisitionSineList.size() > 0) acquisitionSineList.clear();
		if(acquisitionDataMap.at(COSINE) == false && acquisitionCosineList.size() > 0) acquisitionCosineList.clear();
		if(acquisitionDataMap.at(RADIUS) == false && acquisitionRadiusList.size() > 0) acquisitionRadiusList.clear();

		if(acquisitionDataMap.at(ANGLE)) prependAcquisitionData(angle, acquisitionAngleList);
		if(acquisitionDataMap.at(ABSANGLE)) prependAcquisitionData(rotation, acquisitionABSAngleList);
		if(acquisitionDataMap.at(TURNCOUNT)) prependAcquisitionData(count, acquisitionTurnCountList);
		if(acquisitionDataMap.at(TMP0)) prependAcquisitionData(temp, acquisitionTmp0List);
		if(acquisitionDataMap.at(SINE)) prependAcquisitionData(getAcquisitionParameterValue(SINE), acquisitionSineList);
		if(acquisitionDataMap.at(COSINE)) prependAcquisitionData(getAcquisitionParameterValue(COSINE), acquisitionCosineList);
		if(acquisitionDataMap.at(RADIUS)) prependAcquisitionData(getAcquisitionParameterValue(RADIUS), acquisitionRadiusList);

		QThread::msleep(sampleRate);
	}
}

double HarmonicCalibration::getAcquisitionParameterValue(const AcquisitionDataKey &key)
{
	uint32_t *readValue = new uint32_t;
	switch(key)
	{
		case SINE:
		{
			if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				m_admtController->getSensorRegister(ADMTController::SensorRegister::SINE),
				readValue) == -1) return qQNaN();
			map<string, double> sineRegisterMap = m_admtController->getSineRegisterBitMapping(static_cast<uint16_t>(*readValue));
			return sineRegisterMap.at("SINE");
			break;
		}
		case COSINE:
		{
			if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				m_admtController->getSensorRegister(ADMTController::SensorRegister::COSINE),
				readValue) == -1) return qQNaN();
			map<string, double> cosineRegisterMap = m_admtController->getCosineRegisterBitMapping(static_cast<uint16_t>(*readValue));
			return cosineRegisterMap.at("COSINE");
			break;
		}
		case RADIUS:
		{
			if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				m_admtController->getSensorRegister(ADMTController::SensorRegister::RADIUS),
				readValue) == -1) return qQNaN();
			map<string, double> radiusRegisterMap = m_admtController->getRadiusRegisterBitMapping(static_cast<uint16_t>(*readValue));
			return radiusRegisterMap.at("RADIUS");
			break;
		}
		default: 
			return qQNaN();
			break;
	}
}

void HarmonicCalibration::plotAcquisition(QVector<double>& list, PlotChannel* channel)
{
	channel->curve()->setSamples(list);
	auto result = minmax_element(list.begin(), list.end());
	if(*result.first < acquisitionGraphYMin) acquisitionGraphYMin = *result.first;
	if(*result.second > acquisitionGraphYMax) acquisitionGraphYMax = *result.second;
}

void HarmonicCalibration::prependAcquisitionData(const double& data, QVector<double>& list)
{
	list.prepend(data);
	if(list.size() >= acquisitionDisplayLength){
		list.resize(acquisitionDisplayLength);
		list.squeeze();
	}
}

void HarmonicCalibration::resetAcquisitionYAxisScale()
{
	acquisitionGraphYMin = 0;
	acquisitionGraphYMax = 360;
	acquisitionYPlotAxis->setInterval(acquisitionGraphYMin, acquisitionGraphYMax);
	acquisitionGraphPlotWidget->replot();
}

void HarmonicCalibration::acquisitionPlotTask(int sampleRate)
{
	while(isStartAcquisition){
		if(acquisitionDataMap.at(ANGLE))
			plotAcquisition(acquisitionAngleList, acquisitionAnglePlotChannel);
		if(acquisitionDataMap.at(ABSANGLE))
			plotAcquisition(acquisitionABSAngleList, acquisitionABSAnglePlotChannel);
		if(acquisitionDataMap.at(TURNCOUNT))
			plotAcquisition(acquisitionTurnCountList, acquisitionTurnCountPlotChannel);
		if(acquisitionDataMap.at(TMP0))
			plotAcquisition(acquisitionTmp0List, acquisitionTmp0PlotChannel);
		if(acquisitionDataMap.at(SINE))
			plotAcquisition(acquisitionSineList, acquisitionSinePlotChannel);
		if(acquisitionDataMap.at(COSINE))
			plotAcquisition(acquisitionCosineList, acquisitionCosinePlotChannel);
		if(acquisitionDataMap.at(RADIUS))
			plotAcquisition(acquisitionRadiusList, acquisitionRadiusPlotChannel);

		acquisitionYPlotAxis->setInterval(acquisitionGraphYMin, acquisitionGraphYMax);
		acquisitionGraphPlotWidget->replot();

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::acquisitionUITask(int sampleRate)
{
	while(isAcquisitionTab)
	{
		if(isStartAcquisition) updateLineEditValues();

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::startAcquisitionUITask()
{
	if(!m_acquisitionUIThread.isRunning())
	{
		isAcquisitionTab = true;
		m_acquisitionUIThread = QtConcurrent::run(this, &HarmonicCalibration::acquisitionUITask, acquisitionUITimerRate);
		m_acquisitionUIWatcher.setFuture(m_acquisitionUIThread);
	}
}

void HarmonicCalibration::stopAcquisitionUITask()
{
	isAcquisitionTab = false;
	if(m_acquisitionUIThread.isRunning())
	{
		m_acquisitionUIThread.cancel();
		m_acquisitionUIWatcher.waitForFinished();
	}
}

void HarmonicCalibration::updateSequenceWidget(){
	if(GENERALRegisterMap.at("Sequence Type") == -1){ sequenceTypeMenuCombo->combo()->setCurrentText("Reserved"); }
	else{ sequenceTypeMenuCombo->combo()->setCurrentIndex(sequenceTypeMenuCombo->combo()->findData(GENERALRegisterMap.at("Sequence Type"))); }
	conversionTypeMenuCombo->combo()->setCurrentIndex(conversionTypeMenuCombo->combo()->findData(GENERALRegisterMap.at("Conversion Type")));
	if(GENERALRegisterMap.at("Convert Synchronization") == -1){ convertSynchronizationMenuCombo->combo()->setCurrentText("Reserved"); }
	else{ convertSynchronizationMenuCombo->combo()->setCurrentIndex(convertSynchronizationMenuCombo->combo()->findData(GENERALRegisterMap.at("Convert Synchronization"))); }
	angleFilterMenuCombo->combo()->setCurrentIndex(angleFilterMenuCombo->combo()->findData(GENERALRegisterMap.at("Angle Filter")));
	eighthHarmonicMenuCombo->combo()->setCurrentIndex(eighthHarmonicMenuCombo->combo()->findData(GENERALRegisterMap.at("8th Harmonic")));
}

void HarmonicCalibration::applySequenceAndUpdate()
{
	applySequence();
	updateSequenceWidget();
}

void HarmonicCalibration::updateGeneralSettingEnabled(bool value)
{
	graphUpdateIntervalLineEdit->setEnabled(value);
	displayLengthLineEdit->setEnabled(value);
}

void HarmonicCalibration::connectCheckBoxToAcquisitionGraph(QCheckBox* widget, PlotChannel* channel, AcquisitionDataKey key)
{
	connect(widget, &QCheckBox::stateChanged, [=](int state){
		if(state == Qt::Checked){
			channel->setEnabled(true);
			acquisitionDataMap[key] = true;
		}
		else{
			channel->setEnabled(false);
			acquisitionDataMap[key] = false;
		}
	});
}

void HarmonicCalibration::GMRReset()
{
	// Set Motor Angle to 315 degrees
	target_pos = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos); 

	// Write 1 to ADMT IIO Attribute coil_rs
	m_admtController->setDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
											  m_admtController->getDeviceAttribute(ADMTController::DeviceAttribute::SDP_COIL_RS), 1);

	// Write 0xc000 to CNVPAGE
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE), 0xc000) != -1)
	{
		// Write 0x0000 to CNVPAGE
		if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE), 0x0000) != -1)
		{
			// Read ABSANGLE

			StatusBarManager::pushMessage("GMR Reset Done");
		}
		else { StatusBarManager::pushMessage("Failed to write CNVPAGE Register"); }
	}
	else { StatusBarManager::pushMessage("Failed to write CNVPAGE Register"); }
}

void HarmonicCalibration::restart()
{
	if(m_running) {
		run(false);
		run(true);
	}
}

bool HarmonicCalibration::running() const { return m_running; }

void HarmonicCalibration::setRunning(bool newRunning)
{
	if(m_running == newRunning)
		return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

void HarmonicCalibration::start() { run(true); }

void HarmonicCalibration::stop() { run(false); }

void HarmonicCalibration::run(bool b)
{
	qInfo() << b;
	QElapsedTimer tim;
	tim.start();

	if(!b) {
		isStartAcquisition = false;
		runButton->setChecked(false);
	}
	else{
		startAcquisition();
	}

	updateGeneralSettingEnabled(!b);
}
#pragma endregion

#pragma region Calibration Methods
void HarmonicCalibration::startCalibrationUITask()
{
	if(!m_calibrationUIThread.isRunning())
	{
		isCalibrationTab = true;
		m_calibrationUIThread = QtConcurrent::run(this, &HarmonicCalibration::calibrationUITask, calibrationUITimerRate);
		m_calibrationUIWatcher.setFuture(m_calibrationUIThread);
	}
}

void HarmonicCalibration::stopCalibrationUITask()
{
	isCalibrationTab = false;
	if(m_calibrationUIThread.isRunning())
	{
		m_calibrationUIThread.cancel();
		m_calibrationUIWatcher.waitForFinished();
	}
}

void HarmonicCalibration::calibrationUITask(int sampleRate)
{
	while (isCalibrationTab)
	{
		if(isStartMotor)
		{
			if(isPostCalibration){
				postCalibrationRawDataPlotChannel->curve()->setSamples(graphPostDataList);
				postCalibrationRawDataPlotWidget->replot();
			}
			else{
				calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
				calibrationRawDataPlotWidget->replot();
			}
		}

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::updateCalibrationMotorRPM()
{
	calibrationMotorRPMLineEdit->setText(QString::number(motor_rpm));
}

void HarmonicCalibration::updateCalibrationMotorRotationDirection()
{
	calibrationMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
}

void HarmonicCalibration::getCalibrationSamples()
{
	if(resetCurrentPositionToZero()){
		double step = 0;
		if(isMotorRotationClockwise) step = motorFullStepPerRevolution;
		else step = -motorFullStepPerRevolution;
		if(isPostCalibration){
			int currentSamplesCount = graphPostDataList.size();
			while(isStartMotor && currentSamplesCount < totalSamplesCount){
				target_pos = current_pos + step;
				moveMotorToPosition(target_pos, true);
				updateChannelValue(ADMTController::Channel::ANGLE);
				graphPostDataList.append(angle);
				currentSamplesCount++;
			}
		}
		else{
			int currentSamplesCount = graphDataList.size();
			while(isStartMotor && currentSamplesCount < totalSamplesCount){
				target_pos = current_pos + step;
				if(moveMotorToPosition(target_pos, true) == false) { m_admtController->disconnectADMT(); }
				if(updateChannelValue(ADMTController::Channel::ANGLE)) { break; }
				graphDataList.append(angle);
				currentSamplesCount++;
			}
		}
	}

	stopMotor();
}

void HarmonicCalibration::startCalibration()
{
	totalSamplesCount = cycleCount * samplesPerCycle;
	graphPostDataList.reserve(totalSamplesCount);
	graphPostDataList.squeeze();
	graphDataList.reserve(totalSamplesCount);
	graphDataList.squeeze();

	//configureConversionType(calibrationMode); // TODO uncomment when conversion type is okay
	configureCalibrationSequenceSettings();
	clearHarmonicRegisters();

	toggleTabSwitching(false);
	toggleCalibrationControls(false);

	calibrationDataGraphTabWidget->setCurrentIndex(0);
	calibrationRawDataXPlotAxis->setInterval(0, totalSamplesCount);

	toggleCalibrationButtonState(1);
	if(calibrationMode == 0) startContinuousCalibration();
	else startOneShotCalibration();
}

void HarmonicCalibration::stopCalibration()
{
	isStartMotor = false;
	if(calibrationMode == 0)
	{
		stopContinuousCalibration();
	}

	toggleTabSwitching(true);
	toggleCalibrationControls(true);
}

void HarmonicCalibration::startContinuousCalibration()
{
	stopCurrentMotorPositionMonitor();
	stopDeviceStatusMonitor();

	if(isPostCalibration) StatusBarManager::pushMessage("Acquiring Post Calibration Samples, Please Wait...");
	else StatusBarManager::pushMessage("Acquiring Calibration Samples, Please Wait...");
	startResetMotorToZero();
}

void HarmonicCalibration::stopContinuousCalibration()
{
	stopMotor();
	stopWaitForVelocityReachedThread();
	stopCalibrationStreamThread();

	startCurrentMotorPositionMonitor();
	startDeviceStatusMonitor();
}

void HarmonicCalibration::startResetMotorToZero()
{
	isResetMotorToZero = true;
	if(!m_resetMotorToZeroThread.isRunning())
	{
		m_resetMotorToZeroThread = QtConcurrent::run(this, &HarmonicCalibration::resetMotorToZero);
		m_resetMotorToZeroWatcher.setFuture(m_resetMotorToZeroThread);
	}
}

void HarmonicCalibration::stopResetMotorToZero()
{
	isResetMotorToZero = false;
	if(m_resetMotorToZeroThread.isRunning())
	{
		m_resetMotorToZeroThread.cancel();
		m_resetMotorToZeroWatcher.waitForFinished();
	}
}

void HarmonicCalibration::resetMotorToZero()
{
	if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0)
	{
		if(current_pos != 0)
		{
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, convertRPStoVMAX(convertRPMtoRPS(fast_motor_rpm)));
			writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0);
			readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
			while(isResetMotorToZero && current_pos != 0){
				if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) != 0) break;
				QThread::msleep(readMotorDebounce);
			}
			if(current_pos == 0)
			{
				writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
				writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0);
			}
		}
	}
}

void HarmonicCalibration::startCalibrationStreamThread()
{
	if(!m_calibrationStreamThread.isRunning())
	{
		m_admtController->stopStream = false;

		continuousCalibrationSampleRate = calculateContinuousCalibrationSampleRate(convertVMAXtoRPS(rotate_vmax), samplesPerCycle);
		m_calibrationStreamThread = QtConcurrent::run([this]() { m_admtController->bufferedStreamIO(totalSamplesCount, continuousCalibrationSampleRate); });
		m_calibrationStreamWatcher.setFuture(m_calibrationStreamThread);
	}
}

void HarmonicCalibration::stopCalibrationStreamThread()
{
	m_admtController->stopStream = true;
	if(m_calibrationStreamThread.isRunning())
	{
		m_calibrationStreamThread.cancel();
		m_calibrationStreamWatcher.waitForFinished();
	}
}

void HarmonicCalibration::startWaitForVelocityReachedThread(int mode)
{
	if(!m_calibrationWaitVelocityWatcher.isRunning())
	{
		isMotorVelocityCheck = true;
		m_calibrationWaitVelocityThread = QtConcurrent::run(this, &HarmonicCalibration::waitForVelocityReached, mode, motorWaitVelocityReachedSampleRate);
		m_calibrationWaitVelocityWatcher.setFuture(m_calibrationWaitVelocityThread);
	}
}

void HarmonicCalibration::stopWaitForVelocityReachedThread()
{
	isMotorVelocityCheck = false;
	if(m_calibrationWaitVelocityWatcher.isRunning())
	{
		m_calibrationWaitVelocityThread.cancel();
		m_calibrationWaitVelocityWatcher.waitForFinished();
	}
}

void HarmonicCalibration::waitForVelocityReached(int mode, int sampleRate)
{
	isMotorVelocityReached = false;
	if(mode == 0)
	{
		QThread::msleep(floor(convertAMAXtoAccelTime(amax) * 1000));
		isMotorVelocityReached = true;
	}
	else if(mode == 1)
	{
		moveMotorContinuous();
		uint32_t *registerValue = new uint32_t;
		while(isMotorVelocityCheck && !isMotorVelocityReached)
		{
			if(readMotorRegisterValue(m_admtController->getRampGeneratorDriverFeatureControlRegister(ADMTController::RampGeneratorDriverFeatureControlRegister::RAMP_STAT), registerValue) == 0){
				isMotorVelocityReached = m_admtController->checkVelocityReachedFlag(static_cast<uint16_t>(*registerValue));
			}

			QThread::msleep(sampleRate);
		}

		delete registerValue;
	}
}

int HarmonicCalibration::calculateContinuousCalibrationSampleRate(double motorRPS, int samplesPerCycle)
{
	return static_cast<int>(floor(1 / motorRPS / samplesPerCycle * 1000 * 1000 * 1000)); // In nanoseconds
}

void HarmonicCalibration::configureConversionType(int mode)
{
	readSequence();
	GENERALRegisterMap.at("Conversion Type") = mode;
	writeSequence(GENERALRegisterMap);
}

void HarmonicCalibration::configureCalibrationSequenceSettings()
{
	readSequence();
	GENERALRegisterMap.at("8th Harmonic") = 1; // User-supplied 8th Harmonic
	writeSequence(GENERALRegisterMap);
}

void HarmonicCalibration::getStreamedCalibrationSamples(int microSampleRate)
{
	int currentSamplesCount = graphDataList.size();
	moveMotorContinuous();
	while(isStartMotor && currentSamplesCount < totalSamplesCount)
	{
		graphDataList.append(m_admtController->streamedValue);
		// graphPostDataList.append(m_admtController->streamedValue);
		currentSamplesCount = graphDataList.size();
		// currentSamplesCount = graphDataPostList.size();

		QThread::usleep(microSampleRate);
	}
}

void HarmonicCalibration::startOneShotCalibration()
{
	if(resetToZero && !isPostCalibration){
		clearCalibrationSamples();
		clearPostCalibrationSamples();
		clearAngleErrorGraphs();
		clearCorrectedAngleErrorGraphs();
	}
	else if(resetToZero){
		clearPostCalibrationSamples();
	}

	QFuture<void> future = QtConcurrent::run(this, &HarmonicCalibration::getCalibrationSamples);
	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

	connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
		toggleTabSwitching(true);
		toggleCalibrationControls(true);

		calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
		calibrationRawDataPlotChannel->xAxis()->setMax(graphDataList.size());
		calibrationRawDataPlotWidget->replot();
		isStartMotor = false;
		
		if(isPostCalibration)
		{
			if(static_cast<int>(graphPostDataList.size()) == totalSamplesCount) 
			{
				computeSineCosineOfAngles(graphPostDataList);
				m_admtController->postcalibrate(vector<double>(graphPostDataList.begin(), graphPostDataList.end()), cycleCount, samplesPerCycle, !isMotorRotationClockwise);
				populateCorrectedAngleErrorGraphs();
				isPostCalibration = false;
				isStartMotor = false;
				resetToZero = true;
				toggleCalibrationButtonState(4);
			}
			else toggleCalibrationButtonState(2);
		}
		else{
			if(static_cast<int>(graphDataList.size()) == totalSamplesCount) 
			{
				computeSineCosineOfAngles(graphDataList);
				calibrationLogWrite(m_admtController->calibrate(vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount, samplesPerCycle, !isMotorRotationClockwise));
				populateAngleErrorGraphs();
				calculateHarmonicValues();
				toggleCalibrationButtonState(2);
			}
			else{
				resetToZero = true;
				toggleCalibrationButtonState(0);
			}
		}
	});
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
	watcher->setFuture(future);
}

void HarmonicCalibration::postCalibrateData()
{
	calibrationLogWrite("==== Post Calibration Start ====\n");
	flashHarmonicValues();
	isPostCalibration = true;
	isStartMotor = true;
	resetToZero = true;
	
	toggleTabSwitching(false);
	toggleCalibrationControls(false);

	calibrationDataGraphTabWidget->setCurrentIndex(1);
	postCalibrationRawDataXPlotAxis->setInterval(0, totalSamplesCount);

	toggleCalibrationButtonState(3);
	if(calibrationMode == 0) startContinuousCalibration();
	else startOneShotCalibration();
}

void HarmonicCalibration::resetAllCalibrationState()
{
	clearCalibrationSamples();
	clearPostCalibrationSamples();
	calibrationDataGraphTabWidget->setCurrentIndex(0);

	clearAngleErrorGraphs();
	clearCorrectedAngleErrorGraphs();
	resultDataTabWidget->setCurrentIndex(0);

	toggleCalibrationButtonState(0);

	isPostCalibration = false;
	isCalculatedCoeff = false;
	resetToZero = true;
	displayCalculatedCoeff();
}

void HarmonicCalibration::computeSineCosineOfAngles(QVector<double> graphDataList)
{
	m_admtController->computeSineCosineOfAngles(vector<double>(graphDataList.begin(), graphDataList.end()));
	if(isPostCalibration){
		postCalibrationSineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_sine_scaled.data(), m_admtController->calibration_samples_sine_scaled.size());
		postCalibrationCosineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_cosine_scaled.data(), m_admtController->calibration_samples_cosine_scaled.size());
		postCalibrationRawDataPlotWidget->replot();
	}
	else{
		calibrationSineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_sine_scaled.data(), m_admtController->calibration_samples_sine_scaled.size());
		calibrationCosineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_cosine_scaled.data(), m_admtController->calibration_samples_cosine_scaled.size());
		calibrationRawDataPlotWidget->replot();
	}
}

void HarmonicCalibration::populateAngleErrorGraphs()
{
	QVector<double> angleError = QVector<double>(m_admtController->angleError.begin(), m_admtController->angleError.end());
	QVector<double> FFTAngleErrorMagnitude = QVector<double>(m_admtController->FFTAngleErrorMagnitude.begin(), m_admtController->FFTAngleErrorMagnitude.end());
	QVector<double> FFTAngleErrorPhase = QVector<double>(m_admtController->FFTAngleErrorPhase.begin(), m_admtController->FFTAngleErrorPhase.end());

	angleErrorPlotChannel->curve()->setSamples(angleError);
	auto angleErrorMinMax = minmax_element(angleError.begin(), angleError.end());
	angleErrorYPlotAxis->setInterval(*angleErrorMinMax.first, *angleErrorMinMax.second);
	angleErrorXPlotAxis->setInterval(0, angleError.size());
	angleErrorPlotWidget->replot();
	
	FFTAngleErrorPhaseChannel->curve()->setSamples(FFTAngleErrorPhase);
	FFTAngleErrorMagnitudeChannel->curve()->setSamples(FFTAngleErrorMagnitude);
	auto angleErrorMagnitudeMinMax = minmax_element(FFTAngleErrorMagnitude.begin(), FFTAngleErrorMagnitude.end());
	auto angleErrorPhaseMinMax = minmax_element(FFTAngleErrorPhase.begin(), FFTAngleErrorPhase.end());
	double FFTAngleErrorPlotMin = *angleErrorMagnitudeMinMax.first < *angleErrorPhaseMinMax.first ? *angleErrorMagnitudeMinMax.first : *angleErrorPhaseMinMax.first;
	double FFTAngleErrorPlotMax = *angleErrorMagnitudeMinMax.second > *angleErrorPhaseMinMax.second ? *angleErrorMagnitudeMinMax.second : *angleErrorPhaseMinMax.second;
	FFTAngleErrorYPlotAxis->setInterval(FFTAngleErrorPlotMin, FFTAngleErrorPlotMax);
	FFTAngleErrorXPlotAxis->setInterval(0, FFTAngleErrorMagnitude.size());
	FFTAngleErrorPlotWidget->replot();

	resultDataTabWidget->setCurrentIndex(0); // Set tab to Angle Error
}

void HarmonicCalibration::populateCorrectedAngleErrorGraphs()
{
	QVector<double> correctedError(m_admtController->correctedError.begin(), m_admtController->correctedError.end());
	QVector<double> FFTCorrectedErrorMagnitude(m_admtController->FFTCorrectedErrorMagnitude.begin(), m_admtController->FFTCorrectedErrorMagnitude.end());
	QVector<double> FFTCorrectedErrorPhase(m_admtController->FFTCorrectedErrorPhase.begin(), m_admtController->FFTCorrectedErrorPhase.end());

	correctedErrorPlotChannel->curve()->setSamples(correctedError);
	auto correctedErrorMagnitudeMinMax = minmax_element(correctedError.begin(), correctedError.end());
	correctedErrorYPlotAxis->setInterval(*correctedErrorMagnitudeMinMax.first, *correctedErrorMagnitudeMinMax.second);
	correctedErrorXPlotAxis->setMax(correctedError.size());
	correctedErrorPlotWidget->replot();

	FFTCorrectedErrorPhaseChannel->curve()->setSamples(FFTCorrectedErrorPhase);
	FFTCorrectedErrorMagnitudeChannel->curve()->setSamples(FFTCorrectedErrorMagnitude);
	auto FFTCorrectedErrorMagnitudeMinMax = minmax_element(FFTCorrectedErrorMagnitude.begin(), FFTCorrectedErrorMagnitude.end());
	auto FFTCorrectedErrorPhaseMinMax = minmax_element(FFTCorrectedErrorPhase.begin(), FFTCorrectedErrorPhase.end());
	double FFTCorrectedErrorPlotMin = *FFTCorrectedErrorMagnitudeMinMax.first < *FFTCorrectedErrorPhaseMinMax.first ? *FFTCorrectedErrorMagnitudeMinMax.first : *FFTCorrectedErrorPhaseMinMax.first;
	double FFTCorrectedErrorPlotMax = *FFTCorrectedErrorMagnitudeMinMax.second > *FFTCorrectedErrorPhaseMinMax.second ? *FFTCorrectedErrorMagnitudeMinMax.second : *FFTCorrectedErrorPhaseMinMax.second;
	FFTCorrectedErrorYPlotAxis->setInterval(FFTCorrectedErrorPlotMin, FFTCorrectedErrorPlotMax);
	FFTCorrectedErrorXPlotAxis->setMax(FFTCorrectedErrorMagnitude.size());
	FFTCorrectedErrorPlotWidget->replot();

	resultDataTabWidget->setCurrentIndex(2); // Set tab to Angle Error
}

void HarmonicCalibration::clearHarmonicRegisters()
{
	bool success = false;
	uint32_t value = 0x0;
	uint32_t harmonicCNVPage = m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1MAG);
	if(changeCNVPage(harmonicCNVPage))
	{
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), value) == 0 ? true : false;
		success = m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), value) == 0 ? true : false;
	}
	
	if(!success){
		calibrationLogWrite("Unable to clear Harmonic Registers!");
	}
}

void HarmonicCalibration::flashHarmonicValues()
{
	if(changeCNVPage(0x02)){
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 0x01, 0x02);

		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG),
											H1_MAG_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH),
											H1_PHASE_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG),
											H2_MAG_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH),
											H2_PHASE_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG),
											H3_MAG_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH),
											H3_PHASE_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG),
											H8_MAG_HEX);
		m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
											m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH),
											H8_PHASE_HEX);

		isCalculatedCoeff = true;
		displayCalculatedCoeff();
	}
	else{
		calibrationLogWrite("Unabled to flash Harmonic Registers!");
	}
}

void HarmonicCalibration::calculateHarmonicValues()
{
	uint32_t *h1MagCurrent = new uint32_t, 
			 *h1PhaseCurrent = new uint32_t, 
			 *h2MagCurrent = new uint32_t,
			 *h2PhaseCurrent = new uint32_t,
			 *h3MagCurrent = new uint32_t,
			 *h3PhaseCurrent = new uint32_t,
			 *h8MagCurrent = new uint32_t,
			 *h8PhaseCurrent = new uint32_t;
	
	if(changeCNVPage(0x02))
	{
		// Read and store current harmonic values
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), h1MagCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), h2MagCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), h3MagCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), h8MagCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), h1PhaseCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), h2PhaseCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), h3PhaseCurrent);
		m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), h8PhaseCurrent);

		// Calculate harmonic coefficients (Hex)
		H1_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(static_cast<uint16_t>(m_admtController->HAR_MAG_1), static_cast<uint16_t>(*h1MagCurrent), "h1"));
		H2_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(static_cast<uint16_t>(m_admtController->HAR_MAG_2), static_cast<uint16_t>(*h2MagCurrent), "h2"));
		H3_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(static_cast<uint16_t>(m_admtController->HAR_MAG_3), static_cast<uint16_t>(*h3MagCurrent), "h3"));
		H8_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(static_cast<uint16_t>(m_admtController->HAR_MAG_8), static_cast<uint16_t>(*h8MagCurrent), "h8"));
		H1_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(static_cast<uint16_t>(m_admtController->HAR_PHASE_1), static_cast<uint16_t>(*h1PhaseCurrent)));
		H2_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(static_cast<uint16_t>(m_admtController->HAR_PHASE_2), static_cast<uint16_t>(*h2PhaseCurrent)));
		H3_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(static_cast<uint16_t>(m_admtController->HAR_PHASE_3), static_cast<uint16_t>(*h3PhaseCurrent)));
		H8_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(static_cast<uint16_t>(m_admtController->HAR_PHASE_8), static_cast<uint16_t>(*h8PhaseCurrent)));

		calibrationLogWrite();
		calibrationLogWrite(QString("Calculated H1 Mag (Hex): 0x%1").arg(QString::number(H1_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H1 Phase (Hex): 0x%1").arg(QString::number(H1_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H2 Mag (Hex): 0x%1").arg(QString::number(H2_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H2 Phase (Hex): 0x%1").arg(QString::number(H2_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H3 Mag (Hex): 0x%1").arg(QString::number(H3_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H3 Phase (Hex): 0x%1").arg(QString::number(H3_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H8 Mag (Hex): 0x%1").arg(QString::number(H8_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H8 Phase (Hex): 0x%1").arg(QString::number(H8_PHASE_HEX, 16).rightJustified(4, '0')));

		// Get actual harmonic values from hex
		H1_MAG_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H1_MAG_HEX), "h1mag");
		H1_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H1_PHASE_HEX), "h1phase");
		H2_MAG_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H2_MAG_HEX), "h2mag");
		H2_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H2_PHASE_HEX), "h2phase");
		H3_MAG_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H3_MAG_HEX), "h3mag");
		H3_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H3_PHASE_HEX), "h3phase");
		H8_MAG_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H8_MAG_HEX), "h8mag");
		H8_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H8_PHASE_HEX), "h8phase");

		calibrationLogWrite();
		calibrationLogWrite(QString("Calculated H1 Mag (Angle): %1°").arg(QString::number(H1_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H1 Phase (Angle): %1°").arg(QString::number(H1_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H2 Mag (Angle): %1°").arg(QString::number(H2_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H2 Phase (Angle): %1°").arg(QString::number(H2_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H3 Mag (Angle): %1°").arg(QString::number(H3_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H3 Phase (Angle): %1°").arg(QString::number(H3_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H8 Mag (Angle): %1°").arg(QString::number(H8_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H8 Phase (Angle): %1°").arg(QString::number(H8_PHASE_ANGLE)));

		if(isAngleDisplayFormat) updateCalculatedCoeffAngle();
		else updateCalculatedCoeffHex();
		isCalculatedCoeff = true;
	}
}

void HarmonicCalibration::updateCalculatedCoeffAngle()
{
	calibrationH1MagLabel->setText(QString::number(H1_MAG_ANGLE, 'f', 2) + "°");
	calibrationH2MagLabel->setText(QString::number(H2_MAG_ANGLE, 'f', 2) + "°");
	calibrationH3MagLabel->setText(QString::number(H3_MAG_ANGLE, 'f', 2) + "°");
	calibrationH8MagLabel->setText(QString::number(H8_MAG_ANGLE, 'f', 2) + "°");
	calibrationH1PhaseLabel->setText("Φ " + QString::number(H1_PHASE_ANGLE, 'f', 2));
	calibrationH2PhaseLabel->setText("Φ " + QString::number(H2_PHASE_ANGLE, 'f', 2));
	calibrationH3PhaseLabel->setText("Φ " + QString::number(H3_PHASE_ANGLE, 'f', 2));
	calibrationH8PhaseLabel->setText("Φ " + QString::number(H8_PHASE_ANGLE, 'f', 2));
}

void HarmonicCalibration::updateCalculatedCoeffHex()
{
	calibrationH1MagLabel->setText(QString("0x%1").arg(H1_MAG_HEX, 4, 16, QChar('0')));
	calibrationH2MagLabel->setText(QString("0x%1").arg(H2_MAG_HEX, 4, 16, QChar('0')));
	calibrationH3MagLabel->setText(QString("0x%1").arg(H3_MAG_HEX, 4, 16, QChar('0')));
	calibrationH8MagLabel->setText(QString("0x%1").arg(H8_MAG_HEX, 4, 16, QChar('0')));
	calibrationH1PhaseLabel->setText(QString("0x%1").arg(H1_PHASE_HEX, 4, 16, QChar('0')));
	calibrationH2PhaseLabel->setText(QString("0x%1").arg(H2_PHASE_HEX, 4, 16, QChar('0')));
	calibrationH3PhaseLabel->setText(QString("0x%1").arg(H3_PHASE_HEX, 4, 16, QChar('0')));
	calibrationH8PhaseLabel->setText(QString("0x%1").arg(H8_PHASE_HEX, 4, 16, QChar('0')));
}

void HarmonicCalibration::resetCalculatedCoeffAngle()
{
	calibrationH1MagLabel->setText("--.--°");
	calibrationH2MagLabel->setText("--.--°");
	calibrationH3MagLabel->setText("--.--°");
	calibrationH8MagLabel->setText("--.--°");
	calibrationH1PhaseLabel->setText("Φ --.--");
	calibrationH2PhaseLabel->setText("Φ --.--");
	calibrationH3PhaseLabel->setText("Φ --.--");
	calibrationH8PhaseLabel->setText("Φ --.--");
}

void HarmonicCalibration::resetCalculatedCoeffHex()
{
	calibrationH1MagLabel->setText("0x----");
	calibrationH2MagLabel->setText("0x----");
	calibrationH3MagLabel->setText("0x----");
	calibrationH8MagLabel->setText("0x----");
	calibrationH1PhaseLabel->setText("0x----");
	calibrationH2PhaseLabel->setText("0x----");
	calibrationH3PhaseLabel->setText("0x----");
	calibrationH8PhaseLabel->setText("0x----");
}

void HarmonicCalibration::displayCalculatedCoeff()
{
	if(isAngleDisplayFormat){
		if(isCalculatedCoeff){
			updateCalculatedCoeffAngle();
		}
		else{
			resetCalculatedCoeffAngle();
		}
	}
	else{
		if(isCalculatedCoeff){
			updateCalculatedCoeffHex();
		}
		else{
			resetCalculatedCoeffHex();
		}
	}
}

void HarmonicCalibration::calibrationLogWrite(QString message)
{
	logsPlainTextEdit->appendPlainText(message);
}

void HarmonicCalibration::importCalibrationData()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Import"), "",
		tr("Comma-separated values files (*.csv);;"
		   "Tab-delimited values files (*.txt)"),
		nullptr, QFileDialog::Options());

	FileManager fm("HarmonicCalibration");

	try {
		fm.open(fileName, FileManager::IMPORT);

		graphDataList = fm.read(0);
		if(graphDataList.size() > 0)
		{
			calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
			calibrationRawDataXPlotAxis->setInterval(0, graphDataList.size());
			calibrationRawDataPlotWidget->replot();

			computeSineCosineOfAngles(graphDataList);
			calibrationLogWrite(m_admtController->calibrate(vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount, samplesPerCycle, false)); // TODO: Hard-coded Clockwise
			populateAngleErrorGraphs();
			toggleCalibrationButtonState(2);
		}
	} catch(FileManagerException &ex) {
		calibrationLogWrite(QString(ex.what()));
	}
}

void HarmonicCalibration::extractCalibrationData()
{
	QStringList filter;
	filter += QString(tr("Comma-separated values files (*.csv)"));
	filter += QString(tr("Tab-delimited values files (*.txt)"));
	filter += QString(tr("All Files(*)"));

	QString selectedFilter = filter[0];

	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"), "", filter.join(";;"), &selectedFilter, QFileDialog::Options());

	if(fileName.split(".").size() <= 1) {
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	if(!fileName.isEmpty()) {
		bool withScopyHeader = false;
		FileManager fm("HarmonicCalibration");
		fm.open(fileName, FileManager::EXPORT);

		QVector<double> preCalibrationAngleErrorsFFTMagnitude(m_admtController->angle_errors_fft_pre.begin(), m_admtController->angle_errors_fft_pre.end());
		QVector<double> preCalibrationAngleErrorsFFTPhase(m_admtController->angle_errors_fft_phase_pre.begin(), m_admtController->angle_errors_fft_phase_pre.end());

		QVector<double> h1Mag = { H1_MAG_ANGLE };
		QVector<double> h2Mag = { H2_MAG_ANGLE };
		QVector<double> h3Mag = { H3_MAG_ANGLE };
		QVector<double> h8Mag = { H8_MAG_ANGLE };
		QVector<double> h1Phase = { H1_PHASE_ANGLE };
		QVector<double> h2Phase = { H2_PHASE_ANGLE };
		QVector<double> h3Phase = { H3_PHASE_ANGLE };
		QVector<double> h8Phase = { H8_PHASE_ANGLE };

		fm.save(graphDataList, "Raw Data");
		fm.save(preCalibrationAngleErrorsFFTMagnitude, "Pre-Calibration Angle Errors FFT Magnitude");
		fm.save(preCalibrationAngleErrorsFFTPhase, "Pre-Calibration Angle Errors FFT Phase");
		fm.save(h1Mag, "H1 Mag");
		fm.save(h2Mag, "H2 Mag");
		fm.save(h3Mag, "H3 Mag");
		fm.save(h8Mag, "H8 Mag");
		fm.save(h1Phase, "H1 Phase");
		fm.save(h2Phase, "H2 Phase");
		fm.save(h3Phase, "H3 Phase");
		fm.save(h8Phase, "H8 Phase");

		fm.performWrite(withScopyHeader);
	}
}

void HarmonicCalibration::toggleTabSwitching(bool value)
{
	tabWidget->setTabEnabled(0, value);
	tabWidget->setTabEnabled(2, value);
	tabWidget->setTabEnabled(3, value);
}

void HarmonicCalibration::toggleCalibrationButtonState(int state)
{
	switch(state)
	{
		case 0: // Idle
			calibrationStartMotorButton->setEnabled(true);
			calibrationStartMotorButton->setChecked(false);
			calibrateDataButton->setEnabled(false);
			calibrateDataButton->setChecked(false);
			clearCalibrateDataButton->setEnabled(true);
			break;
		case 1: // Start Motor
			calibrationStartMotorButton->setEnabled(true);
			calibrateDataButton->setEnabled(false);
			clearCalibrateDataButton->setEnabled(false);
			break;
		case 2: // After Start Motor
			calibrationStartMotorButton->setEnabled(false);
			calibrateDataButton->setEnabled(true);
			calibrateDataButton->setChecked(false);
			clearCalibrateDataButton->setEnabled(true);
			break;
		case 3: // Post-Calibration
			calibrationStartMotorButton->setEnabled(false);
			calibrateDataButton->setEnabled(true);
			clearCalibrateDataButton->setEnabled(false);
			break;
		case 4: // After Post-Calibration
			calibrationStartMotorButton->setEnabled(false);
			calibrateDataButton->setEnabled(false);
			clearCalibrateDataButton->setEnabled(true);
			break;
	}
}

void HarmonicCalibration::canStartMotor(bool value)
{
	calibrationStartMotorButton->setEnabled(value);
}

void HarmonicCalibration::canCalibrate(bool value)
{
	calibrateDataButton->setEnabled(value);
}

void HarmonicCalibration::toggleCalibrationControls(bool value)
{
	// motorMaxVelocitySpinBox->setEnabled(value);
	// motorAccelTimeSpinBox->setEnabled(value);
	// motorMaxDisplacementSpinBox->setEnabled(value);
	// m_calibrationMotorRampModeMenuCombo->setEnabled(value);
	motorTargetPositionLineEdit->setEnabled(value);
	calibrationModeMenuCombo->setEnabled(value);
}

void HarmonicCalibration::clearCalibrationSamples()
{
	graphDataList.clear();
	calibrationRawDataPlotChannel->curve()->setData(nullptr);
	calibrationSineDataPlotChannel->curve()->setData(nullptr);
	calibrationCosineDataPlotChannel->curve()->setData(nullptr);
	calibrationRawDataPlotWidget->replot();
}

void HarmonicCalibration::clearCalibrationSineCosine()
{
	calibrationSineDataPlotChannel->curve()->setData(nullptr);
	calibrationCosineDataPlotChannel->curve()->setData(nullptr);
	calibrationRawDataPlotWidget->replot();
}

void HarmonicCalibration::clearPostCalibrationSamples()
{
	graphPostDataList.clear();
	postCalibrationRawDataPlotChannel->curve()->setData(nullptr);
	postCalibrationSineDataPlotChannel->curve()->setData(nullptr);
	postCalibrationCosineDataPlotChannel->curve()->setData(nullptr);
	postCalibrationRawDataPlotWidget->replot();
}

void HarmonicCalibration::clearAngleErrorGraphs()
{
	angleErrorPlotChannel->curve()->setData(nullptr);
	angleErrorPlotWidget->replot();
	FFTAngleErrorMagnitudeChannel->curve()->setData(nullptr);
	FFTAngleErrorPhaseChannel->curve()->setData(nullptr);
	FFTAngleErrorPlotWidget->replot();
}

void HarmonicCalibration::clearCorrectedAngleErrorGraphs()
{
	correctedErrorPlotChannel->curve()->setData(nullptr);
	correctedErrorPlotWidget->replot();
	FFTCorrectedErrorMagnitudeChannel->curve()->setData(nullptr);
	FFTCorrectedErrorPhaseChannel->curve()->setData(nullptr);
	FFTCorrectedErrorPlotWidget->replot();
}
#pragma endregion

#pragma region Motor Methods
bool HarmonicCalibration::moveMotorToPosition(double& position, bool validate)
{
	bool success = false;
	bool canRead = true;
	if(writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, position) == 0){
		if(validate){
			if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0)
			{
				while(target_pos != current_pos && canRead) {
					canRead = readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0 ? true : false;
				}
				if(canRead)	success = true;
			}
		}
	}

	return success;
}

void HarmonicCalibration::moveMotorContinuous()
{
	writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
	setRampMode(isMotorRotationClockwise);
}

bool HarmonicCalibration::resetCurrentPositionToZero()
{
	bool success = false;
	if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0)
	{
		if(current_pos != 0 &&
		   writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0) == 0 &&
		   readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0)
		{
			while(current_pos != 0){
				if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) != 0) break;
				QThread::msleep(readMotorDebounce);
			}
			if(current_pos == 0)
			{
				resetToZero = false;
				success = true;
			}
		}
		else{
			success = true;
		}
	}
	
	return success;
}

void HarmonicCalibration::stopMotor()
{
	writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
}

int HarmonicCalibration::readMotorAttributeValue(ADMTController::MotorAttribute attribute, double& value)
{
	int result = -1;
	if(!isDebug){
		result = m_admtController->getDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240),
															m_admtController->getMotorAttribute(attribute),
															&value);
	}
	return result;
}

int HarmonicCalibration::writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value)
{
	int result = -1;
	if(!isDebug){
		result = m_admtController->setDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240), 
															m_admtController->getMotorAttribute(attribute), 
															value);
	}
	return result;
}

int HarmonicCalibration::readMotorRegisterValue(uint32_t address, uint32_t *value)
{
	int result = -1;
	result = m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::TMC5240), 
													address, 
													value);
	return result;
}

void HarmonicCalibration::setRampMode(bool motorRotationClockwise)
{
	// Ramp Mode 1: Clockwise, Ramp Mode 2: Counter-Clockwise
	isMotorRotationClockwise = motorRotationClockwise;
	int mode = isMotorRotationClockwise ? 1 : 2;
	writeMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, mode);
}

void HarmonicCalibration::getRampMode()
{
	readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);
	// Ramp Mode 1: Counter-Clockwise, Ramp Mode 2: Clockwise
	if(ramp_mode == 1) isMotorRotationClockwise = false;
	else if(ramp_mode == 2) isMotorRotationClockwise = true;
}
#pragma endregion

#pragma region Utility Methods
void HarmonicCalibration::startUtilityTask()
{
	if(!m_utilityThread.isRunning())
	{
		isUtilityTab = true;
		m_utilityThread = QtConcurrent::run(this, &HarmonicCalibration::utilityTask, utilityUITimerRate);
		m_utilityWatcher.setFuture(m_utilityThread);
	}
}

void HarmonicCalibration::stopUtilityTask()
{
	isUtilityTab = false;
	if(m_utilityThread.isRunning())
	{
		m_utilityThread.cancel();
		m_utilityWatcher.waitForFinished();
	}
}

void HarmonicCalibration::utilityTask(int sampleRate){
	while(isUtilityTab)
	{
		getDIGIOENRegister();
		getFAULTRegister();
		if(hasMTDiagnostics)
		{
			getDIAG1Register();
			getDIAG2Register();
		}

		Q_EMIT commandLogWriteSignal("");

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::toggleUtilityTask(bool run)
{
	if(run)
	{
		startUtilityTask();
	}
	else
	{
		stopUtilityTask();
	}
}

void HarmonicCalibration::getDIGIOENRegister(){
	uint32_t *digioRegValue = new uint32_t;
	uint32_t digioEnPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN);
	if(changeCNVPage(digioEnPage))
	{
		uint32_t digioRegisterAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN);

		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), digioRegisterAddress, digioRegValue) != -1){
			uint16_t *digioRegValue16 = new uint16_t;
			*digioRegValue16 = static_cast<uint16_t>(*digioRegValue);

			Q_EMIT DIGIORegisterChanged(digioRegValue16);

			Q_EMIT commandLogWriteSignal("DIGIOEN: 0b" + QString::number(*digioRegValue, 2).rightJustified(16, '0'));

			delete digioRegValue16;
		}
		else{ Q_EMIT commandLogWriteSignal("Failed to read DIGIOEN Register"); }
	}

	delete digioRegValue;
}

void HarmonicCalibration::updateDIGIOUI(uint16_t *registerValue)
{
	DIGIOENRegisterMap = m_admtController->getDIGIOENRegisterBitMapping(*registerValue);
	updateDIGIOMonitorUI();
	updateDIGIOControlUI();
}

void HarmonicCalibration::updateDIGIOMonitorUI()
{
	map<string, bool> registerMap = DIGIOENRegisterMap;
	DIGIOBusyStatusLED->setChecked(registerMap.at("BUSY"));
	DIGIOCNVStatusLED->setChecked(registerMap.at("CNV"));
	DIGIOSENTStatusLED->setChecked(registerMap.at("SENT"));
	DIGIOACALCStatusLED->setChecked(registerMap.at("ACALC"));
	DIGIOFaultStatusLED->setChecked(registerMap.at("FAULT"));
	DIGIOBootloaderStatusLED->setChecked(registerMap.at("BOOTLOAD"));

	if(!registerMap.at("BUSY")) DIGIOBusyStatusLED->setText("BUSY (Output)"); 
	else{
		if(registerMap.at("DIGIO0EN")) DIGIOBusyStatusLED->setText("GPIO0 (Output)");
		else DIGIOBusyStatusLED->setText("GPIO0 (Input)");
	}

	if(!registerMap.at("CNV")) DIGIOCNVStatusLED->setText("CNV (Input)");
	else{
		if(registerMap.at("DIGIO1EN")) DIGIOCNVStatusLED->setText("GPIO1 (Output)");
		else DIGIOCNVStatusLED->setText("GPIO1 (Input)");
	}

	if(!registerMap.at("SENT")) DIGIOSENTStatusLED->setText("SENT (Output)");
	else{
		if(registerMap.at("DIGIO2EN")) DIGIOSENTStatusLED->setText("GPIO2 (Output)");
		else DIGIOSENTStatusLED->setText("GPIO2 (Input)");
	}

	if(!registerMap.at("ACALC")) DIGIOACALCStatusLED->setText("ACALC (Output)");
	else{
		if(registerMap.at("DIGIO3EN")) DIGIOACALCStatusLED->setText("GPIO3 (Output)");
		else DIGIOACALCStatusLED->setText("GPIO3 (Input)");
	}

	if(!registerMap.at("FAULT")) DIGIOFaultStatusLED->setText("FAULT (Output)");
	else{
		if(registerMap.at("DIGIO4EN")) DIGIOFaultStatusLED->setText("GPIO4 (Output)");
		else DIGIOFaultStatusLED->setText("GPIO4 (Input)");
	}

	if(!registerMap.at("BOOTLOAD")) DIGIOBootloaderStatusLED->setText("BOOTLOAD (Output)");
	else{
		if(registerMap.at("DIGIO5EN")) DIGIOBootloaderStatusLED->setText("GPIO5 (Output)");
		else DIGIOBootloaderStatusLED->setText("GPIO5 (Input)");
	}
}

void HarmonicCalibration::updateDIGIOControlUI()
{
	map<string, bool> registerMap = DIGIOENRegisterMap;

	DIGIO0ENToggleSwitch->setChecked(registerMap.at("DIGIO0EN"));
	DIGIO1ENToggleSwitch->setChecked(registerMap.at("DIGIO1EN"));
	DIGIO2ENToggleSwitch->setChecked(registerMap.at("DIGIO2EN"));
	DIGIO3ENToggleSwitch->setChecked(registerMap.at("DIGIO3EN"));
	DIGIO4ENToggleSwitch->setChecked(registerMap.at("DIGIO4EN"));
	DIGIO5ENToggleSwitch->setChecked(registerMap.at("DIGIO5EN"));
	DIGIO0FNCToggleSwitch->setChecked(registerMap.at("BUSY"));
	DIGIO1FNCToggleSwitch->setChecked(registerMap.at("CNV"));
	DIGIO2FNCToggleSwitch->setChecked(registerMap.at("SENT"));
	DIGIO3FNCToggleSwitch->setChecked(registerMap.at("ACALC"));
	DIGIO4FNCToggleSwitch->setChecked(registerMap.at("FAULT"));
	DIGIO5FNCToggleSwitch->setChecked(registerMap.at("BOOTLOAD"));
}

void HarmonicCalibration::getDIAG2Register(){
	uint32_t *mtDiag2RegValue = new uint32_t;
	uint32_t *cnvPageRegValue = new uint32_t;
	uint32_t mtDiag2RegisterAddress = m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG2);
	uint32_t mtDiag2PageValue = m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG2);
	uint32_t cnvPageAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, mtDiag2PageValue) != -1){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, cnvPageRegValue) != -1){
			if(*cnvPageRegValue == mtDiag2PageValue){
				if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), mtDiag2RegisterAddress, mtDiag2RegValue) != -1){
					uint16_t *mtDiag2RegValue16 = new uint16_t;
					*mtDiag2RegValue16 = static_cast<uint16_t>(*mtDiag2RegValue);

					Q_EMIT DIAG2RegisterChanged(mtDiag2RegValue16);

					Q_EMIT commandLogWriteSignal("DIAG2: 0b" + QString::number(*mtDiag2RegValue, 2).rightJustified(16, '0'));

					delete mtDiag2RegValue16;
				}
				else{ Q_EMIT commandLogWriteSignal("Failed to read MT Diagnostic 2 Register"); }
			}
			else{ Q_EMIT commandLogWriteSignal("CNVPAGE for MT Diagnostic 2 is a different value, abort reading"); }
		}
		else{ Q_EMIT commandLogWriteSignal("Failed to read CNVPAGE for MT Diagnostic 2"); }
	}
	else{ Q_EMIT commandLogWriteSignal("Failed to write CNVPAGE for MT Diagnostic 2"); }

	delete mtDiag2RegValue, cnvPageRegValue;
}

void HarmonicCalibration::updateMTDiagnosticsUI(uint16_t *registerValue)
{
	DIAG2RegisterMap =  m_admtController->getDiag2RegisterBitMapping(*registerValue);

	map<string, double> regmap = DIAG2RegisterMap;
	afeDiag0 = regmap.at("AFE Diagnostic 0 (-57%)");
	afeDiag1 = regmap.at("AFE Diagnostic 1 (+57%)");
	AFEDIAG0LineEdit->setText(QString::number(afeDiag0) + " V");
	AFEDIAG1LineEdit->setText(QString::number(afeDiag1) + " V");
}

void HarmonicCalibration::getDIAG1Register(){
	uint32_t *mtDiag1RegValue = new uint32_t;
	uint32_t *cnvPageRegValue = new	uint32_t;
	uint32_t mtDiag1RegisterAddress = m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG1);
	uint32_t mtDiag1PageValue = m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG1);
	uint32_t cnvPageAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, mtDiag1PageValue) != -1){
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress, cnvPageRegValue) != -1){
			if(*cnvPageRegValue == mtDiag1PageValue){
				if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), mtDiag1RegisterAddress, mtDiag1RegValue) != -1){
					uint16_t *mtDiag1RegValue16 = new uint16_t;
					*mtDiag1RegValue16 = static_cast<uint16_t>(*mtDiag1RegValue);
					Q_EMIT DIAG1RegisterChanged(mtDiag1RegValue16);

					Q_EMIT commandLogWriteSignal("DIAG1: 0b" + QString::number(*mtDiag1RegValue16, 2).rightJustified(16, '0'));

					delete mtDiag1RegValue16;
				}
				else{ Q_EMIT commandLogWriteSignal("Failed to read MT Diagnostic 1 Register"); }
			}
			else{ Q_EMIT commandLogWriteSignal("CNVPAGE for MT Diagnostic 1 is a different value, abort reading"); }
		}
		else{ Q_EMIT commandLogWriteSignal("Failed to read CNVPAGE for MT Diagnostic 1"); }
	}
	else{ Q_EMIT commandLogWriteSignal("Failed to write CNVPAGE for MT Diagnostic 1"); }

	delete mtDiag1RegValue, cnvPageRegValue;
}

void HarmonicCalibration::updateMTDiagnosticRegisterUI(uint16_t *registerValue)
{
	DIAG1RegisterMap =  m_admtController->getDiag1RegisterBitMapping_Register(*registerValue);
	DIAG1AFERegisterMap = m_admtController->getDiag1RegisterBitMapping_Afe(*registerValue, is5V);

	map<string, bool> regmap = DIAG1RegisterMap;
	map<string, double> afeRegmap = DIAG1AFERegisterMap;
	R0StatusLED->setChecked(regmap.at("R0"));
	R1StatusLED->setChecked(regmap.at("R1"));
	R2StatusLED->setChecked(regmap.at("R2"));
	R3StatusLED->setChecked(regmap.at("R3"));
	R4StatusLED->setChecked(regmap.at("R4"));
	R5StatusLED->setChecked(regmap.at("R5"));
	R6StatusLED->setChecked(regmap.at("R6"));
	R7StatusLED->setChecked(regmap.at("R7"));
	afeDiag2 = afeRegmap.at("AFE Diagnostic 2");
	AFEDIAG2LineEdit->setText(QString::number(afeDiag2) + " V");
}

void HarmonicCalibration::getFAULTRegister(){
	uint32_t *faultRegValue = new uint32_t;
	uint32_t faultRegisterAddress = m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), faultRegisterAddress, 0); // Write all zeros to fault before read

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), faultRegisterAddress, faultRegValue) != -1){
		uint16_t *faultRegValue16 = new uint16_t;
		*faultRegValue16 = static_cast<uint16_t>(*faultRegValue);

		Q_EMIT FaultRegisterChanged(faultRegValue16);

		Q_EMIT commandLogWriteSignal("FAULT: 0b" + QString::number(*faultRegValue, 2).rightJustified(16, '0'));

		delete faultRegValue16;
	}
	else{ Q_EMIT commandLogWriteSignal("Failed to read FAULT Register"); }

	delete faultRegValue;
}

void HarmonicCalibration::updateFaultRegisterUI(uint16_t *faultRegValue)
{
	FAULTRegisterMap = m_admtController->getFaultRegisterBitMapping(*faultRegValue);

	map<string, bool> regmap = FAULTRegisterMap;
	VDDUnderVoltageStatusLED->setChecked(regmap.at("VDD Under Voltage"));
	VDDOverVoltageStatusLED->setChecked(regmap.at("VDD Over Voltage"));
	VDRIVEUnderVoltageStatusLED->setChecked(regmap.at("VDRIVE Under Voltage"));
	VDRIVEOverVoltageStatusLED->setChecked(regmap.at("VDRIVE Over Voltage"));
	AFEDIAGStatusLED->setChecked(regmap.at("AFE Diagnostic"));
	NVMCRCFaultStatusLED->setChecked(regmap.at("NVM CRC Fault"));
	ECCDoubleBitErrorStatusLED->setChecked(regmap.at("ECC Double Bit Error"));
	OscillatorDriftStatusLED->setChecked(regmap.at("Oscillator Drift"));
	CountSensorFalseStateStatusLED->setChecked(regmap.at("Count Sensor False State"));
	AngleCrossCheckStatusLED->setChecked(regmap.at("Angle Cross Check"));
	TurnCountSensorLevelsStatusLED->setChecked(regmap.at("Turn Count Sensor Levels"));
	MTDIAGStatusLED->setChecked(regmap.at("MT Diagnostic"));
	TurnCounterCrossCheckStatusLED->setChecked(regmap.at("Turn Counter Cross Check"));
	RadiusCheckStatusLED->setChecked(regmap.at("AMR Radius Check"));
	SequencerWatchdogStatusLED->setChecked(regmap.at("Sequencer Watchdog"));
}

void HarmonicCalibration::toggleDIGIOEN(string DIGIOENName, bool value)
{
	toggleUtilityTask(false);

	uint32_t *DIGIOENRegisterValue = new uint32_t;
	uint32_t DIGIOENPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN);

	if(changeCNVPage(DIGIOENPage))
	{
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
			m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN), 
			DIGIOENRegisterValue) != -1)
		{
			map<string, bool> DIGIOSettings = m_admtController->getDIGIOENRegisterBitMapping(static_cast<uint16_t>(*DIGIOENRegisterValue));

			DIGIOSettings.at(DIGIOENName) = value;

			uint16_t newRegisterValue = m_admtController->setDIGIOENRegisterBitMapping(static_cast<uint16_t>(*DIGIOENRegisterValue), DIGIOSettings);

			if(changeCNVPage(DIGIOENPage)){
				if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
														m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN), 
														static_cast<uint32_t>(newRegisterValue)) != -1)
				{
					updateDIGIOControlUI();
				}
			}

		}
	}

	toggleUtilityTask(true);
}

void HarmonicCalibration::toggleMTDiagnostics(int mode)
{
	switch(mode){
		case 0:
			MTDiagnosticsScrollArea->hide();
			hasMTDiagnostics = false;
			break;
		case 1:
			MTDiagnosticsScrollArea->show();
			hasMTDiagnostics = true;
			break;
	}
}

void HarmonicCalibration::toggleFaultRegisterMode(int mode)
{
	switch(mode){
		case 0:
			AFEDIAGStatusLED->hide();
			OscillatorDriftStatusLED->hide();
			AngleCrossCheckStatusLED->hide();
			TurnCountSensorLevelsStatusLED->hide();
			MTDIAGStatusLED->hide();
			SequencerWatchdogStatusLED->hide();
			break;
		case 1: 
			AFEDIAGStatusLED->show();
			OscillatorDriftStatusLED->show();
			AngleCrossCheckStatusLED->show();
			TurnCountSensorLevelsStatusLED->show();
			MTDIAGStatusLED->show();
			SequencerWatchdogStatusLED->show();
			break;
	}
}

bool HarmonicCalibration::resetDIGIO()
{
	return (m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN), 
										0x241b) == 0 ? true : false);
}

void HarmonicCalibration::commandLogWrite(QString message)
{
	commandLogPlainTextEdit->appendPlainText(message);
}

void HarmonicCalibration::clearCommandLog(){
	commandLogPlainTextEdit->clear();
}
#pragma endregion

#pragma region Register Methods
void HarmonicCalibration::readAllRegisters()
{
	readAllRegistersButton->setEnabled(false);
	readAllRegistersButton->setText(QString("Reading Registers..."));
	QTimer::singleShot(1000, this, [this](){
		readAllRegistersButton->setEnabled(true);
		readAllRegistersButton->setText(QString("Read All Registers"));
	});

	cnvPageRegisterBlock->readButton()->click();
	digIORegisterBlock->readButton()->click();
	faultRegisterBlock->readButton()->click();
	generalRegisterBlock->readButton()->click();
	digIOEnRegisterBlock->readButton()->click();
	eccDcdeRegisterBlock->readButton()->click();
	eccDisRegisterBlock->readButton()->click();
	absAngleRegisterBlock->readButton()->click();
	angleRegisterBlock->readButton()->click();
	sineRegisterBlock->readButton()->click();
	cosineRegisterBlock->readButton()->click();
	tmp0RegisterBlock->readButton()->click();
	cnvCntRegisterBlock->readButton()->click();
	uniqID0RegisterBlock->readButton()->click();
	uniqID1RegisterBlock->readButton()->click();
	uniqID2RegisterBlock->readButton()->click();
	uniqID3RegisterBlock->readButton()->click();
	h1MagRegisterBlock->readButton()->click();
	h1PhRegisterBlock->readButton()->click();
	h2MagRegisterBlock->readButton()->click();
	h2PhRegisterBlock->readButton()->click();
	h3MagRegisterBlock->readButton()->click();
	h3PhRegisterBlock->readButton()->click();
	h8MagRegisterBlock->readButton()->click();
	h8PhRegisterBlock->readButton()->click();

	if(GENERALRegisterMap.at("Sequence Type") == 1){
		angleSecRegisterBlock->readButton()->click();
		secAnglIRegisterBlock->readButton()->click();
		secAnglQRegisterBlock->readButton()->click();
		tmp1RegisterBlock->readButton()->click();
		angleCkRegisterBlock->readButton()->click();
		radiusRegisterBlock->readButton()->click();
		diag1RegisterBlock->readButton()->click();
		diag2RegisterBlock->readButton()->click();
	}
}

void HarmonicCalibration::toggleRegisters(int mode)
{
	switch(mode){
		case 0:
			angleSecRegisterBlock->hide();
			secAnglIRegisterBlock->hide();
			secAnglQRegisterBlock->hide();
			tmp1RegisterBlock->hide();
			angleCkRegisterBlock->hide();
			radiusRegisterBlock->hide();
			diag1RegisterBlock->hide();
			diag2RegisterBlock->hide();
			break;
		case 1:
			angleSecRegisterBlock->show();
			secAnglIRegisterBlock->show();
			secAnglQRegisterBlock->show();
			tmp1RegisterBlock->show();
			angleCkRegisterBlock->show();
			radiusRegisterBlock->show();
			diag1RegisterBlock->show();
			diag2RegisterBlock->show();
			break;
	}
}
#pragma endregion

#pragma region UI Helper Methods
void HarmonicCalibration::updateLabelValue(QLabel* label, int channelIndex)
{
	switch(channelIndex)
	{
		case ADMTController::Channel::ROTATION:
			label->setText(QString("%1").arg(rotation, 0, 'f', 2) + "°");
			break;
		case ADMTController::Channel::ANGLE:
			label->setText(QString("%1").arg(angle, 0, 'f', 2) + "°");
			break;
		case ADMTController::Channel::COUNT:
			label->setText(QString::number(count));
			break;
		case ADMTController::Channel::TEMPERATURE:
			label->setText(QString("%1").arg(temp, 0, 'f', 2) + "°C");
			break;
	}
}

void HarmonicCalibration::updateLabelValue(QLabel *label, ADMTController::MotorAttribute attribute)
{
	switch(attribute)
	{
		case ADMTController::MotorAttribute::AMAX:
			label->setText(QString::number(amax));
			break;
		case ADMTController::MotorAttribute::ROTATE_VMAX:
			label->setText(QString::number(rotate_vmax));
			break;
		case ADMTController::MotorAttribute::DMAX:
			label->setText(QString::number(dmax));
			break;
		case ADMTController::MotorAttribute::DISABLE:
			label->setText(QString::number(disable));
			break;
		case ADMTController::MotorAttribute::TARGET_POS:
			label->setText(QString::number(target_pos));
			break;
		case ADMTController::MotorAttribute::CURRENT_POS:
			label->setText(QString::number(current_pos));
			break;
		case ADMTController::MotorAttribute::RAMP_MODE:
			label->setText(QString::number(ramp_mode));
			break;
	}
}

bool HarmonicCalibration::updateChannelValue(int channelIndex)
{
	bool success = false;
	switch(channelIndex)
	{
		case ADMTController::Channel::ROTATION:
			rotation = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), rotationChannelName, 1);
			if(rotation == static_cast<double>(UINT64_MAX)) { success = false; }
			break;
		case ADMTController::Channel::ANGLE:
			angle = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), angleChannelName, 1);
			if(angle == static_cast<double>(UINT64_MAX)) { success = false; }
			break;
		case ADMTController::Channel::COUNT:
			count = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), countChannelName, 1);
			if(count == static_cast<double>(UINT64_MAX)) { success = false; }
			break;
		case ADMTController::Channel::TEMPERATURE:
			temp = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), temperatureChannelName, 1);
			if(temp == static_cast<double>(UINT64_MAX)) { success = false; }
			break;
	}
	return success;
}

void HarmonicCalibration::updateLineEditValue(QLineEdit* lineEdit, double value){
	if(value == static_cast<double>(UINT64_MAX)) { lineEdit->setText("N/A"); }
	else { lineEdit->setText(QString::number(value)); }
}

void HarmonicCalibration::toggleWidget(QPushButton *widget, bool value){
	widget->setEnabled(value);
}

void HarmonicCalibration::changeCustomSwitchLabel(CustomSwitch *customSwitch, QString onLabel, QString offLabel)
{
	customSwitch->setOnText(onLabel);
	customSwitch->setOffText(offLabel);
}

QCheckBox *HarmonicCalibration::createStatusLEDWidget(const QString &text, QVariant variant, bool checked, QWidget *parent)
{
	QCheckBox *checkBox = new QCheckBox(text, parent);
	Style::setStyle(checkBox, style::properties::admt::checkBoxLED, variant, true);
	checkBox->setChecked(checked);
	checkBox->setEnabled(false);
	return checkBox;
}

MenuControlButton *HarmonicCalibration::createChannelToggleWidget(const QString title, QColor color, QWidget *parent)
{
	MenuControlButton *menuControlButton = new MenuControlButton(parent);
	menuControlButton->setName(title);
	menuControlButton->setCheckBoxStyle(MenuControlButton::CheckboxStyle::CS_CIRCLE);
	menuControlButton->setOpenMenuChecksThis(true);
	menuControlButton->setDoubleClickToOpenMenu(true);
	menuControlButton->setColor(color);
	menuControlButton->button()->setVisible(false);
	menuControlButton->setCheckable(false);
	menuControlButton->checkBox()->setChecked(true);
	menuControlButton->layout()->setMargin(0);
	return menuControlButton;
}
#pragma endregion

#pragma region Connect Methods
void HarmonicCalibration::connectLineEditToNumber(QLineEdit* lineEdit, int& variable, int min, int max)
{
	QIntValidator *validator = new QIntValidator(min, max, this);
	lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit, min, max]() {
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok && value >= min && value <= max) {
            variable = value;
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectLineEditToNumber(QLineEdit* lineEdit, double& variable, QString unit)
{
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit, unit]() {
        bool ok;
        double value = lineEdit->text().replace(unit, "").trimmed().toDouble(&ok);
        if (ok) {
            variable = value;
			
        } else {
            lineEdit->setText(QString::number(variable) + " " + unit);
        }
    });
}

void HarmonicCalibration::connectLineEditToDouble(QLineEdit* lineEdit, double& variable)
{
	QDoubleValidator *validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit]() {
        bool ok;
        double value = lineEdit->text().toDouble(&ok);
        if (ok){
            variable = value;
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectLineEditToNumberWrite(QLineEdit* lineEdit, double& variable, ADMTController::MotorAttribute attribute)
{
	QDoubleValidator *validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::editingFinished, [=, &variable]() {
        bool ok;
        double value = lineEdit->text().toDouble(&ok);
        if (ok) {
            variable = value;
			writeMotorAttributeValue(attribute, variable);
			
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectMenuComboToNumber(MenuCombo* menuCombo, double& variable)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=, &variable]() {
		variable = qvariant_cast<int>(combo->currentData());
	});
}

void HarmonicCalibration::connectMenuComboToNumber(MenuCombo* menuCombo, int& variable)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=, &variable]() {
		variable = qvariant_cast<int>(combo->currentData());
	});
}

void HarmonicCalibration::connectLineEditToRPSConversion(QLineEdit* lineEdit, double& vmax)
{
	connect(lineEdit, &QLineEdit::editingFinished, [=, &vmax]() {
        bool ok;
        double rps = lineEdit->text().toDouble(&ok);
        if (ok) {
            vmax = convertRPStoVMAX(rps);
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, vmax);
			writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
			amax = convertAccelTimetoAMAX(motorAccelTime);
			writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
			// StatusBarManager::pushMessage("Applied VMAX: " + QString::number(vmax));
			// StatusBarManager::pushMessage("Applied AMAX: " + QString::number(amax));
        } else {
            lineEdit->setText(QString::number(convertVMAXtoRPS(vmax)));
        }
    });
}

void HarmonicCalibration::connectLineEditToAMAXConversion(QLineEdit* lineEdit, double& amax)
{
	connect(lineEdit, &QLineEdit::editingFinished, [=, &amax]() {
        bool ok;
        double accelTime = lineEdit->text().toDouble(&ok);
        if (ok) {
            amax = convertAccelTimetoAMAX(accelTime);
			// StatusBarManager::pushMessage("Applied AMAX: " + QString::number(amax));
        } else {
            lineEdit->setText(QString::number(convertAMAXtoAccelTime(amax)));
        }
    });
}

void HarmonicCalibration::connectRegisterBlockToRegistry(RegisterBlockWidget* widget)
{
	uint32_t *readValue = new uint32_t;
	connect(widget->readButton(), &QPushButton::clicked, this, [=]{
		bool ok = false, success = false;

		if(widget->getCnvPage() != UINT32_MAX)
		{
			ok = this->changeCNVPage(widget->getCnvPage());
		}
		else { ok = true; }

		if(ok){
			if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), readValue) == 0)
			{ widget->setValue(*readValue); }
		}
		else{ StatusBarManager::pushMessage("Failed to read registry"); }
	});
	if(widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::READWRITE || 
	   widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::WRITE){
		connect(widget->writeButton(), &QPushButton::clicked, this, [=]{
			bool ok = false, success = false;

			if(widget->getCnvPage() != UINT32_MAX)
			{
				ok = this->changeCNVPage(widget->getCnvPage());
			}
			else { ok = true; }

			if(ok){
				if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), widget->getValue()) == 0)
				{ 
					if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), readValue) == 0)
					{ 
						widget->setValue(*readValue); 
						success = true;
					}
				}
			}
			
			if(!success) { StatusBarManager::pushMessage("Failed to write to registry"); }
		});
	}
}

void HarmonicCalibration::connectLineEditToRPM(QLineEdit* lineEdit, double& variable)
{
	QDoubleValidator *validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	lineEdit->setValidator(validator);
    connect(lineEdit, &QLineEdit::editingFinished, this, [=, &variable]() {
        bool ok;
        double value = lineEdit->text().toDouble(&ok);
        if (ok){
            variable = value;
			rotate_vmax = convertRPStoVMAX(convertRPMtoRPS(variable));
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
			writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
			amax = convertAccelTimetoAMAX(motorAccelTime);
			writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}
#pragma endregion

#pragma region Convert Methods
double HarmonicCalibration::convertRPStoVMAX(double rps) 
{ 
	return (rps * motorMicrostepPerRevolution * motorTimeUnit); 
}

double HarmonicCalibration::convertVMAXtoRPS(double vmax)
{
	return (vmax / motorMicrostepPerRevolution / motorTimeUnit);
}

double HarmonicCalibration::convertAccelTimetoAMAX(double accelTime)
{
	return (rotate_vmax * static_cast<double>(1 << 17) / accelTime / motorfCLK); // 1 << 17 = 2^17 = 131072
}

double HarmonicCalibration::convertAMAXtoAccelTime(double amax)
{
	return ((rotate_vmax * static_cast<double>(1 << 17)) / (amax * motorfCLK)); // 1 << 17 = 2^17 = 131072
}

double HarmonicCalibration::convertRPMtoRPS(double rpm)
{
	return (rpm / 60);
}

double HarmonicCalibration::convertRPStoRPM(double rps)
{
	return (rps * 60);
}
#pragma endregion

#pragma region Debug Methods
QString HarmonicCalibration::readRegmapDumpAttributeValue()
{
	QString output = "";
	char value[1024];
	int result = -1;
	result = m_admtController->getDeviceAttributeValueString(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
													m_admtController->getDeviceAttribute(ADMTController::DeviceAttribute::REGMAP_DUMP), 
													value, 1024);
	output = QString(value);
	return output;
}
#pragma endregion