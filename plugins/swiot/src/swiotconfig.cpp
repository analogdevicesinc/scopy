#include "swiotconfig.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "swiotconfigcontroller.hpp"
#include "swiotconfigmenu.hpp"
#include "swiotconfigmodel.hpp"
#include <core/toolmenuitem.h>
#include <iio.h>
#include <core/logging_categories.h>
#include <QVBoxLayout>

using namespace adiscope;

SwiotConfig::SwiotConfig(struct iio_context *ctx, QWidget *parent):
        QWidget(parent)
{
	m_funcAvailable.push_back(new QStringList());
	m_funcAvailable.push_back(new QStringList());
	m_configBtn = createConfigBtn();
	this->setLayout(new QVBoxLayout);
	this->layout()->addWidget(m_configBtn);
	setDevices(ctx);
	bool isConfig = isConfigCtx();

	if (isConfig) {
                adiscope::gui::ToolViewRecipe recipe;
                recipe.helpBtnUrl = "";
                recipe.hasRunBtn = false;
                recipe.hasSingleBtn = false;
                recipe.hasPairSettingsBtn = false;
                recipe.hasPrintBtn = false;
                recipe.hasChannels = false;
                recipe.hasHelpBtn = true;

                m_toolView = adiscope::gui::ToolViewBuilder(recipe, nullptr, parent).build();
                m_configMenu = new SwiotConfigMenu();
                m_toolView->addFixedCentralWidget(m_configMenu,0,0,0,0);
                m_toolView->addTopExtraWidget(m_configBtn);
                this->layout()->addWidget(m_toolView);
                init();
                connect(m_configBtn, &QPushButton::pressed, this, &SwiotConfig::configBtnPressed);
        }
}

SwiotConfig::~SwiotConfig()
{}

void SwiotConfig::init()
{
	if (m_iioDevices[AD_NAME] && m_iioDevices[MAX_NAME]) {
		int adChnlsNo = iio_device_get_channels_count(m_iioDevices[AD_NAME]);
		int maxChnlsNo = iio_device_get_channels_count(m_iioDevices[MAX_NAME]);
		if (adChnlsNo == maxChnlsNo) {
			for (int i = 0; i < adChnlsNo; i++) {
				struct iio_channel* adChnl = iio_device_get_channel(m_iioDevices[AD_NAME], i);
				struct iio_channel* maxChnl = iio_device_get_channel(m_iioDevices[MAX_NAME], i);
				gui::SwiotConfigChnlView* chnlsView = new gui::SwiotConfigChnlView(i);
				SwiotConfigModel* configModel = new SwiotConfigModel(adChnl, maxChnl);
				SwiotConfigController* configController = new SwiotConfigController(chnlsView, configModel);
				configController->addMenuAttrValues();
				m_controllers.push_back(configController);
				m_configMenu->addWidget(chnlsView);
			}
		}
		qInfo(CAT_SWIOT_CONFIG) << "Initialized";
	}
}

bool SwiotConfig::isConfigCtx()
{
	bool config = false;
	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "apply")) {
			config = true;
			break;
		}
	}
	return config;
}

void SwiotConfig::setDevices(iio_context* ctx)
{
	int devicesNumber = iio_context_get_devices_count(ctx);
	for (int i = 0; i < devicesNumber; i++) {
		struct iio_device* iioDev = iio_context_get_device(ctx, i);
		if (iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			m_iioDevices[deviceName] = iioDev;
		}
	}
}

QPushButton* SwiotConfig::createConfigBtn()
{
	QPushButton* configBtn = new QPushButton();
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

void SwiotConfig::configBtnPressed()
{
	for(int i = 0; i < m_funcAvailable.size(); i++) {
		m_funcAvailable[i]->clear();
	}
	for (int i = 0; i < m_controllers.size(); i++) {
		QStringList func = m_controllers[i]->getActiveFunctions();
		if (func.size()>0) {
			for(int i = 0; i < m_funcAvailable.size(); i++) {
				m_funcAvailable[i]->push_back(func[i]);
			}
		}
	}

	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "apply")) {
			iio_device_attr_write_bool(m_iioDevices[key], "apply", 1);
		}
	}
	qInfo(CAT_SWIOT_CONFIG) << "Apply bit has been set!";
	for(int i = 0; i < m_funcAvailable.size(); i++) {
		qDebug(CAT_SWIOT_CONFIG) << m_funcAvailable[i]->toVector();
	}
}
