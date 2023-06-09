#include "swiotconfig.h"
#include <gui/tool_view_builder.hpp>
#include "configcontroller.h"
#include "configmodel.h"
#include <iio.h>
#include "src/swiot_logging_categories.h"
#include <QVBoxLayout>
#include <QMessageBox>

using namespace scopy::swiot;

SwiotConfig::SwiotConfig(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent),
	m_context(ctx),
	m_drawArea(nullptr),
	m_scrollArea(nullptr),
	m_toolView(nullptr),
	m_mainView(new QWidget(this)),
	m_statusLabel(new QLabel(this)),
	m_statusContainer(new QWidget(this)),
	ui(new Ui::ConfigMenu) {
	m_swiotDevice = iio_context_find_device(ctx, "swiot");
	if (m_swiotDevice == nullptr) {
		qCritical(CAT_SWIOT_CONFIG) << "Critical error: the \"swiot\" device was not found.";
	}

	this->ui->setupUi(this);
	m_configBtn = createConfigBtn();
	this->setLayout(new QVBoxLayout(this));
	this->layout()->addWidget(m_configBtn);

	this->setupToolView(parent);
	this->init();
	this->createPageLayout();
	QObject::connect(m_configBtn, &QPushButton::pressed, this, &SwiotConfig::configBtnPressed);

	// The "ext_psu" attribute will be checked only once in the config context
	bool extPowerSupplyConnected = false;
	int res = iio_device_attr_read_bool(m_swiotDevice, "ext_psu", &extPowerSupplyConnected);
	if (res < 0) {
		qWarning(CAT_SWIOT_CONFIG) << "Error, could not read value from \"ext_psu\" from the swiot device.";
	} else {
		if (extPowerSupplyConnected) {
			m_statusLabel->hide();
		}
	}
}

SwiotConfig::~SwiotConfig() {}

void SwiotConfig::init() {
	for (int i = 0; i < 4; i++) { // there can only be 4 channels
		auto *channelView = new ConfigChannelView(i);
		auto *configModel = new ConfigModel(m_swiotDevice, i);
		auto *configController = new ConfigController(channelView, configModel, i);
		m_controllers.push_back(configController);
		ui->gridLayout->addWidget(channelView->getChannelLabel(), i + 1, 0);
		ui->gridLayout->addWidget(channelView->getEnabledCheckBox(), i + 1, 1);
		ui->gridLayout->addWidget(channelView->getDeviceOptions(), i + 1, 2);
		ui->gridLayout->addWidget(channelView->getFunctionOptions(), i + 1, 3);
	}
}

void SwiotConfig::setDevices(iio_context *ctx) {
	ssize_t devicesNumber = iio_context_get_devices_count(ctx);
	for (int i = 0; i < devicesNumber; i++) {
		struct iio_device *iioDev = iio_context_get_device(ctx, i);
		if (iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			m_iioDevices[deviceName] = iioDev;
		}
	}
}

QPushButton *SwiotConfig::createConfigBtn() {
	auto *configBtn = new QPushButton();
	configBtn->setObjectName(QString::fromUtf8("configBtn"));
	configBtn->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						   "  width: 95px;\n"
						   "  height: 40px;\n"
						   "  font-size: 12px;\n"
						   "  text-align: center;\n"
						   "  font-weight: bold;\n"
						   "  padding-left: 15px;\n"
						   "  padding-right: 15px;\n"
						   "}"));
	configBtn->setProperty("blue_button", QVariant(true));
	configBtn->setText("Apply");
	return configBtn;
}

void SwiotConfig::configBtnPressed() {
	ssize_t res = iio_device_attr_write(m_swiotDevice, "mode", "runtime");
	if (res >= 0 ) {
		qDebug(CAT_SWIOT_CONFIG) << "Swiot mode changed to runtime!";
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Swiot mode cannot be changed to runtime!";
	}
	Q_EMIT configBtn();

	qInfo(CAT_SWIOT_CONFIG) << "Swiot mode has been changed to runtime";
}

void SwiotConfig::setupToolView(QWidget *parent) {
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = false;
	recipe.hasSingleBtn = false;
	recipe.hasPairSettingsBtn = false;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.hasHelpBtn = true;

	m_toolView = scopy::gui::ToolViewBuilder(recipe, nullptr, parent).build();

	m_drawArea = new DrawArea(this);
	m_scrollArea = new QScrollArea(this);

	m_scrollArea->setLayout(new QVBoxLayout(this));
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}

void SwiotConfig::createPageLayout() {
	auto scrollWidget = new QWidget(this);
	scrollWidget->setLayout(new QVBoxLayout(scrollWidget));
	scrollWidget->layout()->addWidget(this->ui->mainGrid);
	scrollWidget->layout()->addWidget(m_drawArea);
	scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_scrollArea->setWidget(scrollWidget);
	m_scrollArea->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	m_statusLabel->setText("The external power supply is not connected. The MAX14906 chip will not be used at full capacity.");
	m_statusLabel->setWordWrap(true);

	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0,0,0,0);
	m_statusContainer->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 60); border: 1px solid rgba(0, 0, 0, 30); font-size: 11pt");

	auto exclamationButton = new QPushButton(m_statusContainer);
	exclamationButton->setIcon(QIcon::fromTheme(":/swiot/warning.svg"));
	exclamationButton->setIconSize(QSize(32, 32));
	exclamationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_statusContainer->layout()->addWidget(exclamationButton);
	m_statusContainer->layout()->addWidget(m_statusLabel);

	m_mainView->setLayout(new QVBoxLayout(m_mainView));
	m_mainView->layout()->addWidget(m_scrollArea);

	m_toolView->addPlotInfoWidget(m_statusContainer);
	m_toolView->addFixedCentralWidget(m_mainView, 0, 0, 0, 0);
	m_toolView->addTopExtraWidget(m_configBtn);
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);
}

void SwiotConfig::externalPowerSupply(bool ps) {
	if (ps) {
		m_statusContainer->hide();
	} else {
		m_statusContainer->show();
		m_statusLabel->show();
	}
}

#include "moc_swiotconfig.cpp"
