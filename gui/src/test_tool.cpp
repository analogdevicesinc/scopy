#include "menu_anim.hpp"

#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/settings_pair_widget.hpp>
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

	recipe.hasExtraWidget = new QPushButton;

	recipe.hasPairSettingsBtn = true;

	recipe.hasChannels = true;
	recipe.hasAddMathBtn = true;

	recipe.hasCursors = true;
	recipe.hasMeasure = true;
	recipe.hasTrigger = true;

	m_toolView = ToolViewBuilder(recipe).build();

	QWidget* newCh = m_toolView->buildNewChannel(0, true, false, QColor("green"), "Math", "M");
}

ToolView* TestTool::getToolView() { return m_toolView; }
