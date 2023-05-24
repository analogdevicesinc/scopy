#include "max14906.h"
#include <gui/tool_view_builder.hpp>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

Max14906::Max14906(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent) :
	QWidget(parent),
	max14906ToolController(new DioController(ctx)),
	ui(new Ui::Max14906),
	m_backButton(Max14906::createBackButton()),
	m_qTimer(new QTimer(this)),
	m_toolView(nullptr),
	m_readerThread(new ReaderThread(false))
{
	iio_device* device0 = iio_context_find_device(ctx, MAX_NAME);
	if (iio_device_find_attr(device0, "back")) {
		qInfo(CAT_SWIOT_MAX14906) << "Initialising SWIOT MAX14906.";

		this->ui->setupUi(this);
		this->setupDynamicUi(this);
		this->connectSignalsAndSlots();

		this->m_qTimer->setInterval(1000); // poll once every second
		this->m_qTimer->setSingleShot(true);

		this->initChannels();
		this->initMonitorToolView();
		this->ui->mainLayout->addWidget(m_toolView);
	} else {
		qInfo(CAT_SWIOT_MAX14906) << "Could not initialize SWIOT MAX14906, the device seems to be in config mode.";
	}
}

void Max14906::setupDynamicUi(QWidget *parent) {
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.hasHelpBtn = false;

	m_toolView = scopy::gui::ToolViewBuilder(recipe, nullptr, parent).build();

	this->m_generalSettingsMenu = this->createGeneralSettings("General settings", new QColor(0x4a, 0x64, 0xff)); // "#4a64ff"
	this->m_toolView->setGeneralSettingsMenu(this->m_generalSettingsMenu, true);

	this->m_toolView->addFixedCentralWidget(this->ui->grid, 0, 0);
	this->m_toolView->getGeneralSettingsBtn()->setChecked(true);
	this->m_toolView->addTopExtraWidget(m_backButton);
}

scopy::gui::GenericMenu* Max14906::createGeneralSettings(const QString& title, QColor* color) {
	auto generalSettingsMenu = new scopy::gui::GenericMenu(this);
	generalSettingsMenu->initInteractiveMenu();
	generalSettingsMenu->setMenuHeader(title, color, false);

	settingsWidgetSeparator = new scopy::gui::SubsectionSeparator("MAX14906", false, this);
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

	connect(m_tme, &ToolMenuEntry::runToggled, this->m_toolView->getRunBtn(), &QPushButton::setChecked);
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
		if (!this->m_tme->running()) {
			m_tme->setRunning(true);
		}
	} else {
		if (this->m_readerThread->isRunning()) {
			qDebug(CAT_SWIOT_MAX14906) << "Reader thread stopped";
			this->m_readerThread->quit();
			this->m_readerThread->wait();
		}
		if (this->m_tme->running()) {
			m_tme->setRunning(false);
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
		channelControl->getDigitalChannel()->updateTimeScale(value);
	}
}

void Max14906::initMonitorToolView() {
	this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createVLine(this->ui->grid), 1, 0);
	this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createVLine(this->ui->grid), 1, 2);

	this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createHLine(this->ui->grid), 0, 1);
	this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createHLine(this->ui->grid), 2, 1);

	// there can only be 4 channels, so we position them accordingly
	switch (m_channelControls.size()) {
		case 4: {
			DioDigitalChannel* digitalChannel = m_channelControls[3]->getDigitalChannel();
			this->ui->gridLayout->addWidget(digitalChannel, 2, 2);
		}
		case 3: {
			DioDigitalChannel* digitalChannel = m_channelControls[2]->getDigitalChannel();
			this->ui->gridLayout->addWidget(digitalChannel, 2, 0);
		}
		case 2: {
			DioDigitalChannel* digitalChannel = m_channelControls[1]->getDigitalChannel();
			this->ui->gridLayout->addWidget(m_channelControls[1]->getDigitalChannel(), 0, 2);
		}
		case 1: {
			DioDigitalChannel* digitalChannel = m_channelControls[0]->getDigitalChannel();
			this->ui->gridLayout->addWidget(digitalChannel, 0, 0);
		}
		default: {
			break;
		}
	}
	this->ui->gridLayout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0);

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

QFrame *Max14906::createVLine(QWidget* parent) {
	auto *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::VLine);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setStyleSheet("background-color: grey;");
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frame->setFixedHeight(2);

	return frame;
}

QFrame *Max14906::createHLine(QWidget* parent) {
	auto *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::HLine);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setStyleSheet("background-color: grey;");
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frame->setFixedWidth(2);

	return frame;
}

#include "moc_max14906.cpp"
