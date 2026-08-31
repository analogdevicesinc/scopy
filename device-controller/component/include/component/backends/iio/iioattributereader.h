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

#include "component/attributereader.h"
#include "iioutil/handles.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IAttrOps;
class AttrReadCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOAttributeReader : public AttributeReader
{
	Q_OBJECT
public:
	IIOAttributeReader(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle, scopy::ICmdExecutor *executor,
			   QObject *parent = nullptr);

	QCoro::Task<CommandResponse<QByteArray>> readAsync();

private:
	scopy::iio::IAttrOps *m_ops;
	scopy::iio::AttrHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
