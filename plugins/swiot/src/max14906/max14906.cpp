#include "max14906.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"

using namespace adiscope::swiot;

Max14906::Max14906(struct iio_context *ctx, QWidget *parent) :
        QWidget(parent),
	max14906ToolController(new DioController(ctx)),
	ui(new Ui::Max14906),
	m_qTimer(new QTimer(this)),
	m_readerThread(new DioReaderThread()),
        m_customColGrid(new CustomColQGridLayout(4, true, this))
{
        this->ui->setupUi(this);
        this->setupDynamicUi(this);

	this->m_qTimer->setInterval(1000); // poll once every second
	this->m_qTimer->setSingleShot(true);

	this->initChannels();
        this->initMonitorToolView();
        this->ui->mainLayout->addWidget(m_toolView);
	this->connectSignalsAndSlots();
}

void Max14906::setupDynamicUi(QWidget *parent) {
        adiscope::gui::ToolViewRecipe recipe;
        recipe.helpBtnUrl = "";
        recipe.hasRunBtn = true;
        recipe.hasSingleBtn = true;
        recipe.hasPairSettingsBtn = true;
        recipe.hasPrintBtn = false;
        recipe.hasChannels = false;
        recipe.channelsPosition = adiscope::gui::ChannelsPositionEnum::HORIZONTAL;

        this->m_monitorChannelManager = new adiscope::gui::ChannelManager(recipe.channelsPosition);
        m_monitorChannelManager->setChannelIdVisible(false);

        m_toolView = adiscope::gui::ToolViewBuilder(recipe, this->m_monitorChannelManager, parent).build();

        this->m_generalSettingsMenu = this->createGeneralSettings("General settings", new QColor("#4a64ff"));
        this->m_toolView->setGeneralSettingsMenu(this->m_generalSettingsMenu, true);

        this->m_customColGrid = new CustomColQGridLayout(4, true, this); // 4 max channels

        this->m_toolView->addFixedCentralWidget(m_customColGrid, 0, 0, 0, 0);

        this->m_toolView->getGeneralSettingsBtn()->setChecked(true);
}

adiscope::gui::GenericMenu* Max14906::createGeneralSettings(const QString& title, QColor* color) {
        auto generalSettingsMenu = new adiscope::gui::GenericMenu(this);
        generalSettingsMenu->initInteractiveMenu();
        generalSettingsMenu->setMenuHeader(title, color, false);

        settingsWidgetSeparator = new adiscope::gui::SubsectionSeparator("MAX14906", false, this);
        this->m_max14906SettingsTab = new DioSettingsTab(settingsWidgetSeparator);
        settingsWidgetSeparator->setContent(this->m_max14906SettingsTab);
        generalSettingsMenu->insertSection(settingsWidgetSeparator);


        return generalSettingsMenu;
}

void Max14906::connectSignalsAndSlots() {
	connect(this->m_toolView->getRunBtn(), &QPushButton::toggled, this, &Max14906::runButtonToggled);
	connect(this->m_toolView->getSingleBtn(), &QPushButton::clicked, this, &Max14906::singleButtonToggled);

	connect(this->m_max14906SettingsTab, &DioSettingsTab::timeValueChanged, this, &Max14906::timerChanged);
	connect(this->m_qTimer, &QTimer::timeout, this, [&](){
		this->m_readerThread->start();
	});
	connect(m_readerThread, &DioReaderThread::started, this, [&](){
		this->m_qTimer->start(1000);
	});
}

Max14906::~Max14906() {
	if (this->m_toolView->getRunBtn()->isChecked()) {
                this->m_toolView->getRunBtn()->setChecked(false);
	}
	if (this->m_readerThread->isRunning()) {
		this->m_readerThread->quit();
		this->m_readerThread->wait();
	}
//	if (m_toolView) {
        delete m_toolView;
//	}
//        delete settingsWidgetSeparator;
	delete ui;
}

void Max14906::runButtonToggled() {
	qDebug(CAT_MAX14906) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if (this->m_toolView->getRunBtn()->isChecked()) {
                for (auto & channel : this->m_channelControls) {
                        channel->getDigitalChannel()->resetSismograph();
                }
		qDebug(CAT_MAX14906) << "Reader thread started";
		this->m_readerThread->start();
	} else {
		if (this->m_readerThread->isRunning()) {
			qDebug(CAT_MAX14906) << "Reader thread stopped";
			this->m_readerThread->quit();
			this->m_readerThread->wait();
		}
		this->m_qTimer->stop();
	}
}

void Max14906::singleButtonToggled() {
	if (m_toolView->getRunBtn()->isChecked()) {
		this->m_toolView->getRunBtn()->setChecked(false);
//		this->max14906ToolController->stopRead();
	}
	this->m_qTimer->stop();
	this->m_readerThread->singleRun();
	this->m_toolView->getSingleBtn()->setChecked(false);

//	 this->max14906ToolController->singleRead();
}

void Max14906::timerChanged(double value) {
	for (auto & channelControl : this->m_channelControls) {
		channelControl->getDigitalChannel()->updateTimeScale(0, value);
	}
}

void Max14906::initMonitorToolView() {
	for (int i = 0; i < this->m_channelControls.size(); ++i) {
		this->m_customColGrid->addQWidgetToList(this->m_channelControls[i]->getDigitalChannel());
		this->m_customColGrid->addWidget(i);
	}

	setUpdatesEnabled(true);
}

void Max14906::initChannels() {
	int channel_num = this->max14906ToolController->getChannelCount();

	for (int i = 0; i < channel_num - 1; ++i) { // -1 because of the fault channel
		struct iio_channel *channel = iio_device_get_channel(this->max14906ToolController->getDevice(), i);
		auto *channel_control = new DioDigitalChannelController(
					channel,
					this->max14906ToolController->getChannelName(i),
					this->max14906ToolController->getChannelType(i),
					this
					);

		this->m_channelControls.insert(i, channel_control);
		this->m_readerThread->addChannel(i, channel);
                if (this->max14906ToolController->getChannelType(i) == "output") {
                        m_readerThread->setOutputValue(i, true); // by default the button is set to high 
                }
		this->m_readerThread->toggleChannel(i, true);
		connect(this->m_readerThread, &DioReaderThread::channelDataChanged, channel_control,
                        [this, i] (int index, double value){
			if (i == index) {
				this->m_channelControls.value(index)->getDigitalChannel()->addDataSample(value);
			}
		});
                connect(channel_control->getDigitalChannel(), &DioDigitalChannel::outputValueChanged, this,
                        [this, i] (bool value) {
                        m_readerThread->setOutputValue(i, value);
                });
	}
}
