/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <scopy/gui/logic_tool.hpp>

using namespace scopy::logic;

LogicTool::LogicTool(iio_context* ctx, scopy::gui::ToolMenuItem* toolMenuItem, scopy::core::ApiObject* api,
		     const QString& name, scopy::gui::ToolLauncher* parent)
	: Tool(ctx, toolMenuItem, api, name, parent)
	, m_buffer(nullptr)
{}

uint16_t* LogicTool::getData() { return m_buffer; }
