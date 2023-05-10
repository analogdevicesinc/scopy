#include "ad74413r.h"
#include <gui/tool_view_builder.hpp>
#include <gui/generic_menu.hpp>
#include "buffermenuview.h"
#include "buffermenumodel.h"
#include <iio.h>
#include "src/swiot_logging_categories.h"

using namespace scopy;
using namespace scopy::swiot;

Ad74413r::Ad74413r(iio_context *ctx, ToolMenuEntry *tme,
		   QVector<QString> chnlsFunc, QWidget* parent):
	QWidget(parent)
      ,m_tme(tme)
      ,m_swiotAdLogic(nullptr), m_iioDev(nullptr)
      ,m_widget(parent), m_readerThread(nullptr)
{
	m_iioDev = iio_context_find_device(ctx, AD_NAME);
	m_chnlsFunction = chnlsFunc;
	m_enabledChannels = std::vector<bool>(m_chnlsFunction.size(), false);
	m_backBtn = createBackBtn();
	if (iio_device_find_attr(m_iioDev, "back")) {
		m_swiotAdLogic = new BufferLogic(m_iioDev);
		m_readerThread = new ReaderThread(true);
                m_readerThread->addBufferedDevice(m_iioDev);

		QStringList actualSamplingFreq = m_swiotAdLogic->readChnlsSamplingFreqAttr("sampling_frequency");
		int samplingFreq = actualSamplingFreq[0].toInt();
		m_plotHandler = new BufferPlotHandler(this, m_swiotAdLogic->getPlotChnlsNo(), samplingFreq);
		setupToolView();
		initMonitorToolView();
		setupConnections();
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

void Ad74413r::setupToolView()
{
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = true;
	recipe.channelsPosition = scopy::gui::ChannelsPositionEnum::VERTICAL;

	m_monitorChannelManager = new scopy::gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus("Channels");

	m_toolView = scopy::gui::ToolViewBuilder(recipe, m_monitorChannelManager, m_widget).build();

	gui::GenericMenu *settingsMenu = createSettingsMenu("General settings", new QColor("Red"));
	m_toolView->setGeneralSettingsMenu(settingsMenu, true);
	m_toolView->addTopExtraWidget(m_backBtn);
	m_toolView->addFixedCentralWidget(m_plotHandler->getPlotWidget(), 0, 0, 0, 0);
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);
}

void Ad74413r::setupConnections()
{
	connect(m_backBtn, &QPushButton::pressed, this, &Ad74413r::backBtnPressed);
	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, &Ad74413r::onRunBtnPressed);
	connect(m_swiotAdLogic, &BufferLogic::chnlsChanged, m_readerThread, &ReaderThread::onChnlsChange);
	connect(this, &Ad74413r::channelWidgetEnabled, m_plotHandler, &BufferPlotHandler::onChannelWidgetEnabled);
	connect(m_readerThread, &ReaderThread::bufferRefilled, m_plotHandler, &BufferPlotHandler::onBufferRefilled, Qt::QueuedConnection);
	connect(m_readerThread, &ReaderThread::finished, this, &Ad74413r::onReaderThreadFinished, Qt::QueuedConnection);

	connect(m_toolView->getSingleBtn(), &QPushButton::toggled, this, &Ad74413r::onSingleBtnPressed);
	connect(m_plotHandler, &BufferPlotHandler::singleCaptureFinished, this, &Ad74413r::onSingleCaptureFinished);

	connect(m_swiotAdLogic, &BufferLogic::samplingFreqWritten, m_plotHandler, &BufferPlotHandler::onSamplingFreqWritten);
	connect(m_swiotAdLogic, &BufferLogic::samplingFreqWritten, m_readerThread, &ReaderThread::onSamplingFreqWritten);
	//general settings connections
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, m_plotHandler, &BufferPlotHandler::onTimespanChanged);
	connect(m_samplingFreqOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), m_swiotAdLogic, &BufferLogic::onSamplingFreqChanged);

	connect(m_tme, &ToolMenuEntry::runToggled, m_toolView->getRunBtn(), &QPushButton::setChecked);

	connect(m_plotHandler, &BufferPlotHandler::offsetHandleSelected, this, &Ad74413r::onOffsetHdlSelected);
	connect(this, &Ad74413r::channelWidgetSelected, m_plotHandler, &BufferPlotHandler::onChannelWidgetSelected);
}

void Ad74413r::initMonitorToolView()
{
	int chId = 0;
	bool first = true;

	QString deviceName(iio_device_get_name(m_iioDev));
	ChannelWidget *mainCh_widget =
			m_toolView->buildNewChannel(m_monitorChannelManager, nullptr, false, chId, false, false,
						    QColor("green"), deviceName, deviceName);
	chId++;
	for (int i = 0; i < m_chnlsFunction.size(); i++) {
		if (m_chnlsFunction[i].compare("high_z") != 0) {
			QString menuTitle(((deviceName + " - Channel ") + QString::number(i+1)) + (": " + m_chnlsFunction[i]));
			BufferMenuView *menu = new BufferMenuView(m_widget);
			//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
			//that's why we have to decrease by 1
			menu->init(menuTitle, m_chnlsFunction[i], new QColor(m_plotHandler->getCurveColor(chId - 1)));

			struct iio_channel* iioChnl = m_swiotAdLogic->getIioChnl(i, true);
			BufferMenuModel* swiotModel = new BufferMenuModel(iioChnl);
			BufferMenuController* controller = new BufferMenuController(menu, swiotModel, i);

			controller->addMenuAttrValues();
			if (controller) {
				m_controllers.push_back(controller);
			}
			QString chnlWidgetName(m_chnlsFunction[i] +" "+QString::number(i+1));
			//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
			//that's why we have to decrease by 1
			ChannelWidget *chWidget =
					m_toolView->buildNewChannel(m_monitorChannelManager, menu, false, chId, false, false,
								    m_plotHandler->getCurveColor(chId - 1), chnlWidgetName, chnlWidgetName);
			if (first) {
				chWidget->menuButton()->click();
				first = false;
			}
			controller->createConnections();
			m_channelWidgetList.push_back(chWidget);
			chId++;
		}
	}

	m_toolView->buildChannelGroup(m_monitorChannelManager, mainCh_widget, m_channelWidgetList);
	connectChnlsWidgesToPlot();
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

void Ad74413r::connectChnlsWidgesToPlot()
{
	for (int i = 0; i < m_channelWidgetList.size(); i++) {
		connect(m_channelWidgetList[i], SIGNAL(enabled(bool)),
			SLOT(onChannelWidgetEnabled(bool)));
		connect(m_channelWidgetList[i], SIGNAL(selected(bool)),
			SLOT(onChannelWidgetSelected(bool)));
	}
}

void Ad74413r::onChannelWidgetEnabled(bool en)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
	//that's why we have to decrease by 1
	int id = w->id() - 1;
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

void Ad74413r::onChannelWidgetSelected(bool checked)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	//the curves id is in the range (0, chnlsNumber - 1) and the chnlsWidgets id is in the range (1, chnlsNumber)
	//that's why we have to decrease by 1
	int id = w->id() - 1;
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
		Q_EMIT activateExportButton();
		verifyChnlsChanges();
		if (m_readerThread->isRunning()) {
			m_readerThread->requestInterruption();
		} else {
			m_plotHandler->resetPlotParameters();
			m_readerThread->start();
		}
		m_plotHandler->setSingleCapture(true);
		if (m_toolView->getRunBtn()->isChecked()) {
			m_toolView->getRunBtn()->setChecked(false);
		}
	}
}

void Ad74413r::onSingleCaptureFinished()
{
	m_toolView->getSingleBtn()->setChecked(false);
	m_readerThread->requestInterruption();
}

void Ad74413r::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyEnableChanges(m_enabledChannels);
	if (changes) {
		m_readerThread->requestInterruption();
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
	backBtn->setText("Back");
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
