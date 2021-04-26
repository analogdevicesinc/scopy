#include "menu_anim.hpp"

#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>
#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>
#include <scopy/gui/spectrum_analyzer_general_menu.hpp>
#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>
#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>
#include <scopy/gui/test_tool.hpp>
#include <scopy/gui/network_analyzer_general_menu.hpp>
#include <scopy/gui/network_analyzer_channel_menu.hpp>

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

	recipe.hasChannels = true;
	recipe.hasAddMathBtn = true;

	recipe.hasSweep = true;
	recipe.hasMarkers = true;

	m_toolView = ToolViewBuilder(recipe).build();

	QWidget* newCh = m_toolView->buildNewChannel(0, true, false, QColor("green"), "Math", "M");

	SpectrumAnalyzerSweepMenu* sweepMenu = new SpectrumAnalyzerSweepMenu(m_toolView->getMenu(MenusEnum::SWEEP));
	SpectrumAnalyzerGeneralMenu* generalMenu =
		new SpectrumAnalyzerGeneralMenu(m_toolView->getGeneralSettingsMenu());
	SpectrumAnalyzerAddReferenceMenu* addRefMenu =
		new SpectrumAnalyzerAddReferenceMenu(m_toolView->getMenu(MenusEnum::ADD_CHANNEL));
	SpectrumAnalyzerMarkersMenu* markersMenu =
		new SpectrumAnalyzerMarkersMenu(m_toolView->getMenu(MenusEnum::MARKERS));
	SpectrumAnalyzerChannelMenu* channelMenu =
		new SpectrumAnalyzerChannelMenu(m_toolView->getMenu(MenusEnum::CHANNELS_SETTINGS));
	// Network Analyzer
//	QWidget* menu = m_toolView->getGeneralSettingsMenu();
//	menu->setLayout(new QVBoxLayout);
//	NetworkAnalyzerGeneralMenu* generalMenu = new NetworkAnalyzerGeneralMenu(menu);

//	menu = m_toolView->getMenu(MenusEnum::CHANNEL);
//	menu->setLayout(new QVBoxLayout);
//	NetworkAnalyzerChannelMenu* channelMenu = new NetworkAnalyzerChannelMenu(menu);

//	menu = m_toolView->getMenu(MenusEnum::CURSORS);
//	menu->setLayout(new QVBoxLayout);
//	CursorsSettings* cursorsMenu = new CursorsSettings(menu);
//	cursorsMenu->setBtnNormalTrackVisible(false);
//	cursorsMenu->setHorizontalVisible(false);
//	cursorsMenu->setVerticalVisible(false);
}

ToolView* TestTool::getToolView() { return m_toolView; }
