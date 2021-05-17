#include "menu_anim.hpp"

#include <QLayout>

#include <scopy/gui/channel_settings.hpp>
#include <scopy/gui/cursors_settings.hpp>
#include <scopy/gui/logic_analyzer_channel_menu.hpp>
#include <scopy/gui/logic_analyzer_general_menu.hpp>
#include <scopy/gui/logic_pattern_trigger_menu.hpp>
#include <scopy/gui/measure_settings.hpp>
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

	m_toolView->setGeneralSettingsMenu(new SpectrumAnalyzerGeneralMenu, true);

	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerSweepMenu, true, "Sweep");
	m_toolView->buildNewInstrumentMenu(new SpectrumAnalyzerMarkersMenu, true, "Markers");

	//	MeasureSettings* measureMenu = new MeasureSettings;
	//	measureMenu->setChannelName("Channel 1");
	//	measureMenu->setChannelUnderlineColor(new QColor("#FF7200"));

	//	m_toolView->buildNewInstrumentMenu(new CursorsSettings, true, "Cursors", true, true);
	//	m_toolView->buildNewInstrumentMenu(measureMenu, true, "Measure", true, true);
	//	m_toolView->buildNewInstrumentMenu(new TriggerMenu, true, "Trigger");

	SpectrumAnalyzerChannelMenu* ch1Menu = new SpectrumAnalyzerChannelMenu("Channel 1", new QColor("#FF7200"));
	SpectrumAnalyzerChannelMenu* ch2Menu = new SpectrumAnalyzerChannelMenu("Channel 2", new QColor("#9013FE"));
	SpectrumAnalyzerChannelMenu* ch3Menu = new SpectrumAnalyzerChannelMenu("Math 1", new QColor("green"));

	m_toolView->buildNewChannel(channelManager, ch1Menu, true, 0, false, false, QColor("#FF7200"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch2Menu, true, 1, false, false, QColor("#9013FE"), "Channel", "CH");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 0, true, false, QColor("green"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 1, true, false, QColor("pink"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 2, true, false, QColor("red"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 3, true, false, QColor("blue"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 4, true, false, QColor("purple"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 5, true, false, QColor("yellow"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 6, true, false, QColor("aqua"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 7, true, false, QColor("orange"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 8, true, false, QColor("white"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 9, true, false, QColor("blue"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 10, true, false, QColor("magenta"), "Math", "M");
	m_toolView->buildNewChannel(channelManager, ch3Menu, true, 11, true, false, QColor("green"), "Math", "M");

	channelManager->insertAddBtn(new SpectrumAnalyzerAddReferenceMenu, true);

	//	m_toolView->setFixedMenu(new VoltmeterMenu(m_toolView->getStackedWidget()), true);
}

ToolView* TestTool::getToolView() { return m_toolView; }
