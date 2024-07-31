#include "harmoniccalibration.h"
#include <widgets/horizontalspinbox.h>

#include <stylehelper.h>

static int sampleRate = 1000;
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
	applyTextBoxStyle(graphUpdateIntervalLineEdit);
	applyLineEditPadding(graphUpdateIntervalLineEdit);
	applyLineEditAlignment(graphUpdateIntervalLineEdit);
	graphUpdateIntervalLineEdit->setText(QString::number(sampleRate));

	connectLineEditToNumber(graphUpdateIntervalLineEdit, sampleRate);

	generalSection->contentLayout()->addWidget(graphUpdateIntervalLabel);
	generalSection->contentLayout()->addWidget(graphUpdateIntervalLineEdit);

	// Data Sample Size
	QLabel *dataSampleSizeLabel = new QLabel(generalSection);
	dataSampleSizeLabel->setText("Data Sample Size");
	StyleHelper::MenuSmallLabel(dataSampleSizeLabel, "dataSampleSizeLabel");
	dataSampleSizeLineEdit = new QLineEdit(generalSection);
	applyTextBoxStyle(dataSampleSizeLineEdit);
	applyLineEditPadding(dataSampleSizeLineEdit);
	applyLineEditAlignment(dataSampleSizeLineEdit);
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
	applyTextBoxStyle(dataGraphSamplesLineEdit);
	applyLineEditPadding(dataGraphSamplesLineEdit);
	applyLineEditAlignment(dataGraphSamplesLineEdit);
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
	applyTextBoxStyle(tempGraphSamplesLineEdit);
	applyLineEditPadding(tempGraphSamplesLineEdit);
	applyLineEditAlignment(tempGraphSamplesLineEdit);
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
	PlotWidget *calibrationRawDataPlotWidget = new PlotWidget();
	calibrationRawDataPlotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	calibrationRawDataGraphLayout->addWidget(calibrationRawDataGraphLabel);
	calibrationRawDataGraphLayout->addWidget(calibrationRawDataPlotWidget);

	QWidget *calibrationFFTDataGraphWidget = new QWidget(calibrationDataGraphWidget);
	QVBoxLayout *calibrationFFTDataGraphLayout = new QVBoxLayout(calibrationFFTDataGraphWidget);
	calibrationFFTDataGraphWidget->setLayout(calibrationFFTDataGraphLayout);
	calibrationFFTDataGraphLayout->setMargin(0);
	calibrationFFTDataGraphLayout->setSpacing(4);
	QLabel *calibrationFFTDataGraphLabel = new QLabel("FFT Data", calibrationFFTDataGraphWidget);
	StyleHelper::MenuCollapseHeaderLabel(calibrationFFTDataGraphLabel, "calibrationFFTDataGraphLabel");
	PlotWidget *calibrationFFTDataPlotWidget = new PlotWidget();
	calibrationFFTDataPlotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	calibrationFFTDataGraphLayout->addWidget(calibrationFFTDataGraphLabel);
	calibrationFFTDataGraphLayout->addWidget(calibrationFFTDataPlotWidget);

	calibrationDataGraphLayout->addWidget(calibrationRawDataGraphWidget);
	calibrationDataGraphLayout->addWidget(calibrationFFTDataGraphWidget);
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
	QLabel *calibrationH1MagLabel = new QLabel("999.99°", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH1PhaseLabel = new QLabel("Φ 999.99", calibrationCalculatedCoeffWidget);
	calibrationH1Label->setFixedWidth(52);
	applyLabelStyle(calibrationH1Label, "LabelText", true);
	applyLabelStyle(calibrationH1MagLabel, "CH0");
	applyLabelStyle(calibrationH1PhaseLabel, "CH1");

	h1RowLayout->addWidget(calibrationH1Label);
	h1RowLayout->addWidget(calibrationH1MagLabel);
	h1RowLayout->addWidget(calibrationH1PhaseLabel, 0, Qt::AlignRight);

	// H2
	QWidget *h2RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h2RowLayout = new QHBoxLayout(h2RowContainer);
	h2RowContainer->setLayout(h2RowLayout);
	h2RowContainer->setStyleSheet(rowContainerStyle);
	h2RowContainer->setFixedHeight(30);
	h2RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h2RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH2Label = new QLabel("H2", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH2MagLabel = new QLabel("999.99°", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH2PhaseLabel = new QLabel("Φ 999.99", calibrationCalculatedCoeffWidget);
	calibrationH2Label->setFixedWidth(52);
	applyLabelStyle(calibrationH2Label, "LabelText", true);
	applyLabelStyle(calibrationH2MagLabel, "CH0");
	applyLabelStyle(calibrationH2PhaseLabel, "CH1");

	h2RowLayout->addWidget(calibrationH2Label);
	h2RowLayout->addWidget(calibrationH2MagLabel);
	h2RowLayout->addWidget(calibrationH2PhaseLabel, 0, Qt::AlignRight);

	// H3
	QWidget *h3RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h3RowLayout = new QHBoxLayout(h3RowContainer);
	h3RowContainer->setLayout(h3RowLayout);
	h3RowContainer->setStyleSheet(rowContainerStyle);
	h3RowContainer->setFixedHeight(30);
	h3RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h3RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH3Label = new QLabel("H3", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH3MagLabel = new QLabel("999.99°", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH3PhaseLabel = new QLabel("Φ 999.99", calibrationCalculatedCoeffWidget);
	calibrationH3Label->setFixedWidth(52);
	applyLabelStyle(calibrationH3Label, "LabelText", true);
	applyLabelStyle(calibrationH3MagLabel, "CH0");
	applyLabelStyle(calibrationH3PhaseLabel, "CH1");

	h3RowLayout->addWidget(calibrationH3Label);
	h3RowLayout->addWidget(calibrationH3MagLabel);
	h3RowLayout->addWidget(calibrationH3PhaseLabel, 0, Qt::AlignRight);

	// H8
	QWidget *h8RowContainer = new QWidget(calibrationCalculatedCoeffWidget);
	QHBoxLayout *h8RowLayout = new QHBoxLayout(h8RowContainer);
	h8RowContainer->setLayout(h8RowLayout);
	h8RowContainer->setStyleSheet(rowContainerStyle);
	h8RowContainer->setFixedHeight(30);
	h8RowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	h8RowLayout->setContentsMargins(12, 4, 12, 4);
	QLabel *calibrationH8Label = new QLabel("H8", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH8MagLabel = new QLabel("999.99°", calibrationCalculatedCoeffWidget);
	QLabel *calibrationH8PhaseLabel = new QLabel("Φ 999.99", calibrationCalculatedCoeffWidget);
	calibrationH8Label->setFixedWidth(52);
	applyLabelStyle(calibrationH8Label, "LabelText", true);
	applyLabelStyle(calibrationH8MagLabel, "CH0");
	applyLabelStyle(calibrationH8PhaseLabel, "CH1");

	h8RowLayout->addWidget(calibrationH8Label);
	h8RowLayout->addWidget(calibrationH8MagLabel);
	h8RowLayout->addWidget(calibrationH8PhaseLabel, 0, Qt::AlignRight);

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
	// calibrationMotorCurrentPositionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	calibrationMotorCurrentPositionLabel->setAlignment(Qt::AlignRight);
	applyTextBoxStyle(calibrationMotorCurrentPositionLabel);
	applyLabelPadding(calibrationMotorCurrentPositionLabel);

	HorizontalSpinBox *motorTargetPositionSpinBox = new HorizontalSpinBox("Target Position", 9999.99, "°", motorControlSectionWidget);
	HorizontalSpinBox *motorVelocitySpinBox = new HorizontalSpinBox("Velocity", 9999.99, "rpm", motorControlSectionWidget);

	QPushButton *calibrationStartMotorButton = new QPushButton(motorControlSectionWidget);
	calibrationStartMotorButton->setCheckable(true);
	calibrationStartMotorButton->setChecked(false);
	calibrationStartMotorButton->setText("Start Motor");
	calibrationStartMotorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	calibrationStartMotorButton->setFixedHeight(36);
	connect(calibrationStartMotorButton, &QPushButton::toggled, this, [=](bool b) { calibrationStartMotorButton->setText(b ? " Stop Motor" : " Start Motor"); });
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

	#pragma region Raw Data Section Widget
	MenuSectionWidget *rawDataWidget = new MenuSectionWidget(calibrationDataGraphWidget);
	MenuCollapseSection *rawDataSection = new MenuCollapseSection("Raw Data", MenuCollapseSection::MHCW_NONE, calibrationDataGraphWidget);
	rawDataWidget->contentLayout()->setSpacing(10);
	rawDataWidget->contentLayout()->addWidget(rawDataSection);
	rawDataSection->contentLayout()->setSpacing(10);

	rawDataListWidget = new QListWidget(rawDataWidget);
	rawDataSection->contentLayout()->addWidget(rawDataListWidget);
	#pragma endregion

	#pragma region Logs Section Widget
	MenuSectionWidget *logsSectionWidget = new MenuSectionWidget(calibrationSettingsWidget);
	MenuCollapseSection *logsCollapseSection = new MenuCollapseSection("Logs", MenuCollapseSection::MHCW_NONE, logsSectionWidget);
	logsSectionWidget->contentLayout()->setSpacing(10);
	logsSectionWidget->contentLayout()->addWidget(logsCollapseSection);

	logsPlainTextEdit = new QPlainTextEdit(logsSectionWidget);
	logsPlainTextEdit->setReadOnly(true);

	logsCollapseSection->contentLayout()->setSpacing(10);
	logsCollapseSection->contentLayout()->addWidget(logsPlainTextEdit);
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

	debugCollapseSection->contentLayout()->setSpacing(10);
	debugCollapseSection->contentLayout()->addWidget(addCalibrationDataButton);
	debugCollapseSection->contentLayout()->addWidget(removeLastCalibrationDataButton);
	debugCollapseSection->contentLayout()->addWidget(calibrateDataButton);
	#pragma endregion

	calibrationSettingsLayout->setMargin(0);
	calibrationSettingsLayout->addWidget(calibrationCoeffSectionWidget);
	calibrationSettingsLayout->addWidget(calibrationDataSectionWidget);
	calibrationSettingsLayout->addWidget(motorConfigurationSectionWidget);
	calibrationSettingsLayout->addWidget(motorControlSectionWidget);
	calibrationSettingsLayout->addWidget(rawDataWidget);
	calibrationSettingsLayout->addWidget(logsSectionWidget);
	calibrationSettingsLayout->addWidget(debugSectionWidget);
	calibrationSettingsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	#pragma endregion

	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(false);
	tool->topContainerMenuControl()->setVisible(false);
	tool->leftContainer()->setVisible(false);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(false);
	tool->setRightContainerWidth(270);
	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	tool->addWidgetToCentralContainerHelper(calibrationDataGraphWidget);
	tool->rightStack()->add("calibrationSettingsScrollArea", calibrationSettingsScrollArea);

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
	updateLabelValue(calibrationMotorCurrentPositionLabel, ADMTController::Channel::ANGLE);
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

void HarmonicCalibration::applyTextBoxStyle(QWidget *widget)
{
	applyLabelStyle(widget);
	QString existingStyle = widget->styleSheet();
	QString style = QString(R"css(
								background-color: black;
								border-radius: 4px;
								border: none;
							)css");
	widget->setStyleSheet(existingStyle + style);
	widget->setFixedHeight(30);
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

void HarmonicCalibration::applyLabelPadding(QLabel *widget)
{
	widget->setContentsMargins(12, 4, 12, 4);
}

void HarmonicCalibration::applyLineEditPadding(QLineEdit *widget)
{
	widget->setContentsMargins(0, 0, 0, 0);
	widget->setTextMargins(12, 4, 12, 4);
}

void HarmonicCalibration::applyLineEditAlignment(QLineEdit *widget)
{
	widget->setAlignment(Qt::AlignRight);
}

void HarmonicCalibration::applyLabelStyle(QWidget *widget, const QString& styleHelperColor, bool isBold)
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