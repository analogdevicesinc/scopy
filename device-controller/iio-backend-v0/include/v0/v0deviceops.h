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

#include "iioutil/ideviceops.h"

namespace scopy::iio {

class V0DeviceOps : public IDeviceOps
{
public:
	QString id(DeviceHandle dev) const override;
	QString name(DeviceHandle dev) const override;
	QString label(DeviceHandle dev) const override;
	bool isTrigger(DeviceHandle dev) const override;
	bool isHwmon(DeviceHandle dev) const override;
	unsigned int channelsCount(DeviceHandle dev) const override;
	ChannelHandle getChannel(DeviceHandle dev, unsigned int index) const override;
	ChannelHandle findChannel(DeviceHandle dev, const QString &name, bool output) const override;
	unsigned int attrsCount(DeviceHandle dev) const override;
	QString attrName(DeviceHandle dev, unsigned int index) const override;
	unsigned int debugAttrsCount(DeviceHandle dev) const override;
	QString debugAttrName(DeviceHandle dev, unsigned int index) const override;
	unsigned int buffersCount(DeviceHandle dev) const override;
	unsigned int bufferAttrsCount(DeviceHandle dev, unsigned int bufferIdx) const override;
	QString bufferAttrName(DeviceHandle dev, unsigned int bufferIdx, unsigned int index) const override;
	DeviceHandle getTrigger(DeviceHandle dev) const override;
	ssize_t sampleSize(DeviceHandle dev, ChannelsMaskHandle mask) const override;

	Result<uint32_t> regRead(DeviceHandle dev, uint32_t addr) override;
	Result<void> regWrite(DeviceHandle dev, uint32_t addr, uint32_t val) override;
	Result<void> setTrigger(DeviceHandle dev, DeviceHandle trigger) override;
};

} // namespace scopy::iio
