#include "swiotconfig.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "configcontroller.hpp"
#include "configmenu.hpp"
#include "configmodel.hpp"
#include "core/toolmenuitem.h"
#include <iio.h>
#include "core/logging_categories.h"
#include <QVBoxLayout>
#include "core/logging_categories.h"

using namespace adiscope::swiot;

SwiotConfig::SwiotConfig(struct iio_context *ctx, QWidget *parent):
//        m_drawArea(new DrawArea(this)),
//        m_scrollArea(new QScrollArea(this)),
        QWidget(parent)
{
//        this->installEventFilter(this);

	m_funcAvailable.push_back(new QStringList());
	m_funcAvailable.push_back(new QStringList());
	m_configBtn = createConfigBtn();
//	this->setLayout(new QVBoxLayout);
//        this->m_scrollArea->setStyleSheet("background-color: red;");
//	this->layout()->addWidget(m_configBtn);
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

                m_configMenu = new ConfigMenu();
//                this->m_configMenu->setFixedSize(640, 480);
//                this->m_configMenu->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//		m_drawArea = new DrawArea(this);
//                m_scrollArea = new QScrollArea(this);
//                m_drawArea->setMaximumSize(400, 400);

		m_toolView->addFixedCentralWidget(m_configMenu,0,0,0,0);
//                m_scrollArea->setLayout(new QVBoxLayout(this));
//                m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
//                m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

                init();
//                m_configMenu->setMinimumSize(m_configMenu->size());
//                auto scrollWidget = new QWidget(this);
//                scrollWidget->setLayout(new QVBoxLayout(this));
//                scrollWidget->layout()->addWidget(m_configMenu);
//                scrollWidget->layout()->addWidget(m_drawArea);
//                m_scrollArea->setWidget(scrollWidget);
//                m_scrollArea->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
//                m_scrollArea->setStyleSheet("background-color: green");
//                m_configMenu->setStyleSheet("background-color: red;");
//                m_configMenu->setFixedSize(400, 400);
//                m_drawArea->setStyleSheet("background-color: blue");
                m_toolView->addTopExtraWidget(m_configBtn);
		this->setLayout(new QVBoxLayout());
		this->layout()->addWidget(m_toolView);
                connect(m_configBtn, &QPushButton::pressed, this, &SwiotConfig::configBtnPressed);
//                qDebug(CAT_SWIOT_CONFIG) << this->size() << this->sizeHint();
//                qDebug(CAT_SWIOT_CONFIG) << this->m_drawArea->size() << this->m_drawArea->sizeHint();
//                qDebug(CAT_SWIOT_CONFIG) << this->m_configMenu->size() << this->m_configMenu->sizeHint();
        }
//        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//        this->setStyleSheet("background-color: blue;");
//        auto idk = new QFrame(this);
//        idk->setFixedSize(200, 200);
//        idk->setStyleSheet("background-color: green;");
//        idk->setLayout(new QVBoxLayout(idk));
//        idk->layout()->addWidget(new DrawArea(idk));
//        this->layout()->addWidget(idk);
//        m_drawArea->setStyleSheet("background-color: green;");
//        m_drawArea->setMaximumSize(500, 500);
//        this->layout()->addWidget(m_drawArea);
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
				ConfigChannelView* chnlsView = new ConfigChannelView(i);
				ConfigModel* configModel = new ConfigModel(adChnl, maxChnl);
				ConfigController* configController = new ConfigController(chnlsView, configModel);
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
		if (func.size() == m_funcAvailable.size()) {
			for(int j = 0; j < m_funcAvailable.size(); j++) {
				m_funcAvailable[j]->push_back(func[j]);
			}
		}
	}

	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "apply")) {
			iio_device_attr_write_bool(m_iioDevices[key], "apply", 1);
		}
	}
	Q_EMIT configBtn(m_funcAvailable);

	qInfo(CAT_SWIOT_CONFIG) << "Apply bit has been set!";
}

//bool SwiotConfig::eventFilter(QObject *object, QEvent *event) { // FIXME: delete this and uninstall event filter
//	if (event->type() == QEvent::Resize) {
//		qDebug(CAT_SWIOT_CONFIG) << "this:" << this->size() << this->sizeHint();
//		qDebug(CAT_SWIOT_CONFIG) << "scroll: " << m_scrollArea->size() << m_scrollArea->sizeHint();
//		qDebug(CAT_SWIOT_CONFIG) << "draw:" << this->m_drawArea->size() << this->m_drawArea->sizeHint();
//		qDebug(CAT_SWIOT_CONFIG) << "menu:" << this->m_configMenu->size() << this->m_configMenu->sizeHint();
//	}
//	return QObject::eventFilter(object, event);
//}
