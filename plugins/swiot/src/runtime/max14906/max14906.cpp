/*
 * Copyright (c) 2023 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "max14906.h"

#include "src/swiot_logging_categories.h"
#include "utils.h"

#include <QDockWidget>
#include <QLineEdit>
#include <QMainWindow>

#include <gui/tool_view_builder.hpp>
#include <iioutil/commandqueueprovider.h>

#define MAX14906_POLLING_TIME 1000

using namespace scopy::swiot;

Max14906::Max14906(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, max14906ToolController(new DioController(ctx))
	, ui(new Ui::Max14906)
	, m_backButton(Max14906::createBackButton())
	, m_qTimer(new QTimer(this))
	, m_toolView(nullptr)
	, m_tme(tme)
	, m_max14906SettingsTab(nullptr)
	, m_generalSettingsMenu()
	, settingsWidgetSeparator()
	, m_statusLabel(new QLabel(this))
	, m_statusContainer(new QWidget(this))
	, m_ctx(ctx)
	, m_cmdQueue(CommandQueueProvider::GetInstance()->open(m_ctx))
	, m_readerThread(new ReaderThread(false, m_cmdQueue))
{
	this->ui->setupUi(this);
	m_nbDioChannels = max14906ToolController->getChannelCount();

	this->setupDynamicUi(this);
	this->connectSignalsAndSlots();

	this->m_qTimer->setInterval(MAX14906_POLLING_TIME); // poll once every second
	this->m_qTimer->setSingleShot(true);

	this->initChannels();
	this->initMonitorToolView();
	this->ui->mainLayout->addWidget(m_toolView);
}

void Max14906::setupDynamicUi(QWidget *parent)
{
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = false;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.hasHelpBtn = false;
	recipe.hasHamburgerMenuBtn = false;

	m_toolView = scopy::gui::ToolViewBuilder(recipe, nullptr, parent).build();

	this->m_generalSettingsMenu =
		this->createGeneralSettings("General settings", new QColor(0x4a, 0x64, 0xff)); // "#4a64ff"
	this->m_generalSettingsMenu->setProperty("tutorial_name", "GENERAL_SETTINGS");
	this->m_toolView->setGeneralSettingsMenu(this->m_generalSettingsMenu, true);

	m_statusLabel->setText("The system is powered at limited capacity.");
	m_statusLabel->setWordWrap(true);

	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0, 0, 0, 0);
	m_statusContainer->setStyleSheet("QWidget{color: #ffc904; background-color: rgba(0, 0, 0, 60); border: 1px "
					 "solid rgba(0, 0, 0, 30); font-size: 11pt}");

	auto exclamationLabel = new QPushButton(m_statusContainer);
	exclamationLabel->setIcon(QIcon::fromTheme(":/swiot/warning.svg"));
	exclamationLabel->setIconSize(QSize(32, 32));
	exclamationLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_statusContainer->layout()->addWidget(exclamationLabel);
	m_statusContainer->layout()->addWidget(m_statusLabel);

	m_toolView->addPlotInfoWidget(m_statusContainer);
	m_toolView->addFixedCentralWidget(this->ui->grid, 0, 0);
	m_toolView->getGeneralSettingsBtn()->setChecked(true);
	m_toolView->addTopExtraWidget(m_backButton);
	ui->grid->setStyleSheet("background-color: #1C1C20");

	m_toolView->getRunBtn()->setEnabled(!!m_nbDioChannels);
	m_toolView->getGeneralSettingsBtn()->setChecked(!!m_nbDioChannels);
	m_tme->setRunBtnVisible(!!m_nbDioChannels);

	m_toolView->getRunBtn()->setProperty("tutorial_name", "RUN_BUTTON");
	m_backButton->setProperty("tutorial_name", "CONFIG_BUTTON");
}

scopy::gui::GenericMenu *Max14906::createGeneralSettings(const QString &title, QColor *color)
{
	auto generalSettingsMenu = new scopy::gui::GenericMenu(this);
	generalSettingsMenu->initInteractiveMenu();
	generalSettingsMenu->setMenuHeader(title, color, false);

	settingsWidgetSeparator = new scopy::gui::SubsectionSeparator("MAX14906", false, this);
	this->m_max14906SettingsTab = new DioSettingsTab(settingsWidgetSeparator);
	settingsWidgetSeparator->setContent(this->m_max14906SettingsTab);
	generalSettingsMenu->insertSection(settingsWidgetSeparator);

	return generalSettingsMenu;
}

void Max14906::connectSignalsAndSlots()
{
	connect(this->m_toolView->getRunBtn(), &QPushButton::toggled, this, &Max14906::runButtonToggled);
	//	connect(this->m_toolView->getSingleBtn(), &QPushButton::clicked, this, &Max14906::singleButtonToggled);
	QObject::connect(m_backButton, &QPushButton::clicked, this, &Max14906::onBackBtnPressed);

	connect(this->m_max14906SettingsTab, &DioSettingsTab::timeValueChanged, this, &Max14906::timerChanged);
	connect(this->m_qTimer, &QTimer::timeout, this, [&]() { this->m_readerThread->start(); });
	connect(m_readerThread, &ReaderThread::started, this, [&]() { this->m_qTimer->start(1000); });

	connect(m_tme, &ToolMenuEntry::runToggled, this->m_toolView->getRunBtn(), &QPushButton::setChecked);
}

void Max14906::onBackBtnPressed()
{
	bool runBtnChecked = m_toolView->getRunBtn()->isChecked();
	bool singleBtnChecked = m_toolView->getSingleBtn()->isChecked();

	if(runBtnChecked) {
		m_toolView->getRunBtn()->setChecked(false);
	}
	if(singleBtnChecked) {
		m_toolView->getSingleBtn()->setChecked(false);
	}
	Q_EMIT backBtnPressed();
}

Max14906::~Max14906()
{
	if(this->m_toolView) {
		if(this->m_toolView->getRunBtn()->isChecked()) {
			this->m_toolView->getRunBtn()->setChecked(false);
		}
		if(this->m_readerThread->isRunning()) {
			this->m_readerThread->forcedStop();
			this->m_readerThread->wait();
		}
		delete m_readerThread;
	}
	if(m_cmdQueue) {
		CommandQueueProvider::GetInstance()->close(m_ctx);
		m_cmdQueue = nullptr;
		m_ctx = nullptr;
	}
	delete m_toolView;
	delete ui;
}

void Max14906::runButtonToggled()
{
	qDebug(CAT_SWIOT_MAX14906) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if(this->m_toolView->getRunBtn()->isChecked()) {
		for(auto &channel : this->m_channelControls) {
			channel->getDigitalChannel()->resetSismograph();
		}
		qDebug(CAT_SWIOT_MAX14906) << "Reader thread started";
		this->m_readerThread->start();
		if(!this->m_tme->running()) {
			m_tme->setRunning(true);
		}
	} else {
		if(this->m_readerThread->isRunning()) {
			qDebug(CAT_SWIOT_MAX14906) << "Reader thread stopped";
			this->m_readerThread->forcedStop();
			this->m_readerThread->wait();
		}
		if(this->m_tme->running()) {
			m_tme->setRunning(false);
		}
		this->m_qTimer->stop();
	}
}

void Max14906::timerChanged(double value)
{
	for(auto &channelControl : this->m_channelControls) {
		channelControl->getDigitalChannel()->updateTimeScale(value);
	}
}

void Max14906::initMonitorToolView()
{
	this->ui->grid->setProperty("tutorial_name", "DIO_DIGITAL_CHANNEL_SPACE");
	int nbChannels = m_channelControls.size();
	int lineSpan;
	if(nbChannels > 2) {
		lineSpan = 3;
		this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createVLine(this->ui->grid), 1, 0, 1, lineSpan);

		this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createHLine(this->ui->grid), 0, 1, lineSpan, 1);
	} else if(nbChannels == 2) {
		lineSpan = 1;
		this->ui->gridLayout->addWidget(scopy::swiot::Max14906::createVLine(this->ui->grid), 1, 0, 1, lineSpan);
	}

	// there can only be 4 channels, so we position them accordingly
	switch(m_channelControls.size()) {
	case 4: {
		DioDigitalChannel *digitalChannel = m_channelControls[3]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, this);

		this->ui->gridLayout->addWidget(mainWindow, 2, 2);
	}
	case 3: {
		DioDigitalChannel *digitalChannel = m_channelControls[2]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, this);

		this->ui->gridLayout->addWidget(mainWindow, 0, 2);
	}
	case 2: {
		DioDigitalChannel *digitalChannel = m_channelControls[1]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, this);

		this->ui->gridLayout->addWidget(mainWindow, 2, 0);
	}
	case 1: {
		DioDigitalChannel *digitalChannel = m_channelControls[0]->getDigitalChannel();
		auto mainWindow = createDockableMainWindow("", digitalChannel, this);

		this->ui->gridLayout->addWidget(mainWindow, 0, 0);
	}
	default: {
		break;
	}
	}
	this->ui->gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0);

	setUpdatesEnabled(true);
}

void Max14906::initChannels()
{
	for(int i = 0; i < m_nbDioChannels; ++i) {
		struct iio_channel *channel = iio_device_get_channel(this->max14906ToolController->getDevice(), i);
		DioDigitalChannelController *channel_control = new DioDigitalChannelController(
			channel, this->max14906ToolController->getChannelName(i),
			this->max14906ToolController->getChannelType(i), m_cmdQueue, this);

		this->m_channelControls.insert(i, channel_control);
		this->m_readerThread->addDioChannel(i, channel);
		connect(this->m_readerThread, &ReaderThread::channelDataChanged, channel_control,
			[this, i](int index, double value) {
				if(i == index) {
					this->m_channelControls.value(index)->getDigitalChannel()->addDataSample(value);
				}
			});
	}
}

QPushButton *Max14906::createBackButton()
{
	auto *backButton = new QPushButton();
	backButton->setObjectName(QString::fromUtf8("backButton"));
	backButton->setLayoutDirection(Qt::RightToLeft);
	backButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						    "  width: 80px;\n"
						    "  height: 40px;\n"
						    "\n"
						    "  font-size: 12px;\n"
						    "  text-align: left;\n"
						    "  font-weight: bold;\n"
						    "  padding-left: 15px;\n"
						    "  padding-right: 15px;\n"
						    "}"));
	backButton->setProperty("blue_button", QVariant(true));
	backButton->setText("Config");
	QIcon icon = QIcon(":/gui/icons/scopy-default/icons/preferences.svg");
	backButton->setIcon(icon);
	backButton->setIconSize(QSize(13, 13));
	return backButton;
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

void Max14906::externalPowerSupply(bool ps)
{
	if(ps) {
		m_statusContainer->hide();
	} else {
		m_statusContainer->show();
		m_statusLabel->show();
	}
}

#include "moc_max14906.cpp"
