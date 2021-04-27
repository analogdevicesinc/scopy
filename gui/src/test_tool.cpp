#include "menu_anim.hpp"

#include <QLayout>

#include <scopy/gui/cursors_settings.hpp>
#include <scopy/gui/logic_analyzer_channel_menu.hpp>
#include <scopy/gui/logic_analyzer_general_menu.hpp>
#include <scopy/gui/logic_pattern_trigger_menu.hpp>
#include <scopy/gui/network_analyzer_channel_menu.hpp>
#include <scopy/gui/network_analyzer_general_menu.hpp>
#include <scopy/gui/signal_generator_menu.hpp>
#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>
#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>
#include <scopy/gui/spectrum_analyzer_general_menu.hpp>
#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>
#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>
#include <scopy/gui/test_tool.hpp>
#include <scopy/gui/pattern_generator_channel_menu.hpp>

using namespace scopy::gui;

TestTool::TestTool()
{
	ToolViewRecipe recipe;
	recipe.url = "https://stackoverflow.com/questions/45481362/set-parent-qwidget-for-promoted-widgets";
	recipe.hasSingleBtn = true;
	recipe.hasPrintBtn = true;
	recipe.hasGroupBtn = true;

	recipe.hasPairSettingsBtn = true;

	recipe.hasChannelSettings = true;

	m_toolView = ToolViewBuilder(recipe).build();

	//	ChannelWidget* newCh = m_toolView->buildNewChannel(0, true, false, QColor("green"), "Math", "M");

	// Spectrum Analyzer
	//	SpectrumAnalyzerSweepMenu* sweepMenu = new
	//SpectrumAnalyzerSweepMenu(m_toolView->getMenu(MenusEnum::SWEEP)); 	SpectrumAnalyzerGeneralMenu* generalMenu =
	//		new SpectrumAnalyzerGeneralMenu(m_toolView->getGeneralSettingsMenu());
	//	SpectrumAnalyzerAddReferenceMenu* addRefMenu =
	//		new SpectrumAnalyzerAddReferenceMenu(m_toolView->getMenu(MenusEnum::ADD_CHANNEL));
	//	SpectrumAnalyzerMarkersMenu* markersMenu =
	//		new SpectrumAnalyzerMarkersMenu(m_toolView->getMenu(MenusEnum::MARKERS));
	//	SpectrumAnalyzerChannelMenu* channelMenu =
	//		new SpectrumAnalyzerChannelMenu(m_toolView->getMenu(MenusEnum::CHANNELS_SETTINGS));

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

	// Signal Generator
	//	QWidget* parent = m_toolView->getMenu(MenusEnum::CHANNELS_SETTINGS);
	//	SignalGeneratorMenu* menu = new SignalGeneratorMenu(parent);
	//	//todo: remove layout, setParent only
	//	parent->layout()->addWidget(menu);

	//	m_toolView->getStackedWidget()->setMinimumWidth(430);

	// Logic Analyzer
//	LogicAnalyzerChannelMenu* channelMenu = new LogicAnalyzerChannelMenu(m_toolView->getMenu(MenusEnum::CHANNEL));
//	TriggerMenu* triggerMenu = new TriggerMenu(m_toolView->getMenu(MenusEnum::TRIGGER));
//	LogicAnalyzerGeneralMenu* generalMenu = new LogicAnalyzerGeneralMenu(m_toolView->getGeneralSettingsMenu());

	// Pattern Generator
	PatternGeneratorChannelMenu* channelMenu = new PatternGeneratorChannelMenu(m_toolView->getMenu(MenusEnum::CHANNEL));

}

ToolView* TestTool::getToolView() { return m_toolView; }
