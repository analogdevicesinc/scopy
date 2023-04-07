#include "faults.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"

#include <QTimer>
#include <QThread>

#include "core/logging_categories.h"
#include "src/refactoring/maingui/channel_manager.hpp"

using namespace adiscope::swiot;

#define POLLING_INTERVAL 1000
#define FAULT_CHANNEL_NAME "voltage"

Faults::Faults(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent),
	ctx(ctx),
	ui(new Ui::Faults),
	timer(new QTimer()),
	thread(new QThread(this)) {

        iio_device* device0 = iio_context_get_device(ctx, 0);
        if (iio_device_find_attr(device0, "back")) {
                m_faultsPage = new FaultsPage(this);

                qInfo(CAT_SWIOT_FAULTS) << "Initialising SWIOT faults page.";

                ui->setupUi(this);

                this->setupDynamicUi(parent);
                this->connectSignalsAndSlots();
        } else {
                qInfo(CAT_SWIOT_FAULTS) << "Could not initialize SWIOT faults page, the device seems to be in config mode.";
        }
}

Faults::~Faults() {
	if (this->thread->isRunning()) {
		this->thread->quit();
		this->thread->wait();
	}
	delete thread;
	delete ui;
}

void Faults::setupDynamicUi(QWidget *parent) {
	adiscope::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = false;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.channelsPosition = adiscope::gui::ChannelsPositionEnum::HORIZONTAL;

	this->m_monitorChannelManager = new adiscope::gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);

	this->m_toolView = gui::ToolViewBuilder(recipe, this->m_monitorChannelManager, parent).build();

//	this->m_generalSettingsMenu = this->createGeneralSettings("General settings", new QColor(0x4a, 0x64, 0xff)); // "#4a64ff"
//	this->m_toolView->setGeneralSettingsMenu(this->m_generalSettingsMenu, true);

	this->m_toolView->addFixedCentralWidget(m_faultsPage,0,0,-1,-1);

	this->ui->mainLayout->addWidget(m_toolView);
	this->m_toolView->getGeneralSettingsBtn()->setChecked(true);
}

adiscope::gui::GenericMenu *Faults::createGeneralSettings(const QString &title, QColor *color) {
	auto generalSettingsMenu = new adiscope::gui::GenericMenu(this);
	generalSettingsMenu->initInteractiveMenu();
	generalSettingsMenu->setMenuHeader(title, color, false);

	auto *testLabel = new QLabel("coming soon");
	auto *settingsWidgetSeparator = new adiscope::gui::SubsectionSeparator("MAX14906", false, this);
	settingsWidgetSeparator->setContent(testLabel);
	generalSettingsMenu->insertSection(settingsWidgetSeparator);

	return generalSettingsMenu;
}

void Faults::connectSignalsAndSlots() {
	QObject::connect(this->m_toolView->getRunBtn(), &QPushButton::toggled, this,
			 &Faults::runButtonClicked);
	QObject::connect(this->m_toolView->getSingleBtn(), &QPushButton::clicked, this,
			 &Faults::singleButtonClicked);

	QObject::connect(this->timer, &QTimer::timeout, this, &Faults::pollFaults);
	QObject::connect(this->thread, &QThread::started, this, [&](){
		qDebug(CAT_SWIOT_FAULTS) << "Faults reader thread started";
		this->timer->start(POLLING_INTERVAL);
	});
}

void Faults::getAd74413rFaultsNumeric()
{
	iio_device *dev = nullptr;
	unsigned int devCount = iio_context_get_devices_count(ctx);
	for (int i = 0; i < devCount; ++i) {
		iio_device *aux = iio_context_get_device(ctx, i);
		std::string name = iio_device_get_name(aux);
		if (name == "ad74413r") {
			dev = aux;
			break;
		}
	}

	if (dev == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "No device was found";
		return;
	}

	iio_channel *chn = iio_device_find_channel(dev, FAULT_CHANNEL_NAME, false);
	if (chn == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "Device is not found";
		return;
	}

	char fau[100];
	iio_channel_attr_read(chn, "raw", fau, 100);

	qDebug(CAT_SWIOT_FAULTS) << "ad74413r_numeric: " << fau;
	this->ad74413r_numeric = std::stoi(fau);
}

void Faults::getMax14906FaultsNumeric() {
	iio_device *dev = nullptr;
	unsigned int devCount = iio_context_get_devices_count(this->ctx);
	for (int i = 0; i < devCount; ++i) {
		iio_device *aux = iio_context_get_device(this->ctx, i);
		std::string name = iio_device_get_name(aux);
		if (name == "max14906") {
			dev = aux;
			break;
		}
	}

	if (dev == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "No device was found";
		return;
	}

	iio_channel *chn = iio_device_find_channel(dev, FAULT_CHANNEL_NAME, false);
	if (chn == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "Device is not found";
		return;
	}

	char fau[100];
	iio_channel_attr_read(chn, "raw", fau, 100);

	qDebug(CAT_SWIOT_FAULTS) << "max14906_numeric: " << fau;
	this->max14906_numeric = std::stoi(fau);
}

void Faults::runButtonClicked() {
	qDebug(CAT_SWIOT_FAULTS) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if (this->m_toolView->getRunBtn()->isChecked()) {
		qWarning() << "thread started";
		this->thread->start();
	} else {
		if (this->thread->isRunning()) {
			qWarning() << "thread stopped";
			this->thread->quit();
			this->thread->wait();
		}
		this->timer->stop();
	}
}

void Faults::singleButtonClicked() {
	qDebug(CAT_SWIOT_FAULTS) << "Single button clicked";
	this->m_toolView->getRunBtn()->setChecked(false);
	this->timer->stop();
	this->pollFaults();
	this->m_toolView->getSingleBtn()->setChecked(false);
}

void Faults::pollFaults() {
	qDebug(CAT_SWIOT_FAULTS) << "Polling faults...";
	this->getAd74413rFaultsNumeric();
	this->getMax14906FaultsNumeric();
	this->m_faultsPage->update(this->ad74413r_numeric, this->max14906_numeric);
}
