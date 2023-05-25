#include "swiotconfig.h"
#include <gui/tool_view_builder.hpp>
#include "configcontroller.h"
#include "configmenu.h"
#include "configmodel.h"
#include <iio.h>
#include "src/swiot_logging_categories.h"
#include <QVBoxLayout>

using namespace scopy::swiot;

SwiotConfig::SwiotConfig(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConfigMenu)
{
	m_swiotDevice = iio_context_find_device(ctx, "swiot"); // TODO: clean

	this->ui->setupUi(this);
	m_configBtn = createConfigBtn();
	this->setLayout(new QVBoxLayout(this));
	this->layout()->addWidget(m_configBtn);

	this->setupToolView(parent);
	this->init(ctx);
	this->createPageLayout();
	connect(m_configBtn, &QPushButton::pressed, this, &SwiotConfig::configBtnPressed);
}

SwiotConfig::~SwiotConfig() {}

void SwiotConfig::init(struct iio_context *ctx) {
	for (int i = 0; i < 4; i++) { // there can only be 4 channels
		auto *channelView = new ConfigChannelView(i);
		auto *configModel = new ConfigModel(m_swiotDevice, i);
		auto *configController = new ConfigController(channelView, configModel, i);
		m_controllers.push_back(configController);
		ui->gridLayout->addWidget(channelView->getChannelLabel(), i+1, 0);
		ui->gridLayout->addWidget(channelView->getEnabledCheckBox(), i+1, 1);
		ui->gridLayout->addWidget(channelView->getDeviceOptions(), i+1, 2);
		ui->gridLayout->addWidget(channelView->getFunctionOptions(), i+1, 3);
	}
}

void SwiotConfig::setDevices(iio_context *ctx) {
	int devicesNumber = iio_context_get_devices_count(ctx);
	for (int i = 0; i < devicesNumber; i++) {
		struct iio_device *iioDev = iio_context_get_device(ctx, i);
		if (iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			m_iioDevices[deviceName] = iioDev;
		}
	}
}

QPushButton *SwiotConfig::createConfigBtn() {
	QPushButton *configBtn = new QPushButton();
	configBtn->setObjectName(QString::fromUtf8("configBtn"));
	configBtn->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						   "  width: 95px;\n"
						   "  height: 40px;\n"
						   "\n"
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
//	for (int i = 0; i < m_funcAvailable.size(); i++) {
//		m_funcAvailable[i]->clear();
//	}
//	for (int i = 0; i < m_controllers.size(); i++) {
//		QStringList func = m_controllers[i]->getActiveFunctions();
//		if (func.size() == m_funcAvailable.size()) {
//			for (int j = 0; j < m_funcAvailable.size(); j++) {
//				m_funcAvailable[j]->push_back(func[j]);
//			}
//		}
//	}
//
//	for (const auto &key: m_iioDevices.keys()) {
//		if (iio_device_find_attr(m_iioDevices[key], "apply")) {
//			iio_device_attr_write_bool(m_iioDevices[key], "apply", 1);
//		}
//	}
	iio_device_attr_write(m_swiotDevice, "mode", "runtime");

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
//	this->setMinimumSize(m_configMenu->size());
	auto scrollWidget = new QWidget(this);
	scrollWidget->setLayout(new QVBoxLayout(scrollWidget));
	scrollWidget->layout()->addWidget(this->ui->mainGrid);
	scrollWidget->layout()->addWidget(m_drawArea);
	scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_scrollArea->setWidget(scrollWidget);
	m_scrollArea->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	m_toolView->addFixedCentralWidget(m_scrollArea, 0, 0, 0, 0);
	m_toolView->addTopExtraWidget(m_configBtn);
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);
}
