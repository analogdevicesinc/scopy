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

#include "iioutil/iattrops.h"

namespace scopy::iio {

// In v1, AttrHandle.ptr stores const iio_attr* directly — no wrapper struct needed.
class V1AttrOps : public IAttrOps
{
public:
	AttrHandle contextAttr(ContextHandle ctx, const QString &name) override;
	AttrHandle deviceAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle debugAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) override;
	AttrHandle channelAttr(ChannelHandle ch, const QString &name) override;
	void releaseAttr(AttrHandle attr) override; // no-op: iio_attr* lifetime is tied to context

	Result<QByteArray> read(AttrHandle attr) override;
	Result<void> write(AttrHandle attr, const QString &value) override;

	Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const override;
	Result<void> getAvailable(AttrHandle attr, QStringList &values) const override;
};

} // namespace scopy::iio
