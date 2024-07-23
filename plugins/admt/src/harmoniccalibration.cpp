#include "harmoniccalibration.h"

#include <stylehelper.h>

static int sampleRate = 50;
static int bufferSize = 1;
static int dataGraphSamples = 100;
static int tempGraphSamples = 100;
static bool running = false;
static double *dataGraphValue;

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
	tabWidget->addTab(tool, "Acquisition");
	lay->insertWidget(0, tabWidget);

    openLastMenuButton = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuButtonGroup = dynamic_cast<OpenLastMenuBtn *>(openLastMenuButton)->getButtonGroup();

    settingsButton = new GearBtn(this);
    infoButton = new InfoBtn(this);
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
	dataGraph->setNumSamples(dataGraphSamples);
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
	graphUpdateIntervalLineEdit->setText(QString::number(sampleRate));

	connectLineEditToNumber(graphUpdateIntervalLineEdit, sampleRate);

	generalSection->contentLayout()->addWidget(graphUpdateIntervalLabel);
	generalSection->contentLayout()->addWidget(graphUpdateIntervalLineEdit);

	// Data Sample Size
	QLabel *dataSampleSizeLabel = new QLabel(generalSection);
	dataSampleSizeLabel->setText("Data Sample Size");
	StyleHelper::MenuSmallLabel(dataSampleSizeLabel, "dataSampleSizeLabel");
	dataSampleSizeLineEdit = new QLineEdit(generalSection);
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

	connectMenuComboToGraphChannel(m_dataGraphChannelMenuCombo, dataGraph);

	dataGraphSection->contentLayout()->addWidget(m_dataGraphChannelMenuCombo);

	// Graph Samples
	QLabel *dataGraphSamplesLabel = new QLabel(generalSection);
	dataGraphSamplesLabel->setText("Samples");
	StyleHelper::MenuSmallLabel(dataGraphSamplesLabel, "dataGraphSamplesLabel");
	dataGraphSamplesLineEdit = new QLineEdit(generalSection);
	dataGraphSamplesLineEdit->setText(QString::number(dataGraphSamples));

	connectLineEditToGraphSamples(dataGraphSamplesLineEdit, dataGraphSamples, dataGraph);
	
	dataGraphSection->contentLayout()->addWidget(dataGraphSamplesLabel);
	dataGraphSection->contentLayout()->addWidget(dataGraphSamplesLineEdit);

	// Graph Direction
	m_dataGraphDirectionMenuCombo = new MenuCombo("Direction", dataGraphSection);
	auto dataGraphDirectionCombo = m_dataGraphDirectionMenuCombo->combo();
	dataGraphDirectionCombo->addItem("Left to right", Sismograph::LEFT_TO_RIGHT);
	dataGraphDirectionCombo->addItem("Right to left", Sismograph::RIGHT_TO_LEFT);
	dataGraphSection->contentLayout()->addWidget(m_dataGraphDirectionMenuCombo);

	dataGraphWidget->contentLayout()->addWidget(dataGraphSection);

	connectMenuComboToGraphDirection(m_dataGraphDirectionMenuCombo, dataGraph);

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
	tempGraphSamplesLineEdit->setText(QString::number(tempGraphSamples));
	tempGraphSection->contentLayout()->addWidget(tempGraphSamplesLabel);
	tempGraphSection->contentLayout()->addWidget(tempGraphSamplesLineEdit);

	connectLineEditToGraphSamples(tempGraphSamplesLineEdit, tempGraphSamples, tempGraph);

	// Graph Direction
	m_tempGraphDirectionMenuCombo = new MenuCombo("Direction", tempGraphSection);
	auto tempGraphDirectionCombo = m_tempGraphDirectionMenuCombo->combo();
	tempGraphDirectionCombo->addItem("Left to right", Sismograph::LEFT_TO_RIGHT);
	tempGraphDirectionCombo->addItem("Right to left", Sismograph::RIGHT_TO_LEFT);
	tempGraphSection->contentLayout()->addWidget(m_tempGraphDirectionMenuCombo);
	tempGraphWidget->contentLayout()->addWidget(tempGraphSection);

	connectMenuComboToGraphDirection(m_tempGraphDirectionMenuCombo, tempGraph);

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
    tool->addWidgetToTopContainerHelper(infoButton, TTA_LEFT);
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

		if(index == 1) { calibrationTimer->start(sampleRate); }
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
	rotation = m_admtController->getChannelValue(rotationChannelName, bufferSize);
	angle = m_admtController->getChannelValue(angleChannelName, bufferSize);
	count = m_admtController->getChannelValue(countChannelName, bufferSize);
	temp = m_admtController->getChannelValue(temperatureChannelName, bufferSize);
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
	m_dataGraphDirectionMenuCombo->setEnabled(value);
	m_tempGraphDirectionMenuCombo->setEnabled(value);
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
	int index = m_admtController->getChannelIndex(channelName);
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
	ToolTemplate *tool = new ToolTemplate(this);
	
	QScrollArea *calibrationControlScrollArea = new QScrollArea(this);
	calibrationControlScrollArea->setWidgetResizable(true);
	QWidget *controlWidget = new QWidget(calibrationControlScrollArea);
	calibrationControlScrollArea->setWidget(controlWidget);
	QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
	controlLayout->setMargin(0);
	controlWidget->setLayout(controlLayout);

	// Angle Widget
	MenuSectionWidget *angleWidget = new MenuSectionWidget(controlWidget);
	angleWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *angleSection = new MenuCollapseSection("Angle", MenuCollapseSection::MHCW_NONE, angleWidget);
	angleSection->contentLayout()->setSpacing(10);
	angleWidget->contentLayout()->addWidget(angleSection);
	calibrationAngleLabel = new QLabel(angleSection);
	StyleHelper::MenuControlLabel(calibrationAngleLabel, "calibrationAngleLabel");
	angleSection->contentLayout()->addWidget(calibrationAngleLabel);

	// Calibration Widget
	MenuSectionWidget *calibrationControlWidget = new MenuSectionWidget(controlWidget);
	calibrationControlWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *calibrationSection = new MenuCollapseSection("Calibration", MenuCollapseSection::MHCW_NONE, calibrationControlWidget);
	calibrationSection->contentLayout()->setSpacing(10);
	calibrationControlWidget->contentLayout()->addWidget(calibrationSection);

	QPushButton *addCalibrationDataButton = new QPushButton(calibrationControlWidget);
	addCalibrationDataButton->setText("Add Data");
	StyleHelper::BlueButton(addCalibrationDataButton, "addCalibrationDataButton");

	QPushButton *removeLastCalibrationDataButton = new QPushButton(calibrationControlWidget);
	removeLastCalibrationDataButton->setText("Remove Last Data");
	StyleHelper::BlueButton(removeLastCalibrationDataButton, "removeLastCalibrationDataButton");

	QPushButton *calibrateDataButton = new QPushButton(calibrationControlWidget);
	calibrateDataButton->setText("Calibrate");
	StyleHelper::BlueButton(calibrateDataButton, "calibrateDataButton");

	QPushButton *extractDataButton = new QPushButton(calibrationControlWidget);
	extractDataButton->setText("Extract");
	StyleHelper::BlueButton(extractDataButton, "extractDataButton");

	QPushButton *importDataButton = new QPushButton(calibrationControlWidget);
	importDataButton->setText("Import");
	StyleHelper::BlueButton(importDataButton, "importDataButton");

	calibrationSection->contentLayout()->addWidget(addCalibrationDataButton);
	calibrationSection->contentLayout()->addWidget(removeLastCalibrationDataButton);
	calibrationSection->contentLayout()->addWidget(calibrateDataButton);
	calibrationSection->contentLayout()->addWidget(extractDataButton);
	calibrationSection->contentLayout()->addWidget(importDataButton);

	controlLayout->addWidget(angleWidget);
	controlLayout->addWidget(calibrationControlWidget);
	controlLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Raw Data Widget
	QWidget *calibrationDataWidget = new QWidget(this);
	QVBoxLayout *calibrationDataLayout = new QVBoxLayout(calibrationDataWidget);
	calibrationDataLayout->setMargin(0);
	calibrationDataWidget->setLayout(calibrationDataLayout);

	MenuSectionWidget *rawDataWidget = new MenuSectionWidget(calibrationDataWidget);
	rawDataWidget->contentLayout()->setSpacing(10);
	rawDataWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	MenuCollapseSection *rawDataSection = new MenuCollapseSection("Raw Data", MenuCollapseSection::MHCW_NONE, calibrationDataWidget);
	rawDataSection->contentLayout()->setSpacing(10);
	rawDataWidget->contentLayout()->addWidget(rawDataSection);

	rawDataListWidget = new QListWidget(rawDataWidget);
	rawDataSection->contentLayout()->addWidget(rawDataListWidget);

	// Logs Widget
	MenuSectionWidget *logsWidget = new MenuSectionWidget(calibrationDataWidget);
	logsWidget->contentLayout()->setSpacing(10);
	MenuCollapseSection *logsSection = new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE, logsWidget);
	logsSection->contentLayout()->setSpacing(10);
	logsWidget->contentLayout()->addWidget(logsSection);

	logsPlainTextEdit = new QPlainTextEdit(logsWidget);
	logsPlainTextEdit->setReadOnly(true);

	logsSection->contentLayout()->addWidget(logsPlainTextEdit);

	calibrationDataLayout->addWidget(rawDataWidget);
	calibrationDataLayout->addWidget(logsWidget);

	// Result Widget
	QWidget *calibrationResultWidget = new QWidget(this);
	QVBoxLayout *calibrationResultLayout = new QVBoxLayout(calibrationResultWidget);
	calibrationResultLayout->setMargin(0);
	calibrationResultWidget->setLayout(calibrationResultLayout);

	// Register Widget
	QWidget *calibrationRegisterWidget = new QWidget(calibrationResultWidget);
	QHBoxLayout *calibrationRegisterLayout = new QHBoxLayout(calibrationRegisterWidget);
	calibrationRegisterLayout->setMargin(0);
	calibrationRegisterLayout->setSpacing(10);
	calibrationRegisterWidget->setLayout(calibrationRegisterLayout);

	QWidget *calibrationMagWidget = new QWidget(calibrationRegisterWidget);
	QVBoxLayout *calibrationMagLayout = new QVBoxLayout(calibrationMagWidget);
	calibrationMagLayout->setMargin(0);
	calibrationMagLayout->setSpacing(10);
	calibrationMagWidget->setLayout(calibrationMagLayout);

	QLabel *calibrationH1MagLabel = new QLabel(calibrationMagWidget);
	calibrationH1MagLabel->setText("H1Mag");
	StyleHelper::MenuSmallLabel(calibrationH1MagLabel, "calibrationH1MagLabel");
	calibrationH1MagLineEdit = new QLineEdit(calibrationMagWidget);

	QLabel *calibrationH2MagLabel = new QLabel(calibrationMagWidget);
	calibrationH2MagLabel->setText("H2Mag");
	StyleHelper::MenuSmallLabel(calibrationH2MagLabel, "calibrationH2MagLabel");
	calibrationH2MagLineEdit = new QLineEdit(calibrationMagWidget);

	QLabel *calibrationH3MagLabel = new QLabel(calibrationMagWidget);
	calibrationH3MagLabel->setText("H3Mag");
	StyleHelper::MenuSmallLabel(calibrationH3MagLabel, "calibrationH3MagLabel");
	calibrationH3MagLineEdit = new QLineEdit(calibrationMagWidget);

	QLabel *calibrationH8MagLabel = new QLabel(calibrationMagWidget);
	calibrationH8MagLabel->setText("H8Mag");
	StyleHelper::MenuSmallLabel(calibrationH8MagLabel, "calibrationH8MagLabel");
	calibrationH8MagLineEdit = new QLineEdit(calibrationMagWidget);

	calibrationMagLayout->addWidget(calibrationH1MagLabel);
	calibrationMagLayout->addWidget(calibrationH1MagLineEdit);
	calibrationMagLayout->addWidget(calibrationH2MagLabel);
	calibrationMagLayout->addWidget(calibrationH2MagLineEdit);
	calibrationMagLayout->addWidget(calibrationH3MagLabel);
	calibrationMagLayout->addWidget(calibrationH3MagLineEdit);
	calibrationMagLayout->addWidget(calibrationH8MagLabel);
	calibrationMagLayout->addWidget(calibrationH8MagLineEdit);

	QWidget *calibrationPhaseWidget = new QWidget(calibrationRegisterWidget);
	QVBoxLayout *calibrationPhaseLayout = new QVBoxLayout(calibrationPhaseWidget);
	calibrationPhaseLayout->setMargin(0);
	calibrationPhaseLayout->setSpacing(10);
	calibrationPhaseWidget->setLayout(calibrationPhaseLayout);

	QLabel *calibrationH1PhaseLabel = new QLabel(calibrationPhaseWidget);
	calibrationH1PhaseLabel->setText("H1Phase");
	StyleHelper::MenuSmallLabel(calibrationH1PhaseLabel, "calibrationH1PhaseLabel");
	calibrationH1PhaseLineEdit = new QLineEdit(calibrationPhaseWidget);

	QLabel *calibrationH2PhaseLabel = new QLabel(calibrationPhaseWidget);
	calibrationH2PhaseLabel->setText("H2Phase");
	StyleHelper::MenuSmallLabel(calibrationH2PhaseLabel, "calibrationH2PhaseLabel");
	calibrationH2PhaseLineEdit = new QLineEdit(calibrationPhaseWidget);

	QLabel *calibrationH3PhaseLabel = new QLabel(calibrationPhaseWidget);
	calibrationH3PhaseLabel->setText("H3Phase");
	StyleHelper::MenuSmallLabel(calibrationH3PhaseLabel, "calibrationH3PhaseLabel");
	calibrationH3PhaseLineEdit = new QLineEdit(calibrationPhaseWidget);

	QLabel *calibrationH8PhaseLabel = new QLabel(calibrationPhaseWidget);
	calibrationH8PhaseLabel->setText("H8Phase");
	StyleHelper::MenuSmallLabel(calibrationH8PhaseLabel, "calibrationH8PhaseLabel");
	calibrationH8PhaseLineEdit = new QLineEdit(calibrationPhaseWidget);

	calibrationPhaseLayout->addWidget(calibrationH1PhaseLabel);
	calibrationPhaseLayout->addWidget(calibrationH1PhaseLineEdit);
	calibrationPhaseLayout->addWidget(calibrationH2PhaseLabel);
	calibrationPhaseLayout->addWidget(calibrationH2PhaseLineEdit);
	calibrationPhaseLayout->addWidget(calibrationH3PhaseLabel);
	calibrationPhaseLayout->addWidget(calibrationH3PhaseLineEdit);
	calibrationPhaseLayout->addWidget(calibrationH8PhaseLabel);
	calibrationPhaseLayout->addWidget(calibrationH8PhaseLineEdit);

	QPushButton *applyCalibrationDataButton = new QPushButton(calibrationRegisterWidget);
	applyCalibrationDataButton->setText("Apply");
	StyleHelper::BlueButton(applyCalibrationDataButton, "applyCalibrationDataButton");

	calibrationRegisterLayout->addWidget(calibrationMagWidget);
	calibrationRegisterLayout->addWidget(calibrationPhaseWidget);

	calibrationResultLayout->addWidget(calibrationRegisterWidget);
	calibrationResultLayout->addWidget(applyCalibrationDataButton);

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(false);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);
    tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(500);
	tool->setTopContainerHeight(0);
	tool->setBottomContainerHeight(90);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->leftStack()->add("calibrationControlScrollArea", calibrationControlScrollArea);
	tool->addWidgetToCentralContainerHelper(calibrationDataWidget);
	tool->rightStack()->add("calibrationResultWidget", calibrationResultWidget);

	connect(addCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::addAngleToRawDataList);
	connect(removeLastCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::removeLastItemFromRawDataList);
	connect(calibrateDataButton, &QPushButton::clicked, this, &HarmonicCalibration::calibrateData);
	connect(extractDataButton, &QPushButton::clicked, this, &HarmonicCalibration::extractCalibrationData);
	connect(importDataButton, &QPushButton::clicked, this, &HarmonicCalibration::importCalibrationData);
	connect(applyCalibrationDataButton, &QPushButton::clicked, this, &HarmonicCalibration::registerCalibrationData);

	return tool;
}

void HarmonicCalibration::updateLabelValue(QLabel* label, int channelIndex)
{
	switch(channelIndex)
	{
		case ADMTController::Channel::ROTATION:
			label->setText(QString::number(rotation) + "°");
			break;
		case ADMTController::Channel::ANGLE:
			label->setText(QString::number(angle) + "°");
			break;
		case ADMTController::Channel::COUNT:
			label->setText(QString::number(count));
			break;
		case ADMTController::Channel::TEMPERATURE:
			label->setText(QString::number(temp) + "°C");
			break;
	}
}

void HarmonicCalibration::updateChannelValue(int channelIndex)
{
	switch(channelIndex)
	{
		case ADMTController::Channel::ROTATION:
			rotation = m_admtController->getChannelValue(rotationChannelName, 1);
			break;
		case ADMTController::Channel::ANGLE:
			angle = m_admtController->getChannelValue(angleChannelName, 1);
			break;
		case ADMTController::Channel::COUNT:
			count = m_admtController->getChannelValue(countChannelName, 1);
			break;
		case ADMTController::Channel::TEMPERATURE:
			temp = m_admtController->getChannelValue(temperatureChannelName, 1);
			break;
	}
}

void HarmonicCalibration::calibrationTask()
{
	updateChannelValue(ADMTController::Channel::ANGLE);
	updateLabelValue(calibrationAngleLabel, ADMTController::Channel::ANGLE);
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
	logsPlainTextEdit->appendPlainText("\n======= Calibration Start =======\n");
	QVector<double> rawData;

	for (int i = 0; i < rawDataListWidget->count(); ++i) {
		QListWidgetItem* item = rawDataListWidget->item(i);
		std::string text = item->text().toStdString();
		double value = std::stod(text);
		rawData.append(value);
	}
	std::vector<double> stdData(rawData.begin(), rawData.end());

	logsPlainTextEdit->appendPlainText(m_admtController->calibrate(stdData));
}

void HarmonicCalibration::registerCalibrationData()
{
	logsPlainTextEdit->appendPlainText("\n=== Register Calibration Start ===\n");
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