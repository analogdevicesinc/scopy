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
	ToolViewRecipe recipe;
	recipe.url = "https://stackoverflow.com/questions/45481362/set-parent-qwidget-for-promoted-widgets";

	m_toolView = ToolViewBuilder(recipe).build();

	// Power Supply
	// PowerSupplyMenu* menu = new PowerSupplyMenu(m_toolView->getStackedWidget());

	// Voltmeter
	VoltmeterMenu* menu = new VoltmeterMenu(m_toolView->getStackedWidget());
	m_toolView->setFixedMenu(menu);
}

ToolView* TestTool::getToolView() { return m_toolView; }
