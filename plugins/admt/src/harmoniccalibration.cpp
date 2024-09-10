#include "harmoniccalibration.h"
#include <widgets/horizontalspinbox.h>

#include <stylehelper.h>

static int sampleRate = 50;
static int calibrationTimerRate = 100;
static int motorCalibrationAcquisitionTimerRate = 20;
static int bufferSize = 1;
static int dataGraphSamples = 100;
static int tempGraphSamples = 100;
static bool running = false;
static double *dataGraphValue;

static int cycleCount = 11;
static int samplesPerCycle = 256;
static int totalSamplesCount = cycleCount * samplesPerCycle;
static bool startMotor = false;

static bool isDebug = true;
static bool isCalibrated = false;

static double motorTimeUnit = 1.048576; // t = 2^24/16Mhz
static int motorMicrostepPerRevolution = 51200;
static int motorfCLK = 16000000; // 16Mhz

static bool autoCalibrate = false;

static uint32_t h1MagDeviceRegister = 0x15;
static uint32_t h2MagDeviceRegister = 0x17;
static uint32_t h3MagDeviceRegister = 0x19;
static uint32_t h8MagDeviceRegister = 0x1B;
static uint32_t h1PhaseDeviceRegister = 0x16;
static uint32_t h2PhaseDeviceRegister = 0x18;
static uint32_t h3PhaseDeviceRegister = 0x1A;
static uint32_t h8PhaseDeviceRegister = 0x1C;

static vector<double> rawDataList;

static const QColor sineColor = QColor("#85e94c");
static const QColor cosineColor = QColor("#91e6cf");
static const QColor faultLEDColor = QColor("#c81a28");
static const QColor statusLEDColor = QColor("#2e9e6f");

using namespace scopy;
using namespace scopy::admt;
using namespace scopy::grutil;

HarmonicCalibration::HarmonicCalibration(ADMTController *m_admtController, QWidget *parent)
	: QWidget(parent)
	, m_admtController(m_admtController)
{
	rotationChannelName = m_admtController->getChannelId(ADMTController::Channel::ROTATION);
	angleChannelName = m_admtController->getChannelId(ADMTController::Channel::ANGLE);
	countChannelName = m_admtController->getChannelId(ADMTController::Channel::COUNT);
	temperatureChannelName = m_admtController->getChannelId(ADMTController::Channel::TEMPERATURE);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *lay = new QHBoxLayout(this);
    tool = new ToolTemplate(this);
	setLayout(lay);
    lay->setMargin(0);
	tabWidget = new QTabWidget(this);
	tabWidget->setObjectName("HarmonicTabWidget");
	QString tabWidgetStyle = QString(R"css(
						QTabWidget::tab-bar { left: 240px; }
						)css");
	tabWidget->tabBar()->setStyleSheet(tabWidgetStyle);
	tabWidget->addTab(tool, "Acquisition");

    openLastMenuButton = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuButtonGroup = dynamic_cast<OpenLastMenuBtn *>(openLastMenuButton)->getButtonGroup();

    settingsButton = new GearBtn(this);
	lay->insertWidget(1, tabWidget);

    runButton = new RunBtn(this);


	rightMenuButtonGroup->addButton(settingsButton);

	// Raw Data Widget
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
	rotationWidget->contentLayout()->setSpacing(8);
	angleWidget->contentLayout()->setSpacing(8);
	countWidget->contentLayout()->setSpacing(8);
	tempWidget->contentLayout()->setSpacing(8);
	MenuCollapseSection *rotationSection = new MenuCollapseSection("Rotation", MenuCollapseSection::MHCW_NONE, rotationWidget);
	MenuCollapseSection *angleSection = new MenuCollapseSection("Angle", MenuCollapseSection::MHCW_NONE, angleWidget);
	MenuCollapseSection *countSection = new MenuCollapseSection("Count", MenuCollapseSection::MHCW_NONE, countWidget);
	MenuCollapseSection *tempSection = new MenuCollapseSection("Temperature", MenuCollapseSection::MHCW_NONE, tempWidget);
	rotationSection->contentLayout()->setSpacing(8);
	angleSection->contentLayout()->setSpacing(8);
	countSection->contentLayout()->setSpacing(8);
	tempSection->contentLayout()->setSpacing(8);

	rotationWidget->contentLayout()->addWidget(rotationSection);
	angleWidget->contentLayout()->addWidget(angleSection);
	countWidget->contentLayout()->addWidget(countSection);
	tempWidget->contentLayout()->addWidget(tempSection);

	rawDataLayout->addWidget(rotationWidget);
	rawDataLayout->addWidget(angleWidget);
	rawDataLayout->addWidget(countWidget);
	rawDataLayout->addWidget(tempWidget);
	rawDataLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	rotationValueLabel = new QLabel(rotationSection);
	StyleHelper::MenuControlLabel(rotationValueLabel, "rotationValueLabel");
	angleValueLabel = new QLabel(angleSection);
	StyleHelper::MenuControlLabel(angleValueLabel, "angleValueLabel");
	countValueLabel = new QLabel(countSection);
	StyleHelper::MenuControlLabel(countValueLabel, "countValueLabel");
	tempValueLabel = new QLabel(tempSection);
	StyleHelper::MenuControlLabel(tempValueLabel, "tempValueLabel");
	
	rotationValueLabel->setText("--.--°");
	angleValueLabel->setText("--.--°");
	countValueLabel->setText("--");
	tempValueLabel->setText("--.-- °C");

	rotationSection->contentLayout()->addWidget(rotationValueLabel);
	angleSection->contentLayout()->addWidget(angleValueLabel);
	countSection->contentLayout()->addWidget(countValueLabel);
	tempSection->contentLayout()->addWidget(tempValueLabel);

	QWidget *historicalGraphWidget = new QWidget();
	QVBoxLayout *historicalGraphLayout = new QVBoxLayout(this);

	QLabel *dataGraphLabel = new QLabel(historicalGraphWidget);
	dataGraphLabel->setText("Phase");
	StyleHelper::MenuSmallLabel(dataGraphLabel, "dataGraphLabel");

	dataGraph = new Sismograph(this);
	changeGraphColorByChannelName(dataGraph, rotationChannelName);
	dataGraph->setPlotAxisXTitle("Degree (°)");
	dataGraph->setUnitOfMeasure("Degree", "°");
	dataGraph->setAutoscale(false);
	dataGraph->setAxisScale(QwtAxis::YLeft, -30.0, 390.0);
	//dataGraph->setNumSamples(dataGraphSamples);
	dataGraph->setHistoryDuration(10.0);
	dataGraphValue = &rotation;

	QLabel *tempGraphLabel = new QLabel(historicalGraphWidget);
	tempGraphLabel->setText("Temperature");
	StyleHelper::MenuSmallLabel(tempGraphLabel, "tempGraphLabel");

	tempGraph = new Sismograph(this);
	changeGraphColorByChannelName(tempGraph, temperatureChannelName);
	tempGraph->setPlotAxisXTitle("Celsius (°C)");
	tempGraph->setUnitOfMeasure("Celsius", "°C");
	tempGraph->setAutoscale(false);
	tempGraph->addScale(0.0, 100.0, 25, 5);
	tempGraph->setNumSamples(tempGraphSamples);

	historicalGraphLayout->addWidget(dataGraphLabel);
	historicalGraphLayout->addWidget(dataGraph);
	historicalGraphLayout->addWidget(tempGraphLabel);
	historicalGraphLayout->addWidget(tempGraph);

	historicalGraphWidget->setLayout(historicalGraphLayout);

	// General Setting
	QScrollArea *generalSettingScroll = new QScrollArea(this);
	generalSettingScroll->setWidgetResizable(true);
	QWidget *generalSettingWidget = new QWidget(generalSettingScroll);
	generalSettingScroll->setWidget(generalSettingWidget);
	QVBoxLayout *generalSettingLayout = new QVBoxLayout(generalSettingWidget);
	generalSettingLayout->setMargin(0);
	generalSettingWidget->setLayout(generalSettingLayout);

	header = new MenuHeaderWidget("ADMT4000", QPen(StyleHelper::getColor("ScopyBlue")), this);

	// General Setting Widget
	MenuSectionWidget *generalWidget = new MenuSectionWidget(generalSettingWidget);
	generalWidget->contentLayout()->setSpacing(8);
	MenuCollapseSection *generalSection = new MenuCollapseSection("Data Acquisition", MenuCollapseSection::MHCW_NONE, generalWidget);
	generalSection->contentLayout()->setSpacing(8);
	generalWidget->contentLayout()->addWidget(generalSection);

	// Graph Update Interval
	QLabel *graphUpdateIntervalLabel = new QLabel(generalSection);
	graphUpdateIntervalLabel->setText("Graph Update Interval (ms)");
	StyleHelper::MenuSmallLabel(graphUpdateIntervalLabel, "graphUpdateIntervalLabel");
	graphUpdateIntervalLineEdit = new QLineEdit(generalSection);
	applyLineEditStyle(graphUpdateIntervalLineEdit);
	graphUpdateIntervalLineEdit->setText(QString::number(sampleRate));

	connectLineEditToNumber(graphUpdateIntervalLineEdit, sampleRate);

	generalSection->contentLayout()->addWidget(graphUpdateIntervalLabel);
	generalSection->contentLayout()->addWidget(graphUpdateIntervalLineEdit);

	// Data Sample Size
	QLabel *dataSampleSizeLabel = new QLabel(generalSection);
	dataSampleSizeLabel->setText("Data Sample Size");
	StyleHelper::MenuSmallLabel(dataSampleSizeLabel, "dataSampleSizeLabel");
	dataSampleSizeLineEdit = new QLineEdit(generalSection);
	applyLineEditStyle(dataSampleSizeLineEdit);
	dataSampleSizeLineEdit->setText(QString::number(bufferSize));

	connectLineEditToNumber(dataSampleSizeLineEdit, bufferSize);

	generalSection->contentLayout()->addWidget(dataSampleSizeLabel);
	generalSection->contentLayout()->addWidget(dataSampleSizeLineEdit);

	MenuSectionWidget *sequenceWidget = new MenuSectionWidget(generalSettingWidget);
	MenuCollapseSection *sequenceSection = new MenuCollapseSection("Sequence", MenuCollapseSection::MHCW_NONE, sequenceWidget);
	sequenceWidget->contentLayout()->addWidget(sequenceSection);
	sequenceSection->contentLayout()->setSpacing(8);

	MenuCombo *sequenceTypeMenuCombo = new MenuCombo("Sequence Type", sequenceSection);
	QComboBox *sequenceTypeComboBox = sequenceTypeMenuCombo->combo();
	sequenceTypeComboBox->addItem("Mode 1", QVariant(0));
	sequenceTypeComboBox->addItem("Mode 2", QVariant(1));
	sequenceTypeComboBox->setCurrentIndex(1);
	applyComboBoxStyle(sequenceTypeComboBox);

	MenuCombo *conversionTypeMenuCombo = new MenuCombo("Conversion Type", sequenceSection);
	QComboBox *conversionTypeComboBox = conversionTypeMenuCombo->combo();
	conversionTypeComboBox->addItem("Continuous", QVariant(0));
	conversionTypeComboBox->addItem("One Shot", QVariant(1));
	applyComboBoxStyle(conversionTypeComboBox);

	MenuCombo *cnvSourceMenuCombo = new MenuCombo("CNV Source", sequenceSection);
	QComboBox *cnvSourceComboBox = cnvSourceMenuCombo->combo();
	cnvSourceComboBox->addItem("External", QVariant(0));
	cnvSourceComboBox->addItem("Software", QVariant(1));
	applyComboBoxStyle(cnvSourceComboBox);

	MenuCombo *convertSynchronizationMenuCombo = new MenuCombo("Convert Synchronization", sequenceSection);
	QComboBox *convertSynchronizationComboBox = convertSynchronizationMenuCombo->combo();
	convertSynchronizationComboBox->addItem("Enabled", QVariant(0));
	convertSynchronizationComboBox->addItem("Disabled", QVariant(1));
	convertSynchronizationComboBox->setCurrentIndex(1);
	applyComboBoxStyle(convertSynchronizationComboBox);

	MenuCombo *angleFilterMenuCombo = new MenuCombo("Angle Filter", sequenceSection);
	QComboBox *angleFilterComboBox = angleFilterMenuCombo->combo();
	angleFilterComboBox->addItem("Enabled", QVariant(0));
	angleFilterComboBox->addItem("Disabled", QVariant(1));
	angleFilterComboBox->setCurrentIndex(1);
	applyComboBoxStyle(angleFilterComboBox);

	MenuCombo *eighthHarmonicMenuCombo = new MenuCombo("8th Harmonic", sequenceSection);
	QComboBox *eighthHarmonicComboBox = eighthHarmonicMenuCombo->combo();
	eighthHarmonicComboBox->addItem("Factory Set", QVariant(0));
	eighthHarmonicComboBox->addItem("User", QVariant(1));
	applyComboBoxStyle(eighthHarmonicComboBox);

	sequenceSection->contentLayout()->addWidget(sequenceTypeMenuCombo);
	sequenceSection->contentLayout()->addWidget(conversionTypeMenuCombo);
	sequenceSection->contentLayout()->addWidget(cnvSourceMenuCombo);
	sequenceSection->contentLayout()->addWidget(convertSynchronizationMenuCombo);
	sequenceSection->contentLayout()->addWidget(angleFilterMenuCombo);
	sequenceSection->contentLayout()->addWidget(eighthHarmonicMenuCombo);

	// Data Graph Setting Widget
	MenuSectionWidget *dataGraphWidget = new MenuSectionWidget(generalSettingWidget);
	dataGraphWidget->contentLayout()->setSpacing(8);
	MenuCollapseSection *dataGraphSection = new MenuCollapseSection("Data Graph", MenuCollapseSection::MHCW_NONE, dataGraphWidget);
	dataGraphSection->contentLayout()->setSpacing(8);

	// Graph Channel
	m_dataGraphChannelMenuCombo = new MenuCombo("Channel", dataGraphSection);
	auto dataGraphChannelCombo = m_dataGraphChannelMenuCombo->combo();
	dataGraphChannelCombo->addItem("Rotation", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(rotationChannelName))));
	dataGraphChannelCombo->addItem("Angle", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(angleChannelName))));
	dataGraphChannelCombo->addItem("Count", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(countChannelName))));
	applyComboBoxStyle(dataGraphChannelCombo);

	connectMenuComboToGraphChannel(m_dataGraphChannelMenuCombo, dataGraph);

	dataGraphSection->contentLayout()->addWidget(m_dataGraphChannelMenuCombo);

	// Graph Samples
	QLabel *dataGraphSamplesLabel = new QLabel(generalSection);
	dataGraphSamplesLabel->setText("Samples");
	StyleHelper::MenuSmallLabel(dataGraphSamplesLabel, "dataGraphSamplesLabel");
	dataGraphSamplesLineEdit = new QLineEdit(generalSection);
	applyLineEditStyle(dataGraphSamplesLineEdit);
	dataGraphSamplesLineEdit->setText(QString::number(dataGraphSamples));

	connectLineEditToGraphSamples(dataGraphSamplesLineEdit, dataGraphSamples, dataGraph);
	
	dataGraphSection->contentLayout()->addWidget(dataGraphSamplesLabel);
	dataGraphSection->contentLayout()->addWidget(dataGraphSamplesLineEdit);

	dataGraphWidget->contentLayout()->addWidget(dataGraphSection);

	// Temperature Graph
	MenuSectionWidget *tempGraphWidget = new MenuSectionWidget(generalSettingWidget);
	tempGraphWidget->contentLayout()->setSpacing(8);
	MenuCollapseSection *tempGraphSection = new MenuCollapseSection("Temperature Graph", MenuCollapseSection::MHCW_NONE, tempGraphWidget);
	tempGraphSection->contentLayout()->setSpacing(8);

	// Graph Samples
	QLabel *tempGraphSamplesLabel = new QLabel(generalSection);
	tempGraphSamplesLabel->setText("Samples");
	StyleHelper::MenuSmallLabel(tempGraphSamplesLabel, "tempGraphSamplesLabel");
	tempGraphSamplesLineEdit = new QLineEdit(generalSection);
	applyLineEditStyle(tempGraphSamplesLineEdit);
	tempGraphSamplesLineEdit->setText(QString::number(tempGraphSamples));
	tempGraphSection->contentLayout()->addWidget(tempGraphSamplesLabel);
	tempGraphSection->contentLayout()->addWidget(tempGraphSamplesLineEdit);

	connectLineEditToGraphSamples(tempGraphSamplesLineEdit, tempGraphSamples, tempGraph);

	tempGraphWidget->contentLayout()->addWidget(tempGraphSection);

	generalSettingLayout->addWidget(header);
	generalSettingLayout->addSpacerItem(new QSpacerItem(0, 3, QSizePolicy::Fixed, QSizePolicy::Fixed));
	generalSettingLayout->addWidget(generalWidget);
	generalSettingLayout->addWidget(sequenceWidget);
	generalSettingLayout->addWidget(dataGraphWidget);
	generalSettingLayout->addWidget(tempGraphWidget);
	generalSettingLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);
    tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);
    tool->addWidgetToTopContainerMenuControlHelper(openLastMenuButton, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);
    tool->addWidgetToTopContainerHelper(runButton, TTA_RIGHT);
	tool->leftStack()->add("rawDataScroll", rawDataScroll);
	tool->rightStack()->add("generalSettingScroll", generalSettingScroll);
	tool->addWidgetToCentralContainerHelper(historicalGraphWidget);

	connect(runButton, &QPushButton::toggled, this, &HarmonicCalibration::setRunning);
	connect(this, &HarmonicCalibration::runningChanged, this, &HarmonicCalibration::run);
	connect(this, &HarmonicCalibration::runningChanged, runButton, &QAbstractButton::setChecked);

	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &HarmonicCalibration::timerTask);

	calibrationTimer = new QTimer(this);
	connect(calibrationTimer, &QTimer::timeout, this, &HarmonicCalibration::calibrationTask);

	motorCalibrationAcquisitionTimer = new QTimer(this);
	connect(motorCalibrationAcquisitionTimer, &QTimer::timeout, this, &HarmonicCalibration::motorCalibrationAcquisitionTask);

	tabWidget->addTab(createCalibrationWidget(), "Calibration");
	tabWidget->addTab(createUtilityWidget(), "Utility");
	tabWidget->addTab(createRegistersWidget(), "Registers");

	connect(tabWidget, &QTabWidget::currentChanged, [=](int index){
		tabWidget->setCurrentIndex(index);

		if(index == 1) { calibrationTimer->start(calibrationTimerRate); }
		else { calibrationTimer->stop(); }
	});
}

HarmonicCalibration::~HarmonicCalibration() {}

ToolTemplate* HarmonicCalibration::createCalibrationWidget()
{
	initializeMotor();
	ToolTemplate *tool = new ToolTemplate(this);

	#pragma region Calibration Data Graph Widget
	QWidget *calibrationDataGraphWidget = new QWidget();
	QGridLayout *calibrationDataGraphLayout = new QGridLayout(calibrationDataGraphWidget);
	calibrationDataGraphWidget->setLayout(calibrationDataGraphLayout);
	calibrationDataGraphLayout->setMargin(0);
	calibrationDataGraphLayout->setSpacing(5);

	MenuSectionWidget *calibrationDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	QTabWidget *calibrationDataGraphTabWidget = new QTabWidget(calibrationDataGraphSectionWidget);
	applyTabWidgetStyle(calibrationDataGraphTabWidget);
	calibrationDataGraphSectionWidget->contentLayout()->setSpacing(8);

	// Raw Data Plot Widget
	calibrationRawDataPlotWidget = new PlotWidget();
	calibrationRawDataPlotWidget->setContentsMargins(10, 20, 10, 10);
	calibrationRawDataPlotWidget->xAxis()->setVisible(false);
	calibrationRawDataPlotWidget->yAxis()->setVisible(false);
	QPen calibrationRawDataPen = QPen(StyleHelper::getColor("ScopyBlue"));
	QPen calibrationSineDataPen = QPen(sineColor);
	QPen calibrationCosineDataPen = QPen(cosineColor);

	calibrationRawDataXPlotAxis = new PlotAxis(QwtAxis::XBottom, calibrationRawDataPlotWidget, calibrationRawDataPen);
	calibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, calibrationRawDataPlotWidget, calibrationRawDataPen);
	calibrationRawDataYPlotAxis->setInterval(0, 400);

	calibrationRawDataPlotChannel = new PlotChannel("Samples", calibrationRawDataPen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationSineDataPlotChannel = new PlotChannel("Sine", calibrationSineDataPen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationCosineDataPlotChannel = new PlotChannel("Cosine", calibrationCosineDataPen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);

	calibrationRawDataPlotWidget->addPlotChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationSineDataPlotChannel);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationCosineDataPlotChannel);
	// calibrationRawDataPlotChannel->setStyle(PlotChannel::PCS_DOTS);
	calibrationRawDataPlotChannel->setEnabled(true);
	calibrationSineDataPlotChannel->setEnabled(true);
	calibrationCosineDataPlotChannel->setEnabled(true);
	calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotWidget->replot();

	calibrationRawDataPlotWidget->setShowXAxisLabels(true);
	calibrationRawDataPlotWidget->setShowYAxisLabels(true);
	calibrationRawDataPlotWidget->showAxisLabels();

	calibrationDataGraphTabWidget->addTab(calibrationRawDataPlotWidget, "Calibration Samples");

	QWidget *calibrationDataGraphChannelsWidget = new QWidget(calibrationDataGraphSectionWidget);
	QHBoxLayout *calibrationDataGraphChannelsLayout = new QHBoxLayout(calibrationDataGraphChannelsWidget);
	calibrationDataGraphChannelsWidget->setLayout(calibrationDataGraphChannelsLayout);
	QString calibrationDataGraphChannelsStyle = QString(R"css(
														background-color: &&colorname&&;
														)css");
	calibrationDataGraphChannelsStyle.replace(QString("&&colorname&&"), StyleHelper::getColor("UIElementBackground"));
	calibrationDataGraphChannelsWidget->setStyleSheet(calibrationDataGraphChannelsStyle);
	calibrationDataGraphChannelsLayout->setContentsMargins(20, 5, 20, 5);
	calibrationDataGraphChannelsLayout->setSpacing(20);
	
	MenuControlButton *toggleAngleButton = createChannelToggleWidget("Angle", QColor(StyleHelper::getColor("ScopyBlue")), calibrationDataGraphChannelsWidget);
	MenuControlButton *toggleSineButton = createChannelToggleWidget("Sine", sineColor, calibrationDataGraphChannelsWidget);
	MenuControlButton *toggleCosineButton = createChannelToggleWidget("Cosine", cosineColor, calibrationDataGraphChannelsWidget);

	calibrationDataGraphChannelsLayout->addWidget(toggleAngleButton);
	calibrationDataGraphChannelsLayout->addWidget(toggleSineButton);
	calibrationDataGraphChannelsLayout->addWidget(toggleCosineButton);
	calibrationDataGraphChannelsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));

	calibrationDataGraphSectionWidget->contentLayout()->addWidget(calibrationDataGraphTabWidget);
	calibrationDataGraphSectionWidget->contentLayout()->addWidget(calibrationDataGraphChannelsWidget);

	MenuSectionWidget *FFTDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	QTabWidget *FFTDataGraphTabWidget = new QTabWidget(FFTDataGraphSectionWidget);
	applyTabWidgetStyle(FFTDataGraphTabWidget);
	FFTDataGraphSectionWidget->contentLayout()->setSpacing(8);
	FFTDataGraphSectionWidget->contentLayout()->addWidget(FFTDataGraphTabWidget);

	// FFT Plot Widget
	calibrationFFTDataPlotWidget = new PlotWidget();
	calibrationFFTDataPlotWidget->setContentsMargins(10, 20, 10, 10);
	calibrationFFTDataPlotWidget->xAxis()->setVisible(false);
	calibrationFFTDataPlotWidget->yAxis()->setVisible(false);
	QPen calibrationFFTPen = QPen(StyleHelper::getColor("ScopyBlue"));
	QPen calibrationFFTPhasePen = QPen(StyleHelper::getColor("CH0"));

	calibrationFFTXPlotAxis = new PlotAxis(QwtAxis::XBottom, calibrationFFTDataPlotWidget, calibrationFFTPen);
	calibrationFFTYPlotAxis = new PlotAxis(QwtAxis::YLeft, calibrationFFTDataPlotWidget, calibrationFFTPen);
	calibrationFFTYPlotAxis->setInterval(-4, 4);

	calibrationFFTPlotChannel = new PlotChannel("FFT", calibrationFFTPen, calibrationFFTXPlotAxis, calibrationFFTYPlotAxis);
	calibrationFFTPhasePlotChannel = new PlotChannel("FFT Phase", calibrationFFTPhasePen, calibrationFFTXPlotAxis, calibrationFFTYPlotAxis);
	calibrationFFTDataPlotWidget->addPlotChannel(calibrationFFTPlotChannel);
	calibrationFFTDataPlotWidget->addPlotChannel(calibrationFFTPhasePlotChannel);

	calibrationFFTPlotChannel->setEnabled(true);
	calibrationFFTPhasePlotChannel->setEnabled(true);
	calibrationFFTDataPlotWidget->selectChannel(calibrationFFTPlotChannel);
	calibrationFFTDataPlotWidget->replot();

	calibrationFFTDataPlotWidget->setShowXAxisLabels(true);
	calibrationFFTDataPlotWidget->setShowYAxisLabels(true);
	calibrationFFTDataPlotWidget->showAxisLabels();

	PlotWidget *postCalibrationAngularErrorPlotWidget = new PlotWidget();

	FFTDataGraphTabWidget->addTab(calibrationFFTDataPlotWidget, "Pre-Calibration Angular Error");
	FFTDataGraphTabWidget->addTab(postCalibrationAngularErrorPlotWidget, "Post-Calibration Angular Error");

	calibrationDataGraphLayout->addWidget(calibrationDataGraphSectionWidget, 0, 0);
	calibrationDataGraphLayout->addWidget(FFTDataGraphSectionWidget, 1, 0);

	calibrationDataGraphLayout->setColumnStretch(0, 1);
	calibrationDataGraphLayout->setRowStretch(0, 1);
	calibrationDataGraphLayout->setRowStretch(1, 1);
	#pragma endregion
	
	#pragma region Calibration Settings Widget
	QScrollArea *calibrationSettingsScrollArea = new QScrollArea();
	QWidget *calibrationSettingsWidget = new QWidget(calibrationSettingsScrollArea);
	QVBoxLayout *calibrationSettingsLayout = new QVBoxLayout(calibrationSettingsWidget);
	calibrationSettingsScrollArea->setWidgetResizable(true);
	calibrationSettingsScrollArea->setWidget(calibrationSettingsWidget);
	calibrationSettingsWidget->setFixedWidth(260);
	calibrationSettingsWidget->setLayout(calibrationSettingsLayout);

	#pragma region Calibration Coefficient Section Widget
	MenuSectionWidget *calibrationCoeffSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	QLabel *calibrationDisplayFormatLabel = new QLabel(calibrationCoeffSectionWidget);
	calibrationDisplayFormatLabel->setText("Display Format");
	StyleHelper::MenuCollapseHeaderLabel(calibrationDisplayFormatLabel, "calibrationDisplayFormatLabel");
	QLabel *calibrationCalculatedCoeffLabel = new QLabel(calibrationCoeffSectionWidget);
	calibrationCalculatedCoeffLabel->setText("Calculated Coefficients");
	StyleHelper::MenuCollapseHeaderLabel(calibrationCalculatedCoeffLabel, "calibrationCalculatedCoeffLabel");

	CustomSwitch *calibrationDisplayFormatSwitch = new CustomSwitch();
	calibrationDisplayFormatSwitch->setOffText("Hex");
	calibrationDisplayFormatSwitch->setOnText("Angle");
	calibrationDisplayFormatSwitch->setProperty("bigBtn", true);
	applyCalibrationDataButton = new QPushButton(calibrationCoeffSectionWidget);
	applyCalibrationDataButton->setText("Apply");
	StyleHelper::BlueButton(applyCalibrationDataButton, "applyCalibrationDataButton");
	applyCalibrationDataButton->setEnabled(false);

	// Calculated Coefficients Widget
	QWidget *calibrationCalculatedCoeffWidget = new QWidget(calibrationCoeffSectionWidget);
	QGridLayout *calibrationCalculatedCoeffLayout = new QGridLayout(calibrationCalculatedCoeffWidget);
	calibrationCalculatedCoeffWidget->setLayout(calibrationCalculatedCoeffLayout);
	calibrationCalculatedCoeffLayout->setMargin(0);
	calibrationCalculatedCoeffLayout->setVerticalSpacing(4);
	QString calibrationCalculatedCoeffStyle = QString(R"css(
													background-color: &&colorname&&;
													)css");
	calibrationCalculatedCoeffStyle.replace(QString("&&colorname&&"), StyleHelper::getColor("UIElementBackground"));
	calibrationCalculatedCoeffWidget->setStyleSheet(calibrationCalculatedCoeffStyle);

	QString rowContainerStyle = QString(R"css(
										background-color: &&colorname&&;
										border-radius: 4px;
										)css");
	rowContainerStyle.replace(QString("&&colorname&&"), StyleHelper::getColor("ScopyBackground"));

	// H1
	QWidget *h1RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h1RowLayout = new QHBoxLayout(h1RowContainer);
	h1RowContainer->setLayout(h1RowLayout);
	h1RowContainer->setStyleSheet(rowContainerStyle);
	h1RowContainer->setFixedHeight(30);
	h1RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h1RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH1Label = new QLabel("H1", calibrationCalculatedCoeffWidget);
	calibrationH1MagLabel = new QLabel("--.--°", calibrationCalculatedCoeffWidget);
	calibrationH1PhaseLabel = new QLabel("Φ --.--", calibrationCalculatedCoeffWidget);
	applyTextStyle(calibrationH1Label, "LabelText", true);
	applyTextStyle(calibrationH1MagLabel, "CH0");
	applyTextStyle(calibrationH1PhaseLabel, "CH1");
	calibrationH1Label->setFixedWidth(24);
	calibrationH1MagLabel->setContentsMargins(0, 0, 48, 0);
	calibrationH1PhaseLabel->setFixedWidth(56);

	h1RowLayout->addWidget(calibrationH1Label);
	h1RowLayout->addWidget(calibrationH1MagLabel, 0, Qt::AlignRight);
	h1RowLayout->addWidget(calibrationH1PhaseLabel);

	// H2
	QWidget *h2RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h2RowLayout = new QHBoxLayout(h2RowContainer);
	h2RowContainer->setLayout(h2RowLayout);
	h2RowContainer->setStyleSheet(rowContainerStyle);
	h2RowContainer->setFixedHeight(30);
	h2RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h2RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH2Label = new QLabel("H2", calibrationCalculatedCoeffWidget);
	calibrationH2MagLabel = new QLabel("--.--°", calibrationCalculatedCoeffWidget);
	calibrationH2PhaseLabel = new QLabel("Φ --.--", calibrationCalculatedCoeffWidget);
	applyTextStyle(calibrationH2Label, "LabelText", true);
	applyTextStyle(calibrationH2MagLabel, "CH0");
	applyTextStyle(calibrationH2PhaseLabel, "CH1");
	calibrationH2Label->setFixedWidth(24);
	calibrationH2MagLabel->setContentsMargins(0, 0, 48, 0);
	calibrationH2PhaseLabel->setFixedWidth(56);

	h2RowLayout->addWidget(calibrationH2Label);
	h2RowLayout->addWidget(calibrationH2MagLabel, 0, Qt::AlignRight);
	h2RowLayout->addWidget(calibrationH2PhaseLabel);

	// H3
	QWidget *h3RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h3RowLayout = new QHBoxLayout(h3RowContainer);
	h3RowContainer->setLayout(h3RowLayout);
	h3RowContainer->setStyleSheet(rowContainerStyle);
	h3RowContainer->setFixedHeight(30);
	h3RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h3RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH3Label = new QLabel("H3", calibrationCalculatedCoeffWidget);
	calibrationH3MagLabel = new QLabel("--.--°", calibrationCalculatedCoeffWidget);
	calibrationH3PhaseLabel = new QLabel("Φ --.--", calibrationCalculatedCoeffWidget);
	applyTextStyle(calibrationH3Label, "LabelText", true);
	applyTextStyle(calibrationH3MagLabel, "CH0");
	applyTextStyle(calibrationH3PhaseLabel, "CH1");
	calibrationH3Label->setFixedWidth(24);
	calibrationH3MagLabel->setContentsMargins(0, 0, 48, 0);
	calibrationH3PhaseLabel->setFixedWidth(56);

	h3RowLayout->addWidget(calibrationH3Label);
	h3RowLayout->addWidget(calibrationH3MagLabel, 0, Qt::AlignRight);
	h3RowLayout->addWidget(calibrationH3PhaseLabel);

	// H8
	QWidget *h8RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h8RowLayout = new QHBoxLayout(h8RowContainer);
	h8RowContainer->setLayout(h8RowLayout);
	h8RowContainer->setStyleSheet(rowContainerStyle);
	h8RowContainer->setFixedHeight(30);
	h8RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h8RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH8Label = new QLabel("H8", calibrationCalculatedCoeffWidget);
	calibrationH8MagLabel = new QLabel("--.--°", calibrationCalculatedCoeffWidget);
	calibrationH8PhaseLabel = new QLabel("Φ --.--", calibrationCalculatedCoeffWidget);
	applyTextStyle(calibrationH8Label, "LabelText", true);
	applyTextStyle(calibrationH8MagLabel, "CH0");
	applyTextStyle(calibrationH8PhaseLabel, "CH1");
	calibrationH8Label->setFixedWidth(24);
	calibrationH8MagLabel->setContentsMargins(0, 0, 48, 0);
	calibrationH8PhaseLabel->setFixedWidth(56);

	h8RowLayout->addWidget(calibrationH8Label);
	h8RowLayout->addWidget(calibrationH8MagLabel, 0, Qt::AlignRight);
	h8RowLayout->addWidget(calibrationH8PhaseLabel);

	calibrationCalculatedCoeffLayout->addWidget(h1RowContainer, 0, 0);
	calibrationCalculatedCoeffLayout->addWidget(h2RowContainer, 1, 0);
	calibrationCalculatedCoeffLayout->addWidget(h3RowContainer, 2, 0);
	calibrationCalculatedCoeffLayout->addWidget(h8RowContainer, 3, 0);

	calibrationCoeffSectionWidget->contentLayout()->setSpacing(8);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatSwitch);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffWidget);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(applyCalibrationDataButton);
	#pragma endregion

	#pragma region Calibration Dataset Configuration
	MenuSectionWidget *calibrationDatasetConfigSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *calibrationDatasetConfigCollapseSection = new MenuCollapseSection("Dataset Configuration", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, calibrationDatasetConfigSectionWidget);
	calibrationDatasetConfigSectionWidget->contentLayout()->setSpacing(8);
	calibrationDatasetConfigSectionWidget->contentLayout()->addWidget(calibrationDatasetConfigCollapseSection);

	QLabel *calibrationCycleCountLabel = new QLabel("Cycle Count", calibrationDatasetConfigCollapseSection);
	StyleHelper::MenuSmallLabel(calibrationCycleCountLabel);
	QLineEdit *calibrationCycleCountLineEdit = new QLineEdit(calibrationDatasetConfigCollapseSection);
	applyLineEditStyle(calibrationCycleCountLineEdit);
	calibrationCycleCountLineEdit->setText(QString::number(cycleCount));
	connectLineEditToNumber(calibrationCycleCountLineEdit, cycleCount);

	QLabel *calibrationSamplesPerCycleLabel = new QLabel("Samples Per Cycle", calibrationDatasetConfigCollapseSection);
	StyleHelper::MenuSmallLabel(calibrationSamplesPerCycleLabel);
	QLineEdit *calibrationSamplesPerCycleLineEdit = new QLineEdit(calibrationDatasetConfigCollapseSection);
	applyLineEditStyle(calibrationSamplesPerCycleLineEdit);
	calibrationSamplesPerCycleLineEdit->setText(QString::number(samplesPerCycle));
	connectLineEditToNumber(calibrationSamplesPerCycleLineEdit, samplesPerCycle);

	calibrationDatasetConfigCollapseSection->contentLayout()->setSpacing(8);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountLabel);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountLineEdit);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationSamplesPerCycleLabel);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationSamplesPerCycleLineEdit);

	#pragma endregion

	#pragma region Calibration Data Section Widget
	MenuSectionWidget *calibrationDataSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *calibrationDataCollapseSection = new MenuCollapseSection("Calibration Data", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, calibrationDataSectionWidget);
	calibrationDataSectionWidget->contentLayout()->setSpacing(8);
	calibrationDataSectionWidget->contentLayout()->addWidget(calibrationDataCollapseSection);

	QPushButton *importDataButton = new QPushButton(calibrationDataCollapseSection);
	importDataButton->setText("Import from CSV");
	StyleHelper::BlueButton(importDataButton, "importDataButton");
	QPushButton *extractDataButton = new QPushButton(calibrationDataCollapseSection);
	extractDataButton->setText("Extract to CSV");
	StyleHelper::BlueButton(extractDataButton, "extractDataButton");
	QPushButton *clearCalibrateDataButton = new QPushButton(calibrationDataCollapseSection);
	clearCalibrateDataButton->setText("Clear All Data");
	StyleHelper::BlueButton(clearCalibrateDataButton, "clearCalibrateDataButton");

	calibrationDataCollapseSection->contentLayout()->setSpacing(8);
	calibrationDataCollapseSection->contentLayout()->addWidget(importDataButton);
	calibrationDataCollapseSection->contentLayout()->addWidget(extractDataButton);
	calibrationDataCollapseSection->contentLayout()->addWidget(clearCalibrateDataButton);
	#pragma endregion

	#pragma region Motor Configuration Section Widget
	MenuSectionWidget *motorConfigurationSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *motorConfigurationCollapseSection = new MenuCollapseSection("Motor Configuration", MenuCollapseSection::MHCW_NONE, motorConfigurationSectionWidget);
	motorConfigurationSectionWidget->contentLayout()->addWidget(motorConfigurationCollapseSection);

	motorMaxVelocitySpinBox = new HorizontalSpinBox("Max Velocity", convertVMAXtoRPS(rotate_vmax), "rps", motorConfigurationSectionWidget);
	motorAccelTimeSpinBox = new HorizontalSpinBox("Acceleration Time", convertAMAXtoAccelTime(amax), "sec", motorConfigurationSectionWidget);
	motorMaxDisplacementSpinBox = new HorizontalSpinBox("Max Displacement", dmax, "", motorConfigurationSectionWidget);

	MenuCombo *m_calibrationMotorRampModeMenuCombo = new MenuCombo("Ramp Mode", motorConfigurationSectionWidget);
	auto calibrationMotorRampModeCombo = m_calibrationMotorRampModeMenuCombo->combo();
	calibrationMotorRampModeCombo->addItem("Position", QVariant(ADMTController::MotorRampMode::POSITION));
	calibrationMotorRampModeCombo->addItem("Ramp Mode 1", QVariant(ADMTController::MotorRampMode::RAMP_MODE_1));
	applyComboBoxStyle(calibrationMotorRampModeCombo);

	motorConfigurationCollapseSection->contentLayout()->setSpacing(8);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxVelocitySpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorAccelTimeSpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxDisplacementSpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(m_calibrationMotorRampModeMenuCombo);
	#pragma endregion

	#pragma region Motor Control Section Widget
	MenuSectionWidget *motorControlSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *motorControlCollapseSection = new MenuCollapseSection("Motor Control", MenuCollapseSection::MHCW_NONE, motorControlSectionWidget);
	motorControlSectionWidget->contentLayout()->setSpacing(8);
	motorControlSectionWidget->contentLayout()->addWidget(motorControlCollapseSection);
	QLabel *currentPositionLabel = new QLabel("Current Position", motorControlSectionWidget);
	StyleHelper::MenuSmallLabel(currentPositionLabel, "currentPositionLabel");
	calibrationMotorCurrentPositionLabel = new QLabel("--.--", motorControlSectionWidget);
	calibrationMotorCurrentPositionLabel->setAlignment(Qt::AlignRight);
	applyLabelStyle(calibrationMotorCurrentPositionLabel);

	motorTargetPositionSpinBox = new HorizontalSpinBox("Target Position", target_pos, "", motorControlSectionWidget);

	calibrationStartMotorButton = new QPushButton(motorControlSectionWidget);
	calibrationStartMotorButton->setCheckable(true);
	calibrationStartMotorButton->setChecked(false);
	calibrationStartMotorButton->setText("Start Motor");
	calibrationStartMotorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	calibrationStartMotorButton->setFixedHeight(36);
	connect(calibrationStartMotorButton, &QPushButton::toggled, this, [=](bool b) { 
		calibrationStartMotorButton->setText(b ? " Stop Motor" : " Start Motor"); 
		totalSamplesCount = cycleCount * samplesPerCycle;
		startMotor = b;
		if(b){
			motorCalibrationAcquisitionTimer->start(motorCalibrationAcquisitionTimerRate);
		}
		else{
			motorCalibrationAcquisitionTimer->stop();
		}
	});
	QString calibrationStartMotorButtonStyle = QString(R"css(
			QPushButton {
				border-radius: 2px;
				padding-left: 20px;
				padding-right: 20px;
				color: white;
				font-weight: 700;
				font-size: 14px;
			}

			QPushButton:!checked {
				background-color: #27b34f;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			})css");
	calibrationStartMotorButton->setStyleSheet(calibrationStartMotorButtonStyle);
	QIcon playIcon;
	playIcon.addPixmap(Util::ChangeSVGColor(":/gui/icons/play.svg", "white", 1), QIcon::Normal, QIcon::Off);
	playIcon.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/play_stop.svg", "white", 1),
			QIcon::Normal, QIcon::On);
	calibrationStartMotorButton->setIcon(playIcon);
	calibrationStartMotorButton->setIconSize(QSize(64, 64));

	calibrateDataButton = new QPushButton(motorControlSectionWidget);
	calibrateDataButton->setText("Calibrate");
	StyleHelper::BlueButton(calibrateDataButton, "calibrateDataButton");

	autoCalibrateCheckBox = new QCheckBox("Auto Calibrate", motorControlSectionWidget);
	StyleHelper::BlueSquareCheckbox(autoCalibrateCheckBox, "autoCalibrateCheckBox");

	motorControlCollapseSection->contentLayout()->setSpacing(8);
	motorControlCollapseSection->contentLayout()->addWidget(currentPositionLabel);
	motorControlCollapseSection->contentLayout()->addWidget(calibrationMotorCurrentPositionLabel);
	motorControlCollapseSection->contentLayout()->addWidget(motorTargetPositionSpinBox);
	motorControlCollapseSection->contentLayout()->addWidget(calibrationStartMotorButton);
	motorControlCollapseSection->contentLayout()->addWidget(calibrateDataButton);
	motorControlCollapseSection->contentLayout()->addWidget(autoCalibrateCheckBox);
	#pragma endregion

	#pragma region Logs Section Widget
	MenuSectionWidget *logsSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *logsCollapseSection = new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE, logsSectionWidget);
	logsSectionWidget->contentLayout()->setSpacing(8);
	logsSectionWidget->contentLayout()->addWidget(logsCollapseSection);

	logsPlainTextEdit = new QPlainTextEdit(logsSectionWidget);
	logsPlainTextEdit->setReadOnly(true);
	logsPlainTextEdit->setFixedHeight(320);

	logsCollapseSection->contentLayout()->setSpacing(8);
	logsCollapseSection->contentLayout()->addWidget(logsPlainTextEdit);
	#pragma endregion

	calibrationSettingsLayout->setMargin(0);
	calibrationSettingsLayout->addWidget(calibrationCoeffSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationDatasetConfigSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationDataSectionWidget);
	calibrationSettingsLayout->addWidget(motorConfigurationSectionWidget);
	calibrationSettingsLayout->addWidget(motorControlSectionWidget);
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
	tool->rightStack()->add("calibrationSettingsScrollArea", calibrationSettingsScrollArea);

	connect(calibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::calibrateData);
	connect(extractDataButton, &QPushButton::clicked, this, &HarmonicCalibration::extractCalibrationData);
	connect(importDataButton, &QPushButton::clicked, this, &HarmonicCalibration::importCalibrationData);
	connect(applyCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::registerCalibrationData);
	connect(clearCalibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::clearRawDataList);
	connectLineEditToRPSConversion(motorMaxVelocitySpinBox->lineEdit(), rotate_vmax);
	connectLineEditToAMAXConversion(motorAccelTimeSpinBox->lineEdit(), amax);
	connectLineEditToNumberWrite(motorMaxDisplacementSpinBox->lineEdit(), dmax, ADMTController::MotorAttribute::DMAX);
	connectLineEditToNumberWrite(motorTargetPositionSpinBox->lineEdit(), target_pos, ADMTController::MotorAttribute::TARGET_POS);
	connectMenuComboToNumber(m_calibrationMotorRampModeMenuCombo, ramp_mode);
	connect(autoCalibrateCheckBox, &QCheckBox::toggled, [=](bool toggled){ 
		autoCalibrate = toggled; 
		StatusBarManager::pushMessage(QString("Auto Calibrate: ") + QString((toggled ? "True" : "False")));
	});
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

	return tool;
}

ToolTemplate* HarmonicCalibration::createRegistersWidget()
{
	ToolTemplate *tool = new ToolTemplate(this);

	QScrollArea *registerScrollArea = new QScrollArea();
	QWidget *registerWidget = new QWidget(registerScrollArea);
	QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
	registerScrollArea->setWidgetResizable(true);
	registerScrollArea->setWidget(registerWidget);
	registerWidget->setLayout(registerLayout);
	registerLayout->setMargin(0);
	registerLayout->setSpacing(8);

	QLabel *registerConfigurationLabel = new QLabel("Configuration", registerWidget);
	StyleHelper::MenuControlLabel(registerConfigurationLabel, "registerConfigurationLabel");
	QWidget *registerConfigurationGridWidget = new QWidget(registerWidget);
	QGridLayout *registerConfigurationGridLayout = new QGridLayout(registerConfigurationGridWidget);
	registerConfigurationGridWidget->setLayout(registerConfigurationGridLayout);
	registerConfigurationGridLayout->setMargin(0);
	registerConfigurationGridLayout->setSpacing(8);

	QLabel *registerDeviceIDLabel = new QLabel("Device ID", registerWidget);
	StyleHelper::MenuControlLabel(registerDeviceIDLabel, "registerDeviceIDLabel");
	QWidget *registerDeviceIDGridWidget = new QWidget(registerWidget);
	QGridLayout *registerDeviceIDGridLayout = new QGridLayout(registerDeviceIDGridWidget);
	registerDeviceIDGridWidget->setLayout(registerDeviceIDGridLayout);
	registerDeviceIDGridLayout->setMargin(0);
	registerDeviceIDGridLayout->setSpacing(8);

	QLabel *registerHarmonicsLabel = new QLabel("Harmonics", registerWidget);
	StyleHelper::MenuControlLabel(registerHarmonicsLabel, "registerHarmonicsLabel");
	QWidget *registerHarmonicsGridWidget = new QWidget(registerWidget);
	QGridLayout *registerHarmonicsGridLayout = new QGridLayout(registerHarmonicsGridWidget);
	registerHarmonicsGridWidget->setLayout(registerHarmonicsGridLayout);
	registerHarmonicsGridLayout->setMargin(0);
	registerHarmonicsGridLayout->setSpacing(8);

	QLabel *registerSensorDataLabel = new QLabel("Sensor Data", registerWidget);
	StyleHelper::MenuControlLabel(registerSensorDataLabel, "registerSensorDataLabel");
	QWidget *registerSensorDataGridWidget = new QWidget(registerWidget);
	QGridLayout *registerSensorDataGridLayout = new QGridLayout(registerSensorDataGridWidget);
	registerSensorDataGridWidget->setLayout(registerSensorDataGridLayout);
	registerSensorDataGridLayout->setMargin(0);
	registerSensorDataGridLayout->setSpacing(8);

	RegisterBlockWidget *cnvPageRegisterBlock = new RegisterBlockWidget("CNVPAGE", "Convert Start and Page Select", 0x01, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *digIORegisterBlock = new RegisterBlockWidget("DIGIO", "Digital Input Output", 0x04, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *faultRegisterBlock = new RegisterBlockWidget("FAULT", "Fault Register", 0x06, 0xFFFF, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *generalRegisterBlock = new RegisterBlockWidget("GENERAL", "General Device Configuration", 0x10, 0x1230, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *digIOEnRegisterBlock = new RegisterBlockWidget("DIGIOEN", "Enable Digital Input/Outputs", 0x12, 0x241B, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *angleCkRegisterBlock = new RegisterBlockWidget("ANGLECK", "Primary vs Secondary Angle Check", 0x13, 0x000F, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *eccDcdeRegisterBlock = new RegisterBlockWidget("ECCDCDE", "Error Correction Codes", 0x1D, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *eccDisRegisterBlock = new RegisterBlockWidget("ECCDIS", "Error Correction Code disable", 0x23, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);

	RegisterBlockWidget *absAngleRegisterBlock = new RegisterBlockWidget("ABSANGLE", "Absolute Angle", 0x03, 0xDB00, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *angleRegisterBlock = new RegisterBlockWidget("ANGLE", "Angle Register", 0x05, 0x8000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *angleSecRegisterBlock = new RegisterBlockWidget("ANGLESEC", "Secondary Angle", 0x08, 0x8000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *sineRegisterBlock = new RegisterBlockWidget("SINE", "Sine Measurement", 0x10, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *cosineRegisterBlock = new RegisterBlockWidget("COSINE", "Cosine Measurement", 0x11, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *secAnglIRegisterBlock = new RegisterBlockWidget("SECANGLI", "In-phase secondary angle measurement", 0x12, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *secAnglQRegisterBlock = new RegisterBlockWidget("SECANGLQ", "Quadrature phase secondary angle measurement", 0x13, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *radiusRegisterBlock = new RegisterBlockWidget("RADIUS", "Angle measurement radius", 0x18, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *diag1RegisterBlock = new RegisterBlockWidget("DIAG1", "State of the MT reference resistors and AFE fixed input voltage", 0x1D, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *diag2RegisterBlock = new RegisterBlockWidget("DIAG2", "Measurements of two diagnostics resistors of fixed value", 0x1E, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *tmp0RegisterBlock = new RegisterBlockWidget("TMP0", "Primary Temperature Sensor", 0x20, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *tmp1RegisterBlock = new RegisterBlockWidget("TMP1", "Secondary Temperature Sensor", 0x23, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *cnvCntRegisterBlock = new RegisterBlockWidget("CNVCNT", "Conversion Count", 0x14, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	RegisterBlockWidget *uniqID0RegisterBlock = new RegisterBlockWidget("UNIQID0", "Unique ID Register 0", 0x1E, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *uniqID1RegisterBlock = new RegisterBlockWidget("UNIQID1", "Unique ID Register 1", 0x1F, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *uniqID2RegisterBlock = new RegisterBlockWidget("UNIQID2", "Unique ID Register 2", 0x20, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);
	RegisterBlockWidget *uniqID3RegisterBlock = new RegisterBlockWidget("UNIQID3", "Product, voltage supply. ASIL and ASIC revision identifiers", 0x21, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READ, registerWidget);

	RegisterBlockWidget *h1MagRegisterBlock = new RegisterBlockWidget("H1MAG", "1st Harmonic error magnitude", 0x15, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h1PhRegisterBlock = new RegisterBlockWidget("H1PH", "1st Harmonic error phase", 0x16, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h2MagRegisterBlock = new RegisterBlockWidget("H2MAG", "2nd Harmonic error magnitude", 0x17, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h2PhRegisterBlock = new RegisterBlockWidget("H2PH", "2nd Harmonic error phase", 0x18, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h3MagRegisterBlock = new RegisterBlockWidget("H3MAG", "3rd Harmonic error magnitude", 0x19, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h3PhRegisterBlock = new RegisterBlockWidget("H3PH", "3rd Harmonic error phase", 0x1A, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h8MagRegisterBlock = new RegisterBlockWidget("H8MAG", "8th Harmonic error magnitude", 0x1B, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	RegisterBlockWidget *h8PhRegisterBlock = new RegisterBlockWidget("H8PH", "8th Harmonic error phase", 0x1C, 0x0000, RegisterBlockWidget::ACCESS_PERMISSION::READWRITE, registerWidget);
	
	registerConfigurationGridLayout->addWidget(cnvPageRegisterBlock, 0, 0);
	registerConfigurationGridLayout->addWidget(digIORegisterBlock, 0, 1);
	registerConfigurationGridLayout->addWidget(faultRegisterBlock, 0, 2);
	registerConfigurationGridLayout->addWidget(generalRegisterBlock, 0, 3);
	registerConfigurationGridLayout->addWidget(digIOEnRegisterBlock, 0, 4);
	registerConfigurationGridLayout->addWidget(angleCkRegisterBlock, 1, 0);
	registerConfigurationGridLayout->addWidget(eccDcdeRegisterBlock, 1, 1);
	registerConfigurationGridLayout->addWidget(eccDisRegisterBlock, 1, 2);

	registerSensorDataGridLayout->addWidget(absAngleRegisterBlock, 0, 0);
	registerSensorDataGridLayout->addWidget(angleRegisterBlock, 0, 1);
	registerSensorDataGridLayout->addWidget(angleSecRegisterBlock, 0, 2);
	registerSensorDataGridLayout->addWidget(sineRegisterBlock, 0, 3);
	registerSensorDataGridLayout->addWidget(cosineRegisterBlock, 0, 4);
	registerSensorDataGridLayout->addWidget(secAnglIRegisterBlock, 1, 0);
	registerSensorDataGridLayout->addWidget(secAnglQRegisterBlock, 1, 1);
	registerSensorDataGridLayout->addWidget(radiusRegisterBlock, 1, 2);
	registerSensorDataGridLayout->addWidget(diag1RegisterBlock, 1, 3);
	registerSensorDataGridLayout->addWidget(diag2RegisterBlock, 1, 4);
	registerSensorDataGridLayout->addWidget(tmp0RegisterBlock, 2, 0);
	registerSensorDataGridLayout->addWidget(tmp1RegisterBlock, 2, 1);
	registerSensorDataGridLayout->addWidget(cnvCntRegisterBlock, 2, 2);

	registerDeviceIDGridLayout->addWidget(uniqID0RegisterBlock, 0, 0);
	registerDeviceIDGridLayout->addWidget(uniqID1RegisterBlock, 0, 1);
	registerDeviceIDGridLayout->addWidget(uniqID2RegisterBlock, 0, 2);
	registerDeviceIDGridLayout->addWidget(uniqID3RegisterBlock, 0, 3);
	QSpacerItem *registerDeviceSpacer = new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Preferred);
	registerDeviceIDGridLayout->addItem(registerDeviceSpacer, 0, 4);

	registerHarmonicsGridLayout->addWidget(h1MagRegisterBlock, 0, 0);
	registerHarmonicsGridLayout->addWidget(h1PhRegisterBlock, 0, 1);
	registerHarmonicsGridLayout->addWidget(h2MagRegisterBlock, 0, 2);
	registerHarmonicsGridLayout->addWidget(h2PhRegisterBlock, 0, 3);
	registerHarmonicsGridLayout->addWidget(h3MagRegisterBlock, 0, 4);
	registerHarmonicsGridLayout->addWidget(h3PhRegisterBlock, 1, 0);
	registerHarmonicsGridLayout->addWidget(h8MagRegisterBlock, 1, 1);
	registerHarmonicsGridLayout->addWidget(h8PhRegisterBlock, 1, 2);
	

	// for(int c=0; c < registerGridLayout->columnCount(); ++c) registerGridLayout->setColumnStretch(c,1);
	// for(int r=0; r < registerGridLayout->rowCount(); ++r)  registerGridLayout->setRowStretch(r,1);
	for(int c=0; c < registerConfigurationGridLayout->columnCount(); ++c) registerConfigurationGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerDeviceIDGridLayout->columnCount(); ++c) registerDeviceIDGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerHarmonicsGridLayout->columnCount(); ++c) registerHarmonicsGridLayout->setColumnStretch(c,1);
	for(int c=0; c < registerSensorDataGridLayout->columnCount(); ++c) registerSensorDataGridLayout->setColumnStretch(c,1);

	// registerLayout->addWidget(registerGridWidget);
	registerLayout->addWidget(registerConfigurationLabel);
	registerLayout->addWidget(registerConfigurationGridWidget);
	registerLayout->addWidget(registerSensorDataLabel);
	registerLayout->addWidget(registerSensorDataGridWidget);
	registerLayout->addWidget(registerDeviceIDLabel);
	registerLayout->addWidget(registerDeviceIDGridWidget);
	registerLayout->addWidget(registerHarmonicsLabel);
	registerLayout->addWidget(registerHarmonicsGridWidget);
	registerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(false);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(false);
	tool->rightContainer()->setVisible(false);
	tool->bottomContainer()->setVisible(false);
	tool->setLeftContainerWidth(270);
	tool->setRightContainerWidth(270);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->addWidgetToCentralContainerHelper(registerScrollArea);

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
	MenuCollapseSection *commandLogCollapseSection = new MenuCollapseSection("Command Log", MenuCollapseSection::MHCW_NONE, commandLogSectionWidget);
	commandLogSectionWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	commandLogSectionWidget->contentLayout()->addWidget(commandLogCollapseSection);

	QPlainTextEdit *commandLogPlainTextEdit = new QPlainTextEdit(commandLogSectionWidget);
	commandLogPlainTextEdit->setReadOnly(true);
	commandLogPlainTextEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	commandLogCollapseSection->contentLayout()->addWidget(commandLogPlainTextEdit);

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
	// centerUtilityLayout->setAlignment(Qt::AlignTop);

	#pragma region DIGIO Monitor
	MenuSectionWidget *DIGIOMonitorSectionWidget = new MenuSectionWidget(centerUtilityWidget);
	MenuCollapseSection *DIGIOMonitorCollapseSection = new MenuCollapseSection("DIGIO Monitor", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, DIGIOMonitorSectionWidget);
	DIGIOMonitorSectionWidget->contentLayout()->addWidget(DIGIOMonitorCollapseSection);

	MenuControlButton *DIGIOBusyStatusLED = createStatusLEDWidget("BUSY", statusLEDColor, DIGIOMonitorCollapseSection);
	MenuControlButton *DIGIOCNVStatusLED = createStatusLEDWidget("CNV", statusLEDColor, DIGIOMonitorCollapseSection);
	MenuControlButton *DIGIOSENTStatusLED = createStatusLEDWidget("SENT", statusLEDColor, DIGIOMonitorCollapseSection);
	MenuControlButton *DIGIOACALCStatusLED = createStatusLEDWidget("ACALC", statusLEDColor, DIGIOMonitorCollapseSection);
	MenuControlButton *DIGIOFaultStatusLED = createStatusLEDWidget("FAULT", statusLEDColor, DIGIOMonitorCollapseSection);
	MenuControlButton *DIGIOBootloaderStatusLED = createStatusLEDWidget("BOOTLOADER", statusLEDColor, DIGIOMonitorCollapseSection);

	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOBusyStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOCNVStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOSENTStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOACALCStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOFaultStatusLED);
	DIGIOMonitorCollapseSection->contentLayout()->addWidget(DIGIOBootloaderStatusLED);
	#pragma endregion

	#pragma region MTDIAG1 Widget
	MenuSectionWidget *MTDIAG1SectionWidget = new MenuSectionWidget(centerUtilityWidget);
	MenuCollapseSection *MTDIAG1CollapseSection = new MenuCollapseSection("MT Diagnostic Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MTDIAG1SectionWidget);
	MTDIAG1SectionWidget->contentLayout()->addWidget(MTDIAG1CollapseSection);

	MenuControlButton *R0StatusLED = createStatusLEDWidget("R0", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R1StatusLED = createStatusLEDWidget("R1", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R2StatusLED = createStatusLEDWidget("R2", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R3StatusLED = createStatusLEDWidget("R3", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R4StatusLED = createStatusLEDWidget("R4", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R5StatusLED = createStatusLEDWidget("R5", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R6StatusLED = createStatusLEDWidget("R6", statusLEDColor, MTDIAG1SectionWidget);
	MenuControlButton *R7StatusLED = createStatusLEDWidget("R7", statusLEDColor, MTDIAG1SectionWidget);

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
	QScrollArea *MTDiagnosticsScrollArea = new QScrollArea(centerUtilityWidget);
	QWidget *MTDiagnosticsWidget = new QWidget(MTDiagnosticsScrollArea);
	MTDiagnosticsScrollArea->setWidget(MTDiagnosticsWidget);
	MTDiagnosticsScrollArea->setWidgetResizable(true);
	QVBoxLayout *MTDiagnosticsLayout = new QVBoxLayout(MTDiagnosticsWidget);
	MTDiagnosticsLayout->setMargin(0);
	MTDiagnosticsLayout->setSpacing(5);

	MenuSectionWidget *MTDiagnosticsSectionWidget = new MenuSectionWidget(centerUtilityWidget);
	MenuCollapseSection *MTDiagnosticsCollapseSection = new MenuCollapseSection("MT Diagnostics", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, MTDiagnosticsSectionWidget);
	MTDiagnosticsSectionWidget->contentLayout()->addWidget(MTDiagnosticsCollapseSection);
	MTDiagnosticsCollapseSection->contentLayout()->setSpacing(8);

	QLabel *AFEDIAG0Label = new QLabel("AFEDIAG0 (%)");
	StyleHelper::MenuSmallLabel(AFEDIAG0Label, "AFEDIAG0Label");
	QLabel *AFEDIAG1Label = new QLabel("AFEDIAG1 (%)");
	StyleHelper::MenuSmallLabel(AFEDIAG1Label, "AFEDIAG1Label");
	QLabel *AFEDIAG2Label = new QLabel("AFEDIAG2 (V)");
	StyleHelper::MenuSmallLabel(AFEDIAG2Label, "AFEDIAG2Label");

	QLineEdit *AFEDIAG0LineEdit = new QLineEdit("-57.0312", MTDiagnosticsSectionWidget);
	QLineEdit *AFEDIAG1LineEdit = new QLineEdit("56.25", MTDiagnosticsSectionWidget);
	QLineEdit *AFEDIAG2LineEdit = new QLineEdit("-312.499m", MTDiagnosticsSectionWidget);
	applyLineEditStyle(AFEDIAG0LineEdit);
	applyLineEditStyle(AFEDIAG1LineEdit);
	applyLineEditStyle(AFEDIAG2LineEdit);

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

	centerUtilityLayout->addWidget(DIGIOMonitorSectionWidget, 1, Qt::AlignTop);
	centerUtilityLayout->addWidget(MTDiagnosticsScrollArea);
	// centerUtilityLayout->addWidget(MTDIAG1SectionWidget, 0, Qt::AlignTop);
	// centerUtilityLayout->addWidget(MTDiagnosticsSectionWidget, 0, Qt::AlignTop);

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
	MenuCollapseSection *faultRegisterCollapseSection = new MenuCollapseSection("Fault Register", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, faultRegisterSectionWidget);
	faultRegisterSectionWidget->contentLayout()->addWidget(faultRegisterCollapseSection);
	
	MenuControlButton *vddUnderVoltageStatusLED = createStatusLEDWidget("VDD Under Voltage", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *vddOverVoltageStatusLED = createStatusLEDWidget("VDD Over Voltage", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *vDriveUnderVoltageStatusLED = createStatusLEDWidget("VDRIVE Under Voltage", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *vDriveOverVoltageStatusLED = createStatusLEDWidget("VDRIVE Over Voltage", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *AFEDIAGStatusLED = createStatusLEDWidget("AFEDIAG", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *NVMCRCFaultStatusLED = createStatusLEDWidget("NVM CRC Fault", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *ECCDoubleBitErrorStatusLED = createStatusLEDWidget("ECC Double Bit Error", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *OscillatorDriftStatusLED = createStatusLEDWidget("Oscillator Drift", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *CountSensorFalseStateStatusLED = createStatusLEDWidget("Count Sensor False State", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *AngleCrossCheckStatusLED = createStatusLEDWidget("Angle Cross Check", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *TurnCountSensorLevelsStatusLED = createStatusLEDWidget("Turn Count Sensor Levels", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *MTDIAGStatusLED = createStatusLEDWidget("MTDIAG", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *TurnCounterCrossCheckStatusLED = createStatusLEDWidget("Turn Counter Cross Check", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *RadiusCheckStatusLED = createStatusLEDWidget("Radius Check", faultLEDColor, faultRegisterCollapseSection);
	MenuControlButton *SequencerWatchdogStatusLED = createStatusLEDWidget("Sequencer Watchdog", faultLEDColor, faultRegisterCollapseSection);

	faultRegisterCollapseSection->contentLayout()->addWidget(vddUnderVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(vddOverVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(vDriveUnderVoltageStatusLED);
	faultRegisterCollapseSection->contentLayout()->addWidget(vDriveOverVoltageStatusLED);
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
	tool->setLeftContainerWidth(270);
	tool->setRightContainerWidth(270);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->leftStack()->addWidget(leftUtilityWidget);
	tool->addWidgetToCentralContainerHelper(centerUtilityWidget);
	tool->rightStack()->addWidget(rightUtilityScrollArea);

	return tool;
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
		runButton->setChecked(false);
		timer->stop();
	}
	else{
		timer->start(sampleRate);
	}

	updateGeneralSettingEnabled(!b);
}

void HarmonicCalibration::canCalibrate(bool value)
{
	calibrateDataButton->setEnabled(value);
	
}

void HarmonicCalibration::timerTask(){
	updateChannelValues();
	updateLineEditValues();

	dataGraph->plot(*dataGraphValue);
	tempGraph->plot(temp);
}

void HarmonicCalibration::updateChannelValues(){
	rotation = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), rotationChannelName, bufferSize);
	angle = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), angleChannelName, bufferSize);
	count = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), countChannelName, bufferSize);
	temp = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), temperatureChannelName, bufferSize);
}

void HarmonicCalibration::updateLineEditValues(){
	rotationValueLabel->setText(QString::number(rotation) + "°");
	angleValueLabel->setText(QString::number(angle) + "°");
	countValueLabel->setText(QString::number(count));
	tempValueLabel->setText(QString::number(temp) + " °C");
}

void HarmonicCalibration::updateGeneralSettingEnabled(bool value)
{
	graphUpdateIntervalLineEdit->setEnabled(value);
	dataSampleSizeLineEdit->setEnabled(value);
	dataGraphSamplesLineEdit->setEnabled(value);
	tempGraphSamplesLineEdit->setEnabled(value);
}

MenuControlButton *HarmonicCalibration::createStatusLEDWidget(const QString title, QColor color, QWidget *parent)
{
	MenuControlButton *menuControlButton = new MenuControlButton(parent);
	menuControlButton->setName(title);
	menuControlButton->setCheckBoxStyle(MenuControlButton::CheckboxStyle::CS_CIRCLE);
	menuControlButton->setOpenMenuChecksThis(true);
	menuControlButton->setDoubleClickToOpenMenu(true);
	menuControlButton->setColor(color);
	menuControlButton->button()->setVisible(false);
	menuControlButton->setCheckable(true);
	menuControlButton->checkBox()->setChecked(true);
	menuControlButton->setEnabled(false);
	menuControlButton->layout()->setMargin(8);
	return menuControlButton;
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

void HarmonicCalibration::connectLineEditToNumber(QLineEdit* lineEdit, int& variable)
{
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit]() {
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok) {
            variable = value;
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectLineEditToNumber(QLineEdit* lineEdit, double& variable)
{
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit]() {
        bool ok;
        double value = lineEdit->text().toDouble(&ok);
        if (ok) {
            variable = value;
			
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectLineEditToNumberWrite(QLineEdit* lineEdit, double& variable, ADMTController::MotorAttribute attribute)
{
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

void HarmonicCalibration::connectLineEditToGraphSamples(QLineEdit* lineEdit, int& variable, Sismograph* graph)
{
    connect(lineEdit, &QLineEdit::editingFinished, this, [&variable, lineEdit, graph]() {
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok) {
            variable = value;
			graph->setNumSamples(variable);
        } else {
            lineEdit->setText(QString::number(variable));
        }
    });
}

void HarmonicCalibration::connectMenuComboToGraphDirection(MenuCombo* menuCombo, Sismograph* graph)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [combo, graph]() {
		int value = qvariant_cast<int>(combo->currentData());
		switch(value)
		{
			case Sismograph::LEFT_TO_RIGHT:
				graph->setPlotDirection(Sismograph::LEFT_TO_RIGHT);
				graph->reset();
				break;
			case Sismograph::RIGHT_TO_LEFT:
				graph->setPlotDirection(Sismograph::RIGHT_TO_LEFT);
				graph->reset();
				break;
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

void HarmonicCalibration::changeGraphColorByChannelName(Sismograph* graph, const char* channelName)
{
	int index = m_admtController->getChannelIndex(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), channelName);
	if(index > -1){
		graph->setColor(StyleHelper::getColor( QString::fromStdString("CH" + std::to_string(index) )));
	}
}

void HarmonicCalibration::connectMenuComboToGraphChannel(MenuCombo* menuCombo, Sismograph* graph)
{
	QComboBox *combo = menuCombo->combo();
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo, graph]() {
		int currentIndex = combo->currentIndex();
		QVariant currentData = combo->currentData();
		char *value = reinterpret_cast<char*>(currentData.value<void*>());
		switch(currentIndex)
		{
			case ADMTController::Channel::ROTATION:
				dataGraphValue = &rotation;
				graph->setUnitOfMeasure("Degree", "°");
				graph->setAxisScale(QwtAxis::YLeft, -30.0, 390.0);
				graph->setNumSamples(dataGraphSamples);
				graph->setAxisTitle(QwtAxis::YLeft, tr("Degree (°)"));
				break;
			case ADMTController::Channel::ANGLE:
				dataGraphValue = &angle;
				graph->setUnitOfMeasure("Degree", "°");
				graph->setAxisScale(QwtAxis::YLeft, -30.0, 390.0);
				graph->setNumSamples(dataGraphSamples);
				graph->setAxisTitle(QwtAxis::YLeft, tr("Degree (°)"));
				break;
			case ADMTController::Channel::COUNT:
				dataGraphValue = &count;
				graph->setUnitOfMeasure("Count", "");
				graph->setAxisScale(QwtAxis::YLeft, -1.0, 20.0);
				graph->setNumSamples(dataGraphSamples);
				graph->setAxisTitle(QwtAxis::YLeft, tr("Count"));
				break;
		}
		changeGraphColorByChannelName(graph, value);
		graph->reset();
	});
}

void HarmonicCalibration::connectLineEditToRPSConversion(QLineEdit* lineEdit, double& vmax)
{
	connect(lineEdit, &QLineEdit::editingFinished, [=, &vmax]() {
        bool ok;
        double rps = lineEdit->text().toDouble(&ok);
        if (ok) {
            vmax = convertRPStoVMAX(rps);
			StatusBarManager::pushMessage("Converted VMAX: " + QString::number(vmax));
			writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, vmax);
			writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
			amax = convertAccelTimetoAMAX(motorAccelTimeSpinBox->lineEdit()->text().toDouble());
			StatusBarManager::pushMessage("Converted AMAX: " + QString::number(amax));
			writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
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
			StatusBarManager::pushMessage("Converted AMAX: " + QString::number(amax));
        } else {
            lineEdit->setText(QString::number(convertAMAXtoAccelTime(amax)));
        }
    });
}

void HarmonicCalibration::connectRegisterBlockToRegistry(RegisterBlockWidget* widget)
{
	uint32_t *readValue = new uint32_t;
	connect(widget->readButton(), &QPushButton::clicked, this, [=]{
		if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), readValue) == 0)
		{ widget->setValue(*readValue); }
	});
	if(widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::READWRITE || 
	   widget->getAccessPermission() == RegisterBlockWidget::ACCESS_PERMISSION::WRITE){
		connect(widget->writeButton(), &QPushButton::clicked, this, [=]{
			if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), widget->getValue()) == 0)
			{ 
				if(m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), widget->getAddress(), readValue) == 0)
				{ widget->setValue(*readValue); }
			}
		});
	}
}

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
	return (rotate_vmax * 131072 / accelTime / motorfCLK);
}

double HarmonicCalibration::convertAMAXtoAccelTime(double amax)
{
	return ((rotate_vmax * 131072) / (amax * motorfCLK));
}

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

void HarmonicCalibration::updateChannelValue(int channelIndex)
{
	switch(channelIndex)
	{
		case ADMTController::Channel::ROTATION:
			rotation = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), rotationChannelName, 1);
			break;
		case ADMTController::Channel::ANGLE:
			angle = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), angleChannelName, 1);
			break;
		case ADMTController::Channel::COUNT:
			count = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), countChannelName, 1);
			break;
		case ADMTController::Channel::TEMPERATURE:
			temp = m_admtController->getChannelValue(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), temperatureChannelName, 1);
			break;
	}
}

void HarmonicCalibration::readMotorAttributeValue(ADMTController::MotorAttribute attribute, double& value)
{
	if(!isDebug){
		int result = m_admtController->getDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240),
															m_admtController->getMotorAttribute(attribute),
															&value);
		if(result != 0) { calibrationLogWriteLn(QString(m_admtController->getMotorAttribute(attribute)) + ": Read Error " + QString::number(result)); }
	}
}

void HarmonicCalibration::writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value)
{
	if(!isDebug){
		int result = m_admtController->setDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240), 
															m_admtController->getMotorAttribute(attribute), 
															value);
		if(result != 0) { calibrationLogWriteLn(QString(m_admtController->getMotorAttribute(attribute)) + ": Write Error " + QString::number(result)); }
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

void HarmonicCalibration::calibrationTask()
{
	if(!isDebug){
		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
		updateLabelValue(calibrationMotorCurrentPositionLabel, ADMTController::MotorAttribute::CURRENT_POS);
	}
}

void HarmonicCalibration::motorCalibrationAcquisitionTask()
{
	if(startMotor && rawDataList.size() < totalSamplesCount){
		stepMotorAcquisition();
		updateChannelValue(ADMTController::Channel::ANGLE);
		double currentAngle = angle;
		QVector<double> angles = { currentAngle };
		appendSamplesToPlotCurve(calibrationRawDataPlotWidget, angles);
		rawDataList.push_back(currentAngle);
	}
	else if(rawDataList.size() == totalSamplesCount)
	{
		startMotor = false;
		calibrationStartMotorButton->setChecked(false);
	}
}

void HarmonicCalibration::appendSamplesToPlotCurve(PlotWidget *plotWidget, QVector<double>& newYData)
{
	const QwtSeriesData<QPointF> *seriesData = plotWidget->selectedChannel()->curve()->data();
	QVector<double> yData;

	if(seriesData != nullptr){
		for (int i = 0; i < seriesData->size(); ++i) {
			calibrationLogWriteLn("append: " + QString::number(seriesData->sample(i).y()));
			yData.append(seriesData->sample(i).y());
		}
	}

	yData.append(newYData);
	calibrationLogWriteLn("yData Size: " + QString::number(yData.size()));
	plotWidget->selectedChannel()->curve()->setSamples(yData);
	plotWidget->selectedChannel()->xAxis()->setMax(yData.size());
}

void HarmonicCalibration::addAngleToRawDataList()
{
	QVector<double> angles = { angle };
	appendSamplesToPlotCurve(calibrationRawDataPlotWidget, angles);
	rawDataList.push_back(angle);
}

void HarmonicCalibration::calibrateData()
{
	calibrationLogWrite("==== Calibration Start ====\n");

	calibrationLogWriteLn(m_admtController->calibrate(rawDataList, cycleCount, samplesPerCycle));

	uint32_t *h1MagCurrent = new uint32_t, 
			 *h1PhaseCurrent = new uint32_t, 
			 *h2MagCurrent = new uint32_t,
			 *h2PhaseCurrent = new uint32_t,
			 *h3MagCurrent = new uint32_t,
			 *h3PhaseCurrent = new uint32_t,
			 *h8MagCurrent = new uint32_t,
			 *h8PhaseCurrent = new uint32_t,
			 h1MagScaled,
			 h1PhaseScaled,
			 h2MagScaled,
			 h2PhaseScaled,
			 h3MagScaled,
			 h3PhaseScaled,
			 h8MagScaled,
			 h8PhaseScaled,
			 h1MagConverted,
			 h1PhaseConverted,
			 h2MagConverted,
			 h2PhaseConverted,
			 h3MagConverted,
			 h3PhaseConverted,
			 h8MagConverted,
			 h8PhaseConverted;

	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), h1MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), h1PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), h2MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), h2PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), h3MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), h3PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), h8MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), h8PhaseCurrent);

	calibrationLogWriteLn();
	calibrationLogWrite("H1 Mag Current: " + QString::number(*h1MagCurrent) + "\n");
	calibrationLogWrite("H1 Phase Current: " + QString::number(*h1PhaseCurrent) + "\n");
	calibrationLogWrite("H2 Mag Current: " + QString::number(*h2MagCurrent) + "\n");
	calibrationLogWrite("H2 Phase Current: " + QString::number(*h2PhaseCurrent) + "\n");
	calibrationLogWrite("H3 Mag Current: " + QString::number(*h3MagCurrent) + "\n");
	calibrationLogWrite("H3 Phase Current: " + QString::number(*h3PhaseCurrent) + "\n");
	calibrationLogWrite("H8 Mag Current: " + QString::number(*h8MagCurrent) + "\n");
	calibrationLogWrite("H8 Phase Current: " + QString::number(*h8PhaseCurrent) + "\n");

	h1MagScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(m_admtController->HAR_MAG_1, static_cast<uint16_t>(*h1MagCurrent), "h1"));
	h1PhaseScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(m_admtController->HAR_PHASE_1, static_cast<uint16_t>(*h1PhaseCurrent)));
	h2MagScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(m_admtController->HAR_MAG_2, static_cast<uint16_t>(*h2MagCurrent), "h2"));
	h2PhaseScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(m_admtController->HAR_PHASE_2, static_cast<uint16_t>(*h2PhaseCurrent)));
	h3MagScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(m_admtController->HAR_MAG_3, static_cast<uint16_t>(*h3MagCurrent), "h3"));
	h3PhaseScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(m_admtController->HAR_PHASE_3, static_cast<uint16_t>(*h3PhaseCurrent)));
	h8MagScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientMagnitude(m_admtController->HAR_MAG_8, static_cast<uint16_t>(*h8MagCurrent), "h8"));
	h8PhaseScaled = static_cast<uint32_t>(m_admtController->calculateHarmonicCoefficientPhase(m_admtController->HAR_PHASE_8, static_cast<uint16_t>(*h8PhaseCurrent)));

	calibrationLogWriteLn();
	calibrationLogWrite("H1 Mag Scaled: " + QString::number(h1MagScaled) + "\n");
	calibrationLogWrite("H1 Phase Scaled: " + QString::number(h1PhaseScaled) + "\n");
	calibrationLogWrite("H2 Mag Scaled: " + QString::number(h2MagScaled) + "\n");
	calibrationLogWrite("H2 Phase Scaled: " + QString::number(h2PhaseScaled) + "\n");
	calibrationLogWrite("H3 Mag Scaled: " + QString::number(h3MagScaled) + "\n");
	calibrationLogWrite("H3 Phase Scaled: " + QString::number(h3PhaseScaled) + "\n");
	calibrationLogWrite("H8 Mag Scaled: " + QString::number(h8MagScaled) + "\n");
	calibrationLogWrite("H8 Phase Scaled: " + QString::number(h8PhaseScaled) + "\n");

	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 0x01, 0x02);

	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG),
										  h1MagScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH),
										  h1PhaseScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG),
										  h2MagScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH),
										  h2PhaseScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG),
										  h3MagScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH),
										  h3PhaseScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG),
										  h8MagScaled);
	m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), 
										  m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH),
										  h8PhaseScaled);

	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1MAG), h1MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H1PH), h1PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2MAG), h2MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H2PH), h2PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3MAG), h3MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H3PH), h3PhaseCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8MAG), h8MagCurrent);
	m_admtController->readDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000), m_admtController->getHarmonicRegister(ADMTController::HarmonicRegister::H8PH), h8PhaseCurrent);

	h1MagConverted = m_admtController->readRegister(static_cast<uint16_t>(*h1MagCurrent), "h1mag");
	h1PhaseConverted = m_admtController->readRegister(static_cast<uint16_t>(*h1PhaseCurrent), "h1phase");
	h2MagConverted = m_admtController->readRegister(static_cast<uint16_t>(*h2MagCurrent), "h2mag");
	h2PhaseConverted = m_admtController->readRegister(static_cast<uint16_t>(*h2PhaseCurrent), "h2phase");
	h3MagConverted = m_admtController->readRegister(static_cast<uint16_t>(*h3MagCurrent), "h3mag");
	h3PhaseConverted = m_admtController->readRegister(static_cast<uint16_t>(*h3PhaseCurrent), "h3phase");
	h8MagConverted = m_admtController->readRegister(static_cast<uint16_t>(*h8MagCurrent), "h8mag");
	h8PhaseConverted = m_admtController->readRegister(static_cast<uint16_t>(*h8PhaseCurrent), "h8phase");

	calibrationLogWriteLn();
	calibrationLogWrite("H1 Mag Converted: " + QString::number(h1MagConverted) + "\n");
	calibrationLogWrite("H1 Phase Converted: " + QString::number(h1PhaseConverted) + "\n");
	calibrationLogWrite("H2 Mag Converted: " + QString::number(h2MagConverted) + "\n");
	calibrationLogWrite("H2 Phase Converted: " + QString::number(h2PhaseConverted) + "\n");
	calibrationLogWrite("H3 Mag Converted: " + QString::number(h3MagConverted) + "\n");
	calibrationLogWrite("H3 Phase Converted: " + QString::number(h3PhaseConverted) + "\n");
	calibrationLogWrite("H8 Mag Converted: " + QString::number(h8MagConverted) + "\n");
	calibrationLogWrite("H8 Phase Converted: " + QString::number(h8PhaseConverted));

	updateCalculatedCoeff();

	vector<double> calibrationAngleErrorsFFT = m_admtController->angle_errors_fft;
	vector<double> calibrationAngleErrorsFFTPhase = m_admtController->angle_errors_fft_phase;

	// Frequency axis (assuming sampling rate of 1 Hz for simplicity)
    std::vector<double> frequencyAxis(calibrationAngleErrorsFFT.size());
    for (size_t i = 0; i < frequencyAxis.size(); ++i)
    {
        frequencyAxis[i] = i; // Replace with actual frequency values if needed
    }

	calibrationFFTPlotChannel->curve()->setSamples(frequencyAxis.data(), calibrationAngleErrorsFFT.data(), (calibrationAngleErrorsFFT.size() / 2)); // divide size by 2 for now, will be half the size
	calibrationFFTPhasePlotChannel->curve()->setSamples(frequencyAxis.data(), calibrationAngleErrorsFFTPhase.data(), calibrationAngleErrorsFFTPhase.size());
	calibrationFFTXPlotAxis->setInterval(0, (calibrationAngleErrorsFFT.size() / 2)); // divide size by 2 for now, will be half the size
}

void HarmonicCalibration::updateCalculatedCoeff()
{
	calibrationH1MagLabel->setText(QString::number(m_admtController->HAR_MAG_1) + "°");
	calibrationH2MagLabel->setText(QString::number(m_admtController->HAR_MAG_2) + "°");
	calibrationH3MagLabel->setText(QString::number(m_admtController->HAR_MAG_3) + "°");
	calibrationH8MagLabel->setText(QString::number(m_admtController->HAR_MAG_8) + "°");
	calibrationH1PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_1));
	calibrationH2PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_2));
	calibrationH3PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_3));
	calibrationH8PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_8));
	applyCalibrationDataButton->setEnabled(true);
}

void HarmonicCalibration::resetCalculatedCoeff()
{
	calibrationH1MagLabel->setText("--.--°");
	calibrationH2MagLabel->setText("--.--°");
	calibrationH3MagLabel->setText("--.--°");
	calibrationH8MagLabel->setText("--.--°");
	calibrationH1PhaseLabel->setText("Φ --.--");
	calibrationH2PhaseLabel->setText("Φ --.--");
	calibrationH3PhaseLabel->setText("Φ --.--");
	calibrationH8PhaseLabel->setText("Φ --.--");
	applyCalibrationDataButton->setEnabled(false);
}

void HarmonicCalibration::registerCalibrationData()
{
	calibrationLogWrite("=== Apply Calibration ===\n");

	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h1MagDeviceRegister, m_admtController->HAR_MAG_1) != 0) 
		{ calibrationLogWriteLn("Failed to write to H1 Mag Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h2MagDeviceRegister, m_admtController->HAR_MAG_2) != 0)
		{ calibrationLogWriteLn("Failed to write to H2 Mag Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h3MagDeviceRegister, m_admtController->HAR_MAG_3) != 0)
		{ calibrationLogWriteLn("Failed to write to H3 Mag Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h8MagDeviceRegister, m_admtController->HAR_MAG_8) != 0)
		{ calibrationLogWriteLn("Failed to write to H8 Mag Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h1PhaseDeviceRegister, m_admtController->HAR_PHASE_1) != 0)
		{ calibrationLogWriteLn("Failed to write to H1 Phase Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h2PhaseDeviceRegister, m_admtController->HAR_PHASE_2) != 0)
		{ calibrationLogWriteLn("Failed to write to H2 Phase Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h3PhaseDeviceRegister, m_admtController->HAR_PHASE_3) != 0)
		{ calibrationLogWriteLn("Failed to write to H3 Phase Register"); }
	if(m_admtController->writeDeviceRegistry(m_admtController->getDeviceId(ADMTController::Device::ADMT4000),
		h8PhaseDeviceRegister, m_admtController->HAR_PHASE_8) != 0)
		{ calibrationLogWriteLn("Failed to write to H8 Phase Register"); }

	calibrationLogWrite("=== Calibration Complete ===\n");
}

void HarmonicCalibration::calibrationLogWrite(QString message)
{
	logsPlainTextEdit->appendPlainText(message);
}

void HarmonicCalibration::calibrationLogWriteLn(QString message)
{
	logsPlainTextEdit->appendPlainText("\n" + message);
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
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	if(!fileName.isEmpty()) {
		bool withScopyHeader = false;
		FileManager fm("HarmonicCalibration");
		fm.open(fileName, FileManager::EXPORT);

		QVector<double> rawData(rawDataList.begin(), rawDataList.end());

		QVector<double> angleErrorsFFT(m_admtController->angle_errors_fft.begin(), m_admtController->angle_errors_fft.end());
		QVector<double> angleErrorsFFTPhase(m_admtController->angle_errors_fft_phase.begin(), m_admtController->angle_errors_fft_phase.end());

		QVector<double> h1Mag = { static_cast<double>(m_admtController->HAR_MAG_1) };
		QVector<double> h2Mag = { static_cast<double>(m_admtController->HAR_MAG_2) };
		QVector<double> h3Mag = { static_cast<double>(m_admtController->HAR_MAG_3) };
		QVector<double> h8Mag = { static_cast<double>(m_admtController->HAR_MAG_8) };
		QVector<double> h1Phase = { static_cast<double>(m_admtController->HAR_PHASE_1) };
		QVector<double> h2Phase = { static_cast<double>(m_admtController->HAR_PHASE_2) };
		QVector<double> h3Phase = { static_cast<double>(m_admtController->HAR_PHASE_3) };
		QVector<double> h8Phase = { static_cast<double>(m_admtController->HAR_PHASE_8) };

		fm.save(rawData, "Raw Data");
		fm.save(angleErrorsFFT, "Angle Errors FFT");
		fm.save(angleErrorsFFTPhase, "Angle Errors FFT Phase");
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

		QVector<double> data = fm.read(0);
		if(data.size() > 0)
		{
			calibrationRawDataPlotChannel->curve()->setSamples(data.data(), data.size());
			calibrationRawDataXPlotAxis->setInterval(0, data.size());
			m_admtController->computeSineCosineOfAngles(vector<double>(data.begin(), data.end()));
			calibrationSineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_sine_scaled.data(), m_admtController->calibration_samples_sine_scaled.size());
			calibrationCosineDataPlotChannel->curve()->setSamples(m_admtController->calibration_samples_cosine_scaled.data(), m_admtController->calibration_samples_cosine_scaled.size());
			for(int i = 0; i < data.size(); ++i) {
				rawDataList.push_back(data[i]);
			}
			calibrationRawDataPlotWidget->replot();
		}
	} catch(FileManagerException &ex) {
		calibrationLogWriteLn(QString(ex.what()));
	}
}

void HarmonicCalibration::initializeMotor()
{
	amax = 1200;
	writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
	readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);

	rotate_vmax = 600000;
	writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
	readMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);

	writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);

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

void HarmonicCalibration::stepMotorAcquisition(double step)
{
	readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
	target_pos = current_pos + step;
	writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);

	while(target_pos != current_pos) {
		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, current_pos);
	}
}

void HarmonicCalibration::clearRawDataList()
{
	rawDataList.clear();

	calibrationRawDataPlotChannel->curve()->setData(nullptr);
	calibrationSineDataPlotChannel->curve()->setData(nullptr);
	calibrationCosineDataPlotChannel->curve()->setData(nullptr);
	calibrationRawDataPlotWidget->replot();

	calibrationFFTPlotChannel->curve()->setData(nullptr);
	calibrationFFTPhasePlotChannel->curve()->setData(nullptr);
	calibrationFFTDataPlotWidget->replot();

	resetCalculatedCoeff();
}

void HarmonicCalibration::applyLineEditStyle(QLineEdit *widget)
{
	applyTextStyle(widget);
	QString existingStyle = widget->styleSheet();
	QString style = QString(R"css(
								background-color: black;
								border-radius: 4px;
								border: none;
							)css");
	widget->setStyleSheet(existingStyle + style);
	widget->setFixedHeight(30);
	widget->setContentsMargins(0, 0, 0, 0);
	widget->setTextMargins(12, 4, 12, 4);
	widget->setAlignment(Qt::AlignRight);
}

void HarmonicCalibration::applyComboBoxStyle(QComboBox *widget, const QString& styleHelperColor)
{
	QString style = QString(R"css(
						QWidget {
						}
						QComboBox {
							text-align: right;
							color: &&colorname&&;
							border-radius: 4px;
							height: 30px;
							border-bottom: 0px solid none;
							padding-left: 12px;
							padding-right: 12px;
							font-weight: normal;
							font-size: 16px;
							background-color: black;
						}
						QComboBox:disabled, QLineEdit:disabled { color: #555555; }
						QComboBox QAbstractItemView {
							border: none;
							color: transparent;
							outline: none;
							background-color: black;
							border-bottom: 0px solid transparent;
 							border-top: 0px solid transparent;
						}
						QComboBox QAbstractItemView::item {
							text-align: right;
						}
						QComboBox::item:selected {
							font-weight: bold;
							font-size: 18px;
							background-color: transparent;
						}
						QComboBox::drop-down  {
							border-image: none;
							border: 0px;
							width: 16px;
 							height: 16px;
							margin-right: 12px;
						}
						QComboBox::down-arrow {
							image: url(:/admt/chevron-down-s.svg);
						}
						QComboBox::indicator {
							background-color: transparent;
							selection-background-color: transparent;
							color: transparent;
							selection-color: transparent;
						}
						)css");
	style = style.replace(QString("&&colorname&&"), StyleHelper::getColor(styleHelperColor));
	widget->setStyleSheet(style);
	widget->setFixedHeight(30);
}

void HarmonicCalibration::applyTextStyle(QWidget *widget, const QString& styleHelperColor, bool isBold)
{
	QString existingStyle = widget->styleSheet();
	QString style = QString(R"css(
								font-family: Open Sans;
								font-size: 16px;
								font-weight: &&fontweight&&;
								text-align: right;
								color: &&colorname&&;
							)css");
	style = style.replace(QString("&&colorname&&"), StyleHelper::getColor(styleHelperColor));
	QString fontWeight = QString("normal");
	if(isBold){
		fontWeight = QString("bold");
	}
	style = style.replace(QString("&&fontweight&&"), fontWeight);
	widget->setStyleSheet(existingStyle + style);
}

void HarmonicCalibration::applyLabelStyle(QLabel *widget)
{
	applyTextStyle(widget);
	QString existingStyle = widget->styleSheet();
	QString style = QString(R"css(
								background-color: black;
								border-radius: 4px;
								border: none;
							)css");
	widget->setStyleSheet(existingStyle + style);
	widget->setFixedHeight(30);
	widget->setContentsMargins(12, 4, 12, 4);
}

void HarmonicCalibration::applyTabWidgetStyle(QTabWidget *widget, const QString& styleHelperColor)
{
	QString style = QString(R"css(
		QTabWidget::tab-bar {
		 left: 5px; /* move to the right by 5px */
		}
		QTabBar::tab {
		 min-width: 100px;
		 min-height: 32px;
		 padding-bottom: 5px;
		 padding-left: 16px;
		 padding-right: 16px;
		 background-color: &&UIElementBackground&&;
		 font: normal;
		}
		QTabBar::tab:selected {
		 color: white;
		 border-bottom: 2px solid &&ScopyBlue&&;
		 margin-top: 0px;
		}
		)css");
	style.replace("&&ScopyBlue&&", StyleHelper::getColor(styleHelperColor));
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	widget->tabBar()->setStyleSheet(style);
}