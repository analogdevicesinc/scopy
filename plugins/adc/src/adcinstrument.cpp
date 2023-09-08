#include "adcinstrument.h"
#include <gui/tool_view.hpp>
#include <gui/tool_view_builder.hpp>

using namespace scopy;
AdcInstrument::AdcInstrument(PlotProxy* proxy, QWidget *parent) : QWidget(parent)
{
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = true;
	recipe.channelsPosition = scopy::gui::ChannelsPositionEnum::VERTICAL;

	auto m_monitorChannelManager = new scopy::gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus(" Channels");



	auto m_toolView = scopy::gui::ToolViewBuilder(recipe, m_monitorChannelManager, this).build();
	for(auto c : proxy->getChannelAddons()) {
		auto ch = m_toolView->buildNewChannel(m_monitorChannelManager,new gui::GenericMenu(),false,-1,true,false,QColor("red"),"LongName - Test1",c->getName());
		ch->setMenuButtonVisibility(false);

	}


	m_toolView->addDockableTabbedWidget(proxy->getPlotAddon()->getWidget(),"");
	m_toolView->setGeneralSettingsMenu(proxy->getPlotSettings()->getWidget(),false);


//	gui::GenericMenu *settingsMenu = createSettingsMenu("General settings", new QColor("Red"));
//	m_toolView->setGeneralSettingsMenu(settingsMenu, true);

}

