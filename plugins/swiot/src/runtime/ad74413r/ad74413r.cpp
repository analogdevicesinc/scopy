#include "ad74413r.h"
#include <gui/tool_view_builder.hpp>
#include <gui/generic_menu.hpp>
#include "buffermenuview.h"
#include "buffermenumodel.h"
#include <iio.h>
#include "src/swiot_logging_categories.h"

using namespace scopy;
using namespace scopy::swiot;

Ad74413r::Ad74413r(iio_context *ctx, ToolMenuEntry *tme, QWidget* parent):
	QWidget(parent)
      ,m_tme(tme), m_statusLabel(new QLabel(this))
      ,m_swiotAdLogic(nullptr), m_widget(parent)
      ,m_readerThread(nullptr)
{
	createDevicesMap(ctx);
	if (m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		char mode[64];
		ssize_t result = iio_device_attr_read(m_iioDevicesMap[SWIOT_DEVICE_NAME], "mode", mode, 64);
		if ((result >= 0) && (strcmp(mode, "runtime") == 0)) {
			m_backBtn = createBackBtn();
			m_enabledChannels = std::vector<bool>(MAX_CURVES_NUMBER, false);

			m_swiotAdLogic = new BufferLogic(m_iioDevicesMap);
			m_readerThread = new ReaderThread(true);
			m_readerThread->addBufferedDevice(m_iioDevicesMap[AD_NAME]);

			QStringList actualSamplingFreq = m_swiotAdLogic->readChnlsSamplingFreqAttr("sampling_frequency");
			int samplingFreq = actualSamplingFreq[0].toInt();
			m_readerThread->onSamplingFreqWritten(samplingFreq);
			m_plotHandler = new BufferPlotHandler(this, m_swiotAdLogic->getPlotChnlsNo(), samplingFreq);
			QVector<QString> chnlsUnitOfMeasure = m_swiotAdLogic->getPlotChnlsUnitOfMeasure();
			m_plotHandler->setChnlsUnitOfMeasure(chnlsUnitOfMeasure);
			QVector<std::pair<int, int>> chnlsRangeValues = m_swiotAdLogic->getPlotChnlsRangeValues();
			m_plotHandler->setChnlsRangeValues(chnlsRangeValues);
			QMap<int, QString> chnlsId = m_swiotAdLogic->getPlotChnlsId();
			m_plotHandler->setHandlesName(chnlsId);

			gui::GenericMenu *settingsMenu = createSettingsMenu("General settings", new QColor(0x4a, 0x64, 0xff));
			setupToolView(settingsMenu);
			setupConnections();
			initMonitorToolView(settingsMenu);
		}
	}

}

Ad74413r::~Ad74413r()
{
	if (m_readerThread) {
		if (m_readerThread->isRunning()) {
			m_readerThread->requestInterruption();
			m_readerThread->quit();
			m_readerThread->wait();
		}
		delete m_readerThread;
	}
	if (m_controllers.size() > 0) {
		m_controllers.clear();
	}
}

void Ad74413r::setupToolView(gui::GenericMenu *settingsMenu)
{
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = true;
	recipe.hasChannels = true;
	recipe.channelsPosition = scopy::gui::ChannelsPositionEnum::VERTICAL;

	m_monitorChannelManager = new scopy::gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus(QString(AD_NAME).toUpper());

	m_toolView = scopy::gui::ToolViewBuilder(recipe, m_monitorChannelManager, m_widget).build();
	m_toolView->setGeneralSettingsMenu(settingsMenu, true);
	m_toolView->addTopExtraWidget(m_backBtn);

	m_statusLabel->setText(
		"The external power supply is not connected. The MAX14906 chip will not be used at full capacity.");
	m_statusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_statusLabel->setStyleSheet("color: red;");
	m_statusLabel->setContentsMargins(6, 0, 6, 0);

	auto *container = new QWidget(this);
	container->setLayout(new QVBoxLayout(container));
	container->layout()->addWidget(m_statusLabel);
	container->layout()->addWidget(m_plotHandler->getPlotWidget());

	m_toolView->addFixedCentralWidget(container, 0, 0, 0, 0);
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);
}

void Ad74413r::setupConnections()
{
	connect(m_backBtn, &QPushButton::pressed, this, &Ad74413r::backBtnPressed);
	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, &Ad74413r::onRunBtnPressed);
	connect(m_swiotAdLogic, &BufferLogic::chnlsChanged, m_readerThread, &ReaderThread::onChnlsChange);

	connect(m_monitorChannelManager, &scopy::gui::ChannelManager::enabledChannel, this, &Ad74413r::onChannelWidgetEnabled);
	connect(this, &Ad74413r::channelWidgetEnabled, m_plotHandler, &BufferPlotHandler::onChannelWidgetEnabled);

	connect(m_readerThread, &ReaderThread::bufferRefilled, m_plotHandler, &BufferPlotHandler::onBufferRefilled, Qt::QueuedConnection);
	connect(m_readerThread, &ReaderThread::finished, this, &Ad74413r::onReaderThreadFinished, Qt::QueuedConnection);

	connect(m_toolView->getSingleBtn(), &QPushButton::toggled, this, &Ad74413r::onSingleBtnPressed);
	connect(m_plotHandler, &BufferPlotHandler::singleCaptureFinished, this, &Ad74413r::onSingleCaptureFinished);

	connect(m_swiotAdLogic, &BufferLogic::samplingFreqWritten, m_plotHandler, &BufferPlotHandler::onSamplingFreqWritten);
	connect(m_swiotAdLogic, &BufferLogic::samplingFreqWritten, m_readerThread, &ReaderThread::onSamplingFreqWritten);

	connect(m_timespanSpin, &PositionSpinButton::valueChanged, m_plotHandler, &BufferPlotHandler::onTimespanChanged);
	connect(m_samplingFreqOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), m_swiotAdLogic, &BufferLogic::onSamplingFreqChanged);

	connect(m_tme, &ToolMenuEntry::runToggled, m_toolView->getRunBtn(), &QPushButton::setChecked);

	connect(m_plotHandler, &BufferPlotHandler::offsetHandleSelected, this, &Ad74413r::onOffsetHdlSelected);

	connect(m_monitorChannelManager, &scopy::gui::ChannelManager::selectedChannel, this, &Ad74413r::onChannelWidgetSelected);
	connect(this, &Ad74413r::channelWidgetSelected, m_plotHandler, &BufferPlotHandler::onChannelWidgetSelected);

	connect(m_toolView->getPrintBtn(), &QPushButton::clicked, m_plotHandler, &BufferPlotHandler::onPrintBtnClicked);
}

void Ad74413r::initMonitorToolView(gui::GenericMenu *settingsMenu)
{
	int chId = 1;
	bool first = true;
	QVector<QString> chnlsFunctions = m_swiotAdLogic->getAd74413rChnlsFunctions();

	for (int i = 0; i < chnlsFunctions.size(); i++) {
		if (chnlsFunctions[i].compare("no_config") != 0) {

			QString menuTitle(((QString(AD_NAME).toUpper() + " - Channel ") + QString::number(i+1)) + (": " + chnlsFunctions[i]));
			BufferMenuView *menu = new BufferMenuView(this);
			//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
			//that's why we have to decrease by 1
			menu->init(menuTitle, chnlsFunctions[i], new QColor(m_plotHandler->getCurveColor(chId-1)));

			struct iio_channel* iioChnl = m_swiotAdLogic->getIioChnl(i, true);
			BufferMenuModel* swiotModel = new BufferMenuModel(iioChnl);
			BufferMenuController* controller = new BufferMenuController(menu, swiotModel, i);

			controller->addMenuAttrValues();
			if (controller) {
				m_controllers.push_back(controller);
			}
			QString chnlWidgetName(chnlsFunctions[i] +" "+QString::number(i+1));
			//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
			//that's why we have to decrease by 1
			ChannelWidget *chWidget =
					m_toolView->buildNewChannel(m_monitorChannelManager, menu, false, -1, false, false,
								    m_plotHandler->getCurveColor(chId-1), chnlWidgetName, chnlWidgetName);
			if (first) {
				chWidget->menuButton()->click();
				first = false;
			}
			controller->createConnections();
			m_channelWidgetList.push_back(chWidget);
			chId++;
		}
	}
}

void Ad74413r::initExportSettings(QWidget *parent)
{
	int plotChnlsNo = m_swiotAdLogic->getPlotChnlsNo();
	m_exportSettings = new ExportSettings(parent);
	m_exportSettings->enableExportButton(false);

	for (int i = 0; i < plotChnlsNo; i++){
		m_exportSettings->addChannel(i, QString("Channel") +
					     QString::number(i+1));
	}

	connect(m_exportSettings->getExportButton(), &QPushButton::clicked, this, [=](){
		Q_EMIT exportBtnClicked(m_exportSettings->getExportConfig());
	});
	connect(this, &Ad74413r::exportBtnClicked, m_plotHandler, &BufferPlotHandler::onBtnExportClicked);
	connect(this, &Ad74413r::activateExportButton,
		[=](){
		m_exportSettings->enableExportButton(true);
	});
}

scopy::gui::GenericMenu* Ad74413r::createSettingsMenu(QString title, QColor* color)
{
	scopy::gui::GenericMenu *menu = new scopy::gui::GenericMenu(this);
	menu->initInteractiveMenu();
	menu->setMenuHeader(title,color,false);

	auto *generalSubsection = new scopy::gui::SubsectionSeparator("ACQUISITION SETTINGS", false, this);
	generalSubsection->getLabel()->setStyleSheet("color:gray;");
	generalSubsection->layout()->setSpacing(10);
	generalSubsection->getContentWidget()->layout()->setSpacing(10);

	//channels sampling freq
	auto *samplingFreqLayout = new QHBoxLayout();
	m_samplingFreqOptions = new QComboBox(generalSubsection->getContentWidget());

	QStringList actualSamplingFreq = m_swiotAdLogic->readChnlsSamplingFreqAttr("sampling_frequency");
	QStringList samplingFreqValues = m_swiotAdLogic->readChnlsSamplingFreqAttr("sampling_frequency_available");
	int actualSamplingFreqIdx = 0;
	int valuesIdx = 0;
	for (QString val : samplingFreqValues) {
		m_samplingFreqOptions->addItem(val);
		if (val.compare(actualSamplingFreq[0]) == 0) {
			actualSamplingFreqIdx = valuesIdx;
		}
		valuesIdx++;
	}
	m_samplingFreqOptions->setCurrentIndex(actualSamplingFreqIdx);
	samplingFreqLayout->addWidget(new QLabel("Sampling frequency",generalSubsection->getContentWidget()));
	samplingFreqLayout->addWidget(m_samplingFreqOptions);

	//plot timespan
	auto *timespanLayout = new QHBoxLayout();
	m_timespanSpin = new PositionSpinButton({
							{"ms",1E-3},
							{"s", 1E0}},
							"Timespan",0.1,10,
							true, false, generalSubsection->getContentWidget());
	m_timespanSpin->setStep(0.1);
	m_timespanSpin->setValue(1);
	timespanLayout->addWidget(m_timespanSpin);

	//export section
	auto *exportLayout = new QHBoxLayout();

	initExportSettings(generalSubsection->getContentWidget());
	exportLayout->addWidget(m_exportSettings);
	generalSubsection->getContentWidget()->layout()->addItem(samplingFreqLayout);
	generalSubsection->getContentWidget()->layout()->addItem(timespanLayout);
	generalSubsection->getContentWidget()->layout()->addItem(exportLayout);

	menu->insertSection(generalSubsection);

	return menu;
}

void Ad74413r::onChannelWidgetEnabled(int chnWidgetId, bool en)
{
	int id = chnWidgetId;

	int chnlIdx = m_controllers[id]->getChnlIdx();

	if (en) {
		m_enabledChannels[chnlIdx] = true;
		verifyChnlsChanges();
	}
	else {
		m_enabledChannels[chnlIdx] = false;
		verifyChnlsChanges();
	}
	Q_EMIT channelWidgetEnabled(id, en);
}

void Ad74413r::onChannelWidgetSelected(int chnWidgetId, bool en)
{
	if (!en) { return; }

	int id = chnWidgetId;
	Q_EMIT channelWidgetSelected(id);
}

void Ad74413r::onOffsetHdlSelected(int hdlIdx, bool selected)
{
	for (int i = 0; i < m_channelWidgetList.size(); i++) {
		if (m_channelWidgetList[i] != nullptr) {
			m_channelWidgetList[i]->nameButton()->setChecked(i == hdlIdx);
		}
	}
}

void Ad74413r::onRunBtnPressed()
{
	Q_EMIT activateExportButton();
	bool runBtnChecked = m_toolView->getRunBtn()->isChecked();
	if (runBtnChecked) {
		m_samplingFreqOptions->setEnabled(false);
		verifyChnlsChanges();
		if (!m_readerThread->isRunning()) {
			m_plotHandler->resetPlotParameters();
			m_readerThread->start();
		}
		if (!m_tme->running()) {
			m_tme->setRunning(runBtnChecked);
		}
	} else {
		m_samplingFreqOptions->setEnabled(true);
		m_readerThread->requestInterruption();
		if (m_tme->running()) {
			m_tme->setRunning(runBtnChecked);
		}
	}
}

void Ad74413r::onSingleBtnPressed()
{
	if (m_toolView->getSingleBtn()->isChecked()) {
		bool runBtnChecked = m_toolView->getRunBtn()->isChecked();
		Q_EMIT activateExportButton();
		verifyChnlsChanges();
		if (m_readerThread->isRunning()) {
			m_readerThread->requestInterruption();
		} else {
			m_plotHandler->resetPlotParameters();
			m_readerThread->start();
		}
		m_plotHandler->setSingleCapture(true);
		if (runBtnChecked) {
			m_toolView->getRunBtn()->setChecked(false);
		}
		m_toolView->getSingleBtn()->setEnabled(false);
	} else {
		if (m_tme->running()) {
			m_tme->setRunning(false);
		}
	}
}

void Ad74413r::onSingleCaptureFinished()
{
	if (m_tme->running()) {
		m_tme->setRunning(false);
	}
	m_toolView->getSingleBtn()->setChecked(false);
	m_toolView->getSingleBtn()->setEnabled(true);
	m_readerThread->requestInterruption();
}

void Ad74413r::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyEnableChanges(m_enabledChannels);
	if (changes) {
		m_readerThread->requestInterruption();
	}
}

void Ad74413r::createDevicesMap(iio_context *ctx)
{
	int devicesCount = iio_context_get_devices_count(ctx);
	for (int i = 0; i < devicesCount; i++) {
		struct iio_device* iioDev = iio_context_get_device(ctx, i);
		if (iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			if ((deviceName.compare(AD_NAME) && deviceName.compare(SWIOT_DEVICE_NAME)) == 0)
			{
				m_iioDevicesMap[deviceName] = iioDev;
			}
		}
	}
}

QPushButton* Ad74413r::createBackBtn()
{
	QPushButton* backBtn = new QPushButton();
	backBtn->setObjectName(QString::fromUtf8("backBtn"));
	backBtn->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						 "  width: 95px;\n"
						 "  height: 40px;\n"
						 "\n"
						 "  font-size: 12px;\n"
						 "  text-align: center;\n"
						 "  font-weight: bold;\n"
						 "  padding-left: 15px;\n"
						 "  padding-right: 15px;\n"
						 "}"));
	backBtn->setProperty("blue_button", QVariant(true));
	backBtn->setText("Config");
	return backBtn;
}

void Ad74413r::onReaderThreadFinished()
{
	qDebug(CAT_SWIOT_AD74413R) << "reader thread finished";
	bool singleCaptureOn = m_plotHandler->singleCapture();
	if (singleCaptureOn) {
		m_plotHandler->setSingleCapture(false);
	}
	if (m_toolView->getRunBtn()->isChecked() || m_toolView->getSingleBtn()->isChecked()) {
		m_plotHandler->resetPlotParameters();
		m_readerThread->start();
	}
}

void Ad74413r::externalPowerSupply(bool ps) {
	if (ps) {
		m_statusLabel->hide();
	} else {
		m_statusLabel->show();
	}
}
