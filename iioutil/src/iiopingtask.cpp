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

#include "iiopingtask.h"
#include "iiocpp/iiocontext.h"
#include "iiocpp/iiodevice.h"
#include "iiocpp/iioresult.h"

#include <QDebug>
#include <cerrno>

using namespace scopy;
IIOPingTask::IIOPingTask(iio_context *c, QObject *parent)
	: PingTask(parent)
	, m_ctx(c)
{
}

IIOPingTask::~IIOPingTask() {}

void IIOPingTask::run()
{
	bool ret = ping();

	if(isInterruptionRequested())
		return;
	if(ret)
		Q_EMIT pingSuccess();
	else
		Q_EMIT pingFailed();
}

bool IIOPingTask::ping() { return pingCtx(m_ctx); }

bool IIOPingTask::pingCtx(iio_context *ctx)
{
	auto dev = IIOContext::get_device(ctx, 0).expect("There should be at least one device in the context");
	const iio_device *test_device = nullptr;

	IIOResult<const iio_device *> ret = IIODevice::get_trigger(dev);
	if(ret.ok()) {
		test_device = ret.data();
	} else {
		// FIXME: fix this
		qDebug() << "Error: " << ret.error();
		return false;
	}

	return true;
}

#include "moc_iiopingtask.cpp"
