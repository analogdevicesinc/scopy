#include "datalogger_tool.hpp"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include "dataloggerplugin.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"


using namespace adiscope::datalogger;

DataloggerTool::DataloggerTool(struct iio_context *ctx, QWidget *parent):
	parent(parent),
	ctx(ctx)
{
	this->installEventFilter(this);

	this->setLayout(new QVBoxLayout);
//        this->m_scrollArea->setStyleSheet("background-color: red;");
	this->layout()->addWidget(new QLabel("DATALOGGER"));
//	bool isConfig = isConfigCtx();

//	adiscope::gui::ToolViewRecipe recipe;
//	recipe.helpBtnUrl = "";
//	recipe.hasRunBtn = true;
//	recipe.hasSingleBtn = true;
//	recipe.hasPairSettingsBtn = true;
//	recipe.hasPrintBtn = false;
//	recipe.hasChannels = false;
//	recipe.hasHelpBtn = false;

//	m_toolView = adiscope::gui::ToolViewBuilder(recipe, nullptr, parent).build();

	init();
//	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

DataloggerTool::~DataloggerTool()
{

}

void DataloggerTool::init()
{
	datalogger = new DataLogger(ctx, parent);
	this->setLayout(new QVBoxLayout);
	this->layout()->addWidget(datalogger->getToolView());

//	if (m_iioDevices[AD_NAME] && m_iioDevices[MAX_NAME]) {
//		int adChnlsNo = iio_device_get_channels_count(m_iioDevices[AD_NAME]);
//		int maxChnlsNo = iio_device_get_channels_count(m_iioDevices[MAX_NAME]);
//		if (adChnlsNo == maxChnlsNo) {
//			for (int i = 0; i < adChnlsNo; i++) {
//				struct iio_channel* adChnl = iio_device_get_channel(m_iioDevices[AD_NAME], i);
//				struct iio_channel* maxChnl = iio_device_get_channel(m_iioDevices[MAX_NAME], i);
//				ConfigChannelView* chnlsView = new ConfigChannelView(i);
//				ConfigModel* configModel = new ConfigModel(adChnl, maxChnl);
//				ConfigController* configController = new ConfigController(chnlsView, configModel);
//				configController->addMenuAttrValues();
//				m_controllers.push_back(configController);
//				m_configMenu->addWidget(chnlsView);
//			}
//		}
		qInfo(CAT_DATALOGGER_TOOL) << "Initialized";
//	}
}

bool DataloggerTool::eventFilter(QObject *object, QEvent *event)
{ // FIXME: delete this and uninstall event filter
	if (event->type() == QEvent::Resize) {
	}
	return QObject::eventFilter(object, event);
}
