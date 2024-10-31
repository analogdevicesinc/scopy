/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "tool_view_builder.hpp"

#include "dynamicWidget.h"

using namespace scopy::m2kgui;

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
