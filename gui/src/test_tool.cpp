#include "menu_anim.hpp"

#include <QLayout>

#include <scopy/gui/cursors_settings.hpp>
#include <scopy/gui/logic_analyzer_channel_menu.hpp>
#include <scopy/gui/logic_analyzer_general_menu.hpp>
#include <scopy/gui/logic_pattern_trigger_menu.hpp>
#include <scopy/gui/network_analyzer_channel_menu.hpp>
#include <scopy/gui/network_analyzer_general_menu.hpp>
#include <scopy/gui/pattern_generator_channel_menu.hpp>
#include <scopy/gui/pattern_generator_general_menu.hpp>
#include <scopy/gui/signal_generator_menu.hpp>
#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>
#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>
#include <scopy/gui/spectrum_analyzer_general_menu.hpp>
#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>
#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>
#include <scopy/gui/test_tool.hpp>

using namespace scopy::gui;

TestTool::TestTool()
{
	ToolViewRecipe recipe;
	recipe.url = "https://stackoverflow.com/questions/45481362/set-parent-qwidget-for-promoted-widgets";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPrintBtn = true;
	recipe.hasGroupBtn = true;

	recipe.hasPairSettingsBtn = true;

	recipe.hasChannelSettings = true;

	m_toolView = ToolViewBuilder(recipe).build();

	//	ChannelWidget* newCh = m_toolView->buildNewChannel(0, true, false, QColor("green"), "Math", "M");

	// Pattern Generator
	PatternGeneratorChannelMenu* channelMenu =
		new PatternGeneratorChannelMenu(m_toolView->getMenu(MenusEnum::CHANNEL));
	PatternGeneratorGeneralMenu* generalMenu =
		new PatternGeneratorGeneralMenu(m_toolView->getGeneralSettingsMenu());
}

ToolView* TestTool::getToolView() { return m_toolView; }
