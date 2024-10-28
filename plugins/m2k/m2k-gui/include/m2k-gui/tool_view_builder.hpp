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

#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include "scopy-m2k-gui_export.h"
#include "tool_view.hpp"

namespace scopy {
namespace m2kgui {

struct ToolViewRecipe
{
	QString helpBtnUrl{""};
	bool hasHelpBtn{true};
	bool hasPrintBtn{false};
	bool hasGroupBtn{false};

	bool hasRunBtn{false};
	bool hasSingleBtn{false};

	bool hasPairSettingsBtn{false};

	bool hasChannels{false};
	ChannelsPositionEnum channelsPosition{ChannelsPositionEnum::HORIZONTAL};
	bool hasHeader{true};
	bool hasRightMenu{false};
	bool hasHamburgerMenuBtn{true};
};

class SCOPY_M2K_GUI_EXPORT ToolViewBuilder
{
public:
	ToolViewBuilder(const ToolViewRecipe &recipe, ChannelManager *channelManager = nullptr,
			QWidget *parent = nullptr);

	ToolView *build();

private:
	ToolView *m_toolView;
};
} // namespace m2kgui
} // namespace scopy

#endif // TOOLVIEWBUILDER_HPP
