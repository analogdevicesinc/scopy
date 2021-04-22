#include "menu_anim.hpp"

#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/settings_pair_widget.hpp>
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

	recipe.hasChannels = true;
	recipe.hasAddMathBtn = true;

	recipe.hasSweep = true;
	recipe.hasMarkers = true;

	m_toolView = ToolViewBuilder(recipe).build();

	QWidget* newCh = m_toolView->buildNewChannel(0, true, false, QColor("green"), "Math", "M");

	SpectrumAnalyzerSweepMenu* sweepMenu = new SpectrumAnalyzerSweepMenu(m_toolView->getMenu(MenusEnum::SWEEP));
	SpectrumAnalyzerGeneralMenu* generalMenu =
		new SpectrumAnalyzerGeneralMenu(m_toolView->getGeneralSettingsMenu());
	SpectrumAnalyzerMarkersMenu* markersMenu =
		new SpectrumAnalyzerMarkersMenu(m_toolView->getMenu(MenusEnum::MARKERS));
	SpectrumAnalyzerChannelMenu* channelMenu =
		new SpectrumAnalyzerChannelMenu(m_toolView->getMenu(MenusEnum::CHANNELS_SETTINGS));
}

ToolView* TestTool::getToolView() { return m_toolView; }
