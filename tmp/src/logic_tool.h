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


#ifndef LOGICTOOL_H
#define LOGICTOOL_H

#include "tool.hpp"

namespace adiscope {
namespace logic {
class LogicTool : public Tool
{
	Q_OBJECT
public:
	LogicTool(struct iio_context *ctx, ToolMenuItem *toolMenuItem,
	          ApiObject *api, const QString& name,
	          ToolLauncher *parent);
	virtual ~LogicTool() = default;

	uint16_t *getData();

Q_SIGNALS:
	void dataAvailable(uint64_t, uint64_t);

protected:
	uint16_t *m_buffer;
};
} // namespace logic
} // namespace adiscope

#endif // LOGICTOOL_H
