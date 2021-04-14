#include "dynamic_widget.hpp"

#include <scopy/gui/tool_view_builder.hpp>

using namespace scopy::gui;

ToolViewBuilder::ToolViewBuilder(const ToolViewRecipe& recipe)
{
	m_toolView = new ToolView();

	QMap<MenusEnum, CustomMenuButton*> btns;

	if (recipe.hasRunBtn) {
		m_toolView->setRunBtnVisible(true);
	}
	if (recipe.hasSingleBtn) {
		m_toolView->setSingleBtnVisible(true);
	}
	if (recipe.hasHelpBtn) {
		m_toolView->setHelpBtnVisible(true);
		m_toolView->setUrlHelpBtn(recipe.url);
	}
	if (recipe.hasPrintBtn) {
		m_toolView->setPrintBtnVisible(true);
	}
	if (recipe.hasGroupBtn) {
		QPushButton* btn = new QPushButton;
		btn->setStyleSheet("QPushButton{ width: 80px;"
				   "height: 40px;"
				   "text-align: left;"
				   "font-weight: bold;"
				   "padding-left: 15px;"
				   "padding-right: 15px;}");
		btn->setText("Group");
		DynamicWidget::setDynamicProperty(btn, "blue_button", true);
		m_toolView->setExtraWidget(btn);
	}
	if (recipe.hasExtraWidget) {
		m_toolView->setExtraWidget(recipe.hasExtraWidget);
	}

	if (recipe.hasPairSettingsBtn) {
		m_toolView->setPairSettingsVisible(true);
		m_toolView->setUpperMenus();
	}

	if (recipe.hasCursors) {
		btns.insert(MenusEnum::CURSORS, new CustomMenuButton("Cursors", true, true));
	}
	if (recipe.hasMeasure) {
		btns.insert(MenusEnum::MEASURE, new CustomMenuButton("Measure", true, true));
	}
	if (recipe.hasTrigger) {
		btns.insert(MenusEnum::TRIGGER, new CustomMenuButton("Trigger"));
	}
	if (recipe.hasSweep) {
		btns.insert(MenusEnum::SWEEP, new CustomMenuButton("Sweep"));
	}
	if (recipe.hasMarkers) {
		btns.insert(MenusEnum::MARKERS, new CustomMenuButton("Markers"));
	}
	if (recipe.hasChannelSettings) {
		btns.insert(MenusEnum::CHANNEL, new CustomMenuButton("Channel"));
	}

	if (recipe.hasInstrumentNotes) {
		m_toolView->setInstrumentNotesVisible(true);
	}

	if (recipe.hasChannels) {
		m_toolView->buildDefaultChannels();
	}
	if (recipe.hasAddMathBtn) {
		m_toolView->configureAddMathBtn();
	}

	if (btns.size()) {
		m_toolView->setBtns(btns);
	}
}

ToolView* ToolViewBuilder::build() { return m_toolView; }
