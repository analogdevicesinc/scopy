#include "harmoniccalibration.h"
#include <widgets/horizontalspinbox.h>

#include <stylehelper.h>

static int sampleRate = 50;
static int calibrationTimerRate = 100;
static int motorCalibrationAcquisitionTimerRate = 5;
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
	rotationWidget->contentLayout()->setSpacing(10);
	angleWidget->contentLayout()->setSpacing(10);
	countWidget->contentLayout()->setSpacing(10);
	tempWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *rotationSection = new MenuCollapseSection("Rotation", MenuCollapseSection::MHCW_NONE, rotationWidget);
	MenuCollapseSection *angleSection = new MenuCollapseSection("Angle", MenuCollapseSection::MHCW_NONE, angleWidget);
	MenuCollapseSection *countSection = new MenuCollapseSection("Count", MenuCollapseSection::MHCW_NONE, countWidget);
	MenuCollapseSection *tempSection = new MenuCollapseSection("Temperature", MenuCollapseSection::MHCW_NONE, tempWidget);
	rotationSection->contentLayout()->setSpacing(10);
	angleSection->contentLayout()->setSpacing(10);
	countSection->contentLayout()->setSpacing(10);
	tempSection->contentLayout()->setSpacing(10);

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
	generalWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *generalSection = new MenuCollapseSection("Data Acquisition", MenuCollapseSection::MHCW_NONE, generalWidget);
	generalSection->contentLayout()->setSpacing(10);
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

	// Data Graph Setting Widget
	MenuSectionWidget *dataGraphWidget = new MenuSectionWidget(generalSettingWidget);
	dataGraphWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *dataGraphSection = new MenuCollapseSection("Data Graph", MenuCollapseSection::MHCW_NONE, dataGraphWidget);
	dataGraphSection->contentLayout()->setSpacing(10);

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
	tempGraphWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *tempGraphSection = new MenuCollapseSection("Temperature Graph", MenuCollapseSection::MHCW_NONE, tempGraphWidget);
	tempGraphSection->contentLayout()->setSpacing(10);

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
	QVBoxLayout *calibrationDataGraphLayout = new QVBoxLayout(calibrationDataGraphWidget);
	calibrationDataGraphWidget->setLayout(calibrationDataGraphLayout);
	calibrationDataGraphLayout->setMargin(0);
	calibrationDataGraphLayout->setSpacing(10);

	MenuSectionWidget *calibrationDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	QTabWidget *calibrationDataGraphTabWidget = new QTabWidget(calibrationDataGraphSectionWidget);
	applyTabWidgetStyle(calibrationDataGraphTabWidget);
	calibrationDataGraphSectionWidget->contentLayout()->setSpacing(10);
	calibrationDataGraphSectionWidget->contentLayout()->addWidget(calibrationDataGraphTabWidget);

	// Raw Data Plot Widget
	calibrationRawDataPlotWidget = new PlotWidget();
	QPen calibrationRawDataPen = QPen(StyleHelper::getColor("ScopyBlue"));
	calibrationRawDataXPlotAxis = new PlotAxis(QwtAxis::XBottom, calibrationRawDataPlotWidget, calibrationRawDataPen);
	calibrationRawDataYPlotAxis = new PlotAxis(QwtAxis::YLeft, calibrationRawDataPlotWidget, calibrationRawDataPen);
	calibrationRawDataYPlotAxis->setInterval(0, 360);
	calibrationRawDataYPlotAxis->setUnits("°");
	calibrationRawDataYPlotAxis->setDivs(4);

	PrefixFormatter *calibrationRawDataFormatter = new PrefixFormatter({});
	calibrationRawDataFormatter->setTrimZeroes(true);
	calibrationRawDataFormatter->setTwoDecimalMode(false);
	calibrationRawDataXPlotAxis->setFormatter(calibrationRawDataFormatter);

	calibrationRawDataPlotChannel = new PlotChannel("Raw Data", calibrationRawDataPen, calibrationRawDataXPlotAxis, calibrationRawDataYPlotAxis);
	calibrationRawDataPlotChannel->setStyle(PlotChannel::PCS_DOTS);
	calibrationRawDataPlotWidget->addPlotChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotChannel->setEnabled(true);
	calibrationRawDataPlotWidget->selectChannel(calibrationRawDataPlotChannel);
	calibrationRawDataPlotWidget->replot();

	// Calibrated Plot Widget
	PlotWidget *calibrationCalibratedDataPlotWidget = new PlotWidget();

	calibrationDataGraphTabWidget->addTab(calibrationRawDataPlotWidget, "Raw");
	calibrationDataGraphTabWidget->addTab(calibrationCalibratedDataPlotWidget, "Calibrated");

	MenuSectionWidget *FFTDataGraphSectionWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	QTabWidget *FFTDataGraphTabWidget = new QTabWidget(FFTDataGraphSectionWidget);
	applyTabWidgetStyle(FFTDataGraphTabWidget);
	FFTDataGraphSectionWidget->contentLayout()->setSpacing(10);
	FFTDataGraphSectionWidget->contentLayout()->addWidget(FFTDataGraphTabWidget);

	// FFT Plot Widget
	calibrationFFTDataPlotWidget = new PlotWidget();
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

	FFTDataGraphTabWidget->addTab(calibrationFFTDataPlotWidget, "FFT");

	calibrationDataGraphLayout->addWidget(calibrationDataGraphSectionWidget);
	calibrationDataGraphLayout->addWidget(FFTDataGraphSectionWidget);
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

	calibrationCoeffSectionWidget->contentLayout()->setSpacing(10);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationDisplayFormatSwitch);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffLabel);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(calibrationCalculatedCoeffWidget);
	calibrationCoeffSectionWidget->contentLayout()->addWidget(applyCalibrationDataButton);
	#pragma endregion

	#pragma region Calibration Dataset Configuration
	MenuSectionWidget *calibrationDatasetConfigSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *calibrationDatasetConfigCollapseSection = new MenuCollapseSection("Dataset Configuration", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, calibrationDatasetConfigSectionWidget);
	calibrationDatasetConfigSectionWidget->contentLayout()->setSpacing(10);
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

	calibrationDatasetConfigCollapseSection->contentLayout()->setSpacing(10);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountLabel);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationCycleCountLineEdit);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationSamplesPerCycleLabel);
	calibrationDatasetConfigCollapseSection->contentLayout()->addWidget(calibrationSamplesPerCycleLineEdit);

	#pragma endregion

	#pragma region Calibration Data Section Widget
	MenuSectionWidget *calibrationDataSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *calibrationDataCollapseSection = new MenuCollapseSection("Calibration Data", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, calibrationDataSectionWidget);
	calibrationDataSectionWidget->contentLayout()->setSpacing(10);
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

	calibrationDataCollapseSection->contentLayout()->setSpacing(10);
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

	motorConfigurationCollapseSection->contentLayout()->setSpacing(10);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxVelocitySpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorAccelTimeSpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxDisplacementSpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(m_calibrationMotorRampModeMenuCombo);
	#pragma endregion

	#pragma region Motor Control Section Widget
	MenuSectionWidget *motorControlSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *motorControlCollapseSection = new MenuCollapseSection("Motor Control", MenuCollapseSection::MHCW_NONE, motorControlSectionWidget);
	motorControlSectionWidget->contentLayout()->setSpacing(10);
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

	motorControlCollapseSection->contentLayout()->setSpacing(10);
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
	logsSectionWidget->contentLayout()->setSpacing(10);
	logsSectionWidget->contentLayout()->addWidget(logsCollapseSection);

	logsPlainTextEdit = new QPlainTextEdit(logsSectionWidget);
	logsPlainTextEdit->setReadOnly(true);
	logsPlainTextEdit->setFixedHeight(320);

	logsCollapseSection->contentLayout()->setSpacing(10);
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

	// tool->leftStack()->add("motorAttributesScroll", motorAttributesScroll);
	tool->addWidgetToCentralContainerHelper(calibrationDataGraphWidget);
	tool->rightStack()->add("calibrationSettingsScrollArea", calibrationSettingsScrollArea);

	connect(calibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::calibrateData);
	connect(extractDataButton, &QPushButton::clicked, this, &HarmonicCalibration::extractCalibrationData);
	connect(importDataButton, &QPushButton::clicked, this, &HarmonicCalibration::importCalibrationData);
	connect(applyCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::registerCalibrationData);
	connect(clearCalibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::clearRawDataList);
	connectLineEditToRPSConversion(motorMaxVelocitySpinBox->lineEdit(), rotate_vmax);
	connectLineEditToAMAXConversion(motorAccelTimeSpinBox->lineEdit(), amax);
	connectLineEditToNumber(motorMaxDisplacementSpinBox->lineEdit(), dmax);
	connectLineEditToNumber(motorTargetPositionSpinBox->lineEdit(), target_pos);
	connectMenuComboToNumber(m_calibrationMotorRampModeMenuCombo, ramp_mode);
	connect(autoCalibrateCheckBox, &QCheckBox::toggled, [=](bool toggled){ 
		autoCalibrate = toggled; 
		StatusBarManager::pushMessage(QString("Auto Calibrate: ") + QString((toggled ? "True" : "False")));
	});

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
		calibrationRawDataPlotChannel->curve()->setSamples(data.data(), data.size());
		calibrationRawDataXPlotAxis->setInterval(0, data.size());
		for(int i = 0; i < data.size(); ++i) {
			rawDataList.push_back(data[i]);
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