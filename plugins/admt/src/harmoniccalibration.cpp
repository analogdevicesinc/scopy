#include "harmoniccalibration.h"
#include <widgets/horizontalspinbox.h>

#include <stylehelper.h>

static int sampleRate = 50;
static int calibrationRate = 20;
static int bufferSize = 1;
static int dataGraphSamples = 100;
static int tempGraphSamples = 100;
static bool running = false;
static double *dataGraphValue;

static int cycleCount = 1;
static int samplesPerCycle = 256;
static int totalSamplesCount = cycleCount * samplesPerCycle;
static bool startMotor = false;

static uint32_t h1MagDeviceRegister = 0x15;
static uint32_t h2MagDeviceRegister = 0x17;
static uint32_t h3MagDeviceRegister = 0x19;
static uint32_t h8MagDeviceRegister = 0x1B;
static uint32_t h1PhaseDeviceRegister = 0x16;
static uint32_t h2PhaseDeviceRegister = 0x18;
static uint32_t h3PhaseDeviceRegister = 0x1A;
static uint32_t h8PhaseDeviceRegister = 0x1C;

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
	QString tabStyle = QString("right: 0;");
	tabWidget->tabBar()->setStyleSheet(tabStyle);
	QString tabWidgetStyle = QString(R"css(
						QTabWidget::tab-bar {
							alignment: center;
						}
						)css");
	tabWidget->tabBar()->setStyleSheet(tabWidgetStyle);
	tabWidget->addTab(tool, "Acquisition");

    openLastMenuButton = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuButtonGroup = dynamic_cast<OpenLastMenuBtn *>(openLastMenuButton)->getButtonGroup();

    settingsButton = new GearBtn(this);
    // infoButton = new InfoBtn(this);

	// lay->insertWidget(0, infoButton);
	
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
    // tool->addWidgetToTopContainerHelper(infoButton, TTA_LEFT);
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

	tabWidget->addTab(createCalibrationWidget(), "Calibration");

	connect(tabWidget, &QTabWidget::currentChanged, [=](int index){
		tabWidget->setCurrentIndex(index);

		if(index == 1) { calibrationTimer->start(calibrationRate); }
		else { calibrationTimer->stop(); }
	});
}

HarmonicCalibration::~HarmonicCalibration() {}

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

ToolTemplate* HarmonicCalibration::createCalibrationWidget()
{
	initializeMotor();
	ToolTemplate *tool = new ToolTemplate(this);

	#pragma region Motor Attributes Widget
	QScrollArea *motorAttributesScroll = new QScrollArea();
	QWidget *motorAttributesWidget = new QWidget();
	QVBoxLayout *motorAttributesLayout = new QVBoxLayout(motorAttributesWidget);
	motorAttributesScroll->setWidgetResizable(true);
	motorAttributesScroll->setWidget(motorAttributesWidget);
	motorAttributesWidget->setLayout(motorAttributesLayout);
	
	// amax
	MenuSectionWidget *amaxSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *amaxCollapseSection = new MenuCollapseSection("amax", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, amaxSectionWidget);
	amaxSectionWidget->contentLayout()->addWidget(amaxCollapseSection);
	motorAmaxValueLabel = new QLabel("-", amaxSectionWidget);
	StyleHelper::MenuSmallLabel(motorAmaxValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, &amax);
	updateLabelValue(motorAmaxValueLabel, ADMTController::MotorAttribute::AMAX);
	QLineEdit *motorAmaxLineEdit = new QLineEdit(amaxSectionWidget);
	applyLineEditStyle(motorAmaxLineEdit);
	motorAmaxLineEdit->setText(QString::number(amax));
	connectLineEditToNumber(motorAmaxLineEdit, amax);
	QWidget *motorAmaxButtonGroupWidget = new QWidget(amaxSectionWidget);
	QHBoxLayout *motorAmaxButtonGroupLayout = new QHBoxLayout(motorAmaxButtonGroupWidget);
	motorAmaxButtonGroupWidget->setLayout(motorAmaxButtonGroupLayout);
	motorAmaxButtonGroupLayout->setMargin(0);
	motorAmaxButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorAmaxButton = new QPushButton("Read", motorAmaxButtonGroupWidget);
	StyleHelper::BlueButton(readMotorAmaxButton);
	connect(readMotorAmaxButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, &amax);
		updateLabelValue(motorAmaxValueLabel, ADMTController::MotorAttribute::AMAX);
	});
	QPushButton *writeMotorAmaxButton = new QPushButton("Write", motorAmaxButtonGroupWidget);
	StyleHelper::BlueButton(writeMotorAmaxButton);
	connect(writeMotorAmaxButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
		readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, &amax);
		updateLabelValue(motorAmaxValueLabel, ADMTController::MotorAttribute::AMAX);
	});
	motorAmaxButtonGroupLayout->addWidget(readMotorAmaxButton);
	motorAmaxButtonGroupLayout->addWidget(writeMotorAmaxButton);
	amaxCollapseSection->contentLayout()->setSpacing(10);
	amaxCollapseSection->contentLayout()->addWidget(motorAmaxValueLabel);
	amaxCollapseSection->contentLayout()->addWidget(motorAmaxLineEdit);
	amaxCollapseSection->contentLayout()->addWidget(motorAmaxButtonGroupWidget);

	//rotate_vmax
	MenuSectionWidget *rotateVmaxSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *rotateVmaxCollapseSection = new MenuCollapseSection("rotate_vmax", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, rotateVmaxSectionWidget);
	rotateVmaxSectionWidget->contentLayout()->addWidget(rotateVmaxCollapseSection);
	motorRotateVmaxValueLabel = new QLabel("-", rotateVmaxSectionWidget);
	StyleHelper::MenuSmallLabel(motorRotateVmaxValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, &rotate_vmax);
	updateLabelValue(motorRotateVmaxValueLabel, ADMTController::MotorAttribute::ROTATE_VMAX);
	QLineEdit *motorRotateVmaxLineEdit = new QLineEdit(rotateVmaxSectionWidget);
	applyLineEditStyle(motorRotateVmaxLineEdit);
	motorRotateVmaxLineEdit->setText(QString::number(rotate_vmax));
	connectLineEditToNumber(motorRotateVmaxLineEdit, rotate_vmax);
	QWidget *motorRotateVmaxButtonGroupWidget = new QWidget(rotateVmaxSectionWidget);
	QHBoxLayout *motorRotateVmaxButtonGroupLayout = new QHBoxLayout(motorRotateVmaxButtonGroupWidget);
	motorRotateVmaxButtonGroupWidget->setLayout(motorRotateVmaxButtonGroupLayout);
	motorRotateVmaxButtonGroupLayout->setMargin(0);
	motorRotateVmaxButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorRotateVmaxButton = new QPushButton("Read", motorRotateVmaxButtonGroupWidget);
	StyleHelper::BlueButton(readMotorRotateVmaxButton);
	connect(readMotorRotateVmaxButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, &rotate_vmax);
		updateLabelValue(motorRotateVmaxValueLabel, ADMTController::MotorAttribute::ROTATE_VMAX);
	});
	QPushButton *writeMotorRotateVmaxButton = new QPushButton("Write", motorRotateVmaxButtonGroupWidget);
	StyleHelper::BlueButton(writeMotorRotateVmaxButton);
	connect(writeMotorRotateVmaxButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
		readMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, &rotate_vmax);
		updateLabelValue(motorRotateVmaxValueLabel, ADMTController::MotorAttribute::ROTATE_VMAX);
	});
	motorRotateVmaxButtonGroupLayout->addWidget(readMotorRotateVmaxButton);
	motorRotateVmaxButtonGroupLayout->addWidget(writeMotorRotateVmaxButton);
	rotateVmaxCollapseSection->contentLayout()->setSpacing(10);
	rotateVmaxCollapseSection->contentLayout()->addWidget(motorRotateVmaxValueLabel);
	rotateVmaxCollapseSection->contentLayout()->addWidget(motorRotateVmaxLineEdit);
	rotateVmaxCollapseSection->contentLayout()->addWidget(motorRotateVmaxButtonGroupWidget);

	//dmax
	MenuSectionWidget *dmaxSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *dmaxCollapseSection = new MenuCollapseSection("dmax", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, dmaxSectionWidget);
	dmaxSectionWidget->contentLayout()->addWidget(dmaxCollapseSection);
	motorDmaxValueLabel = new QLabel("-", dmaxSectionWidget);
	StyleHelper::MenuSmallLabel(motorDmaxValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::DMAX, &dmax);
	updateLabelValue(motorDmaxValueLabel, ADMTController::MotorAttribute::DMAX);
	QLineEdit *motorDmaxLineEdit = new QLineEdit(dmaxSectionWidget);
	applyLineEditStyle(motorDmaxLineEdit);
	motorDmaxLineEdit->setText(QString::number(dmax));
	connectLineEditToNumber(motorDmaxLineEdit, dmax);
	QWidget *motorDmaxButtonGroupWidget = new QWidget(dmaxSectionWidget);
	QHBoxLayout *motorDmaxButtonGroupLayout = new QHBoxLayout(motorDmaxButtonGroupWidget);
	motorDmaxButtonGroupWidget->setLayout(motorDmaxButtonGroupLayout);
	motorDmaxButtonGroupLayout->setMargin(0);
	motorDmaxButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorDmaxButton = new QPushButton("Read", motorDmaxButtonGroupWidget);
	StyleHelper::BlueButton(readMotorDmaxButton);
	connect(readMotorDmaxButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::DMAX, &dmax);
		updateLabelValue(motorDmaxValueLabel, ADMTController::MotorAttribute::DMAX);
	});
	QPushButton *writeMotorDmaxButton = new QPushButton("Write", motorDmaxButtonGroupWidget);
	StyleHelper::BlueButton(writeMotorDmaxButton);
	connect(writeMotorDmaxButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::DMAX, dmax);
		readMotorAttributeValue(ADMTController::MotorAttribute::DMAX, &dmax);
		updateLabelValue(motorDmaxValueLabel, ADMTController::MotorAttribute::DMAX);
	});
	motorDmaxButtonGroupLayout->addWidget(readMotorDmaxButton);
	motorDmaxButtonGroupLayout->addWidget(writeMotorDmaxButton);
	dmaxCollapseSection->contentLayout()->setSpacing(10);
	dmaxCollapseSection->contentLayout()->addWidget(motorDmaxValueLabel);
	dmaxCollapseSection->contentLayout()->addWidget(motorDmaxLineEdit);
	dmaxCollapseSection->contentLayout()->addWidget(motorDmaxButtonGroupWidget);

	//disable
	MenuSectionWidget *disableSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *disableCollapseSection = new MenuCollapseSection("disable", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, disableSectionWidget);
	disableSectionWidget->contentLayout()->addWidget(disableCollapseSection);
	// motorDisableValueLabel = new QLabel("-", disableSectionWidget);
	// readMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, &disable);
	// updateLabelValue(motorDisableValueLabel, ADMTController::MotorAttribute::DISABLE);
	QPushButton *writeMotorDisableButton = new QPushButton("Disable", disableSectionWidget);
	StyleHelper::BlueButton(writeMotorDisableButton);
	connect(writeMotorDisableButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);
	});
	disableCollapseSection->contentLayout()->setSpacing(10);
	// disableCollapseSection->contentLayout()->addWidget(motorDisableValueLabel);
	// disableCollapseSection->contentLayout()->addWidget(motorDisableLineEdit);
	disableCollapseSection->contentLayout()->addWidget(writeMotorDisableButton);

	//target_pos
	MenuSectionWidget *targetPosSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *targetPosCollapseSection = new MenuCollapseSection("target_pos", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, targetPosSectionWidget);
	targetPosSectionWidget->contentLayout()->addWidget(targetPosCollapseSection);
	motorTargetPosValueLabel = new QLabel("-", targetPosSectionWidget);
	StyleHelper::MenuSmallLabel(motorTargetPosValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, &target_pos);
	updateLabelValue(motorTargetPosValueLabel, ADMTController::MotorAttribute::TARGET_POS);
	QLineEdit *motorTargetPosLineEdit = new QLineEdit(targetPosSectionWidget);
	applyLineEditStyle(motorTargetPosLineEdit);
	motorTargetPosLineEdit->setText(QString::number(target_pos));
	connectLineEditToNumber(motorTargetPosLineEdit, target_pos);
	QWidget *motorTargetPosButtonGroupWidget = new QWidget(targetPosSectionWidget);
	QHBoxLayout *motorTargetPosButtonGroupLayout = new QHBoxLayout(motorTargetPosButtonGroupWidget);
	motorTargetPosButtonGroupWidget->setLayout(motorTargetPosButtonGroupLayout);
	motorTargetPosButtonGroupLayout->setMargin(0);
	motorTargetPosButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorTargetPosButton = new QPushButton("Read", motorTargetPosButtonGroupWidget);
	StyleHelper::BlueButton(readMotorTargetPosButton);
	connect(readMotorTargetPosButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, &target_pos);
		updateLabelValue(motorTargetPosValueLabel, ADMTController::MotorAttribute::TARGET_POS);
	});
	QPushButton *writeMotorTargetPosButton = new QPushButton("Write", motorTargetPosButtonGroupWidget);
	StyleHelper::BlueButton(writeMotorTargetPosButton);
	connect(writeMotorTargetPosButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);
		readMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, &target_pos);
		updateLabelValue(motorTargetPosValueLabel, ADMTController::MotorAttribute::TARGET_POS);
	});
	motorTargetPosButtonGroupLayout->addWidget(readMotorTargetPosButton);
	motorTargetPosButtonGroupLayout->addWidget(writeMotorTargetPosButton);
	targetPosCollapseSection->contentLayout()->setSpacing(10);
	targetPosCollapseSection->contentLayout()->addWidget(motorTargetPosValueLabel);
	targetPosCollapseSection->contentLayout()->addWidget(motorTargetPosLineEdit);
	targetPosCollapseSection->contentLayout()->addWidget(motorTargetPosButtonGroupWidget);

	//current_pos
	MenuSectionWidget *currentPosSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *currentPosCollapseSection = new MenuCollapseSection("current_pos", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, currentPosSectionWidget);
	currentPosSectionWidget->contentLayout()->addWidget(currentPosCollapseSection);
	motorCurrentPosValueLabel = new QLabel("-", currentPosSectionWidget);
	StyleHelper::MenuSmallLabel(motorCurrentPosValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, &current_pos);
	updateLabelValue(motorCurrentPosValueLabel, ADMTController::MotorAttribute::CURRENT_POS);
	QWidget *motorCurrentPosButtonGroupWidget = new QWidget(currentPosSectionWidget);
	QHBoxLayout *motorCurrentPosButtonGroupLayout = new QHBoxLayout(motorCurrentPosButtonGroupWidget);
	motorCurrentPosButtonGroupWidget->setLayout(motorCurrentPosButtonGroupLayout);
	motorCurrentPosButtonGroupLayout->setMargin(0);
	motorCurrentPosButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorCurrentPosButton = new QPushButton("Read", motorCurrentPosButtonGroupWidget);
	StyleHelper::BlueButton(readMotorCurrentPosButton);
	connect(readMotorCurrentPosButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, &current_pos);
		updateLabelValue(motorCurrentPosValueLabel, ADMTController::MotorAttribute::CURRENT_POS);
	});
	motorCurrentPosButtonGroupLayout->addWidget(readMotorCurrentPosButton);
	currentPosCollapseSection->contentLayout()->setSpacing(10);
	currentPosCollapseSection->contentLayout()->addWidget(motorCurrentPosValueLabel);
	currentPosCollapseSection->contentLayout()->addWidget(motorCurrentPosButtonGroupWidget);

	//ramp_mode
	MenuSectionWidget *rampModeSectionWidget = new MenuSectionWidget(motorAttributesWidget);
	MenuCollapseSection *rampModeCollapseSection = new MenuCollapseSection("ramp_mode", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, rampModeSectionWidget);
	rampModeSectionWidget->contentLayout()->addWidget(rampModeCollapseSection);
	motorRampModeValueLabel = new QLabel("-", rampModeSectionWidget);
	StyleHelper::MenuSmallLabel(motorRampModeValueLabel);
	readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, &ramp_mode);
	updateLabelValue(motorRampModeValueLabel, ADMTController::MotorAttribute::RAMP_MODE);
	QLineEdit *motorRampModeLineEdit = new QLineEdit(rampModeSectionWidget);
	applyLineEditStyle(motorRampModeLineEdit);
	motorRampModeLineEdit->setText(QString::number(ramp_mode));
	connectLineEditToNumber(motorRampModeLineEdit, ramp_mode);
	QWidget *motorRampModeButtonGroupWidget = new QWidget(rampModeSectionWidget);
	QHBoxLayout *motorRampModeButtonGroupLayout = new QHBoxLayout(motorRampModeButtonGroupWidget);
	motorRampModeButtonGroupWidget->setLayout(motorRampModeButtonGroupLayout);
	motorRampModeButtonGroupLayout->setMargin(0);
	motorRampModeButtonGroupLayout->setSpacing(10);
	QPushButton *readMotorRampModeButton = new QPushButton("Read", motorRampModeButtonGroupWidget);
	StyleHelper::BlueButton(readMotorRampModeButton);
	connect(readMotorRampModeButton, &QPushButton::pressed, this, [=]{
		readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, &ramp_mode);
		updateLabelValue(motorRampModeValueLabel, ADMTController::MotorAttribute::RAMP_MODE);
	});
	QPushButton *writeMotorRampModeButton = new QPushButton("Write", motorRampModeButtonGroupWidget);
	StyleHelper::BlueButton(writeMotorRampModeButton);
	connect(writeMotorRampModeButton, &QPushButton::pressed, this, [=]{
		writeMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);
		readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, &ramp_mode);
		updateLabelValue(motorRampModeValueLabel, ADMTController::MotorAttribute::RAMP_MODE);
	});
	motorRampModeButtonGroupLayout->addWidget(readMotorRampModeButton);
	motorRampModeButtonGroupLayout->addWidget(writeMotorRampModeButton);
	rampModeCollapseSection->contentLayout()->setSpacing(10);
	rampModeCollapseSection->contentLayout()->addWidget(motorRampModeValueLabel);
	rampModeCollapseSection->contentLayout()->addWidget(motorRampModeLineEdit);
	rampModeCollapseSection->contentLayout()->addWidget(motorRampModeButtonGroupWidget);

	motorAttributesLayout->setMargin(0);
	motorAttributesLayout->setSpacing(10);
	motorAttributesLayout->addWidget(amaxSectionWidget);
	motorAttributesLayout->addWidget(rotateVmaxSectionWidget);
	motorAttributesLayout->addWidget(dmaxSectionWidget);
	motorAttributesLayout->addWidget(disableSectionWidget);
	motorAttributesLayout->addWidget(targetPosSectionWidget);
	motorAttributesLayout->addWidget(currentPosSectionWidget);
	motorAttributesLayout->addWidget(rampModeSectionWidget);
	motorAttributesLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	#pragma endregion

	#pragma region Calibration Data Graph Widget
	QWidget *calibrationDataGraphWidget = new QWidget();
	QVBoxLayout *calibrationDataGraphLayout = new QVBoxLayout(calibrationDataGraphWidget);
	calibrationDataGraphWidget->setLayout(calibrationDataGraphLayout);
	calibrationDataGraphLayout->setMargin(0);
	calibrationDataGraphLayout->setSpacing(10);

	QWidget *calibrationRawDataGraphWidget = new QWidget(calibrationDataGraphWidget);
	QVBoxLayout *calibrationRawDataGraphLayout = new QVBoxLayout(calibrationRawDataGraphWidget);
	calibrationRawDataGraphWidget->setLayout(calibrationRawDataGraphLayout);
	calibrationRawDataGraphLayout->setMargin(0);
	calibrationRawDataGraphLayout->setSpacing(4);
	QLabel *calibrationRawDataGraphLabel = new QLabel("Raw Data", calibrationRawDataGraphWidget);
	StyleHelper::MenuCollapseHeaderLabel(calibrationRawDataGraphLabel, "calibrationRawDataGraphLabel");
	calibrationRawDataPlotWidget = new Sismograph();
	// PlotWidget *calibrationRawDataPlotWidget = new PlotWidget();
	calibrationRawDataPlotWidget->setColor(StyleHelper::getColor("ScopyBlue"));
	calibrationRawDataPlotWidget->setPlotAxisXTitle("Degree (°)");
	calibrationRawDataPlotWidget->setUnitOfMeasure("Degree", "°");
	calibrationRawDataPlotWidget->setAutoscale(false);
	calibrationRawDataPlotWidget->setAxisScale(QwtAxis::YLeft, -30.0, 390.0);
	calibrationRawDataPlotWidget->setHistoryDuration(120.0);

	calibrationRawDataPlotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	calibrationRawDataGraphLayout->addWidget(calibrationRawDataGraphLabel);
	calibrationRawDataGraphLayout->addWidget(calibrationRawDataPlotWidget);

	// QWidget *calibrationFFTDataGraphWidget = new QWidget(calibrationDataGraphWidget);
	// QVBoxLayout *calibrationFFTDataGraphLayout = new QVBoxLayout(calibrationFFTDataGraphWidget);
	// calibrationFFTDataGraphWidget->setLayout(calibrationFFTDataGraphLayout);
	// calibrationFFTDataGraphLayout->setMargin(0);
	// calibrationFFTDataGraphLayout->setSpacing(4);
	// QLabel *calibrationFFTDataGraphLabel = new QLabel("FFT Data", calibrationFFTDataGraphWidget);
	// StyleHelper::MenuCollapseHeaderLabel(calibrationFFTDataGraphLabel, "calibrationFFTDataGraphLabel");
	// PlotWidget *calibrationFFTDataPlotWidget = new PlotWidget();
	// calibrationFFTDataPlotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	// calibrationFFTDataGraphLayout->addWidget(calibrationFFTDataGraphLabel);
	// calibrationFFTDataGraphLayout->addWidget(calibrationFFTDataPlotWidget);

	#pragma region Raw Data Section Widget
	QWidget *rawDataContainerWidget = new QWidget(calibrationDataGraphWidget);
	QHBoxLayout *rawDataContainerLayout = new QHBoxLayout(rawDataContainerWidget);
	rawDataContainerWidget->setLayout(rawDataContainerLayout);

	MenuSectionWidget *rawDataWidget = new MenuSectionWidget(rawDataContainerWidget);
	MenuCollapseSection *rawDataSection = new MenuCollapseSection("Raw Data", MenuCollapseSection::MHCW_NONE, calibrationDataGraphWidget);
	rawDataWidget->contentLayout()->setSpacing(10);
	rawDataWidget->contentLayout()->addWidget(rawDataSection);
	rawDataSection->contentLayout()->setSpacing(10);

	rawDataListWidget = new QListWidget(rawDataWidget);
	rawDataSection->contentLayout()->addWidget(rawDataListWidget);

	#pragma region Logs Section Widget
	MenuSectionWidget *logsSectionWidget = new MenuSectionWidget(rawDataContainerWidget);
	MenuCollapseSection *logsCollapseSection = new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE, logsSectionWidget);
	logsSectionWidget->contentLayout()->setSpacing(10);
	logsSectionWidget->contentLayout()->addWidget(logsCollapseSection);

	logsPlainTextEdit = new QPlainTextEdit(logsSectionWidget);
	logsPlainTextEdit->setReadOnly(true);

	logsCollapseSection->contentLayout()->setSpacing(10);
	logsCollapseSection->contentLayout()->addWidget(logsPlainTextEdit);
	#pragma endregion

	rawDataContainerLayout->setMargin(0);
	rawDataContainerLayout->setSpacing(10);
	rawDataContainerLayout->addWidget(rawDataWidget);
	rawDataContainerLayout->addWidget(logsSectionWidget);

	#pragma endregion

	calibrationDataGraphLayout->addWidget(calibrationRawDataGraphWidget);
	// calibrationDataGraphLayout->addWidget(calibrationFFTDataGraphWidget);
	calibrationDataGraphLayout->addWidget(rawDataContainerWidget);
	#pragma endregion
	
	#pragma region Calibration Settings Widget
	QScrollArea *calibrationSettingsScrollArea = new QScrollArea();
	QWidget *calibrationSettingsWidget = new QWidget(calibrationSettingsScrollArea);
	QVBoxLayout *calibrationSettingsLayout = new QVBoxLayout(calibrationSettingsWidget);
	calibrationSettingsScrollArea->setWidgetResizable(true);
	// calibrationSettingsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
	QPushButton *applyCalibrationDataButton = new QPushButton(calibrationCoeffSectionWidget);
	applyCalibrationDataButton->setText("Apply");
	StyleHelper::BlueButton(applyCalibrationDataButton, "applyCalibrationDataButton");

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

	#pragma region Calibration Data Section Widget
	MenuSectionWidget *calibrationDataSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *calibrationDataCollapseSection = new MenuCollapseSection("Calibration Data", MenuCollapseSection::MenuHeaderCollapseStyle::MHCW_NONE, calibrationDataSectionWidget);
	calibrationDataSectionWidget->contentLayout()->setSpacing(10);
	calibrationDataSectionWidget->contentLayout()->addWidget(calibrationDataCollapseSection);

	QPushButton *extractDataButton = new QPushButton(calibrationDataCollapseSection);
	extractDataButton->setText("Extract to CSV");
	StyleHelper::BlueButton(extractDataButton, "extractDataButton");
	QPushButton *importDataButton = new QPushButton(calibrationDataCollapseSection);
	importDataButton->setText("Import from CSV");
	StyleHelper::BlueButton(importDataButton, "importDataButton");

	calibrationDataCollapseSection->contentLayout()->setSpacing(10);
	calibrationDataCollapseSection->contentLayout()->addWidget(extractDataButton);
	calibrationDataCollapseSection->contentLayout()->addWidget(importDataButton);
	#pragma endregion

	#pragma region Motor Configuration Section Widget
	MenuSectionWidget *motorConfigurationSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *motorConfigurationCollapseSection = new MenuCollapseSection("Motor Configuration", MenuCollapseSection::MHCW_NONE, motorConfigurationSectionWidget);
	motorConfigurationSectionWidget->contentLayout()->addWidget(motorConfigurationCollapseSection);

	HorizontalSpinBox *motorMaxAccelerationSpinBox = new HorizontalSpinBox("Max Acceleration", 9999.99, "", motorConfigurationSectionWidget);
	HorizontalSpinBox *motorMaxVelocitySpinBox = new HorizontalSpinBox("Max Velocity", 9999.99, "rpm", motorConfigurationSectionWidget);
	HorizontalSpinBox *motorMaxDisplacementSpinBox = new HorizontalSpinBox("Max Displacement", 9999.99, "", motorConfigurationSectionWidget);

	MenuCombo *m_calibrationMotorRampModeMenuCombo = new MenuCombo("Ramp Mode", motorConfigurationSectionWidget);
	auto calibrationMotorRampModeCombo = m_calibrationMotorRampModeMenuCombo->combo();
	calibrationMotorRampModeCombo->addItem("Position", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(rotationChannelName))));
	calibrationMotorRampModeCombo->addItem("Angle", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(angleChannelName))));
	calibrationMotorRampModeCombo->addItem("Count", QVariant::fromValue(reinterpret_cast<void*>(const_cast<char*>(countChannelName))));
	applyComboBoxStyle(calibrationMotorRampModeCombo);

	motorConfigurationCollapseSection->contentLayout()->setSpacing(10);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxAccelerationSpinBox);
	motorConfigurationCollapseSection->contentLayout()->addWidget(motorMaxVelocitySpinBox);
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
	calibrationMotorCurrentPositionLabel = new QLabel("--.--°", motorControlSectionWidget);
	calibrationMotorCurrentPositionLabel->setAlignment(Qt::AlignRight);
	applyLabelStyle(calibrationMotorCurrentPositionLabel);

	HorizontalSpinBox *motorTargetPositionSpinBox = new HorizontalSpinBox("Target Position", 9999.99, "°", motorControlSectionWidget);
	HorizontalSpinBox *motorVelocitySpinBox = new HorizontalSpinBox("Velocity", 9999.99, "rpm", motorControlSectionWidget);

	calibrationStartMotorButton = new QPushButton(motorControlSectionWidget);
	calibrationStartMotorButton->setCheckable(true);
	calibrationStartMotorButton->setChecked(false);
	calibrationStartMotorButton->setText("Start Motor");
	calibrationStartMotorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	calibrationStartMotorButton->setFixedHeight(36);
	connect(calibrationStartMotorButton, &QPushButton::toggled, this, [=](bool b) { 
		calibrationStartMotorButton->setText(b ? " Stop Motor" : " Start Motor"); 
		startMotor = b;
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

	QCheckBox *autoCalibrateCheckBox = new QCheckBox("Auto Calibrate", motorControlSectionWidget);
	StyleHelper::BlueSquareCheckbox(autoCalibrateCheckBox, "autoCalibrateCheckBox");

	motorControlCollapseSection->contentLayout()->setSpacing(10);
	motorControlCollapseSection->contentLayout()->addWidget(currentPositionLabel);
	motorControlCollapseSection->contentLayout()->addWidget(calibrationMotorCurrentPositionLabel);
	motorControlCollapseSection->contentLayout()->addWidget(motorTargetPositionSpinBox);
	motorControlCollapseSection->contentLayout()->addWidget(motorVelocitySpinBox);
	motorControlCollapseSection->contentLayout()->addWidget(calibrationStartMotorButton);
	motorControlCollapseSection->contentLayout()->addWidget(autoCalibrateCheckBox);
	#pragma endregion

	#pragma region Debug Section Widget
	MenuSectionWidget *debugSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *debugCollapseSection = new MenuCollapseSection("Debug", MenuCollapseSection::MHCW_NONE, debugSectionWidget);
	debugSectionWidget->contentLayout()->setSpacing(10);
	debugSectionWidget->contentLayout()->addWidget(debugCollapseSection);

	QPushButton *addCalibrationDataButton = new QPushButton(debugSectionWidget);
	addCalibrationDataButton->setText("Add Data");
	StyleHelper::BlueButton(addCalibrationDataButton, "addCalibrationDataButton");

	QPushButton *removeLastCalibrationDataButton = new QPushButton(debugSectionWidget);
	removeLastCalibrationDataButton->setText("Remove Last Data");
	StyleHelper::BlueButton(removeLastCalibrationDataButton, "removeLastCalibrationDataButton");

	QPushButton *calibrateDataButton = new QPushButton(debugSectionWidget);
	calibrateDataButton->setText("Calibrate");
	StyleHelper::BlueButton(calibrateDataButton, "calibrateDataButton");

	QPushButton *clearCalibrateDataButton = new QPushButton(debugSectionWidget);
	clearCalibrateDataButton->setText("Clear All Data");
	StyleHelper::BlueButton(clearCalibrateDataButton, "clearCalibrateDataButton");

	debugCollapseSection->contentLayout()->setSpacing(10);
	debugCollapseSection->contentLayout()->addWidget(addCalibrationDataButton);
	debugCollapseSection->contentLayout()->addWidget(removeLastCalibrationDataButton);
	debugCollapseSection->contentLayout()->addWidget(calibrateDataButton);
	debugCollapseSection->contentLayout()->addWidget(clearCalibrateDataButton);
	#pragma endregion

	calibrationSettingsLayout->setMargin(0);
	calibrationSettingsLayout->addWidget(calibrationCoeffSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationDataSectionWidget);
	calibrationSettingsLayout->addWidget(motorConfigurationSectionWidget);
	calibrationSettingsLayout->addWidget(motorControlSectionWidget);
	calibrationSettingsLayout->addWidget(debugSectionWidget);
	calibrationSettingsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
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

	tool->leftStack()->add("motorAttributesScroll", motorAttributesScroll);
	tool->addWidgetToCentralContainerHelper(calibrationDataGraphWidget);
	tool->rightStack()->add("calibrationSettingsScrollArea", calibrationSettingsScrollArea);

	connect(addCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::addAngleToRawDataList);
	connect(removeLastCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::removeLastItemFromRawDataList);
	connect(calibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::calibrateData);
	connect(extractDataButton, &QPushButton::clicked, this, &HarmonicCalibration::extractCalibrationData);
	connect(importDataButton, &QPushButton::clicked, this, &HarmonicCalibration::importCalibrationData);
	connect(applyCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::registerCalibrationData);
	connect(clearCalibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::clearRawDataList);

	return tool;
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

void HarmonicCalibration::readMotorAttributeValue(ADMTController::MotorAttribute attribute, double *value)
{
	int result = m_admtController->getDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240), m_admtController->getMotorAttribute(attribute), value);
}

void HarmonicCalibration::writeMotorAttributeValue(ADMTController::MotorAttribute attribute, double value)
{
	int result = m_admtController->setDeviceAttributeValue(m_admtController->getDeviceId(ADMTController::Device::TMC5240), 
														   m_admtController->getMotorAttribute(attribute), 
														   value);
	if(result != 0) { calibrationLogWriteLn(QString(m_admtController->getMotorAttribute(attribute)) + ": Write Error " + QString::number(result)); }
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
	updateChannelValue(ADMTController::Channel::ANGLE);
	updateLabelValue(calibrationMotorCurrentPositionLabel, ADMTController::Channel::ANGLE);

	startMotorAcquisition();
}

void HarmonicCalibration::addAngleToRawDataList()
{
	QString dataStr = QString::number(angle);
	rawDataListWidget->addItem(dataStr);
}

void HarmonicCalibration::removeLastItemFromRawDataList(){
	rawDataListWidget->takeItem(rawDataListWidget->count() - 1);
}

void HarmonicCalibration::calibrateData()
{
	calibrationLogWrite("==== Calibration Start ====\n");
	QVector<double> rawData;

	for (int i = 0; i < rawDataListWidget->count(); ++i) {
		QListWidgetItem* item = rawDataListWidget->item(i);
		std::string text = item->text().toStdString();
		double value = std::stod(text);
		rawData.append(value);
	}
	std::vector<double> stdData(rawData.begin(), rawData.end());

	calibrationLogWriteLn(m_admtController->calibrate(stdData, cycleCount, samplesPerCycle));

	calibrationH1MagLabel->setText(QString::number(m_admtController->HAR_MAG_1) + "°");
	calibrationH2MagLabel->setText(QString::number(m_admtController->HAR_MAG_2) + "°");
	calibrationH3MagLabel->setText(QString::number(m_admtController->HAR_MAG_3) + "°");
	calibrationH8MagLabel->setText(QString::number(m_admtController->HAR_MAG_8) + "°");
	calibrationH1PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_1));
	calibrationH2PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_2));
	calibrationH3PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_3));
	calibrationH8PhaseLabel->setText("Φ " + QString::number(m_admtController->HAR_PHASE_8));
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

		QVector<double> rawData;

		for (int i = 0; i < rawDataListWidget->count(); ++i) {
			QListWidgetItem* item = rawDataListWidget->item(i);
			bool ok;
			double value = item->text().toDouble(&ok);
			if (ok) {
				rawData.append(value);
			} else {
				// Handle the case where the conversion fails if necessary
			}
		}

		fm.save(rawData, "RawData");

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

		rawDataListWidget->clear();

		QVector<double> data = fm.read(0);
		for(int i = 0; i < data.size(); ++i) {
			QString dataStr = QString::number(data[i]);
			rawDataListWidget->addItem(dataStr);
		}

	} catch(FileManagerException &ex) {
		calibrationLogWriteLn(QString(ex.what()));
	}
}

void HarmonicCalibration::initializeMotor()
{
	amax = 1200;
	writeMotorAttributeValue(ADMTController::MotorAttribute::AMAX, amax);
	readMotorAttributeValue(ADMTController::MotorAttribute::AMAX, &amax);

	rotate_vmax = 600000;
	writeMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, rotate_vmax);
	readMotorAttributeValue(ADMTController::MotorAttribute::ROTATE_VMAX, &rotate_vmax);

	writeMotorAttributeValue(ADMTController::MotorAttribute::DISABLE, 1);

	dmax = 3000;
	writeMotorAttributeValue(ADMTController::MotorAttribute::DMAX, dmax);
	readMotorAttributeValue(ADMTController::MotorAttribute::DMAX, &dmax);

	ramp_mode = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, ramp_mode);
	readMotorAttributeValue(ADMTController::MotorAttribute::RAMP_MODE, &ramp_mode);

	target_pos = 0;
	writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);
}

void HarmonicCalibration::startMotorAcquisition()
{
	if(startMotor && rawDataListWidget->count() < totalSamplesCount){
		double magNum = 408;

		readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, &current_pos);
		target_pos = current_pos - 408;
		writeMotorAttributeValue(ADMTController::MotorAttribute::TARGET_POS, target_pos);

		while(target_pos != current_pos) {
			readMotorAttributeValue(ADMTController::MotorAttribute::CURRENT_POS, &current_pos);
		}

		double currentAngle = angle;
		calibrationRawDataPlotWidget->plot(currentAngle);
		QString dataStr = QString::number(currentAngle);
		rawDataListWidget->addItem(dataStr);
		rawDataListWidget->scrollToBottom();
	}
	else if(rawDataListWidget->count() == totalSamplesCount)
	{
		calibrationStartMotorButton->setChecked(false);
	}
}

void HarmonicCalibration::clearRawDataList()
{
	rawDataListWidget->clear();
	calibrationRawDataPlotWidget->reset();
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