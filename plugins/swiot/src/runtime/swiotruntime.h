/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef SWIOTRUNTIME_H
#define SWIOTRUNTIME_H

#include "src/runtime/ad74413r/ad74413r.h"

namespace scopy {
namespace gui {
class GenericMenu;
class ChannelManager;
} // namespace gui

namespace swiot {
#define AD_NAME "ad74413r"
#define AD_TRIGGER_NAME "ad74413r-dev0"

class SwiotRuntime : public QObject
{
	Q_OBJECT
public:
	SwiotRuntime(struct iio_context *ctx, QObject *parent = nullptr);
	~SwiotRuntime();

public Q_SLOTS:
	void onBackBtnPressed();
	void modeAttributeChanged(std::string mode);
	void writeTriggerDevice();
	void onIsRuntimeCtxChanged(bool isRuntimeCtx);
private Q_SLOTS:
	void setTriggerCommandFinished(scopy::Command *);
Q_SIGNALS:
	void writeModeAttribute(std::string mode);
	void backBtnPressed();

private:
	void createDevicesMap();

private:
	iio_context *m_iioCtx;
	QMap<QString, struct iio_device *> m_iioDevices;
	CommandQueue *m_cmdQueue;
};
} // namespace swiot
} // namespace scopy

#endif // SWIOTRUNTIME_H
