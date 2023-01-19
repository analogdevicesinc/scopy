#include "newinstrument.hpp"
#include <gui/tool_view_builder.hpp>
#include "plugin/ichannelplugin.h"
#include "plugin/irightmenuplugin.h"
#include "plugin/physical_channelplugin.cpp"
#include <plugin/add_channelplugin/cpp.cpp>


using namespace adiscope;
using namespace gui;

NewInstrument::NewInstrument(struct iio_context *ctx, Filter *filt,
			     ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, nullptr, "NewInstrument",
	     parent),
	ui(new Ui::NewInstrument)
{
	m_parent = parent;
	run_button = nullptr;
	ui->setupUi(this);

	m_recipe.helpBtnUrl = "";
	m_recipe.hasRunBtn = true;
	m_recipe.hasSingleBtn = true;
	m_recipe.hasPairSettingsBtn = true;
	m_recipe.hasPrintBtn = true;
	m_recipe.hasChannels = true;
	m_recipe.hasRightMenu = true;
	m_recipe.channelsPosition = adiscope::gui::ChannelsPositionEnum::VERTICAL;

	// managers
	m_channelManager = new adiscope::gui::ChannelManager(m_recipe.channelsPosition);
	m_channelManager->enableSwitchButton(true);
	m_channelManager->setChannelIdVisible(false);

	// tool view
	m_toolView = ToolViewBuilder(m_recipe, m_channelManager, m_parent).build();
	this->setLayout(new QVBoxLayout(this));
	this->layout()->addWidget(m_toolView);

	run_button = m_toolView->getRunBtn();

	initGeneralSettings();
	addPlugins();
}

void NewInstrument::initGeneralSettings()
{
	m_generalSettingsMenu = new GenericMenu(this);
	m_generalSettingsMenu->initInteractiveMenu();
	m_generalSettingsMenu->setMenuHeader("General Settings", new QColor("grey"), false);
	m_toolView->setGeneralSettingsMenu(m_generalSettingsMenu, false);
}

void NewInstrument::addPlugins()
{
	if (m_recipe.hasChannels) {
		// add plugins
		m_channelPluginList.push_back(new PhysicalChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new IChannelPlugin(this, m_toolView, m_channelManager));
		m_channelPluginList.push_back(new AddChannelPlugin(this, m_toolView, m_channelManager));

		m_rightMenuPluginList.push_back(new IRightMenuPlugin(this, m_toolView));
		m_rightMenuPluginList.push_back(new IRightMenuPlugin(this, m_toolView));
		m_rightMenuPluginList.push_back(new IRightMenuPlugin(this, m_toolView));

		// init plugins
		for (auto ch: m_channelPluginList) {
			ch->init();
		}

		for (auto ch: m_rightMenuPluginList) {
			ch->init();
		}
	}
}

NewInstrument::~NewInstrument()
{
	for (auto ch : m_channelPluginList) {
		delete ch;
	}
	for (auto rm : m_rightMenuPluginList) {
		delete rm;
	}
	m_channelPluginList.clear();
	m_rightMenuPluginList.clear();

	if (m_toolView->getRunBtn()->isChecked()) {
			m_toolView->getRunBtn()->setChecked(false);
	}

	if (m_toolView) {
		delete m_toolView;
		run_button = nullptr;
		m_toolView = nullptr;
	}
}
