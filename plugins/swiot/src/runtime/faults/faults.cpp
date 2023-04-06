#include "faults.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"

#include <QTimer>
#include <QThread>

#include "src/swiot_logging_categories.h"
#include "src/refactoring/maingui/channel_manager.hpp"

using namespace adiscope::swiot;

#define POLLING_INTERVAL 1000
#define FAULT_CHANNEL_NAME "voltage"

Faults::Faults(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent),
	ctx(ctx),
	ui(new Ui::Faults),
	timer(new QTimer()),
        ad74413r_numeric(0),
        max14906_numeric(0),
        m_backButton(Faults::createBackButton()),
	thread(new QThread(this)) {

        iio_device* device0 = iio_context_get_device(ctx, 0);
        const char* backAttr = iio_device_find_attr(device0, "back");
        if (backAttr != nullptr) {
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

	this->m_toolView->addFixedCentralWidget(m_faultsPage,0,0,-1,-1);

	this->ui->mainLayout->addWidget(m_toolView);
	this->m_toolView->getGeneralSettingsBtn()->setChecked(true);

        this->m_toolView->addTopExtraWidget(m_backButton);
}

void Faults::connectSignalsAndSlots() {
	QObject::connect(this->m_toolView->getRunBtn(), &QPushButton::toggled, this,
			 &Faults::runButtonClicked);
	QObject::connect(this->m_toolView->getSingleBtn(), &QPushButton::clicked, this,
			 &Faults::singleButtonClicked);
        QObject::connect(m_backButton, &QPushButton::clicked, this, [this] () {
                Q_EMIT backBtnPressed();
        });

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
		qCritical(CAT_SWIOT_FAULTS) << "Channel" << FAULT_CHANNEL_NAME << "was not found";
		return;
	}

	char fau[100];
        ssize_t readResult = iio_channel_attr_read(chn, "raw", fau, 100);

        if (readResult < 0) {
                qCritical(CAT_SWIOT_FAULTS) << "AD74413R faults value could not be read.";
        } else {
                qDebug(CAT_SWIOT_FAULTS) << "AD74413R faults read the value:" << fau;
                try {
                        this->ad74413r_numeric = std::stoi(fau);
                } catch (std::invalid_argument& exception) {
                        qCritical(CAT_SWIOT_FAULTS) << "AD74413R faults value could not be converted from string to int, read" << fau << "; exception message:" << exception.what();
                }
        }
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
		qCritical(CAT_SWIOT_FAULTS) << "No device was found.";
		return;
	}

	iio_channel *chn = iio_device_find_channel(dev, FAULT_CHANNEL_NAME, false);
	if (chn == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "Device" << FAULT_CHANNEL_NAME << "was not found.";
		return;
	}

	char fau[100];
	ssize_t readResult = iio_channel_attr_read(chn, "raw", fau, 100);

        if (readResult < 0) {
                qCritical(CAT_SWIOT_FAULTS) << "MAX14906 faults value could not be read.";
        } else {
                qDebug(CAT_SWIOT_FAULTS) << "MAX14906 faults read the value:" << fau;
                try {
                        this->max14906_numeric = std::stoi(fau);
                } catch (std::invalid_argument& exception) {
                        qCritical(CAT_SWIOT_FAULTS) << "MAX14906 faults value could not be converted from string to int, read" << fau << "; exception message:" << exception.what();
                }
        }
}

void Faults::runButtonClicked() {
	qDebug(CAT_SWIOT_FAULTS) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if (this->m_toolView->getRunBtn()->isChecked()) {
		qDebug(CAT_SWIOT_FAULTS) << "thread started";
		this->thread->start();
	} else {
		if (this->thread->isRunning()) {
                        qDebug(CAT_SWIOT_FAULTS) << "thread stopped";
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

QPushButton *Faults::createBackButton() {
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

#include "moc_faults.cpp"
