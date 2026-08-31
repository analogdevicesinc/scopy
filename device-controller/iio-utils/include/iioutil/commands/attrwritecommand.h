/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include "core/resultcommand.h"
#include "iioutil/iattrops.h"

#include <cerrno>

namespace scopy::iio {

class AttrWriteCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	AttrWriteCommand(IAttrOps *ops, AttrHandle handle, const QString &value, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_value(value)
	{}

protected:
	void run() override { setResult(m_ops->write(m_handle, m_value)); }

private:
	IAttrOps *m_ops;
	AttrHandle m_handle;
	QString m_value;
};

} // namespace scopy::iio
