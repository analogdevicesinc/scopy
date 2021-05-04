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
#include <scopy/gui/power_supply_menu.hpp>
#include <scopy/gui/signal_generator_menu.hpp>
#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>
#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>
#include <scopy/gui/spectrum_analyzer_general_menu.hpp>
#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>
#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>
#include <scopy/gui/test_tool.hpp>
#include <scopy/gui/voltmeter_menu.hpp>

using namespace scopy::gui;

TestTool::TestTool()
{
	// Spectrum Analyzer wannabe

	ToolViewRecipe recipe;
	recipe.helpBtnUrl = "https://google.com";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPrintBtn = true;

	recipe.hasPairSettingsBtn = true;

	recipe.hasChannels = true;
	//	recipe.channelsPosition = ChannelsPositionEnum::VERTICAL;

	ChannelManager* channelManager = new ChannelManager(recipe.channelsPosition);

	m_toolView = ToolViewBuilder(recipe, channelManager).build();

	m_toolView->setGeneralSettingsMenu(new SpectrumAnalyzerGeneralMenu);

	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerSweepMenu, "Sweep");
	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerMarkersMenu, "Markers");

	SpectrumAnalyzerChannelMenu* ch1Menu = new SpectrumAnalyzerChannelMenu("Channel 1", new QColor("#FF7200"));
	SpectrumAnalyzerChannelMenu* ch2Menu = new SpectrumAnalyzerChannelMenu("Channel 2", new QColor("#9013FE"));
	SpectrumAnalyzerChannelMenu* ch3Menu = new SpectrumAnalyzerChannelMenu("Math 1", new QColor("green"));

	m_toolView->buildNewChannel(channelManager, ch1Menu, 0, false, false, QColor("#FF7200"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch2Menu, 1, false, false, QColor("#9013FE"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch3Menu, 0, true, false, QColor("green"), "Math", "M");

	channelManager->insertAddBtn(new SpectrumAnalyzerAddReferenceMenu);
}

ToolView* TestTool::getToolView() { return m_toolView; }
