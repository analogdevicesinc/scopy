#include "max14906/max14906.h"

#include "swiot_logging_categories.h"
#include <iioutil/connectionprovider.h>

#include <QHBoxLayout>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/stylehelper.h>

using namespace scopy::swiotrefactor;

Max14906::Max14906(QString uri, ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
	, m_qTimer(new QTimer(this))
	, m_tme(tme)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);

	// tool template configuration
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);
	m_tool->setRightContainerWidth(280);

	layout->addWidget(m_tool);

	m_configBtn = createConfigBtn(this);
	m_runBtn = new RunBtn(this);
	m_gearBtn = new GearBtn(this);
	m_gearBtn->setChecked(true);

	m_gridWidget = new QWidget(this);
	m_gridWidget->setLayout(new QGridLayout());
	StyleHelper::BackgroundWidget(m_gridWidget);

	m_max14906SettingsTab = new DioSettingsTab(this);
	m_tool->rightStack()->add("settings", m_max14906SettingsTab);
	connect(m_gearBtn, &QPushButton::toggled, this, [=, this](bool b) { m_tool->openRightContainerHelper(b); });

	m_tool->addWidgetToCentralContainerHelper(m_gridWidget);
	m_tool->addWidgetToTopContainerHelper(m_configBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	m_conn = ConnectionProvider::open(m_uri);
	connect(m_conn, &Connection::aboutToBeDestroyed, this, &Max14906::handleConnectionDestroyed);
	m_ctx = m_conn->context();
	m_cmdQueue = m_conn->commandQueue();
	m_max14906ToolController = new DioController(m_ctx);
	m_readerThread = new ReaderThread(false, m_cmdQueue);

	m_nbDioChannels = m_max14906ToolController->getChannelCount();

	setupDynamicUi(this);
	connectSignalsAndSlots();

	m_qTimer->setInterval(MAX14906_POLLING_TIME); // poll once every second
	m_qTimer->setSingleShot(true);

	initChannels();
	initMonitorToolView();
}

Max14906::~Max14906()
{
	if(m_conn) {
		if(m_runBtn->isChecked()) {
			m_runBtn->setChecked(false);
		}
		if(m_readerThread->isRunning()) {
			m_readerThread->forcedStop();
			m_readerThread->wait();
		}
		delete m_readerThread;

		ConnectionProvider::close(m_uri);
	}
}

void Max14906::connectSignalsAndSlots()
{
	connect(m_conn, &Connection::aboutToBeDestroyed, m_readerThread, &ReaderThread::handleConnectionDestroyed);
	connect(m_runBtn, &QPushButton::toggled, this, &Max14906::runButtonToggled);
	connect(m_configBtn, &QPushButton::clicked, this, &Max14906::onConfigBtnPressed);

	connect(m_max14906SettingsTab, &DioSettingsTab::timeValueChanged, this, &Max14906::timerChanged);
	connect(m_qTimer, &QTimer::timeout, this, [&]() { m_readerThread->start(); });
	connect(m_readerThread, &ReaderThread::started, this, [&]() { m_qTimer->start(1000); });

	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QPushButton::setChecked);
}

void Max14906::onConfigBtnPressed()
{
	bool runBtnChecked = m_runBtn->isChecked();
	if(runBtnChecked) {
		m_runBtn->setChecked(false);
	}

	Q_EMIT configBtnPressed();
}

void Max14906::handleConnectionDestroyed()
{
	qDebug(CAT_SWIOT_MAX14906) << "Max14906 connection destroyed slot";
	m_ctx = nullptr;
	m_cmdQueue = nullptr;
	m_conn = nullptr;
}

void Max14906::runButtonToggled()
{
	qDebug(CAT_SWIOT_MAX14906) << "Run button clicked";
	if(m_runBtn->isChecked()) {
		for(auto &channel : m_channelControls) {
			channel->getDigitalChannel()->resetPlot();
		}
		qDebug(CAT_SWIOT_MAX14906) << "Reader thread started";
		m_readerThread->start();
		if(!m_tme->running()) {
			m_tme->setRunning(true);
		}
	} else {
		if(m_readerThread->isRunning()) {
			qDebug(CAT_SWIOT_MAX14906) << "Reader thread stopped";
			m_readerThread->forcedStop();
			m_readerThread->wait();
		}
		if(m_tme->running()) {
			m_tme->setRunning(false);
		}
		m_qTimer->stop();
	}
}

void Max14906::timerChanged(double value)
{
	for(auto &channelControl : m_channelControls) {
		channelControl->getDigitalChannel()->updateTimeScale(value);
	}
}

void Max14906::initMonitorToolView()
{
	m_gridWidget->setProperty("tutorial_name", "DIO_DIGITAL_CHANNEL_SPACE");
	QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(m_gridWidget->layout());
	int nbChannels = m_channelControls.size();
	int lineSpan;
	if(nbChannels > 2) {
		lineSpan = 3;
		gridLayout->addWidget(createVLine(m_gridWidget), 1, 0, 1, lineSpan);

		gridLayout->addWidget(createHLine(m_gridWidget), 0, 1, lineSpan, 1);
	} else if(nbChannels == 2) {
		lineSpan = 1;
		gridLayout->addWidget(createVLine(m_gridWidget), 1, 0, 1, lineSpan);
	}

	// there can only be 4 channels, so we position them accordingly
	switch(m_channelControls.size()) {
	case 4: {
		DioDigitalChannel *digitalChannel = m_channelControls[3]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, m_gridWidget);

		gridLayout->addWidget(mainWindow, 2, 2);
	}
	case 3: {
		DioDigitalChannel *digitalChannel = m_channelControls[2]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, m_gridWidget);

		gridLayout->addWidget(mainWindow, 0, 2);
	}
	case 2: {
		DioDigitalChannel *digitalChannel = m_channelControls[1]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, m_gridWidget);

		gridLayout->addWidget(mainWindow, 2, 0);
	}
	case 1: {
		DioDigitalChannel *digitalChannel = m_channelControls[0]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, m_gridWidget);

		gridLayout->addWidget(mainWindow, 0, 0);
	}
	default: {
		break;
	}
	}
	gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0);

	setUpdatesEnabled(true);
}

QFrame *Max14906::createVLine(QWidget *parent)
{
	auto *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::VLine);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setStyleSheet("background-color: grey;");
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frame->setFixedHeight(2);

	return frame;
}

void Max14906::initChannels()
{
	for(int i = 0; i < m_nbDioChannels; ++i) {
		struct iio_channel *channel = iio_device_get_channel(m_max14906ToolController->getDevice(), i);
		DioDigitalChannelController *channel_control =
			new DioDigitalChannelController(channel, m_max14906ToolController->getChannelName(i),
							m_max14906ToolController->getChannelType(i), m_cmdQueue, this);

		m_channelControls.insert(i, channel_control);
		m_readerThread->addDioChannel(i, channel);
		connect(m_readerThread, &ReaderThread::channelDataChanged, channel_control,
			[this, i](int index, double value) {
				if(i == index) {
					m_channelControls.value(index)->getDigitalChannel()->addDataSample(value);
				}
			});
	}
}

void Max14906::setupDynamicUi(QWidget *parent)
{
	m_runBtn->setEnabled(!!m_nbDioChannels);
	m_tme->setRunBtnVisible(!!m_nbDioChannels);
	m_runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	m_configBtn->setProperty("tutorial_name", "CONFIG_BUTTON");
}

QFrame *Max14906::createHLine(QWidget *parent)
{
	auto *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::HLine);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setStyleSheet("background-color: grey;");
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frame->setFixedWidth(2);

	return frame;
}

QMainWindow *Max14906::createDockableMainWindow(const QString &title, DioDigitalChannel *digitalChannel,
						QWidget *parent)
{
	auto mainWindow = new QMainWindow(parent);
	mainWindow->setCentralWidget(nullptr);
	mainWindow->setWindowFlags(Qt::Widget);

	auto dockWidget = DockerUtils::createDockWidget(mainWindow, digitalChannel, title);

	mainWindow->addDockWidget(Qt::TopDockWidgetArea, dockWidget);
	return mainWindow;
}

QPushButton *Max14906::createConfigBtn(QWidget *parent)
{
	QPushButton *configBtn = new QPushButton(parent);
	StyleHelper::BlueGrayButton(configBtn, "back_btn");
	configBtn->setFixedWidth(128);
	configBtn->setCheckable(false);
	configBtn->setText("Config");
	return configBtn;
}
