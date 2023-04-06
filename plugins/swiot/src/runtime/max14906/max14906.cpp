#include "max14906.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "src/swiot_logging_categories.h"

using namespace adiscope::swiot;

Max14906::Max14906(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent),
	max14906ToolController(new DioController(ctx)),
	ui(new Ui::Max14906),
        m_backButton(Max14906::createBackButton()),
	m_qTimer(new QTimer(this)),
	m_toolView(nullptr),
	m_readerThread(new ReaderThread(false)),
	m_customColGrid(new CustomColQGridLayout(4, true, this))
{
        iio_device* device0 = iio_context_get_device(ctx, 0);
        if (iio_device_find_attr(device0, "back")) {
		qInfo(CAT_SWIOT_MAX14906) << "Initialising SWIOT MAX14906.";

                this->ui->setupUi(this);
                this->setupDynamicUi(this);

                this->m_qTimer->setInterval(1000); // poll once every second
                this->m_qTimer->setSingleShot(true);

                this->initChannels();
                this->initMonitorToolView();
                this->ui->mainLayout->addWidget(m_toolView);
                this->connectSignalsAndSlots();
        } else {
		qInfo(CAT_SWIOT_MAX14906) << "Could not initialize SWIOT MAX14906, the device seems to be in config mode.";
        }
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

	this->m_generalSettingsMenu = this->createGeneralSettings("General settings", new QColor(0x4a, 0x64, 0xff)); // "#4a64ff"
	this->m_toolView->setGeneralSettingsMenu(this->m_generalSettingsMenu, true);

	this->m_customColGrid = new CustomColQGridLayout(4, true, this); // 4 max channels

	this->m_toolView->addFixedCentralWidget(m_customColGrid, 0, 0, 0, 0);

	this->m_toolView->getGeneralSettingsBtn()->setChecked(true);

        this->m_toolView->addTopExtraWidget(m_backButton);
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
        QObject::connect(m_backButton, &QPushButton::clicked, this, [this] () {
            Q_EMIT backBtnPressed();
        });

	connect(this->m_max14906SettingsTab, &DioSettingsTab::timeValueChanged, this, &Max14906::timerChanged);
	connect(this->m_qTimer, &QTimer::timeout, this, [&](){
		this->m_readerThread->start();
	});
	connect(m_readerThread, &ReaderThread::started, this, [&](){
		this->m_qTimer->start(1000);
	});
}

Max14906::~Max14906() {
	if (this->m_toolView) {
		if (this->m_toolView->getRunBtn()->isChecked()) {
			this->m_toolView->getRunBtn()->setChecked(false);
		}
		if (this->m_readerThread->isRunning()) {
			this->m_readerThread->quit();
			this->m_readerThread->wait();
		}
	}
	delete m_toolView;
	delete ui;
}

void Max14906::runButtonToggled() {
	qDebug(CAT_SWIOT_MAX14906) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if (this->m_toolView->getRunBtn()->isChecked()) {
		for (auto & channel : this->m_channelControls) {
			channel->getDigitalChannel()->resetSismograph();
		}
		qDebug(CAT_SWIOT_MAX14906) << "Reader thread started";
		this->m_readerThread->start();
	} else {
		if (this->m_readerThread->isRunning()) {
			qDebug(CAT_SWIOT_MAX14906) << "Reader thread stopped";
			this->m_readerThread->quit();
			this->m_readerThread->wait();
		}
		this->m_qTimer->stop();
	}
}

void Max14906::singleButtonToggled() {
	if (m_toolView->getRunBtn()->isChecked()) {
		this->m_toolView->getRunBtn()->setChecked(false);
	}
	this->m_qTimer->stop();
	this->m_readerThread->singleDio();
	this->m_toolView->getSingleBtn()->setChecked(false);
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
		this->m_readerThread->addDioChannel(i, channel);
		connect(this->m_readerThread, &ReaderThread::channelDataChanged, channel_control,
			[this, i] (int index, double value){
			if (i == index) {
				this->m_channelControls.value(index)->getDigitalChannel()->addDataSample(value);
			}
		});
	}
}

QPushButton *Max14906::createBackButton() {
        auto* backButton = new QPushButton();
        backButton->setObjectName(QString::fromUtf8("backButton"));
        backButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
                                                 "  width: 95px;\n"
                                                 "  height: 40px;\n"
                                                 "\n"
                                                 "  font-size: 12px;\n"
                                                 "  text-align: center;\n"
                                                 "  font-weight: bold;\n"
                                                 "  padding-left: 15px;\n"
                                                 "  padding-right: 15px;\n"
                                                 "}"));
        backButton->setProperty("blue_button", QVariant(true));
        backButton->setText("Back");
        return backButton;
}

#include "moc_max14906.cpp"
