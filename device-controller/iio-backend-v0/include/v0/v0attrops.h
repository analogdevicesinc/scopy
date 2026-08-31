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

// v0 attribute info — wraps the scope-specific details needed for read/write
struct V0AttrInfo
{
	enum Scope
	{
		Context,
		Device,
		Channel,
		Debug,
		Buffer
	};
	Scope scope;
	void *parent; // iio_context*, iio_device*, or iio_channel*
	QByteArray name;
};

class V0AttrOps : public IAttrOps
{
public:
	AttrHandle contextAttr(ContextHandle ctx, const QString &name) override;
	AttrHandle deviceAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle debugAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) override;
	AttrHandle channelAttr(ChannelHandle ch, const QString &name) override;
	void releaseAttr(AttrHandle attr) override;

	Result<QByteArray> read(AttrHandle attr) override;
	Result<void> write(AttrHandle attr, const QString &value) override;

	Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const override;
	Result<void> getAvailable(AttrHandle attr, QStringList &values) const override;

private:
	AttrHandle makeHandle(V0AttrInfo::Scope scope, void *parent, const QString &name);
	Result<QByteArray> readAvailableAttr(const V0AttrInfo *info) const;
};

} // namespace scopy::iio
