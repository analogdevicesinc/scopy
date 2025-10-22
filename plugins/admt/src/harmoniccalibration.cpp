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
#include "pluginbase/preferences.h"

#include <stylehelper.h>

using namespace scopy;
using namespace scopy::admt;

// In milliseconds
static int motorWaitVelocityReachedSampleRate = 50;
static int calibrationUITimerRate = 500;
static int utilityUITimerRate = 1000;
static int deviceStatusMonitorRate = 500;
static int motorPositionMonitorRate = 500;
static int readMotorDebounce = 50;
static int readDeviceDebounce = 60;

// In nanoseconds
static int continuousCalibrationSampleRate = 10000000;

static int bufferSize = 5;
static int cycleCount = 11;
static int samplesPerCycle = 47;
static int totalSamplesCount = 512;
static bool isStartMotor = false;
static bool isPostCalibration = false;
static bool isCalculatedCoeff = false;
static bool isAngleDisplayFormat = false;
static bool resetToZero = true;
static bool hasMTDiagnostics = false;
static bool isMotorRotationClockwise = true;

static double default_motor_rpm = 60;
static double fast_motor_rpm = 200;
static double motorFullStepAngle = 0.9;	 // TODO input as configuration
static double microStepResolution = 256; // TODO input as configuration
static int motorfCLK = 12500000;	 // 12.5 Mhz, TODO input as configuration
static double motorAccelTime = 1;	 // In seconds

static double motorFullStepPerRevolution = 360 / motorFullStepAngle;
static double motorMicrostepPerRevolution = motorFullStepPerRevolution * microStepResolution;
static double motorTimeUnit = static_cast<double>(1 << 24) / motorfCLK; // t = 2^24/motorfCLK

static int acquisitionDisplayLength = 200;
static int turnCount = 0;

static const QColor scopyBlueColor = Style::getColor(json::theme::interactive_primary_idle);

QPen scopyBluePen(scopyBlueColor);

QString deviceName = "", deviceType = "";
static bool is5V = false;

static double H1_MAG_ANGLE, H2_MAG_ANGLE, H3_MAG_ANGLE, H8_MAG_ANGLE, H1_PHASE_ANGLE, H2_PHASE_ANGLE, H3_PHASE_ANGLE,
	H8_PHASE_ANGLE;
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

static int calibrationMode = 0;

static int globalSpacingSmall = Style::getDimension(json::global::unit_0_5);
static int globalSpacingMedium = Style::getDimension(json::global::unit_1);

static double defaultAngleErrorGraphMin = -3;
static double defaultAngleErrorGraphMax = 3;
static double currentAngleErrorGraphMin = defaultAngleErrorGraphMin;
static double currentAngleErrorGraphMax = defaultAngleErrorGraphMax;

static double defaultMagnitudeGraphMin = 0;
static double defaultMagnitudeGraphMax = 1.2;
static double currentMagnitudeGraphMin = defaultMagnitudeGraphMin;
static double currentMagnitudeGraphMax = defaultMagnitudeGraphMax;

static double defaultPhaseGraphMin = -4;
static double defaultPhaseGraphMax = 4;
static double currentPhaseGraphMin = defaultPhaseGraphMin;
static double currentPhaseGraphMax = defaultPhaseGraphMax;

static int sequenceMode = 0;   // 0: Sequence Mode 1, 1: Sequence Mode 2
static int conversionMode = 0; // 0: One-shot Conversion, 1: Continuous Conversion
static int readMode = 2;       // 0: Channel Read, 1: Register Read, 2: Mixed

QMap<SensorData, bool> acquisitionDataMap = {
	{ABSANGLE, false}, {ANGLE, false},    {ANGLESEC, false}, {SINE, false},	 {COSINE, false},
	{SECANGLI, false}, {SECANGLQ, false}, {RADIUS, false},	 {DIAG1, false}, {DIAG2, false},
	{TMP0, false},	   {TMP1, false},     {CNVCNT, false},
};

HarmonicCalibration::HarmonicCalibration(ADMTController *m_admtController, bool isDebug, QWidget *parent)
	: QWidget(parent)
	, isDebug(isDebug)
	, m_admtController(m_admtController)
{
	readDeviceProperties();
	initializeADMT();
	initializeMotor();
	readSequence();

	rotationChannelName = m_admtController->getChannelId(ADMTController::Channel::ROTATION);
	angleChannelName = m_admtController->getChannelId(ADMTController::Channel::ANGL);
	countChannelName = m_admtController->getChannelId(ADMTController::Channel::COUNT);
	temperatureChannelName = m_admtController->getChannelId(ADMTController::Channel::TEMPERATURE);

	initializeChannelColors();

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

	connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
		tabWidget->setCurrentIndex(index);

		if(index == 0 || index == 1)
			startDeviceStatusMonitor();
		else
			stopDeviceStatusMonitor();

		if(index == 0) // Acquisition Tab
		{
			isAcquisitionTab = true;
			readSequence();
			updateSequenceWidget();
			updateAcquisitionMotorRPM();
			updateAcquisitionMotorRotationDirection();
		} else {
			isAcquisitionTab = false;
			stop();
		}

		if(index == 1) // Calibration Tab
		{
			isCalibrationTab = true;
			startCalibrationUITask();
			updateCalibrationMotorRPM();
			updateCalibrationMotorRotationDirection();
		} else {
			isCalibrationTab = false;
			stopCalibrationUITask();
		}

		if(index == 2) // Utility Tab
		{
			readSequence();
			toggleFaultRegisterMode(sequenceMode);
			toggleMTDiagnostics(sequenceMode);
			toggleUtilityTask(true);
		} else {
			toggleUtilityTask(false);
		}

		if(index == 3) // Registers Tab
		{
			readSequence();
			toggleRegisters(sequenceMode);
		}
	});

	qRegisterMetaType<QMap<SensorData, double>>("QMap<SensorData, double>");
	connect(this, &HarmonicCalibration::acquisitionDataChanged, this, &HarmonicCalibration::updateAcquisitionData);
	connect(this, &HarmonicCalibration::acquisitionGraphChanged, this,
		&HarmonicCalibration::updateAcquisitionGraph);
	connect(this, &HarmonicCalibration::calibrationGraphChanged, this,
		&HarmonicCalibration::updateCalibrationGraph);
	connect(this, &HarmonicCalibration::updateFaultStatusSignal, this, &HarmonicCalibration::updateFaultStatus);
	connect(this, &HarmonicCalibration::calibrationLogWriteSignal, this, &HarmonicCalibration::calibrationLogWrite);
	connect(this, &HarmonicCalibration::commandLogWriteSignal, this, &HarmonicCalibration::commandLogWrite);
	connect(this, &HarmonicCalibration::DIGIORegisterChanged, this, &HarmonicCalibration::updateDIGIOUI);
	connect(this, &HarmonicCalibration::FaultRegisterChanged, this, &HarmonicCalibration::updateFaultRegisterUI);
	connect(this, &HarmonicCalibration::DIAG1RegisterChanged, this,
		&HarmonicCalibration::updateMTDiagnosticRegisterUI);
	connect(this, &HarmonicCalibration::DIAG2RegisterChanged, this, &HarmonicCalibration::updateMTDiagnosticsUI);
	qRegisterMetaType<QMap<QString, double>>("QMap<QString, double>");
	connect(m_admtController, &ADMTController::streamChannelData, this,
		&HarmonicCalibration::handleStreamChannelData);
	connect(m_admtController, &ADMTController::streamData, this, &HarmonicCalibration::handleStreamCalibrationData);

	isAcquisitionTab = true;
	startDeviceStatusMonitor();
}

HarmonicCalibration::~HarmonicCalibration() { stopTasks(); }

ToolTemplate *HarmonicCalibration::createAcquisitionWidget()
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
	MenuCollapseSection *rotationSection =
		new MenuCollapseSection("ABS Angle", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, rotationWidget);
	MenuCollapseSection *angleSection =
		new MenuCollapseSection("Angle", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, angleWidget);
	MenuCollapseSection *countSection =
		new MenuCollapseSection("Count", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, countWidget);
	MenuCollapseSection *tempSection =
		new MenuCollapseSection("Temp 0", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, tempWidget);
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
	MenuCollapseSection *motorControlCollapseSection = new MenuCollapseSection(
		"Motor Control", MenuCollapseSection::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, motorControlSectionWidget);
	motorControlSectionWidget->contentLayout()->addWidget(motorControlCollapseSection);

	QWidget *motorRPMWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorRPMLayout = new QVBoxLayout(motorRPMWidget);
	motorRPMWidget->setLayout(motorRPMLayout);
	motorRPMLayout->setMargin(0);
	motorRPMLayout->setSpacing(0);
	QLabel *motorRPMLabel = new QLabel("Motor RPM", motorRPMWidget);
	Style::setStyle(motorRPMLabel, style::properties::label::subtle);
	acquisitionMotorRPMLineEdit = new QLineEdit(QString::number(motor_rpm), motorRPMWidget);
	connectLineEditToRPM(acquisitionMotorRPMLineEdit, motor_rpm, 1, fast_motor_rpm);
	motorRPMLayout->addWidget(motorRPMLabel);
	motorRPMLayout->addWidget(acquisitionMotorRPMLineEdit);

	QWidget *motorDirectionWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *motorDirectionLayout = new QVBoxLayout(motorDirectionWidget);
	motorDirectionWidget->setLayout(motorDirectionLayout);
	motorDirectionLayout->setMargin(0);
	motorDirectionLayout->setSpacing(0);
	QLabel *motorDirectionLabel = new QLabel("Rotation Direction", motorDirectionWidget);
	Style::setStyle(motorDirectionLabel, style::properties::label::subtle);
	acquisitionMotorDirectionSwitch = new CustomSwitch("CW", "CCW", motorControlSectionWidget);
	acquisitionMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
	connect(acquisitionMotorDirectionSwitch, &CustomSwitch::toggled, this,
		[this](bool b) { isMotorRotationClockwise = b; });
	motorDirectionLayout->addWidget(motorDirectionLabel);
	motorDirectionLayout->addWidget(acquisitionMotorDirectionSwitch);

	QWidget *turnCountWidget = new QWidget(motorControlSectionWidget);
	QVBoxLayout *turnCountLayout = new QVBoxLayout(turnCountWidget);
	turnCountWidget->setLayout(turnCountLayout);
	turnCountLayout->setMargin(0);
	turnCountLayout->setSpacing(0);
	QLabel *turnCountLabel = new QLabel("Turn Count", turnCountWidget);
	Style::setStyle(turnCountLabel, style::properties::label::subtle);
	QLineEdit *turnCountLineEdit = new QLineEdit(QString::number(turnCount), turnCountWidget);
	connectLineEditToMotorTurnCount(turnCountLineEdit, turnCount, 0, 100);
	turnCountLayout->addWidget(turnCountLabel);
	turnCountLayout->addWidget(turnCountLineEdit);

	QPushButton *resetMotorPositionButton = new QPushButton("Reset Motor Position", motorControlSectionWidget);
	StyleHelper::BasicButton(resetMotorPositionButton);
	connect(resetMotorPositionButton, &QPushButton::clicked, this, &HarmonicCalibration::startResetMotorToZero);

	QPushButton *continuousRotationButton = new QPushButton("Continuous Rotation", motorControlSectionWidget);
	StyleHelper::BasicButton(continuousRotationButton);
	connect(continuousRotationButton, &QPushButton::clicked, this, &HarmonicCalibration::moveMotorContinuous);

	QPushButton *stopMotorButton = new QPushButton("Stop Motor", motorControlSectionWidget);
	StyleHelper::BasicButton(stopMotorButton);
	connect(stopMotorButton, &QPushButton::clicked, this, &HarmonicCalibration::stopMotor);

	motorControlCollapseSection->contentLayout()->setSpacing(globalSpacingSmall);
	motorControlCollapseSection->contentLayout()->addWidget(motorRPMWidget);
	motorControlCollapseSection->contentLayout()->addWidget(motorDirectionWidget);
	motorControlCollapseSection->contentLayout()->addWidget(turnCountWidget);
	motorControlCollapseSection->contentLayout()->addWidget(resetMotorPositionButton);
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
	acquisitionGraphSectionWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	acquisitionGraphSectionWidget->contentLayout()->setSpacing(globalSpacingSmall);

	QLabel *acquisitionGraphLabel = new QLabel("Captured Data", acquisitionGraphSectionWidget);
	Style::setStyle(acquisitionGraphLabel, style::properties::label::menuMedium);

	acquisitionGraphPlotWidget = new PlotWidget();
	acquisitionGraphPlotWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	acquisitionXPlotAxis = new PlotAxis(QwtAxis::XBottom, acquisitionGraphPlotWidget, scopyBluePen);
	PrefixFormatter *acquisitionXFormatter = new PrefixFormatter({{" ", 1E0}}, acquisitionXPlotAxis);
	acquisitionXFormatter->setTrimZeroes(true);
	acquisitionXPlotAxis->setFormatter(acquisitionXFormatter);
	acquisitionXPlotAxis->setInterval(0, acquisitionDisplayLength);
	acquisitionXPlotAxis->scaleDraw()->setFloatPrecision(0);

	acquisitionYPlotAxis = new PlotAxis(QwtAxis::YLeft, acquisitionGraphPlotWidget, scopyBluePen);
	PrefixFormatter *acquisitionYFormatter = new PrefixFormatter({{" ", 1E0}}, acquisitionYPlotAxis);
	acquisitionYFormatter->setTrimZeroes(true);
	acquisitionYPlotAxis->setFormatter(acquisitionYFormatter);
	acquisitionYPlotAxis->setInterval(0, 360);

	acquisitionAnglePlotChannel = new PlotChannel("Angle", channel0Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSinePlotChannel = new PlotChannel("Sine", channel1Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionCosinePlotChannel =
		new PlotChannel("Cosine", channel2Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionRadiusPlotChannel =
		new PlotChannel("Radius", channel3Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionABSAnglePlotChannel =
		new PlotChannel("ABS Angle", channel4Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionTmp0PlotChannel = new PlotChannel("TMP 0", channel5Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionAngleSecPlotChannel =
		new PlotChannel("SEC Angle", channel6Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSecAnglQPlotChannel =
		new PlotChannel("SECANGLQ", channel7Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionSecAnglIPlotChannel =
		new PlotChannel("SECANGLI", channel0Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);
	acquisitionTmp1PlotChannel = new PlotChannel("TMP 1", channel1Pen, acquisitionXPlotAxis, acquisitionYPlotAxis);

	acquisitionGraphPlotWidget->addPlotChannel(acquisitionAnglePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionABSAnglePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionTmp0PlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSinePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionCosinePlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionRadiusPlotChannel);

	acquisitionGraphPlotWidget->addPlotChannel(acquisitionAngleSecPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSecAnglQPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionSecAnglIPlotChannel);
	acquisitionGraphPlotWidget->addPlotChannel(acquisitionTmp1PlotChannel);
	// Industrial
	acquisitionAnglePlotChannel->setEnabled(true);
	acquisitionABSAnglePlotChannel->setEnabled(false);
	acquisitionTmp0PlotChannel->setEnabled(false);
	acquisitionSinePlotChannel->setEnabled(false);
	acquisitionCosinePlotChannel->setEnabled(false);
	acquisitionRadiusPlotChannel->setEnabled(false);
	// Automotive
	acquisitionAngleSecPlotChannel->setEnabled(false);
	acquisitionSecAnglQPlotChannel->setEnabled(false);
	acquisitionSecAnglIPlotChannel->setEnabled(false);
	acquisitionTmp1PlotChannel->setEnabled(false);

	acquisitionGraphPlotWidget->selectChannel(acquisitionAnglePlotChannel);

	acquisitionGraphPlotWidget->setShowXAxisLabels(true);
	acquisitionGraphPlotWidget->setShowYAxisLabels(true);
	acquisitionGraphPlotWidget->showAxisLabels();
	acquisitionGraphPlotWidget->replot();

#pragma region Channel Selection
	acquisitionGraphChannelWidget = new QWidget(acquisitionGraphSectionWidget);
	Style::setStyle(acquisitionGraphChannelWidget, style::properties::widget::basicBackground);
	acquisitionGraphChannelGridLayout = new QGridLayout(acquisitionGraphChannelWidget);
	acquisitionGraphChannelGridLayout->setContentsMargins(globalSpacingSmall, globalSpacingSmall,
							      globalSpacingSmall, globalSpacingSmall);
	acquisitionGraphChannelGridLayout->setSpacing(globalSpacingSmall);

	angleCheckBox = new QCheckBox("Angle", acquisitionGraphChannelWidget);
	Style::setStyle(angleCheckBox, style::properties::admt::coloredCheckBox, "ch0");
	connectCheckBoxToAcquisitionGraph(angleCheckBox, acquisitionAnglePlotChannel, ANGLE);
	angleCheckBox->setChecked(true);

	sineCheckBox = new QCheckBox("Sine", acquisitionGraphChannelWidget);
	Style::setStyle(sineCheckBox, style::properties::admt::coloredCheckBox, "ch1");
	connectCheckBoxToAcquisitionGraph(sineCheckBox, acquisitionSinePlotChannel, SINE);

	cosineCheckBox = new QCheckBox("Cosine", acquisitionGraphChannelWidget);
	Style::setStyle(cosineCheckBox, style::properties::admt::coloredCheckBox, "ch2");
	connectCheckBoxToAcquisitionGraph(cosineCheckBox, acquisitionCosinePlotChannel, COSINE);

	radiusCheckBox = new QCheckBox("Radius", acquisitionGraphChannelWidget);
	Style::setStyle(radiusCheckBox, style::properties::admt::coloredCheckBox, "ch3");
	connectCheckBoxToAcquisitionGraph(radiusCheckBox, acquisitionRadiusPlotChannel, RADIUS);

	absAngleCheckBox = new QCheckBox("ABS Angle", acquisitionGraphChannelWidget);
	Style::setStyle(absAngleCheckBox, style::properties::admt::coloredCheckBox, "ch4");
	connectCheckBoxToAcquisitionGraph(absAngleCheckBox, acquisitionABSAnglePlotChannel, ABSANGLE);

	temp0CheckBox = new QCheckBox("Temp 0", acquisitionGraphChannelWidget);
	Style::setStyle(temp0CheckBox, style::properties::admt::coloredCheckBox, "ch5");
	connectCheckBoxToAcquisitionGraph(temp0CheckBox, acquisitionTmp0PlotChannel, TMP0);

	angleSecCheckBox = new QCheckBox("SEC Angle", acquisitionGraphChannelWidget);
	Style::setStyle(angleSecCheckBox, style::properties::admt::coloredCheckBox, "ch6");
	connectCheckBoxToAcquisitionGraph(angleSecCheckBox, acquisitionAngleSecPlotChannel, ANGLESEC);

	secAnglQCheckBox = new QCheckBox("SECANGLQ", acquisitionGraphChannelWidget);
	Style::setStyle(secAnglQCheckBox, style::properties::admt::coloredCheckBox, "ch7");
	connectCheckBoxToAcquisitionGraph(secAnglQCheckBox, acquisitionSecAnglQPlotChannel, SECANGLQ);

	secAnglICheckBox = new QCheckBox("SECANGLI", acquisitionGraphChannelWidget);
	Style::setStyle(secAnglICheckBox, style::properties::admt::coloredCheckBox, "ch0");
	connectCheckBoxToAcquisitionGraph(secAnglICheckBox, acquisitionSecAnglIPlotChannel, SECANGLI);

	temp1CheckBox = new QCheckBox("Temp 1", acquisitionGraphChannelWidget);
	Style::setStyle(temp1CheckBox, style::properties::admt::coloredCheckBox, "ch1");
	connectCheckBoxToAcquisitionGraph(temp1CheckBox, acquisitionTmp1PlotChannel, TMP1);

	if(sequenceMode == 0) // Sequence Mode 1
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 1);
		angleSecCheckBox->hide();
		secAnglICheckBox->hide();
		secAnglQCheckBox->hide();
		temp1CheckBox->hide();
	} else if(sequenceMode == 1) // Sequence Mode 2
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(angleSecCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(secAnglQCheckBox, 0, 4);
		acquisitionGraphChannelGridLayout->addWidget(secAnglICheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 1, 1);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 2);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 3);
		acquisitionGraphChannelGridLayout->addWidget(temp1CheckBox, 1, 4);
	}

#pragma endregion

	acquisitionGraphSectionWidget->contentLayout()->addWidget(acquisitionGraphLabel);
	acquisitionGraphSectionWidget->contentLayout()->addWidget(acquisitionGraphPlotWidget);
	acquisitionGraphSectionWidget->contentLayout()->addWidget(acquisitionGraphChannelWidget);
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
	MenuCollapseSection *generalSection =
		new MenuCollapseSection("Data Acquisition", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, generalWidget);
	generalSection->contentLayout()->setSpacing(globalSpacingSmall);
	generalWidget->contentLayout()->addWidget(generalSection);

	// Data Sample Size
	QWidget *displayLengthWidget = new QWidget(generalSection);
	QVBoxLayout *displayLengthLayout = new QVBoxLayout(displayLengthWidget);
	displayLengthWidget->setLayout(displayLengthLayout);
	displayLengthLayout->setMargin(0);
	displayLengthLayout->setSpacing(0);
	QLabel *displayLengthLabel = new QLabel("Display Length", displayLengthWidget);
	Style::setStyle(displayLengthLabel, style::properties::label::subtle);
	displayLengthLineEdit = new QLineEdit(QString::number(acquisitionDisplayLength), displayLengthWidget);
	Style::setStyle(displayLengthLineEdit, style::properties::admt::lineEdit);
	connectLineEditToNumber(displayLengthLineEdit, acquisitionDisplayLength, 1, 2048);
	displayLengthLayout->addWidget(displayLengthLabel);
	displayLengthLayout->addWidget(displayLengthLineEdit);

	QPushButton *resetYAxisButton = new QPushButton("Reset Graph Scale", generalSection);
	StyleHelper::BasicButton(resetYAxisButton);
	connect(resetYAxisButton, &QPushButton::clicked, this, &HarmonicCalibration::resetAcquisitionYAxisScale);

	generalSection->contentLayout()->addWidget(displayLengthWidget);
	generalSection->contentLayout()->addWidget(resetYAxisButton);

	MenuSectionWidget *sequenceWidget = new MenuSectionWidget(generalSettingWidget);
	Style::setStyle(sequenceWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *sequenceSection =
		new MenuCollapseSection("Sequence", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, sequenceWidget);
	sequenceWidget->contentLayout()->addWidget(sequenceSection);
	sequenceSection->contentLayout()->setSpacing(globalSpacingSmall);

	sequenceModeMenuCombo = new MenuCombo("Sequence Mode", sequenceSection);
	QComboBox *sequenceModeComboBox = sequenceModeMenuCombo->combo();
	sequenceModeComboBox->addItem("Mode 1", QVariant(0));
	if(deviceType.toStdString() == "Automotive") {
		sequenceModeComboBox->addItem("Mode 2", QVariant(1));
	}

	conversionModeMenuCombo = new MenuCombo("Conversion Mode", sequenceSection);
	QComboBox *conversionModeComboBox = conversionModeMenuCombo->combo();
	conversionModeComboBox->addItem("Continuous conversions", QVariant(0));
	conversionModeComboBox->addItem("One-shot conversion", QVariant(1));

	convertSynchronizationMenuCombo = new MenuCombo("Convert Start Synchronization Mode", sequenceSection);
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
	connect(applySequenceButton, &QPushButton::clicked, this, [this]() {
		applySequenceAndUpdate();
		toggleWidget(applySequenceButton, false);
		applySequenceButton->setText("Writing...");
		QTimer::singleShot(1000, this, [this]() {
			this->toggleWidget(applySequenceButton, true);
			applySequenceButton->setText("Apply");
		});
	});

	sequenceSection->contentLayout()->addWidget(sequenceModeMenuCombo);
	sequenceSection->contentLayout()->addWidget(conversionModeMenuCombo);
	sequenceSection->contentLayout()->addWidget(convertSynchronizationMenuCombo);
	sequenceSection->contentLayout()->addWidget(angleFilterMenuCombo);
	sequenceSection->contentLayout()->addWidget(eighthHarmonicMenuCombo);
	sequenceSection->contentLayout()->addWidget(applySequenceButton);

#pragma region Device Status Widget
	MenuSectionWidget *acquisitionDeviceStatusWidget = new MenuSectionWidget(generalSettingWidget);
	Style::setStyle(acquisitionDeviceStatusWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *acquisitionDeviceStatusSection =
		new MenuCollapseSection("Device Status", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, generalWidget);
	acquisitionDeviceStatusSection->contentLayout()->setSpacing(globalSpacingSmall);
	acquisitionDeviceStatusWidget->contentLayout()->addWidget(acquisitionDeviceStatusSection);

	acquisitionFaultRegisterLEDWidget =
		createStatusLEDWidget("Fault Register", "status", deviceStatusFault, acquisitionDeviceStatusSection);
	acquisitionDeviceStatusSection->contentLayout()->addWidget(acquisitionFaultRegisterLEDWidget);
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

ToolTemplate *HarmonicCalibration::createCalibrationWidget()
{
	ToolTemplate *tool = new ToolTemplate(this);

#pragma region Calibration Data Graph Widget
	QWidget *calibrationDataGraphWidget = new QWidget();
	QGridLayout *calibrationDataGraphLayout = new QGridLayout(calibrationDataGraphWidget);
	calibrationDataGraphWidget->setLayout(calibrationDataGraphLayout);
	calibrationDataGraphLayout->setMargin(0);
	calibrationDataGraphLayout->setSpacing(globalSpacingSmall);

	MenuSectionWidget *calibrationDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	Style::setStyle(calibrationDataGraphSectionWidget, style::properties::widget::basicComponent);
	calibrationDataGraphTabWidget = new QTabWidget(calibrationDataGraphSectionWidget);
	calibrationDataGraphTabWidget->tabBar()->setStyleSheet("QTabBar::tab { width: 176px; }");
	Style::setStyle(calibrationDataGraphTabWidget->tabBar(), style::properties::admt::tabBar, "rounded", true);
	calibrationDataGraphSectionWidget->contentLayout()->setMargin(1);
	calibrationDataGraphSectionWidget->contentLayout()->addWidget(calibrationDataGraphTabWidget);

#pragma region Calibration Samples
	QWidget *calibrationSamplesWidget = new QWidget(calibrationDataGraphTabWidget);
	Style::setStyle(calibrationSamplesWidget, style::properties::widget::basicBackground);
	QVBoxLayout *calibrationSamplesLayout = new QVBoxLayout(calibrationSamplesWidget);
	calibrationSamplesWidget->setLayout(calibrationSamplesLayout);
	calibrationSamplesLayout->setMargin(0);
	calibrationSamplesLayout->setSpacing(globalSpacingSmall);

	calibrationRawDataPlotWidget = new PlotWidget();
	calibrationRawDataPlotWidget->setContentsMargins(globalSpacingSmall, globalSpacingSmall, globalSpacingSmall, 0);

	calibrationRawDataXPlotAxis = new PlotAxis(QwtAxis::XBottom, calibrationRawDataPlotWidget, scopyBluePen);
	PrefixFormatter *calibrationRawDataXFormatter = new PrefixFormatter({{" ", 1E0}}, calibrationRawDataXPlotAxis);
	calibrationRawDataXFormatter->setTrimZeroes(true);
	calibrationRawDataXPlotAxis->setFormatter(calibrationRawDataXFormatter);
	calibrationRawDataXPlotAxis->setMin(0);
	calibrationRawDataXPlotAxis->scaleDraw()->setFloatPrecision(0);

	calibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, calibrationRawDataPlotWidget, scopyBluePen);
	PrefixFormatter *calibrationRawDataYFormatter = new PrefixFormatter({{" ", 1E0}}, calibrationRawDataYPlotAxis);
	calibrationRawDataYFormatter->setTrimZeroes(true);
	calibrationRawDataYPlotAxis->setFormatter(calibrationRawDataYFormatter);
	calibrationRawDataYPlotAxis->setInterval(0, 360);
	calibrationRawDataYPlotAxis->setUnitsVisible(true);
	calibrationRawDataYPlotAxis->setUnits("°");

	calibrationRawDataPlotChannel =
		new PlotChannel("Angle", channel0Pen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationSineDataPlotChannel =
		new PlotChannel("Sine", channel1Pen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationCosineDataPlotChannel =
		new PlotChannel("Cosine", channel2Pen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);

	calibrationRawDataPlotWidget->addPlotChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationSineDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationCosineDataPlotChannel);
	calibrationRawDataPlotChannel->setEnabled(true);
	calibrationSineDataPlotChannel->setEnabled(false);
	calibrationCosineDataPlotChannel->setEnabled(false);
	calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);

	calibrationRawDataPlotWidget->setShowXAxisLabels(true);
	calibrationRawDataPlotWidget->setShowYAxisLabels(true);
	calibrationRawDataPlotWidget->showAxisLabels();
	calibrationRawDataPlotWidget->replot();

	QWidget *calibrationDataGraphChannelsWidget = new QWidget(calibrationDataGraphTabWidget);
	Style::setStyle(calibrationDataGraphChannelsWidget, style::properties::widget::basicBackground);
	QHBoxLayout *calibrationDataGraphChannelsLayout = new QHBoxLayout(calibrationDataGraphChannelsWidget);
	calibrationDataGraphChannelsWidget->setLayout(calibrationDataGraphChannelsLayout);
	calibrationDataGraphChannelsLayout->setContentsMargins(globalSpacingMedium, 0, globalSpacingMedium,
							       globalSpacingSmall);
	calibrationDataGraphChannelsLayout->setSpacing(globalSpacingMedium);

	QCheckBox *toggleAngleCheckBox = new QCheckBox("Angle", calibrationDataGraphChannelsWidget);
	Style::setStyle(toggleAngleCheckBox, style::properties::admt::coloredCheckBox, "ch0");
	toggleAngleCheckBox->setChecked(true);
	QCheckBox *toggleSineCheckBox = new QCheckBox("Sine", calibrationDataGraphChannelsWidget);
	Style::setStyle(toggleSineCheckBox, style::properties::admt::coloredCheckBox, "ch1");
	toggleSineCheckBox->setChecked(false);
	QCheckBox *toggleCosineCheckBox = new QCheckBox("Cosine", calibrationDataGraphChannelsWidget);
	Style::setStyle(toggleCosineCheckBox, style::properties::admt::coloredCheckBox, "ch2");
	toggleCosineCheckBox->setChecked(false);

	calibrationDataGraphChannelsLayout->addWidget(toggleAngleCheckBox);
	calibrationDataGraphChannelsLayout->addWidget(toggleSineCheckBox);
	calibrationDataGraphChannelsLayout->addWidget(toggleCosineCheckBox);
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
	postCalibrationSamplesLayout->setSpacing(globalSpacingSmall);

	postCalibrationRawDataPlotWidget = new PlotWidget();
	postCalibrationRawDataPlotWidget->setContentsMargins(globalSpacingSmall, globalSpacingSmall, globalSpacingSmall,
							     0);

	postCalibrationRawDataXPlotAxis =
		new PlotAxis(QwtAxis::XBottom, postCalibrationRawDataPlotWidget, scopyBluePen);
	PrefixFormatter *postCalibrationRawDataXFormatter =
		new PrefixFormatter({{" ", 1E0}}, postCalibrationRawDataXPlotAxis);
	postCalibrationRawDataXFormatter->setTrimZeroes(true);
	postCalibrationRawDataXPlotAxis->setFormatter(postCalibrationRawDataXFormatter);
	postCalibrationRawDataXPlotAxis->setMin(0);
	postCalibrationRawDataXPlotAxis->scaleDraw()->setFloatPrecision(0);

	postCalibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, postCalibrationRawDataPlotWidget, scopyBluePen);
	PrefixFormatter *postCalibrationRawDataYFormatter =
		new PrefixFormatter({{" ", 1E0}}, postCalibrationRawDataYPlotAxis);
	postCalibrationRawDataYFormatter->setTrimZeroes(true);
	postCalibrationRawDataYPlotAxis->setFormatter(postCalibrationRawDataYFormatter);
	postCalibrationRawDataYPlotAxis->setInterval(0, 360);
	postCalibrationRawDataYPlotAxis->setUnitsVisible(true);
	postCalibrationRawDataYPlotAxis->setUnits("°");

	postCalibrationRawDataPlotChannel =
		new PlotChannel("Angle", channel0Pen, postCalibrationRawDataXPlotAxis, postCalibrationRawDataYPlotAxis);
	postCalibrationSineDataPlotChannel =
		new PlotChannel("Sine", channel1Pen, postCalibrationRawDataXPlotAxis, postCalibrationRawDataYPlotAxis);
	postCalibrationCosineDataPlotChannel = new PlotChannel("Cosine", channel2Pen, postCalibrationRawDataXPlotAxis,
							       postCalibrationRawDataYPlotAxis);

	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationRawDataPlotChannel);
	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationSineDataPlotChannel);
	postCalibrationRawDataPlotWidget->addPlotChannel(postCalibrationCosineDataPlotChannel);

	postCalibrationRawDataPlotChannel->setEnabled(true);
	postCalibrationSineDataPlotChannel->setEnabled(false);
	postCalibrationCosineDataPlotChannel->setEnabled(false);
	postCalibrationRawDataPlotWidget->selectChannel(postCalibrationRawDataPlotChannel);

	postCalibrationRawDataPlotWidget->setShowXAxisLabels(true);
	postCalibrationRawDataPlotWidget->setShowYAxisLabels(true);
	postCalibrationRawDataPlotWidget->showAxisLabels();
	postCalibrationRawDataPlotWidget->replot();

	QWidget *postCalibrationDataGraphChannelsWidget = new QWidget(calibrationDataGraphTabWidget);
	Style::setStyle(postCalibrationDataGraphChannelsWidget, style::properties::widget::basicBackground);
	QHBoxLayout *postCalibrationDataGraphChannelsLayout = new QHBoxLayout(postCalibrationDataGraphChannelsWidget);
	postCalibrationDataGraphChannelsWidget->setLayout(postCalibrationDataGraphChannelsLayout);
	postCalibrationDataGraphChannelsLayout->setContentsMargins(globalSpacingMedium, 0, globalSpacingMedium,
								   globalSpacingSmall);
	postCalibrationDataGraphChannelsLayout->setSpacing(globalSpacingMedium);

	QCheckBox *togglePostAngleCheckBox = new QCheckBox("Angle", postCalibrationDataGraphChannelsWidget);
	Style::setStyle(togglePostAngleCheckBox, style::properties::admt::coloredCheckBox, "ch0");
	togglePostAngleCheckBox->setChecked(true);
	QCheckBox *togglePostSineCheckBox = new QCheckBox("Sine", postCalibrationDataGraphChannelsWidget);
	Style::setStyle(togglePostSineCheckBox, style::properties::admt::coloredCheckBox, "ch1");
	togglePostSineCheckBox->setChecked(false);
	QCheckBox *togglePostCosineCheckBox = new QCheckBox("Cosine", postCalibrationDataGraphChannelsWidget);
	Style::setStyle(togglePostCosineCheckBox, style::properties::admt::coloredCheckBox, "ch2");
	togglePostCosineCheckBox->setChecked(false);

	postCalibrationDataGraphChannelsLayout->addWidget(togglePostAngleCheckBox);
	postCalibrationDataGraphChannelsLayout->addWidget(togglePostSineCheckBox);
	postCalibrationDataGraphChannelsLayout->addWidget(togglePostCosineCheckBox);
	postCalibrationDataGraphChannelsLayout->addStretch();

	postCalibrationSamplesLayout->addWidget(postCalibrationRawDataPlotWidget);
	postCalibrationSamplesLayout->addWidget(postCalibrationDataGraphChannelsWidget);
#pragma endregion

	calibrationDataGraphTabWidget->addTab(calibrationSamplesWidget, "Calibration Samples");
	calibrationDataGraphTabWidget->addTab(postCalibrationSamplesWidget, "Post Calibration Samples");

	MenuSectionWidget *resultDataSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	Style::setStyle(resultDataSectionWidget, style::properties::widget::basicComponent);
	resultDataTabWidget = new QTabWidget(calibrationDataGraphWidget);
	resultDataTabWidget->tabBar()->setStyleSheet("QTabBar::tab { width: 160px; }");
	Style::setStyle(resultDataTabWidget->tabBar(), style::properties::admt::tabBar, "rounded", true);
	resultDataSectionWidget->contentLayout()->setContentsMargins(1, 1, 1, globalSpacingSmall);
	resultDataSectionWidget->contentLayout()->setSpacing(0);
	resultDataSectionWidget->contentLayout()->addWidget(resultDataTabWidget);

#pragma region Angle Error Widget
	QWidget *angleErrorWidget = new QWidget();
	Style::setStyle(angleErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *angleErrorLayout = new QVBoxLayout(angleErrorWidget);
	angleErrorWidget->setLayout(angleErrorLayout);
	angleErrorLayout->setMargin(0);
	angleErrorLayout->setSpacing(0);

	angleErrorPlotWidget = new PlotWidget();
	angleErrorPlotWidget->setContentsMargins(globalSpacingSmall, globalSpacingSmall, globalSpacingSmall, 0);

	angleErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, angleErrorPlotWidget, scopyBluePen);
	PrefixFormatter *angleErrorXFormatter = new PrefixFormatter({{" ", 1E0}}, angleErrorXPlotAxis);
	angleErrorXFormatter->setTrimZeroes(true);
	angleErrorXPlotAxis->setFormatter(angleErrorXFormatter);
	angleErrorXPlotAxis->setMin(0);
	angleErrorXPlotAxis->scaleDraw()->setFloatPrecision(0);

	angleErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, angleErrorPlotWidget, scopyBluePen);
	PrefixFormatter *angleErrorYFormatter = new PrefixFormatter({{" ", 1E0}}, angleErrorYPlotAxis);
	angleErrorYFormatter->setTrimZeroes(true);
	angleErrorYPlotAxis->setFormatter(angleErrorYFormatter);
	angleErrorYPlotAxis->setInterval(defaultAngleErrorGraphMin, defaultAngleErrorGraphMax);
	angleErrorYPlotAxis->setUnitsVisible(true);
	angleErrorYPlotAxis->setUnits("°");

	angleErrorPlotChannel = new PlotChannel("Angle Error", channel0Pen, angleErrorXPlotAxis, angleErrorYPlotAxis);

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

	QLabel *FFTAngleErrorMagnitudeLabel = new QLabel("Magnitude", FFTAngleErrorWidget);
	Style::setStyle(FFTAngleErrorMagnitudeLabel, style::properties::label::menuSmall);
	FFTAngleErrorMagnitudeLabel->setContentsMargins(globalSpacingMedium, globalSpacingSmall, globalSpacingSmall,
							globalSpacingSmall);

	FFTAngleErrorMagnitudePlotWidget = new PlotWidget();
	FFTAngleErrorMagnitudePlotWidget->setContentsMargins(globalSpacingSmall, 0, globalSpacingSmall, 0);

	FFTAngleErrorMagnitudeXPlotAxis =
		new PlotAxis(QwtAxis::XBottom, FFTAngleErrorMagnitudePlotWidget, scopyBluePen);
	PrefixFormatter *FFTAngleErrorXFormatter = new PrefixFormatter({{" ", 1E0}}, FFTAngleErrorMagnitudeXPlotAxis);
	FFTAngleErrorXFormatter->setTrimZeroes(true);
	FFTAngleErrorMagnitudeXPlotAxis->setFormatter(FFTAngleErrorXFormatter);
	FFTAngleErrorMagnitudeXPlotAxis->setMin(0);
	FFTAngleErrorMagnitudeXPlotAxis->scaleDraw()->setFloatPrecision(0);

	FFTAngleErrorMagnitudeYPlotAxis = new PlotAxis(QwtAxis::YLeft, FFTAngleErrorMagnitudePlotWidget, scopyBluePen);
	PrefixFormatter *FFTAngleErrorYFormatter = new PrefixFormatter({{" ", 1E0}}, FFTAngleErrorMagnitudeYPlotAxis);
	FFTAngleErrorYFormatter->setTrimZeroes(true);
	FFTAngleErrorMagnitudeYPlotAxis->setFormatter(FFTAngleErrorYFormatter);
	FFTAngleErrorMagnitudeYPlotAxis->setInterval(defaultMagnitudeGraphMin, defaultMagnitudeGraphMax);

	FFTAngleErrorMagnitudeChannel =
		new PlotChannel("FFT Angle Error Magnitude", channel0Pen, FFTAngleErrorMagnitudeXPlotAxis,
				FFTAngleErrorMagnitudeYPlotAxis);

	FFTAngleErrorMagnitudePlotWidget->addPlotChannel(FFTAngleErrorMagnitudeChannel);

	FFTAngleErrorMagnitudeChannel->setEnabled(true);
	FFTAngleErrorMagnitudePlotWidget->selectChannel(FFTAngleErrorMagnitudeChannel);

	FFTAngleErrorMagnitudePlotWidget->setShowXAxisLabels(true);
	FFTAngleErrorMagnitudePlotWidget->setShowYAxisLabels(true);
	FFTAngleErrorMagnitudePlotWidget->showAxisLabels();
	FFTAngleErrorMagnitudePlotWidget->replot();

	QLabel *FFTAngleErrorPhaseLabel = new QLabel("Phase", FFTAngleErrorWidget);
	Style::setStyle(FFTAngleErrorPhaseLabel, style::properties::label::menuSmall);
	FFTAngleErrorPhaseLabel->setContentsMargins(globalSpacingMedium, globalSpacingSmall, globalSpacingSmall,
						    globalSpacingSmall);

	FFTAngleErrorPhasePlotWidget = new PlotWidget();
	FFTAngleErrorPhasePlotWidget->setContentsMargins(globalSpacingSmall, 0, globalSpacingSmall, 0);

	FFTAngleErrorPhaseXPlotAxis = new PlotAxis(QwtAxis::XBottom, FFTAngleErrorPhasePlotWidget, scopyBluePen);
	PrefixFormatter *FFTAngleErrorPhaseXFormatter = new PrefixFormatter({{" ", 1E0}}, FFTAngleErrorPhaseXPlotAxis);
	FFTAngleErrorPhaseXFormatter->setTrimZeroes(true);
	FFTAngleErrorPhaseXPlotAxis->setFormatter(FFTAngleErrorPhaseXFormatter);
	FFTAngleErrorPhaseXPlotAxis->setMin(0);
	FFTAngleErrorPhaseXPlotAxis->scaleDraw()->setFloatPrecision(0);

	FFTAngleErrorPhaseYPlotAxis = new PlotAxis(QwtAxis::YLeft, FFTAngleErrorPhasePlotWidget, scopyBluePen);
	PrefixFormatter *FFTAngleErrorPhaseYFormatter = new PrefixFormatter({{" ", 1E0}}, FFTAngleErrorPhaseYPlotAxis);
	FFTAngleErrorPhaseYFormatter->setTrimZeroes(true);
	FFTAngleErrorPhaseYPlotAxis->setFormatter(FFTAngleErrorPhaseYFormatter);
	FFTAngleErrorPhaseYPlotAxis->setInterval(defaultPhaseGraphMin, defaultPhaseGraphMax);

	FFTAngleErrorPhaseChannel = new PlotChannel("FFT Angle Error Phase", channel1Pen, FFTAngleErrorPhaseXPlotAxis,
						    FFTAngleErrorPhaseYPlotAxis);

	FFTAngleErrorPhasePlotWidget->addPlotChannel(FFTAngleErrorPhaseChannel);

	FFTAngleErrorPhaseChannel->setEnabled(true);
	FFTAngleErrorPhasePlotWidget->selectChannel(FFTAngleErrorPhaseChannel);

	FFTAngleErrorPhasePlotWidget->setShowXAxisLabels(true);
	FFTAngleErrorPhasePlotWidget->setShowYAxisLabels(true);
	FFTAngleErrorPhasePlotWidget->showAxisLabels();
	FFTAngleErrorPhasePlotWidget->replot();

	FFTAngleErrorLayout->addWidget(FFTAngleErrorMagnitudeLabel);
	FFTAngleErrorLayout->addWidget(FFTAngleErrorMagnitudePlotWidget);
	FFTAngleErrorLayout->addWidget(FFTAngleErrorPhaseLabel);
	FFTAngleErrorLayout->addWidget(FFTAngleErrorPhasePlotWidget);
#pragma endregion

#pragma region Corrected Error Widget
	QWidget *correctedErrorWidget = new QWidget();
	Style::setStyle(correctedErrorWidget, style::properties::widget::basicBackground);
	QVBoxLayout *correctedErrorLayout = new QVBoxLayout(correctedErrorWidget);
	correctedErrorWidget->setLayout(correctedErrorLayout);
	correctedErrorLayout->setMargin(0);
	correctedErrorLayout->setSpacing(0);

	correctedErrorPlotWidget = new PlotWidget();
	correctedErrorPlotWidget->setContentsMargins(globalSpacingSmall, globalSpacingSmall, globalSpacingSmall, 0);

	correctedErrorXPlotAxis = new PlotAxis(QwtAxis::XBottom, correctedErrorPlotWidget, scopyBluePen);
	PrefixFormatter *correctedErrorXFormatter = new PrefixFormatter({{" ", 1E0}}, correctedErrorXPlotAxis);
	correctedErrorXFormatter->setTrimZeroes(true);
	correctedErrorXPlotAxis->setFormatter(correctedErrorXFormatter);
	correctedErrorXPlotAxis->setMin(0);
	correctedErrorXPlotAxis->scaleDraw()->setFloatPrecision(0);

	correctedErrorYPlotAxis = new PlotAxis(QwtAxis::YLeft, correctedErrorPlotWidget, scopyBluePen);
	PrefixFormatter *correctedErrorYFormatter = new PrefixFormatter({{" ", 1E0}}, correctedErrorYPlotAxis);
	correctedErrorYFormatter->setTrimZeroes(true);
	correctedErrorYPlotAxis->setFormatter(correctedErrorYFormatter);
	correctedErrorYPlotAxis->setInterval(defaultAngleErrorGraphMin, defaultAngleErrorGraphMax);
	correctedErrorYPlotAxis->setUnitsVisible(true);
	correctedErrorYPlotAxis->setUnits("°");

	correctedErrorPlotChannel =
		new PlotChannel("Corrected Error", channel0Pen, correctedErrorXPlotAxis, correctedErrorYPlotAxis);
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

	QLabel *FFTCorrectedErrorMagnitudeLabel = new QLabel("Magnitude", FFTCorrectedErrorWidget);
	Style::setStyle(FFTCorrectedErrorMagnitudeLabel, style::properties::label::menuSmall);
	FFTCorrectedErrorMagnitudeLabel->setContentsMargins(globalSpacingMedium, globalSpacingSmall, globalSpacingSmall,
							    globalSpacingSmall);

	FFTCorrectedErrorMagnitudePlotWidget = new PlotWidget();
	FFTCorrectedErrorMagnitudePlotWidget->setContentsMargins(globalSpacingSmall, 0, globalSpacingSmall, 0);

	FFTCorrectedErrorMagnitudeXPlotAxis =
		new PlotAxis(QwtAxis::XBottom, FFTCorrectedErrorMagnitudePlotWidget, scopyBluePen);
	PrefixFormatter *FFTCorrectedErrorXFormatter =
		new PrefixFormatter({{" ", 1E0}}, FFTCorrectedErrorMagnitudeXPlotAxis);
	FFTCorrectedErrorXFormatter->setTrimZeroes(true);
	FFTCorrectedErrorMagnitudeXPlotAxis->setFormatter(FFTCorrectedErrorXFormatter);
	FFTCorrectedErrorMagnitudeXPlotAxis->setMin(0);
	FFTCorrectedErrorMagnitudeXPlotAxis->scaleDraw()->setFloatPrecision(0);

	FFTCorrectedErrorMagnitudeYPlotAxis =
		new PlotAxis(QwtAxis::YLeft, FFTCorrectedErrorMagnitudePlotWidget, scopyBluePen);
	PrefixFormatter *FFTCorrectedErrorYFormatter =
		new PrefixFormatter({{" ", 1E0}}, FFTCorrectedErrorMagnitudeYPlotAxis);
	FFTCorrectedErrorYFormatter->setTrimZeroes(true);
	FFTCorrectedErrorMagnitudeYPlotAxis->setFormatter(FFTCorrectedErrorYFormatter);
	FFTCorrectedErrorMagnitudeYPlotAxis->setInterval(0, 1.2);

	FFTCorrectedErrorMagnitudeChannel =
		new PlotChannel("FFT Corrected Error Magnitude", channel0Pen, FFTCorrectedErrorMagnitudeXPlotAxis,
				FFTCorrectedErrorMagnitudeYPlotAxis);

	FFTCorrectedErrorMagnitudePlotWidget->addPlotChannel(FFTCorrectedErrorMagnitudeChannel);

	FFTCorrectedErrorMagnitudeChannel->setEnabled(true);
	FFTCorrectedErrorMagnitudePlotWidget->selectChannel(FFTCorrectedErrorMagnitudeChannel);

	FFTCorrectedErrorMagnitudePlotWidget->setShowXAxisLabels(true);
	FFTCorrectedErrorMagnitudePlotWidget->setShowYAxisLabels(true);
	FFTCorrectedErrorMagnitudePlotWidget->showAxisLabels();
	FFTCorrectedErrorMagnitudePlotWidget->replot();

	QLabel *FFTCorrectedErrorPhaseLabel = new QLabel("Phase", FFTCorrectedErrorWidget);
	Style::setStyle(FFTCorrectedErrorPhaseLabel, style::properties::label::menuSmall);
	FFTCorrectedErrorPhaseLabel->setContentsMargins(globalSpacingMedium, globalSpacingSmall, globalSpacingSmall,
							globalSpacingSmall);

	FFTCorrectedErrorPhasePlotWidget = new PlotWidget();
	FFTCorrectedErrorPhasePlotWidget->setContentsMargins(globalSpacingSmall, 0, globalSpacingSmall, 0);

	FFTCorrectedErrorPhaseXPlotAxis =
		new PlotAxis(QwtAxis::XBottom, FFTCorrectedErrorPhasePlotWidget, scopyBluePen);
	PrefixFormatter *FFTCorrectedErrorPhaseXFormatter =
		new PrefixFormatter({{" ", 1E0}}, FFTCorrectedErrorPhaseXPlotAxis);
	FFTCorrectedErrorPhaseXFormatter->setTrimZeroes(true);
	FFTCorrectedErrorPhaseXPlotAxis->setFormatter(FFTCorrectedErrorPhaseXFormatter);
	FFTCorrectedErrorPhaseXPlotAxis->setMin(0);
	FFTCorrectedErrorPhaseXPlotAxis->scaleDraw()->setFloatPrecision(0);

	FFTCorrectedErrorPhaseYPlotAxis = new PlotAxis(QwtAxis::YLeft, FFTCorrectedErrorPhasePlotWidget, scopyBluePen);
	PrefixFormatter *FFTCorrectedErrorPhaseYFormatter =
		new PrefixFormatter({{" ", 1E0}}, FFTCorrectedErrorPhaseYPlotAxis);
	FFTCorrectedErrorPhaseYFormatter->setTrimZeroes(true);
	FFTCorrectedErrorPhaseYPlotAxis->setFormatter(FFTCorrectedErrorPhaseYFormatter);
	FFTCorrectedErrorPhaseYPlotAxis->setInterval(-4, 4);

	FFTCorrectedErrorPhaseChannel =
		new PlotChannel("FFT Corrected Error Phase", channel1Pen, FFTCorrectedErrorPhaseXPlotAxis,
				FFTCorrectedErrorPhaseYPlotAxis);
	FFTCorrectedErrorPhasePlotWidget->addPlotChannel(FFTCorrectedErrorPhaseChannel);

	FFTCorrectedErrorPhaseChannel->setEnabled(true);
	FFTCorrectedErrorPhasePlotWidget->selectChannel(FFTCorrectedErrorPhaseChannel);

	FFTCorrectedErrorPhasePlotWidget->setShowXAxisLabels(true);
	FFTCorrectedErrorPhasePlotWidget->setShowYAxisLabels(true);
	FFTCorrectedErrorPhasePlotWidget->showAxisLabels();
	FFTCorrectedErrorPhasePlotWidget->replot();

	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorMagnitudeLabel);
	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorMagnitudePlotWidget);
	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorPhaseLabel);
	FFTCorrectedErrorLayout->addWidget(FFTCorrectedErrorPhasePlotWidget);
#pragma endregion

	resultDataTabWidget->addTab(angleErrorWidget, "Angle Error");
	resultDataTabWidget->addTab(FFTAngleErrorWidget, "FFT Angle Error");
	resultDataTabWidget->addTab(correctedErrorWidget, "Corrected Error");
	resultDataTabWidget->addTab(FFTCorrectedErrorWidget, "FFT Corrected Error");

	calibrationDataGraphLayout->addWidget(calibrationDataGraphSectionWidget, 0, 0, 3, 0);
	calibrationDataGraphLayout->addWidget(resultDataSectionWidget, 3, 0, 5, 0);
#pragma endregion

#pragma region Calibration Settings Widget
	QWidget *calibrationSettingsGroupWidget = new QWidget();
	QVBoxLayout *calibrationSettingsGroupLayout = new QVBoxLayout(calibrationSettingsGroupWidget);
	calibrationSettingsGroupWidget->setLayout(calibrationSettingsGroupLayout);
	calibrationSettingsGroupLayout->setMargin(0);
	calibrationSettingsGroupLayout->setSpacing(globalSpacingSmall);

#pragma region Device Status Widget
	MenuSectionWidget *calibrationDeviceStatusWidget = new MenuSectionWidget(calibrationSettingsGroupWidget);
	Style::setStyle(calibrationDeviceStatusWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *calibrationDeviceStatusSection = new MenuCollapseSection(
		"Device Status", MenuCollapseSection::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationSettingsGroupWidget);
	calibrationDeviceStatusSection->contentLayout()->setSpacing(globalSpacingSmall);
	calibrationDeviceStatusWidget->contentLayout()->addWidget(calibrationDeviceStatusSection);

	calibrationFaultRegisterLEDWidget =
		createStatusLEDWidget("Fault Register", "status", deviceStatusFault, calibrationDeviceStatusSection);
	calibrationDeviceStatusSection->contentLayout()->addWidget(calibrationFaultRegisterLEDWidget);
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

	connect(calibrationStartMotorButton, &QPushButton::toggled, this, [this](bool toggled) {
		calibrationStartMotorButton->setText(toggled ? " Stop Acquisition" : " Acquire Samples");
		isStartMotor = toggled;
		if(toggled) {
			isPostCalibration = false;
			startCalibration();
		} else {
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

	connect(calibrateDataButton, &QPushButton::toggled, this, [this](bool toggled) {
		calibrateDataButton->setText(toggled ? " Stop Calibration" : " Calibrate");
		if(toggled)
			postCalibrateData();
		else
			stopCalibration();
	});
#pragma endregion

#pragma region Reset Calibration Button
	clearCalibrateDataButton = new QPushButton("Reset Calibration", calibrationSettingsGroupWidget);
	StyleHelper::BasicButton(clearCalibrateDataButton);
	QIcon resetIcon;
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
	calibrationSettingsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	calibrationSettingsWidget->setLayout(calibrationSettingsLayout);

	calibrationSettingsGroupLayout->addWidget(calibrationDeviceStatusWidget);
	calibrationSettingsGroupLayout->addWidget(calibrationStartMotorButton);
	calibrationSettingsGroupLayout->addWidget(calibrateDataButton);
	calibrationSettingsGroupLayout->addWidget(clearCalibrateDataButton);
	calibrationSettingsGroupLayout->addWidget(calibrationSettingsScrollArea);

#pragma region Calibration Coefficient Section Widget
	MenuSectionWidget *calibrationCoeffSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationCoeffSectionWidget, style::properties::widget::basicComponent);

	QLabel *calibrationCalculatedCoeffLabel = new QLabel("Calculated Coefficients", calibrationCoeffSectionWidget);
	Style::setStyle(calibrationCalculatedCoeffLabel, style::properties::label::menuMedium);

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
	configureCoeffRow(h1RowContainer, h1RowLayout, calibrationH1Label, calibrationH1MagLabel,
			  calibrationH1PhaseLabel);

	QWidget *h2RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h2RowLayout = new QHBoxLayout(h2RowContainer);
	QLabel *calibrationH2Label = new QLabel("H2", calibrationCalculatedCoeffWidget);
	calibrationH2MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH2PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	configureCoeffRow(h2RowContainer, h2RowLayout, calibrationH2Label, calibrationH2MagLabel,
			  calibrationH2PhaseLabel);

	QWidget *h3RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h3RowLayout = new QHBoxLayout(h3RowContainer);
	QLabel *calibrationH3Label = new QLabel("H3", calibrationCalculatedCoeffWidget);
	calibrationH3MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH3PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	configureCoeffRow(h3RowContainer, h3RowLayout, calibrationH3Label, calibrationH3MagLabel,
			  calibrationH3PhaseLabel);

	QWidget *h8RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h8RowLayout = new QHBoxLayout(h8RowContainer);
	QLabel *calibrationH8Label = new QLabel("H8", calibrationCalculatedCoeffWidget);
	calibrationH8MagLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	calibrationH8PhaseLabel = new QLabel("0x----", calibrationCalculatedCoeffWidget);
	configureCoeffRow(h8RowContainer, h8RowLayout, calibrationH8Label, calibrationH8MagLabel,
			  calibrationH8PhaseLabel);

	calibrationCalculatedCoeffLayout->addWidget(h1RowContainer, 0, 0);
	calibrationCalculatedCoeffLayout->addWidget(h2RowContainer, 1, 0);
	calibrationCalculatedCoeffLayout->addWidget(h3RowContainer, 2, 0);
	calibrationCalculatedCoeffLayout->addWidget(h8RowContainer, 3, 0);

	QWidget *calibrationDisplayFormatWidget = new QWidget(calibrationCoeffSectionWidget);
	QVBoxLayout *calibrationDisplayFormatLayout = new QVBoxLayout(calibrationDisplayFormatWidget);
	calibrationDisplayFormatWidget->setLayout(calibrationDisplayFormatLayout);
	calibrationDisplayFormatLayout->setMargin(0);
	calibrationDisplayFormatLayout->setSpacing(0);

	QLabel *calibrationDisplayFormatLabel = new QLabel("Display Format", calibrationDisplayFormatWidget);
	Style::setStyle(calibrationDisplayFormatLabel, style::properties::label::subtle);

	calibrationDisplayFormatSwitch = new CustomSwitch();
	calibrationDisplayFormatSwitch->setOffText("Hex");
	calibrationDisplayFormatSwitch->setOnText("Angle");
	calibrationDisplayFormatSwitch->setProperty("bigBtn", true);

	calibrationDisplayFormatLayout->addWidget(calibrationDisplayFormatLabel);
	calibrationDisplayFormatLayout->addWidget(calibrationDisplayFormatSwitch);

	calibrationCoeffSectionWidget->contentLayout()->setSpacing(globalSpacingSmall);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffWidget);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatWidget);
#pragma endregion

#pragma region Calibration Dataset Configuration
	MenuSectionWidget *calibrationDatasetConfigSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationDatasetConfigSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *calibrationDatasetConfigCollapseSection = new MenuCollapseSection(
		"Dataset Configuration", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationDatasetConfigSectionWidget);
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
	Style::setStyle(calibrationCycleCountLabel, style::properties::label::subtle);
	calibrationCycleCountLineEdit = new QLineEdit(QString::number(cycleCount), calibrationCycleCountWidget);
	Style::setStyle(calibrationCycleCountLineEdit, style::properties::admt::lineEdit);
	connectLineEditToNumber(calibrationCycleCountLineEdit, cycleCount, 1, 100);
	calibrationCycleCountLayout->addWidget(calibrationCycleCountLabel);
	calibrationCycleCountLayout->addWidget(calibrationCycleCountLineEdit);

	QWidget *calibrationTotalSamplesWidget = new QWidget(calibrationDatasetConfigCollapseSection);
	QVBoxLayout *calibrationTotalSamplesLayout = new QVBoxLayout(calibrationTotalSamplesWidget);
	calibrationTotalSamplesWidget->setLayout(calibrationTotalSamplesLayout);
	calibrationTotalSamplesLayout->setMargin(0);
	calibrationTotalSamplesLayout->setSpacing(0);
	QLabel *calibrationTotalSamplesLabel = new QLabel("Total Samples", calibrationTotalSamplesWidget);
	Style::setStyle(calibrationTotalSamplesLabel, style::properties::label::subtle);
	calibrationTotalSamplesLineEdit =
		new QLineEdit(QString::number(totalSamplesCount), calibrationTotalSamplesWidget);
	Style::setStyle(calibrationTotalSamplesLineEdit, style::properties::admt::lineEdit);
	connectLineEditToNumber(calibrationTotalSamplesLineEdit, totalSamplesCount, 1, 8192);
	calibrationTotalSamplesLayout->addWidget(calibrationTotalSamplesLabel);
	calibrationTotalSamplesLayout->addWidget(calibrationTotalSamplesLineEdit);

	QWidget *motorRPMWidget = new QWidget(calibrationDatasetConfigCollapseSection);
	QVBoxLayout *motorRPMLayout = new QVBoxLayout(motorRPMWidget);
	motorRPMWidget->setLayout(motorRPMLayout);
	motorRPMLayout->setMargin(0);
	motorRPMLayout->setSpacing(0);
	QLabel *motorRPMLabel = new QLabel("Motor RPM", motorRPMWidget);
	Style::setStyle(motorRPMLabel, style::properties::label::subtle);
	calibrationMotorRPMLineEdit = new QLineEdit(QString::number(motor_rpm), motorRPMWidget);
	Style::setStyle(calibrationMotorRPMLineEdit, style::properties::admt::lineEdit);
	connectLineEditToRPM(calibrationMotorRPMLineEdit, motor_rpm, 1, fast_motor_rpm);
	motorRPMLayout->addWidget(motorRPMLabel);
	motorRPMLayout->addWidget(calibrationMotorRPMLineEdit);

	QWidget *motorDirectionWidget = new QWidget(calibrationDatasetConfigCollapseSection);
	QVBoxLayout *motorDirectionLayout = new QVBoxLayout(motorDirectionWidget);
	motorDirectionWidget->setLayout(motorDirectionLayout);
	motorDirectionLayout->setMargin(0);
	motorDirectionLayout->setSpacing(0);
	QLabel *motorDirectionLabel = new QLabel("Rotation Direction", motorDirectionWidget);
	Style::setStyle(motorDirectionLabel, style::properties::label::subtle);
	calibrationMotorDirectionSwitch = new CustomSwitch("CW", "CCW", motorDirectionWidget);
	calibrationMotorDirectionSwitch->setChecked(isMotorRotationClockwise);
	connect(calibrationMotorDirectionSwitch, &CustomSwitch::toggled, this,
		[this](bool b) { isMotorRotationClockwise = b; });
	motorDirectionLayout->addWidget(motorDirectionLabel);
	motorDirectionLayout->addWidget(calibrationMotorDirectionSwitch);

	calibrationDatasetConfigCollapseSection->contentLayout()->setSpacing(8);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationModeMenuCombo);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountWidget);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationTotalSamplesWidget);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(motorRPMWidget);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(motorDirectionWidget);

#pragma endregion

#pragma region Calibration Data Section Widget
	MenuSectionWidget *calibrationDataSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(calibrationDataSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *calibrationDataCollapseSection = new MenuCollapseSection(
		"Calibration Data", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, calibrationDataSectionWidget);
	calibrationDataSectionWidget->contentLayout()->setSpacing(8);
	calibrationDataSectionWidget->contentLayout()->addWidget(calibrationDataCollapseSection);

	importSamplesButton = new QPushButton("Import Samples", calibrationDataCollapseSection);
	extractDataButton = new QPushButton("Save to CSV", calibrationDataCollapseSection);
	StyleHelper::BasicButton(importSamplesButton);
	StyleHelper::BasicButton(extractDataButton);

	calibrationDataCollapseSection->contentLayout()->setSpacing(8);
	calibrationDataCollapseSection->contentLayout()->addWidget(importSamplesButton);
	calibrationDataCollapseSection->contentLayout()->addWidget(extractDataButton);
#pragma endregion

#pragma region Logs Section Widget
	MenuSectionWidget *logsSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	Style::setStyle(logsSectionWidget, style::properties::widget::basicComponent);
	MenuCollapseSection *logsCollapseSection =
		new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, logsSectionWidget);
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
	calibrationSettingsLayout->addWidget(calibrationCoeffSectionWidget);
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
	connect(toggleAngleCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleSineCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		calibrationRawDataPlotWidget->selectChannel(calibrationSineDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(toggleCosineCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		calibrationRawDataPlotWidget->selectChannel(calibrationCosineDataPlotChannel);
		calibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostAngleCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationRawDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostSineCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationSineDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(togglePostCosineCheckBox, &QCheckBox::toggled, this, [this](bool b) {
		postCalibrationRawDataPlotWidget->selectChannel(postCalibrationCosineDataPlotChannel);
		postCalibrationRawDataPlotWidget->selectedChannel()->setEnabled(b);
	});
	connect(calibrationDisplayFormatSwitch, &CustomSwitch::toggled, this, [this](bool b) {
		isAngleDisplayFormat = b;
		displayCalculatedCoeff();
	});

	connect(&m_calibrationWaitVelocityWatcher, &QFutureWatcher<void>::finished, this, [this]() {
		if(isMotorVelocityReached) {
			startCalibrationStreamThread();
		} else {
			startDeviceStatusMonitor();
		}
	});

	connect(&m_resetMotorToZeroWatcher, &QFutureWatcher<void>::finished, this,
		[this]() { startWaitForVelocityReachedThread(1); });

	connect(&m_calibrationStreamWatcher, &QFutureWatcher<void>::started, this, [this]() {
		QThread *thread = QThread::currentThread();
		thread->setPriority(QThread::TimeCriticalPriority);
	});

	connect(&m_calibrationStreamWatcher, &QFutureWatcher<void>::finished, this, [this]() {
		stopMotor();
		isStartMotor = false;
		toggleTabSwitching(true);

		if(isPostCalibration) {
			if(static_cast<int>(graphPostDataList.size()) == totalSamplesCount) {
				computeSineCosineOfAngles(graphPostDataList);
				m_admtController->postcalibrate(
					vector<double>(graphPostDataList.begin(), graphPostDataList.end()), cycleCount,
					samplesPerCycle, !isMotorRotationClockwise);
				populateCorrectedAngleErrorGraphs();
				isPostCalibration = false;
				isStartMotor = false;
				resetToZero = true;
				toggleCalibrationButtonState(4);
			}
		} else {
			if(static_cast<int>(graphDataList.size()) >= totalSamplesCount) {
				computeSineCosineOfAngles(graphDataList);
				calibrationLogWrite(m_admtController->calibrate(
					vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount,
					samplesPerCycle, !isMotorRotationClockwise));
				populateAngleErrorGraphs();
				calculateHarmonicValues();
				toggleCalibrationButtonState(2);
			} else {
				resetToZero = true;
				toggleCalibrationButtonState(0);
			}
		}

		startDeviceStatusMonitor();
	});

	return tool;
}

ToolTemplate *HarmonicCalibration::createRegistersWidget()
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

	MenuCollapseSection *registerConfigurationCollapseSection =
		new MenuCollapseSection("Configuration", MenuCollapseSection::MHCW_ARROW,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerConfigurationGridWidget = new QWidget(registerWidget);
	QGridLayout *registerConfigurationGridLayout = new QGridLayout(registerConfigurationGridWidget);
	registerConfigurationGridWidget->setLayout(registerConfigurationGridLayout);
	registerConfigurationGridLayout->setMargin(0);
	registerConfigurationGridLayout->setSpacing(globalSpacingSmall);
	registerConfigurationCollapseSection->contentLayout()->addWidget(registerConfigurationGridWidget);

	MenuCollapseSection *registerSensorDataCollapseSection =
		new MenuCollapseSection("Sensor Data", MenuCollapseSection::MHCW_ARROW,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerSensorDataGridWidget = new QWidget(registerWidget);
	QGridLayout *registerSensorDataGridLayout = new QGridLayout(registerSensorDataGridWidget);
	registerSensorDataGridWidget->setLayout(registerSensorDataGridLayout);
	registerSensorDataGridLayout->setMargin(0);
	registerSensorDataGridLayout->setSpacing(globalSpacingSmall);
	registerSensorDataCollapseSection->contentLayout()->addWidget(registerSensorDataGridWidget);

	MenuCollapseSection *registerDeviceIDCollapseSection =
		new MenuCollapseSection("Device ID", MenuCollapseSection::MHCW_ARROW,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerDeviceIDGridWidget = new QWidget(registerWidget);
	QGridLayout *registerDeviceIDGridLayout = new QGridLayout(registerDeviceIDGridWidget);
	registerDeviceIDGridWidget->setLayout(registerDeviceIDGridLayout);
	registerDeviceIDGridLayout->setMargin(0);
	registerDeviceIDGridLayout->setSpacing(globalSpacingSmall);
	registerDeviceIDCollapseSection->contentLayout()->addWidget(registerDeviceIDGridWidget);

	MenuCollapseSection *registerHarmonicsCollapseSection =
		new MenuCollapseSection("Harmonics", MenuCollapseSection::MHCW_ARROW,
					MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, registerWidget);
	QWidget *registerHarmonicsGridWidget = new QWidget(registerWidget);
	QGridLayout *registerHarmonicsGridLayout = new QGridLayout(registerHarmonicsGridWidget);
	registerHarmonicsGridWidget->setLayout(registerHarmonicsGridLayout);
	registerHarmonicsGridLayout->setMargin(0);
	registerHarmonicsGridLayout->setSpacing(globalSpacingSmall);
	registerHarmonicsCollapseSection->contentLayout()->addWidget(registerHarmonicsGridWidget);

	cnvPageRegisterBlock = new RegisterBlockWidget(
		"CNVPAGE", "Convert Start and Page Select",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::CNVPAGE),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	digIORegisterBlock = new RegisterBlockWidget(
		"DIGIO", "Digital Input Output",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIO),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIO),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	faultRegisterBlock = new RegisterBlockWidget(
		"FAULT", "Fault Register",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::FAULT),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	generalRegisterBlock = new RegisterBlockWidget(
		"GENERAL", "General Device Configuration",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	digIOEnRegisterBlock = new RegisterBlockWidget(
		"DIGIOEN", "Enable Digital Input/Outputs",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	angleCkRegisterBlock = new RegisterBlockWidget(
		"ANGLECK", "Primary vs Secondary Angle Check",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ANGLECK),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ANGLECK),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	eccCdeRegisterBlock = new RegisterBlockWidget(
		"ECCCDE", "Error Correction Codes",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ECCCDE),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ECCCDE),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	eccDisRegisterBlock = new RegisterBlockWidget(
		"ECCDIS", "Error Correction Code disable",
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ECCDIS),
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ECCDIS),
		RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);

	absAngleRegisterBlock =
		new RegisterBlockWidget("ABSANGLE", "Absolute Angle",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::ABSANGLE),
					m_admtController->getSensorPage(ADMTController::SensorRegister::ABSANGLE),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	angleRegisterBlock = new RegisterBlockWidget(
		"ANGLE", "Angle Register", m_admtController->getSensorRegister(ADMTController::SensorRegister::ANGLE),
		m_admtController->getSensorPage(ADMTController::SensorRegister::ANGLE),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	angleSecRegisterBlock =
		new RegisterBlockWidget("ANGLESEC", "Secondary Angle",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::ANGLESEC),
					m_admtController->getSensorPage(ADMTController::SensorRegister::ANGLESEC),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	sineRegisterBlock = new RegisterBlockWidget(
		"SINE", "Sine Measurement", m_admtController->getSensorRegister(ADMTController::SensorRegister::SINE),
		m_admtController->getSensorPage(ADMTController::SensorRegister::SINE),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	cosineRegisterBlock =
		new RegisterBlockWidget("COSINE", "Cosine Measurement",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::COSINE),
					m_admtController->getSensorPage(ADMTController::SensorRegister::COSINE),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	secAnglIRegisterBlock =
		new RegisterBlockWidget("SECANGLI", "In-phase secondary angle measurement",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::SECANGLI),
					m_admtController->getSensorPage(ADMTController::SensorRegister::SECANGLI),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	secAnglQRegisterBlock =
		new RegisterBlockWidget("SECANGLQ", "Quadrature phase secondary angle measurement",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::SECANGLQ),
					m_admtController->getSensorPage(ADMTController::SensorRegister::SECANGLQ),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	radiusRegisterBlock =
		new RegisterBlockWidget("RADIUS", "Angle measurement radius",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::RADIUS),
					m_admtController->getSensorPage(ADMTController::SensorRegister::RADIUS),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	diag1RegisterBlock =
		new RegisterBlockWidget("DIAG1", "State of the MT reference resistors and AFE fixed input voltage",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG1),
					m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG1),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	diag2RegisterBlock =
		new RegisterBlockWidget("DIAG2", "Measurements of two diagnostics resistors of fixed value",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG2),
					m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG2),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	tmp0RegisterBlock =
		new RegisterBlockWidget("TMP0", "Primary Temperature Sensor",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::TMP0),
					m_admtController->getSensorPage(ADMTController::SensorRegister::TMP0),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	tmp1RegisterBlock =
		new RegisterBlockWidget("TMP1", "Secondary Temperature Sensor",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::TMP1),
					m_admtController->getSensorPage(ADMTController::SensorRegister::TMP1),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	cnvCntRegisterBlock =
		new RegisterBlockWidget("CNVCNT", "Conversion Count",
					m_admtController->getSensorRegister(ADMTController::SensorRegister::CNVCNT),
					m_admtController->getSensorPage(ADMTController::SensorRegister::CNVCNT),
					RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	uniqID0RegisterBlock = new RegisterBlockWidget(
		"UNIQID0", "Unique ID Register 0",
		m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID0),
		m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID0),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID1RegisterBlock = new RegisterBlockWidget(
		"UNIQID1", "Unique ID Register 1",
		m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID1),
		m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID1),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID2RegisterBlock = new RegisterBlockWidget(
		"UNIQID2", "Unique ID Register 2",
		m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID2),
		m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID2),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	uniqID3RegisterBlock = new RegisterBlockWidget(
		"UNIQID3", "Product, voltage supply. ASIL and ASIC revision identifiers",
		m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID3),
		m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID3),
		RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	h1MagRegisterBlock =
		new RegisterBlockWidget("H1MAG", "1st Harmonic error magnitude",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1MAG),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h1PhRegisterBlock =
		new RegisterBlockWidget("H1PH", "1st Harmonic error phase",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1PH),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h2MagRegisterBlock =
		new RegisterBlockWidget("H2MAG", "2nd Harmonic error magnitude",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H2MAG),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h2PhRegisterBlock =
		new RegisterBlockWidget("H2PH", "2nd Harmonic error phase",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H2PH),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h3MagRegisterBlock =
		new RegisterBlockWidget("H3MAG", "3rd Harmonic error magnitude",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H3MAG),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h3PhRegisterBlock =
		new RegisterBlockWidget("H3PH", "3rd Harmonic error phase",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H3PH),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h8MagRegisterBlock =
		new RegisterBlockWidget("H8MAG", "8th Harmonic error magnitude",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H8MAG),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	h8PhRegisterBlock =
		new RegisterBlockWidget("H8PH", "8th Harmonic error phase",
					m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH),
					m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H8PH),
					RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);

	registerConfigurationGridLayout->addWidget(cnvPageRegisterBlock, 0, 0);
	registerConfigurationGridLayout->addWidget(digIORegisterBlock, 0, 1);
	registerConfigurationGridLayout->addWidget(faultRegisterBlock, 0, 2);
	registerConfigurationGridLayout->addWidget(generalRegisterBlock, 0, 3);
	registerConfigurationGridLayout->addWidget(digIOEnRegisterBlock, 0, 4);
	registerConfigurationGridLayout->addWidget(eccCdeRegisterBlock, 1, 0);
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

	for(int c = 0; c < registerConfigurationGridLayout->columnCount(); ++c)
		registerConfigurationGridLayout->setColumnStretch(c, 1);
	for(int c = 0; c < registerDeviceIDGridLayout->columnCount(); ++c)
		registerDeviceIDGridLayout->setColumnStretch(c, 1);
	for(int c = 0; c < registerHarmonicsGridLayout->columnCount(); ++c)
		registerHarmonicsGridLayout->setColumnStretch(c, 1);
	for(int c = 0; c < registerSensorDataGridLayout->columnCount(); ++c)
		registerSensorDataGridLayout->setColumnStretch(c, 1);

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
	tool->layout()->setContentsMargins(globalSpacingSmall, globalSpacingSmall,
					   Style::getDimension(json::global::unit_0_5), globalSpacingSmall);

	connectRegisterBlockToRegistry(cnvPageRegisterBlock);
	connectRegisterBlockToRegistry(digIORegisterBlock);
	connectRegisterBlockToRegistry(faultRegisterBlock);
	connectRegisterBlockToRegistry(generalRegisterBlock);
	connectRegisterBlockToRegistry(digIOEnRegisterBlock);
	connectRegisterBlockToRegistry(angleCkRegisterBlock);
	connectRegisterBlockToRegistry(eccCdeRegisterBlock);
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

ToolTemplate *HarmonicCalibration::createUtilityWidget()
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
	MenuCollapseSection *commandLogCollapseSection = new MenuCollapseSection(
		"Command Log", MenuCollapseSection::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, commandLogSectionWidget);
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
	MenuCollapseSection *DIGIOMonitorCollapseSection = new MenuCollapseSection(
		"DIGIO Monitor", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, DIGIOMonitorSectionWidget);
	DIGIOMonitorSectionWidget->contentLayout()->addWidget(DIGIOMonitorCollapseSection);
	DIGIOMonitorCollapseSection->contentLayout()->setSpacing(8);

	DIGIOBusyStatusLED = createStatusLEDWidget("BUSY (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOCNVStatusLED = createStatusLEDWidget("CNV (Input)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOSENTStatusLED = createStatusLEDWidget("SENT (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOACALCStatusLED = createStatusLEDWidget("ACALC (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOFaultStatusLED = createStatusLEDWidget("FAULT (Output)", "digio", false, DIGIOMonitorCollapseSection);
	DIGIOBootloaderStatusLED =
		createStatusLEDWidget("BOOTLOADER (Output)", "digio", false, DIGIOMonitorCollapseSection);

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
	MenuCollapseSection *DIGIOControlCollapseSection = new MenuCollapseSection(
		"DIGIO Control", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, DIGIOControlSectionWidget);
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
	DIGIO0ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO0ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO0EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO0FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO0FNCToggleSwitch, "GPIO0", "BUSY");
	DIGIO0FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO0FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("BUSY", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO0" : "BUSY"));
	});

	DIGIO1ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO1ENToggleSwitch, "Output", "Input");
	DIGIO1ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO1ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO1EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO1FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO1FNCToggleSwitch, "GPIO1", "CNV");
	DIGIO1FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO1FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("CNV", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO1" : "CNV"));
	});

	DIGIO2ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO2ENToggleSwitch, "Output", "Input");
	DIGIO2ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO2ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO2EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO2FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO2FNCToggleSwitch, "GPIO2", "SENT");
	DIGIO2FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO2FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("SENT", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO2" : "SENT"));
	});

	DIGIO3ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO3ENToggleSwitch, "Output", "Input");
	DIGIO3ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO3ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO3EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO3FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO3FNCToggleSwitch, "GPIO3", "ACALC");
	DIGIO3FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO3FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("ACALC", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO3" : "ACALC"));
	});

	DIGIO4ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO4ENToggleSwitch, "Output", "Input");
	DIGIO4ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO4ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO4EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO4FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO4FNCToggleSwitch, "GPIO4", "FAULT");
	DIGIO4FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO4FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("FAULT", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO4" : "FAULT"));
	});

	DIGIO5ENToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO5ENToggleSwitch, "Output", "Input");
	DIGIO5ENToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO5ENToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("DIGIO5EN", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "Output" : "Input"));
	});

	DIGIO5FNCToggleSwitch = new CustomSwitch();
	changeCustomSwitchLabel(DIGIO5FNCToggleSwitch, "GPIO5", "BOOT");
	DIGIO5FNCToggleSwitch->setFixedWidth(Style::getDimension(json::global::unit_6));
	connect(DIGIO5FNCToggleSwitch, &CustomSwitch::clicked, this, [this](bool value) {
		if(!toggleDIGIOEN("BOOTLOAD", value))
			StatusBarManager::pushMessage("Failed to toggle " + QString(value ? "GPIO5" : "BOOT"));
	});

	QPushButton *DIGIOResetButton = new QPushButton("Reset DIGIO", DIGIOControlGridWidget);
	DIGIOResetButton->setFixedWidth(100);
	StyleHelper::BasicButton(DIGIOResetButton);
	connect(DIGIOResetButton, &QPushButton::clicked, this, [this] {
		toggleUtilityTask(false);
		bool success = resetDIGIO();
		if(success)
			StatusBarManager::pushMessage("Successfully reset DIGIO");
		else
			StatusBarManager::pushMessage("Failed to reset DIGIO");
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

	if(deviceType == "Industrial") {
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
	MenuCollapseSection *MTDIAG1CollapseSection = new MenuCollapseSection(
		"MT Diagnostic Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, MTDIAG1SectionWidget);
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
	MenuCollapseSection *MTDiagnosticsCollapseSection = new MenuCollapseSection(
		"MT Diagnostics", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, MTDiagnosticsSectionWidget);
	MTDiagnosticsSectionWidget->contentLayout()->addWidget(MTDiagnosticsCollapseSection);
	MTDiagnosticsCollapseSection->contentLayout()->setSpacing(globalSpacingSmall);

	QWidget *AFEDIAG0Widget = new QWidget(MTDiagnosticsSectionWidget);
	QVBoxLayout *AFEDIAG0Layout = new QVBoxLayout(AFEDIAG0Widget);
	AFEDIAG0Widget->setLayout(AFEDIAG0Layout);
	AFEDIAG0Layout->setMargin(0);
	AFEDIAG0Layout->setSpacing(0);
	QLabel *AFEDIAG0Label = new QLabel("AFEDIAG0 (%)", AFEDIAG0Widget);
	Style::setStyle(AFEDIAG0Label, style::properties::label::subtle);
	AFEDIAG0LineEdit = new QLineEdit(AFEDIAG0Widget);
	AFEDIAG0LineEdit->setReadOnly(true);
	connectLineEditToNumber(AFEDIAG0LineEdit, afeDiag0, "V");
	AFEDIAG0Layout->addWidget(AFEDIAG0Label);
	AFEDIAG0Layout->addWidget(AFEDIAG0LineEdit);

	QWidget *AFEDIAG1Widget = new QWidget(MTDiagnosticsSectionWidget);
	QVBoxLayout *AFEDIAG1Layout = new QVBoxLayout(AFEDIAG1Widget);
	AFEDIAG1Widget->setLayout(AFEDIAG1Layout);
	AFEDIAG1Layout->setMargin(0);
	AFEDIAG1Layout->setSpacing(0);
	QLabel *AFEDIAG1Label = new QLabel("AFEDIAG1 (%)", AFEDIAG1Widget);
	Style::setStyle(AFEDIAG1Label, style::properties::label::subtle);
	AFEDIAG1LineEdit = new QLineEdit(AFEDIAG1Widget);
	AFEDIAG1LineEdit->setReadOnly(true);
	connectLineEditToNumber(AFEDIAG1LineEdit, afeDiag1, "V");
	AFEDIAG1Layout->addWidget(AFEDIAG1Label);
	AFEDIAG1Layout->addWidget(AFEDIAG1LineEdit);

	QWidget *AFEDIAG2Widget = new QWidget(MTDiagnosticsSectionWidget);
	QVBoxLayout *AFEDIAG2Layout = new QVBoxLayout(AFEDIAG2Widget);
	AFEDIAG2Widget->setLayout(AFEDIAG2Layout);
	AFEDIAG2Layout->setMargin(0);
	AFEDIAG2Layout->setSpacing(0);
	QLabel *AFEDIAG2Label = new QLabel("AFEDIAG2 (V)", AFEDIAG2Widget);
	Style::setStyle(AFEDIAG2Label, style::properties::label::subtle);
	AFEDIAG2LineEdit = new QLineEdit(AFEDIAG2Widget);
	AFEDIAG2LineEdit->setReadOnly(true);
	connectLineEditToNumber(AFEDIAG2LineEdit, afeDiag2, "V");
	AFEDIAG2Layout->addWidget(AFEDIAG2Label);
	AFEDIAG2Layout->addWidget(AFEDIAG2LineEdit);

	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG0Widget);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG1Widget);
	MTDiagnosticsCollapseSection->contentLayout()->addWidget(AFEDIAG2Widget);

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
	MenuCollapseSection *faultRegisterCollapseSection = new MenuCollapseSection(
		"Fault Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE,
		MenuCollapseSection::MenuHeaderWidgetType::MHW_BASEWIDGET, faultRegisterSectionWidget);
	faultRegisterSectionWidget->contentLayout()->addWidget(faultRegisterCollapseSection);
	faultRegisterCollapseSection->contentLayout()->setSpacing(8);

	VDDUnderVoltageStatusLED =
		createStatusLEDWidget("VDD Under Voltage", "fault", false, faultRegisterCollapseSection);
	VDDOverVoltageStatusLED =
		createStatusLEDWidget("VDD Over Voltage", "fault", false, faultRegisterCollapseSection);
	VDRIVEUnderVoltageStatusLED =
		createStatusLEDWidget("VDRIVE Under Voltage", "fault", false, faultRegisterCollapseSection);
	VDRIVEOverVoltageStatusLED =
		createStatusLEDWidget("VDRIVE Over Voltage", "fault", false, faultRegisterCollapseSection);
	AFEDIAGStatusLED = createStatusLEDWidget("AFEDIAG", "fault", false, faultRegisterCollapseSection);
	NVMCRCFaultStatusLED = createStatusLEDWidget("NVM CRC Fault", "fault", false, faultRegisterCollapseSection);
	ECCDoubleBitErrorStatusLED =
		createStatusLEDWidget("ECC Double Bit Error", "fault", false, faultRegisterCollapseSection);
	OscillatorDriftStatusLED =
		createStatusLEDWidget("Oscillator Drift", "fault", false, faultRegisterCollapseSection);
	CountSensorFalseStateStatusLED =
		createStatusLEDWidget("Count Sensor False State", "fault", false, faultRegisterCollapseSection);
	AngleCrossCheckStatusLED =
		createStatusLEDWidget("Angle Cross Check", "fault", false, faultRegisterCollapseSection);
	TurnCountSensorLevelsStatusLED =
		createStatusLEDWidget("Turn Count Sensor Levels", "fault", false, faultRegisterCollapseSection);
	MTDIAGStatusLED = createStatusLEDWidget("MTDIAG", "fault", false, faultRegisterCollapseSection);
	TurnCounterCrossCheckStatusLED =
		createStatusLEDWidget("Turn Counter Cross Check", "fault", false, faultRegisterCollapseSection);
	RadiusCheckStatusLED = createStatusLEDWidget("Radius Check", "fault", false, faultRegisterCollapseSection);
	SequencerWatchdogStatusLED =
		createStatusLEDWidget("Sequencer Watchdog", "fault", false, faultRegisterCollapseSection);

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

bool HarmonicCalibration::readDeviceProperties()
{
	uint8_t UNIQID3Page = m_admtController->getUniqueIdPage(ADMTController::UniqueIDRegister::UNIQID3);

	bool success = false;

	if(!changeCNVPage(UNIQID3Page))
		return false;

	uint32_t *uniqId3RegisterValue = new uint32_t;
	if(m_admtController->readDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getUniqueIdRegister(ADMTController::UniqueIDRegister::UNIQID3),
		   uniqId3RegisterValue) != -1) {
		deviceRegisterMap =
			QMap(m_admtController->getUNIQID3RegisterMapping(static_cast<uint16_t>(*uniqId3RegisterValue)));

		if(deviceRegisterMap.value("Supply ID") == "5V") {
			is5V = true;
		} else if(deviceRegisterMap.value("Supply ID") == "3.3V") {
			is5V = false;
		} else {
			is5V = false;
		}

		deviceName = QString::fromStdString(deviceRegisterMap.value("Product ID"));

		if(deviceRegisterMap.value("ASIL ID") == "ASIL QM") {
			deviceType = QString::fromStdString("Industrial");
		} else if(deviceRegisterMap.value("ASIL ID") == "ASIL B") {
			deviceType = QString::fromStdString("Automotive");
		} else {
			deviceType = QString::fromStdString(deviceRegisterMap.value("ASIL ID"));
		}

		success = true;
	}
	delete uniqId3RegisterValue;

	if(!success) {
		StatusBarManager::pushMessage("Failed to read device properties");

		// If read device property fails, assumes running emulator
		deviceName = "ADMT4000";
		deviceType = "Emulator";
	}

	return success;
}

void HarmonicCalibration::initializeADMT()
{
	bool resetDIGIOSuccess = resetDIGIO();
	bool resetGENERALSuccess = resetGENERAL();
	bool resetHarmonicSuccess = clearHarmonicRegisters();

	if(!resetDIGIOSuccess || !resetGENERALSuccess || !resetHarmonicSuccess) {
		StatusBarManager::pushMessage("Failed to initialize ADMT");
	}
}

bool HarmonicCalibration::readSequence()
{
	uint32_t generalRegisterAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL);
	uint8_t generalRegisterPage =
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL);

	bool success = false;

	if(!changeCNVPage(generalRegisterPage))
		return false;

	uint32_t *generalRegValue = new uint32_t;
	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						generalRegisterAddress, generalRegValue) != -1) {
		if(*generalRegValue != UINT32_MAX) {
			GENERALRegisterMap = QMap(m_admtController->getGeneralRegisterBitMapping(
				static_cast<uint16_t>(*generalRegValue)));
			sequenceMode = GENERALRegisterMap.value("Sequence Type");
			conversionMode = GENERALRegisterMap.value("Conversion Type");
			success = true;
		}
	}
	delete generalRegValue;

	return success;
}

bool HarmonicCalibration::writeSequence(QMap<string, int> settings)
{
	if(!disableECC(true))
		return false;

	uint8_t generalRegisterPage =
		m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL);

	if(!changeCNVPage(generalRegisterPage))
		return false;

	bool success = false;

	uint32_t generalRegisterAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::GENERAL);
	uint32_t *generalRegValue = new uint32_t;

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						generalRegisterAddress, generalRegValue) == 0) {
		uint32_t newGeneralRegValue =
			m_admtController->setGeneralRegisterBitMapping(*generalRegValue, settings.toStdMap());

		if(convertStart(false) && changeCNVPage(generalRegisterPage) &&
		   m_admtController->writeDeviceRegistry(
			   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), generalRegisterAddress,
			   newGeneralRegValue) == 0 &&
		   readSequence() &&
		   settings.value("Convert Synchronization") == GENERALRegisterMap.value("Convert Synchronization") &&
		   settings.value("Angle Filter") == GENERALRegisterMap.value("Angle Filter") &&
		   settings.value("8th Harmonic") == GENERALRegisterMap.value("8th Harmonic") &&
		   settings.value("Sequence Type") == GENERALRegisterMap.value("Sequence Type") &&
		   settings.value("Conversion Type") == GENERALRegisterMap.value("Conversion Type")) {
			success = true;
		}
	}

	delete generalRegValue;

	return success;
}

void HarmonicCalibration::applySequence()
{
	QMap<string, int> settings;

	bool success = false;

	settings["Sequence Type"] = qvariant_cast<int>(sequenceModeMenuCombo->combo()->currentData()); // sequenceType;
	settings["Conversion Type"] =
		qvariant_cast<int>(conversionModeMenuCombo->combo()->currentData()); // conversionType;
	settings["Convert Synchronization"] =
		qvariant_cast<int>(convertSynchronizationMenuCombo->combo()->currentData());	     // convertSync;
	settings["Angle Filter"] = qvariant_cast<int>(angleFilterMenuCombo->combo()->currentData()); // angleFilter;
	settings["8th Harmonic"] =
		qvariant_cast<int>(eighthHarmonicMenuCombo->combo()->currentData()); // eighthHarmonic;

	success = writeSequence(settings);
	if(!success)
		StatusBarManager::pushUrgentMessage("Failed to apply sequence settings");
	else
		StatusBarManager::pushUrgentMessage("Sequence settings applied successfully");
}

bool HarmonicCalibration::changeCNVPage(uint8_t page)
{
	if(page == UINT8_MAX)
		return true;

	bool success = false;
	uint32_t *cnvPageRegValue = new uint32_t;
	uint32_t cnvPageAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						cnvPageAddress, cnvPageRegValue) == 0) {
		uint32_t newCNVPageRegValue =
			m_admtController->changeCNVPage(static_cast<uint16_t>(*cnvPageRegValue), page);

		if(newCNVPageRegValue != *cnvPageRegValue) {
			if(m_admtController->writeDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
				   newCNVPageRegValue) == 0 &&
			   m_admtController->readDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
				   cnvPageRegValue) == 0)
				if(newCNVPageRegValue == *cnvPageRegValue)
					success = true;
		} else
			success = true;
	}

	delete cnvPageRegValue;

	return success;
}

bool HarmonicCalibration::convertStart(bool start)
{
	bool success = false;
	uint32_t *cnvPageRegValue = new uint32_t;
	uint32_t cnvPageAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						cnvPageAddress, cnvPageRegValue) == 0) {
		uint32_t newCNVPageRegValue =
			m_admtController->convertStart(start, static_cast<uint16_t>(*cnvPageRegValue));

		if(newCNVPageRegValue != *cnvPageRegValue) {
			if(m_admtController->writeDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
				   newCNVPageRegValue) == 0 &&
			   m_admtController->readDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
				   cnvPageRegValue) == 0)
				if(newCNVPageRegValue == *cnvPageRegValue)
					success = true;
		} else
			success = true;
	}

	delete cnvPageRegValue;

	return success;
}

bool HarmonicCalibration::convertRestart()
{
	bool success = false;
	uint32_t *cnvPageRegValue = new uint32_t;
	uint32_t cnvPageAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						cnvPageAddress, cnvPageRegValue) == 0) {
		uint32_t stopConvertCNVPageRegValue =
			m_admtController->convertStart(false, static_cast<uint16_t>(*cnvPageRegValue));
		uint32_t startConvertCNVPageRegValue =
			m_admtController->convertStart(true, static_cast<uint16_t>(*cnvPageRegValue));

		if(m_admtController->writeDeviceRegistry(
			   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
			   stopConvertCNVPageRegValue) == 0 &&
		   m_admtController->writeDeviceRegistry(
			   m_admtController->getDeviceId(ADMTController::Device::ADMT4000), cnvPageAddress,
			   startConvertCNVPageRegValue) &&
		   m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
							cnvPageAddress, cnvPageRegValue) == 0)
			if(startConvertCNVPageRegValue == *cnvPageRegValue)
				success = true;
	}

	delete cnvPageRegValue;

	return success;
}

void HarmonicCalibration::initializeMotor()
{
	motor_rpm = default_motor_rpm;
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
	if(!m_deviceStatusThread.isRunning()) {
		isDeviceStatusMonitor = true;
		m_deviceStatusThread =
			QtConcurrent::run(this, &HarmonicCalibration::getDeviceFaultStatus, deviceStatusMonitorRate);
		m_deviceStatusWatcher.setFuture(m_deviceStatusThread);
	}
}

void HarmonicCalibration::stopDeviceStatusMonitor()
{
	isDeviceStatusMonitor = false;
	if(m_deviceStatusThread.isRunning()) {
		m_deviceStatusThread.cancel();
		m_deviceStatusWatcher.waitForFinished();
	}
}

void HarmonicCalibration::getDeviceFaultStatus(int sampleRate)
{
	bool registerFault = false;
	bool success = true;
	while(isDeviceStatusMonitor) {
		if(m_admtController->writeDeviceRegistry(
			   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT),
			   0) == 0) {
			uint32_t *readValue = new uint32_t;
			if(m_admtController->readDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				   m_admtController->getConfigurationRegister(
					   ADMTController::ConfigurationRegister::FAULT),
				   readValue) == 0) {
				registerFault = m_admtController->checkRegisterFault(static_cast<uint16_t>(*readValue),
										     sequenceMode == 0 ? true : false);

				Q_EMIT updateFaultStatusSignal(registerFault);
			} else {
				success = false;
				delete readValue;
			}
		} else
			success = false;

		if(!success) {
			StatusBarManager::pushMessage("Could not read device, disconnecting device");
			Q_EMIT m_admtController->requestDisconnect();
			break;
		}

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::requestDisconnect() { Q_EMIT m_admtController->requestDisconnect(); }

void HarmonicCalibration::stopTasks()
{
	stopAcquisition();

	stopCalibrationUITask();
	stopUtilityTask();

	stopResetMotorToZero();
	stopCalibrationStreamThread();

	stopDeviceStatusMonitor();
}

void HarmonicCalibration::currentMotorPositionTask(int sampleRate)
{
	double motorPosition = 0;
	while(isMotorPositionMonitor) {
		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, motorPosition);

		Q_EMIT motorPositionChanged(motorPosition);

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::updateMotorPosition(double position)
{
	current_pos = position;
	if(isAcquisitionTab)
		updateLineEditValue(acquisitionMotorCurrentPositionLineEdit, current_pos);
}

bool HarmonicCalibration::resetGENERAL()
{
	bool success = false;

	uint8_t GENERALPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::GENERAL);

	if(!changeCNVPage(GENERALPage))
		return false;

	if(deviceRegisterMap.contains("ASIL ID")) {
		uint32_t resetValue = 0x0000;

		if(deviceRegisterMap.value("ASIL ID") == "ASIL QM")
			resetValue = 0x1230; // Industrial or ASIL QM
		else if(deviceRegisterMap.value("ASIL ID") == "ASIL B")
			resetValue = 0x1200; // Automotive or ASIL B

		if(resetValue != 0x0000) {
			if(m_admtController->writeDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				   m_admtController->getConfigurationRegister(
					   ADMTController::ConfigurationRegister::GENERAL),
				   resetValue) == 0) {
				success = true;
			}
		}
	}

	return success;
}

#pragma region Acquisition Methods
bool HarmonicCalibration::updateChannelValues()
{
	bool success = false;

	if(readMode == 0 || readMode == 2) {
		updateChannelValue(ADMTController::Channel::ROTATION);
		updateChannelValue(ADMTController::Channel::ANGL);
		updateChannelValue(ADMTController::Channel::TEMPERATURE);
	} else if(readMode == 1) {
		if(conversionMode == 1)
			convertRestart();
		updateSensorValue(ADMTController::SensorRegister::ANGLE);
		updateSensorValue(ADMTController::SensorRegister::ABSANGLE);
		updateSensorValue(ADMTController::SensorRegister::TMP0);
	}

	if(rotation == UINT32_MAX)
		return false;
	if(angle == UINT32_MAX)
		return false;
	if(count == UINT32_MAX)
		return false;
	if(temp == UINT32_MAX)
		return false;

	success = true;

	return success;
}

bool HarmonicCalibration::updateSensorValue(ADMTController::SensorRegister sensor)
{
	bool success = false;

	uint32_t sensorPage = m_admtController->getSensorPage(sensor);
	if(!changeCNVPage(sensorPage))
		return false;

	uint32_t *registerValue = new uint32_t;

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						m_admtController->getSensorRegister(sensor), registerValue) == 0) {
		switch(sensor) {
		case ADMTController::SensorRegister::ABSANGLE:
			rotation = m_admtController->getAbsAngle(static_cast<uint16_t>(*registerValue));
			count = m_admtController->getAbsAngleTurnCount(static_cast<uint16_t>(*registerValue));
			break;
		case ADMTController::SensorRegister::ANGLE:
			angle = m_admtController->getAngle(static_cast<uint16_t>(*registerValue));
			break;
		case ADMTController::SensorRegister::TMP0:
			temp = m_admtController->getTemperature(static_cast<uint16_t>(*registerValue));
		default:
			break;
		}
		success = true;
	} else {
		switch(sensor) {
		case ADMTController::SensorRegister::ABSANGLE:
			rotation = UINT32_MAX;
			count = UINT32_MAX;
			break;
		case ADMTController::SensorRegister::ANGLE:
			angle = UINT32_MAX;
			break;
		case ADMTController::SensorRegister::TMP0:
			temp = UINT32_MAX;
		default:
			break;
		}
	}

	delete registerValue;

	return success;
}

void HarmonicCalibration::updateLineEditValues()
{
	if(rotation == UINT32_MAX) {
		rotationValueLabel->setText("N/A");
	} else {
		rotationValueLabel->setText(QString::number(rotation) + "°");
	}
	if(angle == UINT32_MAX) {
		angleValueLabel->setText("N/A");
	} else {
		angleValueLabel->setText(QString::number(angle) + "°");
	}
	if(count == UINT32_MAX) {
		countValueLabel->setText("N/A");
	} else {
		if(count == 54)
			countValueLabel->setText("Invalid turn count (54)");
		else if(count == -9)
			countValueLabel->setText("Invalid turn count (-9)");
		else
			countValueLabel->setText(QString::number(count));
	}
	if(temp == UINT32_MAX) {
		tempValueLabel->setText("N/A");
	} else {
		tempValueLabel->setText(QString::number(temp, 10, 2) + " °C");
	}
}

void HarmonicCalibration::updateAcquisitionData(QMap<SensorData, double> sensorDataMap)
{
	if(isStartAcquisition)
		updateLineEditValues();

	if(acquisitionDataMap.value(ANGLE) && sensorDataMap.contains(ANGLE))
		appendAcquisitionData(sensorDataMap.value(ANGLE), acquisitionAngleList);
	else
		acquisitionAngleList.clear();
	if(acquisitionDataMap.value(ABSANGLE) && sensorDataMap.contains(ABSANGLE))
		appendAcquisitionData(sensorDataMap.value(ABSANGLE), acquisitionABSAngleList);
	else
		acquisitionABSAngleList.clear();
	if(acquisitionDataMap.value(TMP0) && sensorDataMap.contains(TMP0))
		appendAcquisitionData(sensorDataMap.value(TMP0), acquisitionTmp0List);
	else
		acquisitionTmp0List.clear();
	if(acquisitionDataMap.value(SINE) && sensorDataMap.contains(SINE))
		appendAcquisitionData(sensorDataMap.value(SINE), acquisitionSineList);
	else
		acquisitionSineList.clear();
	if(acquisitionDataMap.value(COSINE) && sensorDataMap.contains(COSINE))
		appendAcquisitionData(sensorDataMap.value(COSINE), acquisitionCosineList);
	else
		acquisitionCosineList.clear();
	if(acquisitionDataMap.value(RADIUS) && sensorDataMap.contains(RADIUS))
		appendAcquisitionData(sensorDataMap.value(RADIUS), acquisitionRadiusList);
	else
		acquisitionRadiusList.clear();
	if(acquisitionDataMap.value(ANGLESEC) && sensorDataMap.contains(ANGLESEC))
		appendAcquisitionData(sensorDataMap.value(ANGLESEC), acquisitionAngleSecList);
	else
		acquisitionAngleSecList.clear();
	if(acquisitionDataMap.value(SECANGLI) && sensorDataMap.contains(SECANGLI))
		appendAcquisitionData(sensorDataMap.value(SECANGLI), acquisitionSecAnglIList);
	else
		acquisitionSecAnglIList.clear();
	if(acquisitionDataMap.value(SECANGLQ) && sensorDataMap.contains(SECANGLQ))
		appendAcquisitionData(sensorDataMap.value(SECANGLQ), acquisitionSecAnglQList);
	else
		acquisitionSecAnglQList.clear();
	if(acquisitionDataMap.value(TMP1) && sensorDataMap.contains(TMP1))
		appendAcquisitionData(sensorDataMap.value(TMP1), acquisitionTmp1List);
	else
		acquisitionTmp1List.clear();

	Q_EMIT acquisitionGraphChanged();
}

void HarmonicCalibration::startAcquisition()
{
	isStartAcquisition = true;
	acquisitionXPlotAxis->setInterval(0, acquisitionDisplayLength);

	applySequenceAndUpdate();

	m_admtController->stopStream.storeRelease(false);

	m_acquisitionDataThread =
		QtConcurrent::run(this, &HarmonicCalibration::getAcquisitionSamples, acquisitionDataMap);
	m_acquisitionDataWatcher.setFuture(m_acquisitionDataThread);
}

void HarmonicCalibration::stopAcquisition()
{
	isStartAcquisition = false;
	m_admtController->stopStream.storeRelease(true);
	if(m_acquisitionDataThread.isRunning()) {
		m_acquisitionDataThread.cancel();
		m_acquisitionDataWatcher.waitForFinished();
	}
	if(m_acquisitionGraphThread.isRunning()) {
		m_acquisitionGraphThread.cancel();
		m_acquisitionGraphWatcher.waitForFinished();
	}
}

void HarmonicCalibration::restartAcquisition()
{
	if(m_acquisitionDataThread.isRunning()) {
		stopAcquisition();
		startAcquisition();
	}
}

void HarmonicCalibration::updateFaultStatus(bool status)
{
	bool isChanged = (deviceStatusFault != status);
	if(isChanged) {
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

void HarmonicCalibration::getAcquisitionSamples(QMap<SensorData, bool> dataMap)
{
	if(readMode == 0) {
		QVector<QString> channelNames = {"rot", "angl", "count", "temp"};

		m_admtController->streamChannel(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						channelNames, bufferSize, readDeviceDebounce);
	} else if(readMode == 1 || readMode == 2) {
		QMap<SensorData, double> sensorDataMap;
		while(isStartAcquisition) {
			if(updateChannelValues()) {

				if(acquisitionDataMap.value(ANGLE)) {
					sensorDataMap[ANGLE] = angle;
				}
				if(acquisitionDataMap.value(ABSANGLE)) {
					sensorDataMap[ABSANGLE] = rotation;
				}
				if(acquisitionDataMap.value(TMP0)) {
					sensorDataMap[TMP0] = temp;
				}
				if(acquisitionDataMap.value(SINE)) {
					sensorDataMap[SINE] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::SINE);
				}
				if(acquisitionDataMap.value(COSINE)) {
					sensorDataMap[COSINE] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::COSINE);
				}
				if(acquisitionDataMap.value(RADIUS)) {
					sensorDataMap[RADIUS] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::RADIUS);
				}
				if(acquisitionDataMap.value(ANGLESEC)) {
					sensorDataMap[ANGLESEC] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::ANGLESEC);
				}
				if(acquisitionDataMap.value(SECANGLI)) {
					sensorDataMap[SECANGLI] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::SECANGLI);
				}
				if(acquisitionDataMap.value(SECANGLQ)) {
					sensorDataMap[SECANGLQ] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::SECANGLQ);
				}
				if(acquisitionDataMap.value(TMP1)) {
					sensorDataMap[TMP1] =
						getSensorDataAcquisitionValue(ADMTController::SensorRegister::TMP1);
				}

				if(sensorDataMap.count() > 0)
					Q_EMIT acquisitionDataChanged(sensorDataMap);
			} else
				break;
		}
	}
}

void HarmonicCalibration::handleStreamChannelData(QMap<QString, double> dataMap)
{
	if(dataMap.contains("angl"))
		angle = dataMap.value("angl");
	if(dataMap.contains("rot") && dataMap.contains("count")) {
		double tempRotation = dataMap.value("rot");
		double tempCount = dataMap.value("count");
		rotation = calculateABSAngle(tempRotation, tempCount);
		count = tempCount;
	} else if(dataMap.contains("rot")) {
		count = getTurnCount(dataMap.value("rot"));
		double tempRotation = getABSAngle(dataMap.value("rot"));
		rotation = calculateABSAngle(tempRotation, count);
	}
	if(dataMap.contains("temp"))
		temp = dataMap.value("temp");

	QMap<SensorData, double> sensorDataMap;

	if(acquisitionDataMap.value(ANGLE)) {
		sensorDataMap[ANGLE] = angle;
	}
	if(acquisitionDataMap.value(ABSANGLE)) {
		sensorDataMap[ABSANGLE] = rotation;
	}
	if(acquisitionDataMap.value(TMP0)) {
		sensorDataMap[TMP0] = temp;
	}
	if(acquisitionDataMap.value(SINE)) {
		sensorDataMap[SINE] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::SINE);
	}
	if(acquisitionDataMap.value(COSINE)) {
		sensorDataMap[COSINE] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::COSINE);
	}
	if(acquisitionDataMap.value(RADIUS)) {
		sensorDataMap[RADIUS] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::RADIUS);
	}
	if(acquisitionDataMap.value(ANGLESEC)) {
		sensorDataMap[ANGLESEC] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::ANGLESEC);
	}
	if(acquisitionDataMap.value(SECANGLI)) {
		sensorDataMap[SECANGLI] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::SECANGLI);
	}
	if(acquisitionDataMap.value(SECANGLQ)) {
		sensorDataMap[SECANGLQ] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::SECANGLQ);
	}
	if(acquisitionDataMap.value(TMP1)) {
		sensorDataMap[TMP1] = getSensorDataAcquisitionValue(ADMTController::SensorRegister::TMP1);
	}

	if(sensorDataMap.count() > 0)
		Q_EMIT acquisitionDataChanged(sensorDataMap);
}

double HarmonicCalibration::calculateABSAngle(double &absAngle, double &turnCount)
{
	if(turnCount != 0)
		return absAngle + (turnCount * 360);
	else
		return absAngle;
}

double HarmonicCalibration::getTurnCount(double rawAbsAngleValue)
{
	uint16_t value = static_cast<uint16_t>(rawAbsAngleValue);
	return m_admtController->getAbsAngleTurnCount(value);
}

double HarmonicCalibration::getABSAngle(double rawAbsAngleValue)
{
	uint16_t value = static_cast<uint16_t>(rawAbsAngleValue);
	return m_admtController->getAbsAngle(value);
}

double HarmonicCalibration::getSensorDataAcquisitionValue(const ADMTController::SensorRegister &key)
{
	uint32_t *readValue = new uint32_t;
	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						m_admtController->getSensorRegister(key),
						m_admtController->getSensorPage(key), readValue) != 0)
		return qQNaN();

	switch(key) {
	case ADMTController::SensorRegister::SINE: {
		map<string, double> sineRegisterMap =
			m_admtController->getSineRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return sineRegisterMap.at("SINE");
		break;
	}
	case ADMTController::SensorRegister::COSINE: {
		map<string, double> cosineRegisterMap =
			m_admtController->getCosineRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return cosineRegisterMap.at("COSINE");
		break;
	}
	case ADMTController::SensorRegister::RADIUS: {
		map<string, double> radiusRegisterMap =
			m_admtController->getRadiusRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return radiusRegisterMap.at("RADIUS");
		break;
	}
	case ADMTController::SensorRegister::ANGLESEC: {
		map<string, double> angleSecRegisterMap =
			m_admtController->getAngleSecRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return angleSecRegisterMap.at("ANGLESEC");
		break;
	}
	case ADMTController::SensorRegister::SECANGLI: {
		map<string, double> secAnglIRegisterMap =
			m_admtController->getSecAnglIRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return secAnglIRegisterMap.at("SECANGLI");
		break;
	}
	case ADMTController::SensorRegister::SECANGLQ: {
		map<string, double> secAnglQRegisterMap =
			m_admtController->getSecAnglQRegisterBitMapping(static_cast<uint16_t>(*readValue));
		return secAnglQRegisterMap.at("SECANGLQ");
		break;
	}
	case ADMTController::SensorRegister::TMP1: {
		map<string, double> tmp1RegisterMap =
			m_admtController->getTmp1RegisterBitMapping(static_cast<uint16_t>(*readValue), is5V);
		return tmp1RegisterMap.at("TMP1");
		break;
	}
	default:
		return qQNaN();
		break;
	}
}

void HarmonicCalibration::plotAcquisition(QVector<double> list, PlotChannel *channel)
{
	QVector<double> reverseList(list.rbegin(), list.rend());
	channel->curve()->setSamples(reverseList);
	auto result = minmax_element(list.begin(), list.end());
	if(*result.first < acquisitionGraphYMin)
		acquisitionGraphYMin = *result.first;
	if(*result.second > acquisitionGraphYMax)
		acquisitionGraphYMax = *result.second;
}

void HarmonicCalibration::appendAcquisitionData(double data, QVector<double> &list)
{
	if(data == qQNaN())
		return;
	list.append(data);
	if(list.size() >= acquisitionDisplayLength) {
		list.removeFirst();
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

void HarmonicCalibration::updateAcquisitionGraph()
{
	if(acquisitionDataMap.value(ANGLE))
		plotAcquisition(acquisitionAngleList, acquisitionAnglePlotChannel);
	if(acquisitionDataMap.value(ABSANGLE))
		plotAcquisition(acquisitionABSAngleList, acquisitionABSAnglePlotChannel);
	if(acquisitionDataMap.value(TMP0))
		plotAcquisition(acquisitionTmp0List, acquisitionTmp0PlotChannel);
	if(acquisitionDataMap.value(SINE))
		plotAcquisition(acquisitionSineList, acquisitionSinePlotChannel);
	if(acquisitionDataMap.value(COSINE))
		plotAcquisition(acquisitionCosineList, acquisitionCosinePlotChannel);
	if(acquisitionDataMap.value(RADIUS))
		plotAcquisition(acquisitionRadiusList, acquisitionRadiusPlotChannel);
	if(acquisitionDataMap.value(ANGLESEC))
		plotAcquisition(acquisitionAngleSecList, acquisitionAngleSecPlotChannel);
	if(acquisitionDataMap.value(SECANGLI))
		plotAcquisition(acquisitionSecAnglIList, acquisitionSecAnglIPlotChannel);
	if(acquisitionDataMap.value(SECANGLQ))
		plotAcquisition(acquisitionSecAnglQList, acquisitionSecAnglQPlotChannel);
	if(acquisitionDataMap.value(TMP1))
		plotAcquisition(acquisitionTmp1List, acquisitionTmp1PlotChannel);

	acquisitionYPlotAxis->setInterval(acquisitionGraphYMin, acquisitionGraphYMax);
	acquisitionGraphPlotWidget->replot();
}

void HarmonicCalibration::updateSequenceWidget()
{
	if(!GENERALRegisterMap.contains("Sequence Type") || !GENERALRegisterMap.contains("Conversion Type") ||
	   !GENERALRegisterMap.contains("Convert Synchronization") || !GENERALRegisterMap.contains("Angle Filter") ||
	   !GENERALRegisterMap.contains("8th Harmonic")) {
		return;
	}

	if(GENERALRegisterMap.value("Sequence Type") == -1) {
		sequenceModeMenuCombo->combo()->setCurrentText("Reserved");
	} else {
		sequenceModeMenuCombo->combo()->setCurrentIndex(
			sequenceModeMenuCombo->combo()->findData(GENERALRegisterMap.value("Sequence Type")));
	}
	conversionModeMenuCombo->combo()->setCurrentIndex(
		conversionModeMenuCombo->combo()->findData(GENERALRegisterMap.value("Conversion Type")));
	if(GENERALRegisterMap.value("Convert Synchronization") == -1) {
		convertSynchronizationMenuCombo->combo()->setCurrentText("Reserved");
	} else {
		convertSynchronizationMenuCombo->combo()->setCurrentIndex(
			convertSynchronizationMenuCombo->combo()->findData(
				GENERALRegisterMap.value("Convert Synchronization")));
	}
	angleFilterMenuCombo->combo()->setCurrentIndex(
		angleFilterMenuCombo->combo()->findData(GENERALRegisterMap.value("Angle Filter")));
	eighthHarmonicMenuCombo->combo()->setCurrentIndex(
		eighthHarmonicMenuCombo->combo()->findData(GENERALRegisterMap.value("8th Harmonic")));
}

void HarmonicCalibration::updateCapturedDataCheckBoxes()
{
	acquisitionGraphChannelGridLayout->removeWidget(angleCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(sineCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(cosineCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(radiusCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(absAngleCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(temp0CheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(angleSecCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(secAnglQCheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(secAnglICheckBox);
	acquisitionGraphChannelGridLayout->removeWidget(temp1CheckBox);

	if(sequenceMode == 0) // Sequence Mode 1
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 1);
		angleSecCheckBox->setChecked(false);
		secAnglICheckBox->setChecked(false);
		secAnglQCheckBox->setChecked(false);
		temp1CheckBox->setChecked(false);
		angleSecCheckBox->hide();
		secAnglICheckBox->hide();
		secAnglQCheckBox->hide();
		temp1CheckBox->hide();
	} else if(sequenceMode == 1) // Sequence Mode 2
	{
		acquisitionGraphChannelGridLayout->addWidget(angleCheckBox, 0, 0);
		acquisitionGraphChannelGridLayout->addWidget(sineCheckBox, 0, 1);
		acquisitionGraphChannelGridLayout->addWidget(cosineCheckBox, 0, 2);
		acquisitionGraphChannelGridLayout->addWidget(angleSecCheckBox, 0, 3);
		acquisitionGraphChannelGridLayout->addWidget(secAnglQCheckBox, 0, 4);
		acquisitionGraphChannelGridLayout->addWidget(secAnglICheckBox, 1, 0);
		acquisitionGraphChannelGridLayout->addWidget(radiusCheckBox, 1, 1);
		acquisitionGraphChannelGridLayout->addWidget(absAngleCheckBox, 1, 2);
		acquisitionGraphChannelGridLayout->addWidget(temp0CheckBox, 1, 3);
		acquisitionGraphChannelGridLayout->addWidget(temp1CheckBox, 1, 4);
		angleSecCheckBox->show();
		secAnglICheckBox->show();
		secAnglQCheckBox->show();
		temp1CheckBox->show();
	}

	acquisitionGraphChannelGridLayout->update();
	acquisitionGraphChannelWidget->update();
}

void HarmonicCalibration::applySequenceAndUpdate()
{
	applySequence();
	updateSequenceWidget();
	updateCapturedDataCheckBoxes();
}

void HarmonicCalibration::toggleAcquisitionControls(bool value)
{
	sequenceModeMenuCombo->setEnabled(value);
	conversionModeMenuCombo->setEnabled(value);
	convertSynchronizationMenuCombo->setEnabled(value);
	angleFilterMenuCombo->setEnabled(value);
	eighthHarmonicMenuCombo->setEnabled(value);
	applySequenceButton->setEnabled(value);
	displayLengthLineEdit->setEnabled(value);

	if(sequenceMode == 0) // Sequence Mode 1
	{
		angleCheckBox->setEnabled(value);
		sineCheckBox->setEnabled(value);
		cosineCheckBox->setEnabled(value);
		radiusCheckBox->setEnabled(value);
		absAngleCheckBox->setEnabled(value);
		temp0CheckBox->setEnabled(value);
	} else if(sequenceMode == 1) // Sequence Mode 2
	{
		angleCheckBox->setEnabled(value);
		sineCheckBox->setEnabled(value);
		cosineCheckBox->setEnabled(value);
		angleSecCheckBox->setEnabled(value);
		secAnglQCheckBox->setEnabled(value);
		secAnglICheckBox->setEnabled(value);
		radiusCheckBox->setEnabled(value);
		absAngleCheckBox->setEnabled(value);
		temp0CheckBox->setEnabled(value);
		temp1CheckBox->setEnabled(value);
	}
}

void HarmonicCalibration::connectCheckBoxToAcquisitionGraph(QCheckBox *widget, PlotChannel *channel, SensorData key)
{
	connect(widget, &QCheckBox::stateChanged, this, [this, channel, key](int state) {
		bool value = true;
		if(state == Qt::Checked) {
			value = true;
		} else {
			value = false;
		}
		channel->setEnabled(value);
		acquisitionDataMap[key] = value;
		if(!value)
			acquisitionGraphPlotWidget->replot();
	});
}

void HarmonicCalibration::GMRReset()
{
	// Set Motor Angle to 315 degrees
	target_pos = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);

	// Write 1 to ADMT IIO Attribute coil_rs
	m_admtController->setDeviceAttributeValue(
		m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		m_admtController->getDeviceAttribute(ADMTController::DeviceAttribute::SDP_COIL_RS), 1);

	// Write 0xc000 to CNVPAGE
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE),
		   0xc000) != -1) {
		// Write 0x0000 to CNVPAGE
		if(m_admtController->writeDeviceRegistry(
			   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE),
			   0x0000) != -1) {
			// Read ABSANGLE

			StatusBarManager::pushMessage("GMR Reset Done");
		} else {
			StatusBarManager::pushMessage("Failed to write CNVPAGE Register");
		}
	} else {
		StatusBarManager::pushMessage("Failed to write CNVPAGE Register");
	}
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
		stopAcquisition();
	} else {
		startAcquisition();
	}

	toggleAcquisitionControls(!b);
}
#pragma endregion

#pragma region Calibration Methods
void HarmonicCalibration::startCalibrationUITask()
{
	if(!m_calibrationUIThread.isRunning()) {
		isCalibrationTab = true;
		m_calibrationUIThread =
			QtConcurrent::run(this, &HarmonicCalibration::calibrationUITask, calibrationUITimerRate);
		m_calibrationUIWatcher.setFuture(m_calibrationUIThread);
	}
}

void HarmonicCalibration::stopCalibrationUITask()
{
	isCalibrationTab = false;
	if(m_calibrationUIThread.isRunning()) {
		m_calibrationUIThread.cancel();
		m_calibrationUIWatcher.waitForFinished();
	}
}

void HarmonicCalibration::updateCalibrationGraph()
{
	if(isStartMotor) {
		if(isPostCalibration) {
			postCalibrationRawDataPlotChannel->curve()->setSamples(graphPostDataList);
			postCalibrationRawDataPlotWidget->replot();
		} else {
			calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
			calibrationRawDataPlotWidget->replot();
		}
	}
}

void HarmonicCalibration::calibrationUITask(int sampleRate)
{
	while(isCalibrationTab) {
		Q_EMIT calibrationGraphChanged();

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
	if(resetCurrentPositionToZero()) {
		int step = floor(motorMicrostepPerRevolution / samplesPerCycle);
		if(isMotorRotationClockwise)
			step = -step;
		if(isPostCalibration) {
			int currentSamplesCount = graphPostDataList.size();
			while(isStartMotor && currentSamplesCount < totalSamplesCount) {
				target_pos = current_pos + step;
				if(moveMotorToPosition(target_pos, true) == false)
					m_admtController->disconnectADMT();

				if(readMode == 0 || readMode == 2) {
					if(updateChannelValue(ADMTController::Channel::ANGL))
						break;
				} else if(readMode == 1) {
					convertStart(true);
					if(!updateSensorValue(ADMTController::SensorRegister::ANGLE))
						break;
					convertStart(false);
				}
				graphPostDataList.append(angle);
				currentSamplesCount++;
			}
		} else {
			int currentSamplesCount = graphDataList.size();
			while(isStartMotor && currentSamplesCount < totalSamplesCount) {
				target_pos = current_pos + step;
				if(moveMotorToPosition(target_pos, true) == false)
					m_admtController->disconnectADMT();

				if(readMode == 0 || readMode == 2) {
					if(updateChannelValue(ADMTController::Channel::ANGL))
						break;
				} else if(readMode == 1) {
					convertStart(true);
					if(!updateSensorValue(ADMTController::SensorRegister::ANGLE))
						break;
					convertStart(false);
				}
				graphDataList.append(angle);
				currentSamplesCount++;
			}
		}
	}

	stopMotor();
}

void HarmonicCalibration::startCalibration()
{
	samplesPerCycle = ceil(totalSamplesCount / cycleCount);
	graphPostDataList.reserve(totalSamplesCount);
	graphPostDataList.squeeze();
	graphDataList.reserve(totalSamplesCount);
	graphDataList.squeeze();

	calibrationDataGraphTabWidget->setCurrentIndex(0);
	calibrationRawDataXPlotAxis->setInterval(0, totalSamplesCount);

	toggleTabSwitching(false);
	toggleCalibrationControls(false);
	toggleCalibrationButtonState(1);

	configureCalibrationSequenceSettings(calibrationMode);
	clearHarmonicRegisters();
	convertRestart();
	if(calibrationMode == 0)
		startContinuousCalibration();
	else
		startOneShotCalibration();
}

void HarmonicCalibration::stopCalibration()
{
	isStartMotor = false;
	if(calibrationMode == 0) {
		stopContinuousCalibration();
	}

	toggleTabSwitching(true);
	toggleCalibrationControls(true);
}

void HarmonicCalibration::startContinuousCalibration()
{
	stopDeviceStatusMonitor();

	if(isPostCalibration)
		StatusBarManager::pushMessage("Acquiring Post Calibration Samples, Please Wait...");
	else
		StatusBarManager::pushMessage("Acquiring Calibration Samples, Please Wait...");
	startResetMotorToZero(true);
}

void HarmonicCalibration::stopContinuousCalibration()
{
	stopMotor();
	stopWaitForVelocityReachedThread();
	stopCalibrationStreamThread();

	startDeviceStatusMonitor();
}

void HarmonicCalibration::startResetMotorToZero(bool enableWatcher)
{
	isResetMotorToZero = true;
	if(!m_resetMotorToZeroThread.isRunning()) {
		m_resetMotorToZeroThread = QtConcurrent::run(this, &HarmonicCalibration::resetMotorToZero);
		if(enableWatcher)
			m_resetMotorToZeroWatcher.setFuture(m_resetMotorToZeroThread);
	}
}

void HarmonicCalibration::stopResetMotorToZero()
{
	isResetMotorToZero = false;
	if(m_resetMotorToZeroThread.isRunning()) {
		m_resetMotorToZeroThread.cancel();
		m_resetMotorToZeroWatcher.waitForFinished();
	}
}

void HarmonicCalibration::resetMotorToZero()
{
	if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0) {
		if(current_pos != 0) {
			setRampMode(isMotorRotationClockwise); // Write to ramp mode in case of motor disable
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX,
						 convertRPStoVMAX(convertRPMtoRPS(fast_motor_rpm)));
			writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0);
			readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
			while(isResetMotorToZero && current_pos != 0) {
				if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) !=
				   0)
					break;
				QThread::msleep(readMotorDebounce);
			}
			if(current_pos == 0) {
				writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
				writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0);
			}
		}
	}
}

void HarmonicCalibration::handleStreamCalibrationData(double value)
{
	if(isPostCalibration) {
		graphPostDataList.append(value);
		postCalibrationRawDataPlotChannel->curve()->setSamples(graphPostDataList);
		postCalibrationRawDataPlotWidget->replot();
	} else {
		graphDataList.append(value);
		calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
		calibrationRawDataPlotWidget->replot();
	}
}

void HarmonicCalibration::startCalibrationStreamThread()
{
	if(!m_calibrationStreamThread.isRunning()) {
		m_admtController->stopStream.storeRelease(false);

		continuousCalibrationSampleRate =
			calculateContinuousCalibrationSampleRate(convertVMAXtoRPS(rotate_vmax), samplesPerCycle);
		if(readMode == 0 || readMode == 2) {
			m_calibrationStreamThread = QtConcurrent::run([this]() {
				m_admtController->bufferedStreamIO(totalSamplesCount, continuousCalibrationSampleRate,
								   bufferSize);
			});
		} else if(readMode == 1) {
			m_calibrationStreamThread = QtConcurrent::run([this]() {
				m_admtController->registryStream(totalSamplesCount, continuousCalibrationSampleRate);
			});
		}
		m_calibrationStreamWatcher.setFuture(m_calibrationStreamThread);
	}
}

void HarmonicCalibration::stopCalibrationStreamThread()
{
	m_admtController->stopStream.storeRelease(true);
	if(m_calibrationStreamThread.isRunning()) {
		m_calibrationStreamThread.cancel();
		m_calibrationStreamWatcher.waitForFinished();
	}
}

void HarmonicCalibration::startWaitForVelocityReachedThread(int mode)
{
	if(!m_calibrationWaitVelocityWatcher.isRunning()) {
		isMotorVelocityCheck = true;
		m_calibrationWaitVelocityThread = QtConcurrent::run(this, &HarmonicCalibration::waitForVelocityReached,
								    mode, motorWaitVelocityReachedSampleRate);
		m_calibrationWaitVelocityWatcher.setFuture(m_calibrationWaitVelocityThread);
	}
}

void HarmonicCalibration::stopWaitForVelocityReachedThread()
{
	isMotorVelocityCheck = false;
	if(m_calibrationWaitVelocityWatcher.isRunning()) {
		m_calibrationWaitVelocityThread.cancel();
		m_calibrationWaitVelocityWatcher.waitForFinished();
	}
}

void HarmonicCalibration::waitForVelocityReached(int mode, int sampleRate)
{
	isMotorVelocityReached = false;
	if(mode == 0) {
		QThread::msleep(floor(convertAMAXtoAccelTime(amax) * 1000));
		isMotorVelocityReached = true;
	} else if(mode == 1) {
		moveMotorContinuous();
		uint32_t *registerValue = new uint32_t;
		while(isMotorVelocityCheck && !isMotorVelocityReached) {
			if(readMotorRegisterValue(
				   m_admtController->getRampGeneratorDriverFeatureControlRegister(
					   ADMTController::RampGeneratorDriverFeatureControlRegister::RAMP_STAT),
				   registerValue) == 0) {
				isMotorVelocityReached = m_admtController->checkVelocityReachedFlag(
					static_cast<uint16_t>(*registerValue));
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

void HarmonicCalibration::configureCalibrationSequenceSettings(int conversionMode)
{
	if(!readSequence()) {
		StatusBarManager::pushMessage("Failed to configure calibration sequence settings");
		return;
	}

	GENERALRegisterMap["Conversion Type"] = conversionMode; // 0: One-shot Conversion, 1: Continuous Conversion
	GENERALRegisterMap["8th Harmonic"] = 1;			// Always use user-supplied 8th Harmonic
	writeSequence(GENERALRegisterMap);
}

void HarmonicCalibration::startOneShotCalibration()
{
	if(resetToZero && !isPostCalibration) {
		clearCalibrationSamples();
		clearPostCalibrationSamples();
		clearAngleErrorGraphs();
		clearFFTAngleErrorGraphs();
	} else if(resetToZero) {
		clearPostCalibrationSamples();
	}

	QFuture<void> future = QtConcurrent::run(this, &HarmonicCalibration::getCalibrationSamples);
	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

	connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
		toggleTabSwitching(true);
		toggleCalibrationControls(true);

		calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
		calibrationRawDataPlotChannel->xAxis()->setMax(graphDataList.size());
		calibrationRawDataPlotWidget->replot();
		isStartMotor = false;

		if(isPostCalibration) {
			if(static_cast<int>(graphPostDataList.size()) == totalSamplesCount) {
				computeSineCosineOfAngles(graphPostDataList);
				m_admtController->postcalibrate(
					vector<double>(graphPostDataList.begin(), graphPostDataList.end()), cycleCount,
					samplesPerCycle, !isMotorRotationClockwise);
				populateCorrectedAngleErrorGraphs();
				isPostCalibration = false;
				isStartMotor = false;
				resetToZero = true;
				toggleCalibrationButtonState(4);
			} else
				toggleCalibrationButtonState(2);
		} else {
			if(static_cast<int>(graphDataList.size()) == totalSamplesCount) {
				computeSineCosineOfAngles(graphDataList);
				calibrationLogWrite(m_admtController->calibrate(
					vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount,
					samplesPerCycle, !isMotorRotationClockwise));
				populateAngleErrorGraphs();
				calculateHarmonicValues();
				toggleCalibrationButtonState(2);
			} else {
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
	if(calibrationMode == 0)
		startContinuousCalibration();
	else
		startOneShotCalibration();
}

void HarmonicCalibration::resetAllCalibrationState()
{
	clearCalibrationSamples();
	clearPostCalibrationSamples();
	calibrationDataGraphTabWidget->setCurrentIndex(0);

	clearAngleErrorGraphs();
	clearFFTAngleErrorGraphs();
	resultDataTabWidget->setCurrentIndex(0);

	toggleCalibrationButtonState(0);

	isPostCalibration = false;
	isCalculatedCoeff = false;
	resetToZero = true;
	displayCalculatedCoeff();

	clearHarmonicRegisters();

	toggleCalibrationControls(true);
}

void HarmonicCalibration::computeSineCosineOfAngles(QVector<double> graphDataList)
{
	m_admtController->computeSineCosineOfAngles(vector<double>(graphDataList.begin(), graphDataList.end()));
	if(isPostCalibration) {
		postCalibrationSineDataPlotChannel->curve()->setSamples(
			m_admtController->calibration_samples_sine_scaled.data(),
			m_admtController->calibration_samples_sine_scaled.size());
		postCalibrationCosineDataPlotChannel->curve()->setSamples(
			m_admtController->calibration_samples_cosine_scaled.data(),
			m_admtController->calibration_samples_cosine_scaled.size());
		postCalibrationRawDataPlotWidget->replot();
	} else {
		calibrationSineDataPlotChannel->curve()->setSamples(
			m_admtController->calibration_samples_sine_scaled.data(),
			m_admtController->calibration_samples_sine_scaled.size());
		calibrationCosineDataPlotChannel->curve()->setSamples(
			m_admtController->calibration_samples_cosine_scaled.data(),
			m_admtController->calibration_samples_cosine_scaled.size());
		calibrationRawDataPlotWidget->replot();
	}
}

void HarmonicCalibration::populateAngleErrorGraphs()
{
	QVector<double> angleError =
		QVector<double>(m_admtController->angleError.begin(), m_admtController->angleError.end());
	QVector<double> FFTAngleErrorMagnitude = QVector<double>(m_admtController->FFTAngleErrorMagnitude.begin(),
								 m_admtController->FFTAngleErrorMagnitude.end());
	QVector<double> FFTAngleErrorPhase = QVector<double>(m_admtController->FFTAngleErrorPhase.begin(),
							     m_admtController->FFTAngleErrorPhase.end());

	angleErrorPlotChannel->curve()->setSamples(angleError);
	auto angleErrorMinMax = minmax_element(angleError.begin(), angleError.end());
	if(currentAngleErrorGraphMin > *angleErrorMinMax.first) {
		currentAngleErrorGraphMin = *angleErrorMinMax.first;
		angleErrorYPlotAxis->setMin(currentAngleErrorGraphMin);
		correctedErrorYPlotAxis->setMin(currentAngleErrorGraphMin);
	}
	if(currentAngleErrorGraphMax < *angleErrorMinMax.second) {
		currentAngleErrorGraphMax = *angleErrorMinMax.second;
		angleErrorYPlotAxis->setMax(currentAngleErrorGraphMax);
		correctedErrorYPlotAxis->setMax(currentAngleErrorGraphMax);
	}
	angleErrorXPlotAxis->setMax(angleError.size());
	angleErrorPlotWidget->replot();
	correctedErrorPlotWidget->replot();

	FFTAngleErrorMagnitudeChannel->curve()->setSamples(FFTAngleErrorMagnitude);
	auto angleErrorMagnitudeMinMax = minmax_element(FFTAngleErrorMagnitude.begin(), FFTAngleErrorMagnitude.end());
	if(currentMagnitudeGraphMin > *angleErrorMagnitudeMinMax.first) {
		currentMagnitudeGraphMin = *angleErrorMagnitudeMinMax.first;
		FFTAngleErrorMagnitudeYPlotAxis->setMin(currentMagnitudeGraphMin);
		FFTCorrectedErrorMagnitudeYPlotAxis->setMin(currentMagnitudeGraphMin);
	}
	if(currentMagnitudeGraphMax < *angleErrorMagnitudeMinMax.second) {
		currentMagnitudeGraphMax = *angleErrorMagnitudeMinMax.second;
		FFTAngleErrorMagnitudeYPlotAxis->setMax(currentMagnitudeGraphMax);
		FFTCorrectedErrorMagnitudeYPlotAxis->setMax(currentMagnitudeGraphMax);
	}
	FFTAngleErrorMagnitudeXPlotAxis->setMax(FFTAngleErrorMagnitude.size());
	FFTAngleErrorMagnitudePlotWidget->replot();
	FFTCorrectedErrorMagnitudePlotWidget->replot();

	FFTAngleErrorPhaseChannel->curve()->setSamples(FFTAngleErrorPhase);
	auto angleErrorPhaseMinMax = minmax_element(FFTAngleErrorPhase.begin(), FFTAngleErrorPhase.end());
	if(currentPhaseGraphMin > *angleErrorPhaseMinMax.first) {
		currentPhaseGraphMin = *angleErrorPhaseMinMax.first;
		FFTAngleErrorPhaseYPlotAxis->setMin(currentPhaseGraphMin);
		FFTCorrectedErrorPhaseYPlotAxis->setMin(currentPhaseGraphMin);
	}
	if(currentPhaseGraphMax < *angleErrorPhaseMinMax.second) {
		currentPhaseGraphMax = *angleErrorPhaseMinMax.second;
		FFTAngleErrorPhaseYPlotAxis->setMax(currentPhaseGraphMax);
		FFTCorrectedErrorPhaseYPlotAxis->setMax(currentPhaseGraphMax);
	}
	FFTAngleErrorPhaseXPlotAxis->setInterval(0, FFTAngleErrorPhase.size());
	FFTAngleErrorPhasePlotWidget->replot();
	FFTCorrectedErrorPhasePlotWidget->replot();

	resultDataTabWidget->setCurrentIndex(0); // Set tab to Angle Error
}

void HarmonicCalibration::populateCorrectedAngleErrorGraphs()
{
	QVector<double> correctedError(m_admtController->correctedError.begin(),
				       m_admtController->correctedError.end());
	QVector<double> FFTCorrectedErrorMagnitude(m_admtController->FFTCorrectedErrorMagnitude.begin(),
						   m_admtController->FFTCorrectedErrorMagnitude.end());
	QVector<double> FFTCorrectedErrorPhase(m_admtController->FFTCorrectedErrorPhase.begin(),
					       m_admtController->FFTCorrectedErrorPhase.end());

	correctedErrorPlotChannel->curve()->setSamples(correctedError);
	auto correctedErrorMagnitudeMinMax = minmax_element(correctedError.begin(), correctedError.end());
	if(currentAngleErrorGraphMin > *correctedErrorMagnitudeMinMax.first) {
		currentAngleErrorGraphMin = *correctedErrorMagnitudeMinMax.first;
		angleErrorYPlotAxis->setMin(currentAngleErrorGraphMin);
		correctedErrorYPlotAxis->setMin(currentAngleErrorGraphMin);
	}
	if(currentAngleErrorGraphMax < *correctedErrorMagnitudeMinMax.second) {
		currentAngleErrorGraphMax = *correctedErrorMagnitudeMinMax.second;
		angleErrorYPlotAxis->setMax(currentAngleErrorGraphMax);
		correctedErrorYPlotAxis->setMax(currentAngleErrorGraphMax);
	}
	correctedErrorXPlotAxis->setMax(correctedError.size());
	angleErrorPlotWidget->replot();
	correctedErrorPlotWidget->replot();

	FFTCorrectedErrorMagnitudeChannel->curve()->setSamples(FFTCorrectedErrorMagnitude);
	auto FFTCorrectedErrorMagnitudeMinMax =
		minmax_element(FFTCorrectedErrorMagnitude.begin(), FFTCorrectedErrorMagnitude.end());
	if(currentMagnitudeGraphMin > *FFTCorrectedErrorMagnitudeMinMax.first) {
		currentMagnitudeGraphMin = *FFTCorrectedErrorMagnitudeMinMax.first;
		FFTAngleErrorMagnitudeYPlotAxis->setMin(currentMagnitudeGraphMin);
		FFTCorrectedErrorMagnitudeYPlotAxis->setMin(currentMagnitudeGraphMin);
	}
	if(currentMagnitudeGraphMax < *FFTCorrectedErrorMagnitudeMinMax.second) {
		currentMagnitudeGraphMax = *FFTCorrectedErrorMagnitudeMinMax.second;
		FFTAngleErrorMagnitudeYPlotAxis->setMax(currentMagnitudeGraphMax);
		FFTCorrectedErrorMagnitudeYPlotAxis->setMax(currentMagnitudeGraphMax);
	}
	FFTCorrectedErrorMagnitudeXPlotAxis->setMax(FFTCorrectedErrorMagnitude.size());
	FFTAngleErrorMagnitudePlotWidget->replot();
	FFTCorrectedErrorMagnitudePlotWidget->replot();

	FFTCorrectedErrorPhaseChannel->curve()->setSamples(FFTCorrectedErrorPhase);
	auto FFTCorrectedErrorPhaseMinMax =
		minmax_element(FFTCorrectedErrorPhase.begin(), FFTCorrectedErrorPhase.end());
	if(currentPhaseGraphMin > *FFTCorrectedErrorPhaseMinMax.first) {
		currentPhaseGraphMin = *FFTCorrectedErrorPhaseMinMax.first;
		FFTAngleErrorPhaseYPlotAxis->setMin(currentPhaseGraphMin);
		FFTCorrectedErrorPhaseYPlotAxis->setMin(currentPhaseGraphMin);
	}
	if(currentPhaseGraphMax < *FFTCorrectedErrorPhaseMinMax.second) {
		currentPhaseGraphMax = *FFTCorrectedErrorPhaseMinMax.second;
		FFTAngleErrorPhaseYPlotAxis->setMax(currentPhaseGraphMax);
		FFTCorrectedErrorPhaseYPlotAxis->setMax(currentPhaseGraphMax);
	}
	FFTCorrectedErrorPhaseXPlotAxis->setMax(FFTCorrectedErrorPhase.size());
	FFTAngleErrorPhasePlotWidget->replot();
	FFTCorrectedErrorPhasePlotWidget->replot();

	resultDataTabWidget->setCurrentIndex(2); // Set tab to Angle Error
}

bool HarmonicCalibration::clearHarmonicRegisters()
{
	bool success = false;

	uint32_t value = 0x0;
	uint32_t harmonicCNVPage = m_admtController->getHarmonicPage(ADMTController::HarmonicRegister::H1MAG);

	if(!changeCNVPage(harmonicCNVPage))
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), value) != 0)
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), value) != 0)
		return false;

	success = true;

	return success;
}

bool HarmonicCalibration::flashHarmonicValues()
{
	bool success = false;

	if(!changeCNVPage(0x02))
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), H1_MAG_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), H1_PHASE_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), H2_MAG_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), H2_PHASE_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), H3_MAG_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), H3_PHASE_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), H8_MAG_HEX) != 0)
		return false;
	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), H8_PHASE_HEX) != 0)
		return false;

	isCalculatedCoeff = true;
	displayCalculatedCoeff();

	success = true;

	return success;
}

void HarmonicCalibration::calculateHarmonicValues()
{
	if(changeCNVPage(0x02)) {
		uint32_t *h1MagCurrent = new uint32_t, *h1PhaseCurrent = new uint32_t, *h2MagCurrent = new uint32_t,
			 *h2PhaseCurrent = new uint32_t, *h3MagCurrent = new uint32_t, *h3PhaseCurrent = new uint32_t,
			 *h8MagCurrent = new uint32_t, *h8PhaseCurrent = new uint32_t;

		// Read and store current harmonic values
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), h1MagCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), h2MagCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), h3MagCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), h8MagCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), h1PhaseCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), h2PhaseCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), h3PhaseCurrent);
		m_admtController->readDeviceRegistry(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
			m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), h8PhaseCurrent);

		// Calculate harmonic coefficients (Hex)
		H1_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(
			static_cast<uint16_t>(m_admtController->HAR_MAG_1), static_cast<uint16_t>(*h1MagCurrent),
			"h1"));
		H2_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(
			static_cast<uint16_t>(m_admtController->HAR_MAG_2), static_cast<uint16_t>(*h2MagCurrent),
			"h2"));
		H3_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(
			static_cast<uint16_t>(m_admtController->HAR_MAG_3), static_cast<uint16_t>(*h3MagCurrent),
			"h3"));
		H8_MAG_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(
			static_cast<uint16_t>(m_admtController->HAR_MAG_8), static_cast<uint16_t>(*h8MagCurrent),
			"h8"));
		H1_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(
			static_cast<uint16_t>(m_admtController->HAR_PHASE_1), static_cast<uint16_t>(*h1PhaseCurrent)));
		H2_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(
			static_cast<uint16_t>(m_admtController->HAR_PHASE_2), static_cast<uint16_t>(*h2PhaseCurrent)));
		H3_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(
			static_cast<uint16_t>(m_admtController->HAR_PHASE_3), static_cast<uint16_t>(*h3PhaseCurrent)));
		H8_PHASE_HEX = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(
			static_cast<uint16_t>(m_admtController->HAR_PHASE_8), static_cast<uint16_t>(*h8PhaseCurrent)));

		calibrationLogWrite();
		calibrationLogWrite(QString("Calculated H1 Mag (Hex): 0x%1")
					    .arg(QString::number(H1_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H1 Phase (Hex): 0x%1")
					    .arg(QString::number(H1_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H2 Mag (Hex): 0x%1")
					    .arg(QString::number(H2_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H2 Phase (Hex): 0x%1")
					    .arg(QString::number(H2_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H3 Mag (Hex): 0x%1")
					    .arg(QString::number(H3_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H3 Phase (Hex): 0x%1")
					    .arg(QString::number(H3_PHASE_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H8 Mag (Hex): 0x%1")
					    .arg(QString::number(H8_MAG_HEX, 16).rightJustified(4, '0')));
		calibrationLogWrite(QString("Calculated H8 Phase (Hex): 0x%1")
					    .arg(QString::number(H8_PHASE_HEX, 16).rightJustified(4, '0')));

		// Get actual harmonic values from hex
		H1_MAG_ANGLE =
			m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H1_MAG_HEX), "h1mag");
		H1_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H1_PHASE_HEX),
										  "h1phase");
		H2_MAG_ANGLE =
			m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H2_MAG_HEX), "h2mag");
		H2_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H2_PHASE_HEX),
										  "h2phase");
		H3_MAG_ANGLE =
			m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H3_MAG_HEX), "h3mag");
		H3_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H3_PHASE_HEX),
										  "h3phase");
		H8_MAG_ANGLE =
			m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H8_MAG_HEX), "h8mag");
		H8_PHASE_ANGLE = m_admtController->getActualHarmonicRegisterValue(static_cast<uint16_t>(H8_PHASE_HEX),
										  "h8phase");

		calibrationLogWrite();
		calibrationLogWrite(QString("Calculated H1 Mag (Angle): %1°").arg(QString::number(H1_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H1 Phase (Angle): %1°").arg(QString::number(H1_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H2 Mag (Angle): %1°").arg(QString::number(H2_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H2 Phase (Angle): %1°").arg(QString::number(H2_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H3 Mag (Angle): %1°").arg(QString::number(H3_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H3 Phase (Angle): %1°").arg(QString::number(H3_PHASE_ANGLE)));
		calibrationLogWrite(QString("Calculated H8 Mag (Angle): %1°").arg(QString::number(H8_MAG_ANGLE)));
		calibrationLogWrite(QString("Calculated H8 Phase (Angle): %1°").arg(QString::number(H8_PHASE_ANGLE)));

		if(isAngleDisplayFormat)
			updateCalculatedCoeffAngle();
		else
			updateCalculatedCoeffHex();
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
	if(isAngleDisplayFormat) {
		if(isCalculatedCoeff) {
			updateCalculatedCoeffAngle();
		} else {
			resetCalculatedCoeffAngle();
		}
	} else {
		if(isCalculatedCoeff) {
			updateCalculatedCoeffHex();
		} else {
			resetCalculatedCoeffHex();
		}
	}
}

void HarmonicCalibration::calibrationLogWrite(QString message) { logsPlainTextEdit->appendPlainText(message); }

void HarmonicCalibration::importCalibrationData()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Import"), "",
		tr("Comma-separated values files (*.csv);;"
		   "Tab-delimited values files (*.txt)"),
		nullptr, (useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	FileManager fm("HarmonicCalibration");

	try {
		fm.open(fileName, FileManager::IMPORT);

		graphDataList = fm.read(0);
		if(graphDataList.size() > 0) {
			calibrationRawDataPlotChannel->curve()->setSamples(graphDataList);
			calibrationRawDataXPlotAxis->setInterval(0, graphDataList.size());
			calibrationRawDataPlotWidget->replot();

			computeSineCosineOfAngles(graphDataList);
			calibrationLogWrite(m_admtController->calibrate(
				vector<double>(graphDataList.begin(), graphDataList.end()), cycleCount, samplesPerCycle,
				false)); // TODO: Hard-coded Clockwise
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

	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString fileName = QFileDialog::getSaveFileName(
		this, tr("Export"), "", filter.join(";;"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if(fileName.split(".").size() <= 1) {
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	if(!fileName.isEmpty()) {
		bool withScopyHeader = false;
		FileManager fm("HarmonicCalibration");
		fm.open(fileName, FileManager::EXPORT);

		QVector<double> angleError =
			QVector<double>(m_admtController->angleError.begin(), m_admtController->angleError.end());
		QVector<double> FFTAngleErrorMagnitude =
			QVector<double>(m_admtController->FFTAngleErrorMagnitude.begin(),
					m_admtController->FFTAngleErrorMagnitude.end());
		QVector<double> FFTAngleErrorPhase = QVector<double>(m_admtController->FFTAngleErrorPhase.begin(),
								     m_admtController->FFTAngleErrorPhase.end());

		QVector<double> correctedError(m_admtController->correctedError.begin(),
					       m_admtController->correctedError.end());
		QVector<double> FFTCorrectedErrorMagnitude(m_admtController->FFTCorrectedErrorMagnitude.begin(),
							   m_admtController->FFTCorrectedErrorMagnitude.end());
		QVector<double> FFTCorrectedErrorPhase(m_admtController->FFTCorrectedErrorPhase.begin(),
						       m_admtController->FFTCorrectedErrorPhase.end());

		QVector<QString> h1Mag = {QString::number(H1_MAG_ANGLE)};
		QVector<QString> h2Mag = {QString::number(H2_MAG_ANGLE)};
		QVector<QString> h3Mag = {QString::number(H3_MAG_ANGLE)};
		QVector<QString> h8Mag = {QString::number(H8_MAG_ANGLE)};
		QVector<QString> h1Phase = {QString::number(H1_PHASE_ANGLE)};
		QVector<QString> h2Phase = {QString::number(H2_PHASE_ANGLE)};
		QVector<QString> h3Phase = {QString::number(H3_PHASE_ANGLE)};
		QVector<QString> h8Phase = {QString::number(H8_PHASE_ANGLE)};

		QVector<QString> h1MagHex = {QString("0x%1").arg(H1_MAG_HEX, 4, 16, QChar('0'))};
		QVector<QString> h2MagHex = {QString("0x%1").arg(H2_MAG_HEX, 4, 16, QChar('0'))};
		QVector<QString> h3MagHex = {QString("0x%1").arg(H3_MAG_HEX, 4, 16, QChar('0'))};
		QVector<QString> h8MagHex = {QString("0x%1").arg(H8_MAG_HEX, 4, 16, QChar('0'))};
		QVector<QString> h1PhaseHex = {QString("0x%1").arg(H1_PHASE_HEX, 4, 16, QChar('0'))};
		QVector<QString> h2PhaseHex = {QString("0x%1").arg(H2_PHASE_HEX, 4, 16, QChar('0'))};
		QVector<QString> h3PhaseHex = {QString("0x%1").arg(H3_PHASE_HEX, 4, 16, QChar('0'))};
		QVector<QString> h8PhaseHex = {QString("0x%1").arg(H8_PHASE_HEX, 4, 16, QChar('0'))};

		QVector<QString> stringGraphDataList(graphDataList.size());
		QVector<QString> stringGraphPostDataList(graphPostDataList.size());

		transform(graphDataList.begin(), graphDataList.end(), stringGraphDataList.begin(),
			  [](double value) { return QString::number(value); });
		transform(graphPostDataList.begin(), graphPostDataList.end(), stringGraphPostDataList.begin(),
			  [](double value) { return QString::number(value); });

		QMap<QString, QVector<QString>> headerData;

		headerData["a"] = {"Calibration Samples"};
		headerData["b"] = {"Angle Error"};
		headerData["c"] = {"FFT Angle Error Magnitude"};
		headerData["d"] = {"FFT Angle Error Phase"};
		headerData["e"] = {"Post Calibration Samples"};
		headerData["f"] = {"Corrected Error"};
		headerData["g"] = {"FFT Corrected Error Magnitude"};
		headerData["h"] = {"FFT Corrected Error Phase"};
		headerData["i"] = {"H1 Mag"};
		headerData["j"] = {"H2 Mag"};
		headerData["k"] = {"H3 Mag"};
		headerData["l"] = {"H8 Mag"};
		headerData["m"] = {"H1 Phase"};
		headerData["n"] = {"H2 Phase"};
		headerData["o"] = {"H3 Phase"};
		headerData["p"] = {"H8 Phase"};
		fm.writeToFile(false, headerData);

		bool addedHarmonicAngleValues = false, addedHarmonicHexValues = false;
		for(int i = 0; i < stringGraphDataList.count(); i++) {
			QMap<QString, QVector<QString>> tempData;
			tempData["a"] = {stringGraphDataList[i]};

			if(i < angleError.count())
				tempData["b"] = {QString::number(angleError[i])};

			if(i < FFTAngleErrorMagnitude.count())
				tempData["c"] = {QString::number(FFTAngleErrorMagnitude[i])};

			if(i < FFTAngleErrorPhase.count())
				tempData["d"] = {QString::number(FFTAngleErrorPhase[i])};

			if(stringGraphPostDataList.count() > 0 && i < stringGraphPostDataList.count())
				tempData["e"] = {stringGraphPostDataList[i]};
			else
				tempData["e"] = {""};

			if(correctedError.count() > 0 && i < correctedError.count())
				tempData["f"] = {QString::number(correctedError[i])};
			else
				tempData["f"] = {""};

			if(FFTCorrectedErrorMagnitude.count() > 0 && i < FFTCorrectedErrorMagnitude.count())
				tempData["g"] = {QString::number(FFTCorrectedErrorMagnitude[i])};
			else
				tempData["g"] = {""};

			if(FFTCorrectedErrorPhase.count() > 0 && i < FFTCorrectedErrorPhase.count())
				tempData["h"] = {QString::number(FFTCorrectedErrorPhase[i])};
			else
				tempData["h"] = {""};

			if(!addedHarmonicAngleValues) {
				tempData["i"] = h1Mag;
				tempData["j"] = h2Mag;
				tempData["k"] = h3Mag;
				tempData["l"] = h8Mag;
				tempData["m"] = h1Phase;
				tempData["n"] = h2Phase;
				tempData["o"] = h3Phase;
				tempData["p"] = h8Phase;

				addedHarmonicAngleValues = true;
			} else if(!addedHarmonicHexValues) {
				tempData["i"] = h1MagHex;
				tempData["j"] = h2MagHex;
				tempData["k"] = h3MagHex;
				tempData["l"] = h8MagHex;
				tempData["m"] = h1PhaseHex;
				tempData["n"] = h2PhaseHex;
				tempData["o"] = h3PhaseHex;
				tempData["p"] = h8PhaseHex;

				addedHarmonicHexValues = true;
			}

			fm.writeToFile(false, tempData);
		}
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
	switch(state) {
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
		extractDataButton->setEnabled(true);
		break;
	case 3: // Post-Calibration
		calibrationStartMotorButton->setEnabled(false);
		calibrateDataButton->setEnabled(true);
		clearCalibrateDataButton->setEnabled(false);
		extractDataButton->setEnabled(false);
		break;
	case 4: // After Post-Calibration
		calibrationStartMotorButton->setEnabled(false);
		calibrateDataButton->setEnabled(false);
		clearCalibrateDataButton->setEnabled(true);
		extractDataButton->setEnabled(true);
		break;
	}
}

void HarmonicCalibration::canStartMotor(bool value) { calibrationStartMotorButton->setEnabled(value); }

void HarmonicCalibration::canCalibrate(bool value) { calibrateDataButton->setEnabled(value); }

void HarmonicCalibration::toggleCalibrationControls(bool value)
{
	calibrationModeMenuCombo->setEnabled(value);
	calibrationCycleCountLineEdit->setEnabled(value);
	calibrationTotalSamplesLineEdit->setEnabled(value);
	calibrationMotorRPMLineEdit->setEnabled(value);
	calibrationMotorDirectionSwitch->setEnabled(value);
	importSamplesButton->setEnabled(value);
	extractDataButton->setEnabled(value);
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
	currentAngleErrorGraphMin = defaultAngleErrorGraphMin;
	currentAngleErrorGraphMax = defaultAngleErrorGraphMax;
	angleErrorYPlotAxis->setInterval(currentAngleErrorGraphMin, currentAngleErrorGraphMax);
	angleErrorPlotChannel->curve()->setData(nullptr);
	angleErrorPlotWidget->replot();
	correctedErrorYPlotAxis->setInterval(currentAngleErrorGraphMin, currentAngleErrorGraphMax);
	correctedErrorPlotChannel->curve()->setData(nullptr);
	correctedErrorPlotWidget->replot();
}

void HarmonicCalibration::clearFFTAngleErrorGraphs()
{
	currentMagnitudeGraphMin = defaultMagnitudeGraphMin;
	currentMagnitudeGraphMax = defaultMagnitudeGraphMax;
	FFTAngleErrorMagnitudeYPlotAxis->setInterval(currentMagnitudeGraphMin, currentMagnitudeGraphMax);
	FFTAngleErrorMagnitudeChannel->curve()->setData(nullptr);
	FFTAngleErrorMagnitudePlotWidget->replot();
	FFTCorrectedErrorMagnitudeYPlotAxis->setInterval(currentMagnitudeGraphMin, currentMagnitudeGraphMax);
	FFTCorrectedErrorMagnitudeChannel->curve()->setData(nullptr);
	FFTCorrectedErrorMagnitudePlotWidget->replot();

	currentPhaseGraphMin = defaultPhaseGraphMin;
	currentPhaseGraphMax = defaultPhaseGraphMax;
	FFTAngleErrorPhaseYPlotAxis->setInterval(currentPhaseGraphMin, currentPhaseGraphMax);
	FFTAngleErrorPhaseChannel->curve()->setData(nullptr);
	FFTAngleErrorPhasePlotWidget->replot();
	FFTCorrectedErrorPhaseYPlotAxis->setInterval(currentPhaseGraphMin, currentPhaseGraphMax);
	FFTCorrectedErrorPhaseChannel->curve()->setData(nullptr);
	FFTCorrectedErrorPhasePlotWidget->replot();
}
#pragma endregion

#pragma region Motor Methods
bool HarmonicCalibration::moveMotorToPosition(double &position, bool validate)
{
	bool success = false;
	bool canRead = true;
	if(writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, position) == 0) {
		if(validate) {
			if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0) {
				while(target_pos != current_pos && canRead) {
					canRead = readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS,
									  current_pos) == 0
						? true
						: false;
				}
				if(canRead)
					success = true;
			}
		}
	}

	return success;
}

void HarmonicCalibration::moveMotorContinuous()
{
	stopResetMotorToZero();
	writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
	setRampMode(isMotorRotationClockwise);
}

bool HarmonicCalibration::resetCurrentPositionToZero()
{
	bool success = false;
	if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0) {
		if(current_pos != 0 && writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, 0) == 0 &&
		   readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) == 0) {
			while(current_pos != 0) {
				if(readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos) !=
				   0)
					break;
				QThread::msleep(readMotorDebounce);
			}
			if(current_pos == 0) {
				resetToZero = false;
				success = true;
			}
		} else {
			success = true;
		}
	}

	return success;
}

void HarmonicCalibration::stopMotor()
{
	stopResetMotorToZero();
	writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
}

int HarmonicCalibration::readMotorAttributeValue(ADMTController::MotorAttribute attribute, double &value)
{
	int result = -1;
	if(!isDebug) {
		result = m_admtController->getDeviceAttributeValue(
			m_admtController->getDeviceId(ADMTController::Device::TMC5240),
			m_admtController->getMotorAttribute(attribute), &value);
	}
	return result;
}

int HarmonicCalibration::writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value)
{
	int result = -1;
	if(!isDebug) {
		result = m_admtController->setDeviceAttributeValue(
			m_admtController->getDeviceId(ADMTController::Device::TMC5240),
			m_admtController->getMotorAttribute(attribute), value);
	}
	return result;
}

int HarmonicCalibration::readMotorRegisterValue(uint32_t address, uint32_t *value)
{
	int result = -1;
	result = m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::TMC5240),
						      address, value);
	return result;
}

void HarmonicCalibration::setRampMode(bool motorRotationClockwise)
{
	// Ramp Mode 1: Counter-Clockwise, Ramp Mode 2: Clockwise
	isMotorRotationClockwise = motorRotationClockwise;
	int mode = !isMotorRotationClockwise ? 1 : 2;
	writeMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, mode);
}

void HarmonicCalibration::getRampMode()
{
	readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);
	// Ramp Mode 1: Counter-Clockwise, Ramp Mode 2: Clockwise
	if(ramp_mode == 1)
		isMotorRotationClockwise = false;
	else if(ramp_mode == 2)
		isMotorRotationClockwise = true;
}
#pragma endregion

#pragma region Utility Methods
void HarmonicCalibration::startUtilityTask()
{
	if(!m_utilityThread.isRunning()) {
		isUtilityTab = true;
		m_utilityThread = QtConcurrent::run(this, &HarmonicCalibration::utilityTask, utilityUITimerRate);
		m_utilityWatcher.setFuture(m_utilityThread);
	}
}

void HarmonicCalibration::stopUtilityTask()
{
	isUtilityTab = false;
	if(m_utilityThread.isRunning()) {
		m_utilityThread.cancel();
		m_utilityWatcher.waitForFinished();
	}
}

void HarmonicCalibration::utilityTask(int sampleRate)
{
	while(isUtilityTab) {
		getDIGIOENRegister();
		getFAULTRegister();
		if(hasMTDiagnostics) {
			getDIAG1Register();
			getDIAG2Register();
		}

		Q_EMIT commandLogWriteSignal("");

		QThread::msleep(sampleRate);
	}
}

void HarmonicCalibration::toggleUtilityTask(bool run)
{
	if(run) {
		startUtilityTask();
	} else {
		stopUtilityTask();
	}
}

void HarmonicCalibration::getDIGIOENRegister()
{
	uint32_t *digioRegValue = new uint32_t;
	uint8_t digioEnPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN);
	if(changeCNVPage(digioEnPage)) {
		uint32_t digioRegisterAddress =
			m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN);

		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
							digioRegisterAddress, digioRegValue) != -1) {
			quint16 digioRegValue16 = static_cast<quint16>(*digioRegValue);

			Q_EMIT DIGIORegisterChanged(digioRegValue16);

			Q_EMIT commandLogWriteSignal("DIGIOEN: 0b" +
						     QString::number(digioRegValue16, 2).rightJustified(16, '0'));
		} else {
			Q_EMIT commandLogWriteSignal("Failed to read DIGIOEN Register");
		}
	}

	delete digioRegValue;
}

void HarmonicCalibration::updateDIGIOUI(quint16 registerValue)
{
	DIGIOENRegisterMap = QMap(m_admtController->getDIGIOENRegisterBitMapping(registerValue));
	updateDIGIOMonitorUI();
	updateDIGIOControlUI();
}

void HarmonicCalibration::updateDIGIOMonitorUI()
{
	QMap<string, bool> registerMap = DIGIOENRegisterMap;
	DIGIOBusyStatusLED->setChecked(registerMap.value("BUSY"));
	DIGIOCNVStatusLED->setChecked(registerMap.value("CNV"));
	DIGIOSENTStatusLED->setChecked(registerMap.value("SENT"));
	DIGIOACALCStatusLED->setChecked(registerMap.value("ACALC"));
	DIGIOFaultStatusLED->setChecked(registerMap.value("FAULT"));
	DIGIOBootloaderStatusLED->setChecked(registerMap.value("BOOTLOAD"));

	if(!registerMap.value("BUSY"))
		DIGIOBusyStatusLED->setText("BUSY (Output)");
	else {
		if(registerMap.value("DIGIO0EN"))
			DIGIOBusyStatusLED->setText("GPIO0 (Output)");
		else
			DIGIOBusyStatusLED->setText("GPIO0 (Input)");
	}

	if(!registerMap.value("CNV"))
		DIGIOCNVStatusLED->setText("CNV (Input)");
	else {
		if(registerMap.value("DIGIO1EN"))
			DIGIOCNVStatusLED->setText("GPIO1 (Output)");
		else
			DIGIOCNVStatusLED->setText("GPIO1 (Input)");
	}

	if(!registerMap.value("SENT"))
		DIGIOSENTStatusLED->setText("SENT (Output)");
	else {
		if(registerMap.value("DIGIO2EN"))
			DIGIOSENTStatusLED->setText("GPIO2 (Output)");
		else
			DIGIOSENTStatusLED->setText("GPIO2 (Input)");
	}

	if(!registerMap.value("ACALC"))
		DIGIOACALCStatusLED->setText("ACALC (Output)");
	else {
		if(registerMap.value("DIGIO3EN"))
			DIGIOACALCStatusLED->setText("GPIO3 (Output)");
		else
			DIGIOACALCStatusLED->setText("GPIO3 (Input)");
	}

	if(!registerMap.value("FAULT"))
		DIGIOFaultStatusLED->setText("FAULT (Output)");
	else {
		if(registerMap.value("DIGIO4EN"))
			DIGIOFaultStatusLED->setText("GPIO4 (Output)");
		else
			DIGIOFaultStatusLED->setText("GPIO4 (Input)");
	}

	if(!registerMap.value("BOOTLOAD"))
		DIGIOBootloaderStatusLED->setText("BOOTLOAD (Output)");
	else {
		if(registerMap.value("DIGIO5EN"))
			DIGIOBootloaderStatusLED->setText("GPIO5 (Output)");
		else
			DIGIOBootloaderStatusLED->setText("GPIO5 (Input)");
	}
}

void HarmonicCalibration::updateDIGIOControlUI()
{
	QMap<string, bool> registerMap = DIGIOENRegisterMap;

	DIGIO0ENToggleSwitch->setChecked(registerMap.value("DIGIO0EN"));
	DIGIO1ENToggleSwitch->setChecked(registerMap.value("DIGIO1EN"));
	DIGIO2ENToggleSwitch->setChecked(registerMap.value("DIGIO2EN"));
	DIGIO3ENToggleSwitch->setChecked(registerMap.value("DIGIO3EN"));
	DIGIO4ENToggleSwitch->setChecked(registerMap.value("DIGIO4EN"));
	DIGIO5ENToggleSwitch->setChecked(registerMap.value("DIGIO5EN"));
	DIGIO0FNCToggleSwitch->setChecked(registerMap.value("BUSY"));
	DIGIO1FNCToggleSwitch->setChecked(registerMap.value("CNV"));
	DIGIO2FNCToggleSwitch->setChecked(registerMap.value("SENT"));
	DIGIO3FNCToggleSwitch->setChecked(registerMap.value("ACALC"));
	DIGIO4FNCToggleSwitch->setChecked(registerMap.value("FAULT"));
	DIGIO5FNCToggleSwitch->setChecked(registerMap.value("BOOTLOAD"));
}

void HarmonicCalibration::getDIAG2Register()
{
	uint32_t *mtDiag2RegValue = new uint32_t;
	uint32_t mtDiag2RegisterAddress = m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG2);
	uint32_t mtDiag2PageValue = m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG2);
	uint32_t cnvPageAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						 cnvPageAddress, mtDiag2PageValue) != -1) {
		uint32_t *cnvPageRegValue = new uint32_t;
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
							cnvPageAddress, cnvPageRegValue) != -1) {
			if(*cnvPageRegValue == mtDiag2PageValue) {
				if(m_admtController->readDeviceRegistry(
					   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
					   mtDiag2RegisterAddress, mtDiag2RegValue) != -1) {
					quint16 mtDiag2RegValue16 = static_cast<quint16>(*mtDiag2RegValue);

					Q_EMIT DIAG2RegisterChanged(mtDiag2RegValue16);

					Q_EMIT commandLogWriteSignal(
						"DIAG2: 0b" +
						QString::number(mtDiag2RegValue16, 2).rightJustified(16, '0'));
				} else {
					Q_EMIT commandLogWriteSignal("Failed to read MT Diagnostic 2 Register");
				}
			} else {
				Q_EMIT commandLogWriteSignal(
					"CNVPAGE for MT Diagnostic 2 is a different value, abort reading");
			}
		} else {
			Q_EMIT commandLogWriteSignal("Failed to read CNVPAGE for MT Diagnostic 2");
		}

		delete cnvPageRegValue;
	} else {
		Q_EMIT commandLogWriteSignal("Failed to write CNVPAGE for MT Diagnostic 2");
	}

	delete mtDiag2RegValue;
}

void HarmonicCalibration::updateMTDiagnosticsUI(quint16 registerValue)
{
	DIAG2RegisterMap = QMap(m_admtController->getDiag2RegisterBitMapping(registerValue));

	QMap<string, double> regmap = DIAG2RegisterMap;
	afeDiag0 = regmap.value("AFE Diagnostic 0 (-57%)");
	afeDiag1 = regmap.value("AFE Diagnostic 1 (+57%)");
	AFEDIAG0LineEdit->setText(QString::number(afeDiag0) + " V");
	AFEDIAG1LineEdit->setText(QString::number(afeDiag1) + " V");
}

void HarmonicCalibration::getDIAG1Register()
{
	uint32_t *mtDiag1RegValue = new uint32_t;

	uint32_t mtDiag1RegisterAddress = m_admtController->getSensorRegister(ADMTController::SensorRegister::DIAG1);
	uint32_t mtDiag1PageValue = m_admtController->getSensorPage(ADMTController::SensorRegister::DIAG1);
	uint32_t cnvPageAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::CNVPAGE);

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						 cnvPageAddress, mtDiag1PageValue) != -1) {
		uint32_t *cnvPageRegValue = new uint32_t;
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
							cnvPageAddress, cnvPageRegValue) != -1) {
			if(*cnvPageRegValue == mtDiag1PageValue) {
				if(m_admtController->readDeviceRegistry(
					   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
					   mtDiag1RegisterAddress, mtDiag1RegValue) != -1) {
					quint16 mtDiag1RegValue16 = static_cast<quint16>(*mtDiag1RegValue);
					Q_EMIT DIAG1RegisterChanged(mtDiag1RegValue16);

					Q_EMIT commandLogWriteSignal(
						"DIAG1: 0b" +
						QString::number(mtDiag1RegValue16, 2).rightJustified(16, '0'));
				} else {
					Q_EMIT commandLogWriteSignal("Failed to read MT Diagnostic 1 Register");
				}
			} else {
				Q_EMIT commandLogWriteSignal(
					"CNVPAGE for MT Diagnostic 1 is a different value, abort reading");
			}
		} else {
			Q_EMIT commandLogWriteSignal("Failed to read CNVPAGE for MT Diagnostic 1");
		}

		delete cnvPageRegValue;
	} else {
		Q_EMIT commandLogWriteSignal("Failed to write CNVPAGE for MT Diagnostic 1");
	}

	delete mtDiag1RegValue;
}

void HarmonicCalibration::updateMTDiagnosticRegisterUI(quint16 registerValue)
{
	DIAG1RegisterMap = QMap(m_admtController->getDiag1RegisterBitMapping_Register(registerValue));
	DIAG1AFERegisterMap = QMap(m_admtController->getDiag1RegisterBitMapping_Afe(registerValue, is5V));

	QMap<string, bool> regmap = DIAG1RegisterMap;
	QMap<string, double> afeRegmap = DIAG1AFERegisterMap;
	R0StatusLED->setChecked(regmap.value("R0"));
	R1StatusLED->setChecked(regmap.value("R1"));
	R2StatusLED->setChecked(regmap.value("R2"));
	R3StatusLED->setChecked(regmap.value("R3"));
	R4StatusLED->setChecked(regmap.value("R4"));
	R5StatusLED->setChecked(regmap.value("R5"));
	R6StatusLED->setChecked(regmap.value("R6"));
	R7StatusLED->setChecked(regmap.value("R7"));
	afeDiag2 = afeRegmap.value("AFE Diagnostic 2");
	AFEDIAG2LineEdit->setText(QString::number(afeDiag2) + " V");
}

void HarmonicCalibration::getFAULTRegister()
{
	uint32_t *faultRegValue = new uint32_t;
	uint32_t faultRegisterAddress =
		m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::FAULT);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
					      faultRegisterAddress, 0); // Write all zeros to fault before read

	if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						faultRegisterAddress, faultRegValue) != -1) {
		quint16 faultRegValue16 = static_cast<quint16>(*faultRegValue);

		Q_EMIT FaultRegisterChanged(faultRegValue16);

		Q_EMIT commandLogWriteSignal("FAULT: 0b" + QString::number(faultRegValue16, 2).rightJustified(16, '0'));
	} else {
		Q_EMIT commandLogWriteSignal("Failed to read FAULT Register");
	}

	delete faultRegValue;
}

void HarmonicCalibration::updateFaultRegisterUI(quint16 faultRegValue)
{
	FAULTRegisterMap = QMap(m_admtController->getFaultRegisterBitMapping(faultRegValue));

	QMap<string, bool> regmap = FAULTRegisterMap;
	VDDUnderVoltageStatusLED->setChecked(regmap.value("VDD Under Voltage"));
	VDDOverVoltageStatusLED->setChecked(regmap.value("VDD Over Voltage"));
	VDRIVEUnderVoltageStatusLED->setChecked(regmap.value("VDRIVE Under Voltage"));
	VDRIVEOverVoltageStatusLED->setChecked(regmap.value("VDRIVE Over Voltage"));
	AFEDIAGStatusLED->setChecked(regmap.value("AFE Diagnostic"));
	NVMCRCFaultStatusLED->setChecked(regmap.value("NVM CRC Fault"));
	ECCDoubleBitErrorStatusLED->setChecked(regmap.value("ECC Double Bit Error"));
	OscillatorDriftStatusLED->setChecked(regmap.value("Oscillator Drift"));
	CountSensorFalseStateStatusLED->setChecked(regmap.value("Count Sensor False State"));
	AngleCrossCheckStatusLED->setChecked(regmap.value("Angle Cross Check"));
	TurnCountSensorLevelsStatusLED->setChecked(regmap.value("Turn Count Sensor Levels"));
	MTDIAGStatusLED->setChecked(regmap.value("MT Diagnostic"));
	TurnCounterCrossCheckStatusLED->setChecked(regmap.value("Turn Counter Cross Check"));
	RadiusCheckStatusLED->setChecked(regmap.value("AMR Radius Check"));
	SequencerWatchdogStatusLED->setChecked(regmap.value("Sequencer Watchdog"));
}

bool HarmonicCalibration::toggleDIGIOEN(string DIGIOENName, bool value)
{
	toggleUtilityTask(false);

	bool success = false;

	uint8_t DIGIOENPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN);

	if(!changeCNVPage(DIGIOENPage))
		return false;

	uint32_t *DIGIOENRegisterValue = new uint32_t;

	if(m_admtController->readDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN),
		   DIGIOENRegisterValue) != 0) {
		delete DIGIOENRegisterValue;
		return false;
	}

	map<string, bool> DIGIOSettings =
		m_admtController->getDIGIOENRegisterBitMapping(static_cast<uint16_t>(*DIGIOENRegisterValue));

	DIGIOSettings.at(DIGIOENName) = value;

	uint16_t newRegisterValue = m_admtController->setDIGIOENRegisterBitMapping(
		static_cast<uint16_t>(*DIGIOENRegisterValue), DIGIOSettings);

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN),
		   static_cast<uint32_t>(newRegisterValue)) == 0) {
		success = true;
	} else
		return false;

	updateDIGIOControlUI();

	toggleUtilityTask(true);

	delete DIGIOENRegisterValue;
	return success;
}

void HarmonicCalibration::toggleMTDiagnostics(int mode)
{
	switch(mode) {
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
	switch(mode) {
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
	bool success = false;

	uint8_t DIGIOENPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::DIGIOEN);

	if(!changeCNVPage(DIGIOENPage))
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::DIGIOEN),
		   0x241b) == 0) { // Reset value is 0x241b
		success = true;
	}

	return success;
}

bool HarmonicCalibration::disableECC(bool disable)
{
	bool success = false;

	uint32_t ECCDISRegValue = disable ? 0x4D54 : 0x0000;
	uint8_t ECCDISCNVPage = m_admtController->getConfigurationPage(ADMTController::ConfigurationRegister::ECCDIS);

	if(!changeCNVPage(ECCDISCNVPage))
		return false;

	if(m_admtController->writeDeviceRegistry(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getConfigurationRegister(ADMTController::ConfigurationRegister::ECCDIS),
		   ECCDISRegValue) == 0) {
		success = true;
	}

	return success;
}

void HarmonicCalibration::commandLogWrite(QString message) { commandLogPlainTextEdit->appendPlainText(message); }

void HarmonicCalibration::clearCommandLog() { commandLogPlainTextEdit->clear(); }
#pragma endregion

#pragma region Register Methods
void HarmonicCalibration::readAllRegisters()
{
	readAllRegistersButton->setEnabled(false);
	readAllRegistersButton->setText(QString("Reading Registers..."));
	QTimer::singleShot(1000, this, [this]() {
		readAllRegistersButton->setEnabled(true);
		readAllRegistersButton->setText(QString("Read All Registers"));
	});

	// TODO: Change to a more efficient way to read all registers
	cnvPageRegisterBlock->readButton()->click();
	digIORegisterBlock->readButton()->click();
	faultRegisterBlock->readButton()->click();
	generalRegisterBlock->readButton()->click();
	digIOEnRegisterBlock->readButton()->click();
	eccCdeRegisterBlock->readButton()->click();
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

	if(sequenceMode == 1) {
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
	switch(mode) {
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
bool HarmonicCalibration::updateChannelValue(int channelIndex)
{
	bool success = false;
	switch(channelIndex) {
	case ADMTController::Channel::ROTATION:
		double tempRotation;
		tempRotation = m_admtController->getChannelValue(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000), rotationChannelName,
			bufferSize);

		if(tempRotation == UINT32_MAX) {
			success = false;
			break;
		}

		rotation = getABSAngle(tempRotation);
		count = getTurnCount(tempRotation);
		break;
	case ADMTController::Channel::ANGL:
		angle = m_admtController->getChannelValue(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000), angleChannelName, bufferSize);
		if(angle == UINT32_MAX) {
			success = false;
		}
		break;
	case ADMTController::Channel::COUNT:
		count = m_admtController->getChannelValue(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000), countChannelName, bufferSize);
		if(count == UINT32_MAX) {
			success = false;
		}
		break;
	case ADMTController::Channel::TEMPERATURE:
		temp = m_admtController->getChannelValue(
			m_admtController->getDeviceId(ADMTController::Device::ADMT4000), temperatureChannelName,
			bufferSize);
		if(temp == UINT32_MAX) {
			success = false;
		}
		break;
	}
	return success;
}

void HarmonicCalibration::updateLineEditValue(QLineEdit *lineEdit, double value)
{
	if(value == UINT32_MAX) {
		lineEdit->setText("N/A");
	} else {
		lineEdit->setText(QString::number(value));
	}
}

void HarmonicCalibration::toggleWidget(QPushButton *widget, bool value) { widget->setEnabled(value); }

void HarmonicCalibration::changeCustomSwitchLabel(CustomSwitch *customSwitch, QString onLabel, QString offLabel)
{
	customSwitch->setOnText(onLabel);
	customSwitch->setOffText(offLabel);
}

QCheckBox *HarmonicCalibration::createStatusLEDWidget(const QString &text, QVariant variant, bool checked,
						      QWidget *parent)
{
	QCheckBox *checkBox = new QCheckBox(text, parent);
	Style::setStyle(checkBox, style::properties::admt::checkBoxLED, variant, true);
	checkBox->setChecked(checked);
	checkBox->setEnabled(false);
	return checkBox;
}

void HarmonicCalibration::configureCoeffRow(QWidget *container, QHBoxLayout *layout, QLabel *hLabel, QLabel *hMagLabel,
					    QLabel *hPhaseLabel)
{
	container->setLayout(layout);
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	layout->setContentsMargins(12, 4, 12, 4);
	hLabel->setProperty("hLabel", true);
	hMagLabel->setProperty("hMagLabel", true);
	hPhaseLabel->setProperty("hPhaseLabel", true);
	hLabel->setFixedWidth(24);
	hMagLabel->setContentsMargins(0, 0, 32, 0);
	hPhaseLabel->setFixedWidth(72);
	layout->addWidget(hLabel);
	layout->addWidget(hMagLabel, 0, Qt::AlignRight);
	layout->addWidget(hPhaseLabel);
	Style::setStyle(container, style::properties::admt::coeffRowContainer, true, true);
}

void HarmonicCalibration::initializeChannelColors()
{
	channel0Pen = QPen(StyleHelper::getChannelColor(0));
	channel1Pen = QPen(StyleHelper::getChannelColor(1));
	channel2Pen = QPen(StyleHelper::getChannelColor(2));
	channel3Pen = QPen(StyleHelper::getChannelColor(3));
	channel4Pen = QPen(StyleHelper::getChannelColor(4));
	channel5Pen = QPen(StyleHelper::getChannelColor(5));
	channel6Pen = QPen(StyleHelper::getChannelColor(6));
	channel7Pen = QPen(StyleHelper::getChannelColor(7));
}
#pragma endregion

#pragma region Connect Methods
void HarmonicCalibration::connectLineEditToNumber(QLineEdit *lineEdit, int &variable, int min, int max)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit, min, max]() {
		bool ok;
		int value = lineEdit->text().toInt(&ok);
		if(ok && value >= min && value <= max) {
			variable = value;
		} else {
			lineEdit->setText(QString::number(variable));
		}
	});
}

void HarmonicCalibration::connectLineEditToNumber(QLineEdit *lineEdit, double &variable, QString unit)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit, unit]() {
		bool ok;
		double value = lineEdit->text().replace(unit, "").trimmed().toDouble(&ok);
		if(ok) {
			variable = value;

		} else {
			lineEdit->setText(QString::number(variable) + " " + unit);
		}
	});
}

void HarmonicCalibration::connectLineEditToDouble(QLineEdit *lineEdit, double &variable)
{
	QDoubleValidator *validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	lineEdit->setValidator(validator);
	connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit]() {
		bool ok;
		double value = lineEdit->text().toDouble(&ok);
		if(ok) {
			variable = value;
		} else {
			lineEdit->setText(QString::number(variable));
		}
	});
}

void HarmonicCalibration::connectLineEditToNumberWrite(QLineEdit *lineEdit, double &variable,
						       ADMTController::MotorAttribute attribute)
{
	QDoubleValidator *validator = new QDoubleValidator(this);
	validator->setNotation(QDoubleValidator::StandardNotation);
	lineEdit->setValidator(validator);
	connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, attribute, &variable]() {
		bool ok;
		double value = lineEdit->text().toDouble(&ok);
		if(ok) {
			variable = value;
			writeMotorAttributeValue(attribute, variable);

		} else {
			lineEdit->setText(QString::number(variable));
		}
	});
}

void HarmonicCalibration::connectLineEditToMotorTurnCount(QLineEdit *lineEdit, int &variable, int min, int max)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, &variable, min, max]() {
		bool ok;
		int value = lineEdit->text().toInt(&ok);
		if(ok && value >= min && value <= max && value != 0) {
			readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
			int target_pos = value * motorMicrostepPerRevolution;
			if(isMotorRotationClockwise)
				target_pos *= -1;

			int final_pos = current_pos + target_pos;
			writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, final_pos);
			variable = 0;
		} else
			variable = 0;
		lineEdit->setText(QString::number(variable));
	});
}

void HarmonicCalibration::connectMenuComboToNumber(MenuCombo *menuCombo, double &variable)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this, &combo, &variable]() { variable = qvariant_cast<int>(combo->currentData()); });
}

void HarmonicCalibration::connectMenuComboToNumber(MenuCombo *menuCombo, int &variable)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this, combo, &variable]() { variable = qvariant_cast<int>(combo->currentData()); });
}

void HarmonicCalibration::connectLineEditToRPSConversion(QLineEdit *lineEdit, double &vmax)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, &vmax]() {
		bool ok;
		double rps = lineEdit->text().toDouble(&ok);
		if(ok) {
			vmax = convertRPStoVMAX(rps);
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, vmax);
			writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
			amax = convertAccelTimetoAMAX(motorAccelTime);
			writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
			// StatusBarManager::pushMessage("Applied VMAX: " +
			// QString::number(vmax)); StatusBarManager::pushMessage("Applied AMAX: "
			// + QString::number(amax));
		} else {
			lineEdit->setText(QString::number(convertVMAXtoRPS(vmax)));
		}
	});
}

void HarmonicCalibration::connectLineEditToAMAXConversion(QLineEdit *lineEdit, double &amax)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, &amax]() {
		bool ok;
		double accelTime = lineEdit->text().toDouble(&ok);
		if(ok) {
			amax = convertAccelTimetoAMAX(accelTime);
			// StatusBarManager::pushMessage("Applied AMAX: " +
			// QString::number(amax));
		} else {
			lineEdit->setText(QString::number(convertAMAXtoAccelTime(amax)));
		}
	});
}

void HarmonicCalibration::connectRegisterBlockToRegistry(RegisterBlockWidget *widget)
{
	uint32_t *readValue = new uint32_t;
	connect(widget->readButton(), &QPushButton::clicked, this, [this, widget, readValue] {
		bool ok = false, success = false;

		if(widget->getCnvPage() != UINT8_MAX) {
			ok = this->changeCNVPage(widget->getCnvPage());
		} else {
			ok = true;
		}

		if(ok) {
			if(m_admtController->readDeviceRegistry(
				   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
				   widget->getAddress(), readValue) == 0) {
				widget->setValue(*readValue);
			}
		} else {
			StatusBarManager::pushMessage("Failed to read registry");
		}
	});
	if(widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::READWRITE ||
	   widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::WRITE) {
		connect(widget->writeButton(), &QPushButton::clicked, this, [this, widget, readValue] {
			bool ok = false, success = false;

			if(widget->getCnvPage() != UINT8_MAX) {
				ok = this->changeCNVPage(widget->getCnvPage());
			} else {
				ok = true;
			}

			if(ok) {
				if(m_admtController->writeDeviceRegistry(
					   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
					   widget->getAddress(), widget->getValue()) == 0) {
					if(m_admtController->readDeviceRegistry(
						   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
						   widget->getAddress(), readValue) == 0) {
						widget->setValue(*readValue);
						success = true;
					}
				}
			}

			if(!success) {
				StatusBarManager::pushMessage("Failed to write to registry");
			}
		});
	}
}

void HarmonicCalibration::connectLineEditToRPM(QLineEdit *lineEdit, double &variable, double min, double max)
{
	connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, min, max, &variable]() {
		bool ok;
		double value = lineEdit->text().toDouble(&ok);
		if(ok && variable != value && value >= min && value <= max) {
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
double HarmonicCalibration::convertRPStoVMAX(double rps) { return (rps * motorMicrostepPerRevolution * motorTimeUnit); }

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

double HarmonicCalibration::convertRPMtoRPS(double rpm) { return (rpm / 60); }

double HarmonicCalibration::convertRPStoRPM(double rps) { return (rps * 60); }
#pragma endregion

#pragma region Debug Methods
QString HarmonicCalibration::readRegmapDumpAttributeValue()
{
	QString output = "";
	char value[1024];
	if(m_admtController->getDeviceAttributeValueString(
		   m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		   m_admtController->getDeviceAttribute(ADMTController::DeviceAttribute::REGMAP_DUMP), value,
		   1024) == 0)
		output = QString(value);
	return output;
}
#pragma endregion
#include "moc_harmoniccalibration.cpp"
