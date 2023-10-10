#include "tool_view_builder.hpp"

#include "dynamicWidget.h"

using namespace scopy::gui;

ToolViewBuilder::ToolViewBuilder(const ToolViewRecipe &recipe, ChannelManager *channelManager, QWidget *parent)
{
	m_toolView = new ToolView(parent);

	if(recipe.hasRunBtn) {
		m_toolView->setRunBtnVisible(true);
	}
	if(recipe.hasSingleBtn) {
		m_toolView->setSingleBtnVisible(true);
	}
	if(recipe.hasHelpBtn) {
		m_toolView->setHelpBtnVisible(true);
		m_toolView->setUrlHelpBtn(recipe.helpBtnUrl);
	}
	if(recipe.hasPrintBtn) {
		m_toolView->setPrintBtnVisible(true);
	}
	if(recipe.hasGroupBtn) {
		QPushButton *btn = new QPushButton;
		btn->setStyleSheet("QPushButton{ width: 80px;"
				   "height: 40px;"
				   "text-align: left;"
				   "font-weight: bold;"
				   "padding-left: 15px;"
				   "padding-right: 15px;}");
		btn->setText("Group");
		scopy::setDynamicProperty(btn, "blue_button", true);
		m_toolView->addTopExtraWidget(btn);
	}

	if(recipe.hasPairSettingsBtn) {
		m_toolView->setPairSettingsVisible(true);
		m_toolView->configureLastOpenedMenu();
	}

	if(recipe.hasChannels) {
		m_toolView->buildChannelsContainer(channelManager, recipe.channelsPosition);
	}

	m_toolView->setHeaderVisibility(recipe.hasHeader);
	m_toolView->getSettingsBtn()->setVisible(recipe.hasHamburgerMenuBtn);
}

ToolView *ToolViewBuilder::build() { return m_toolView; }
