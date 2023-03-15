/*
 * Copyright (c) 2019 Analog Devices Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QFileDialog>

#include <boost/math/common_factor.hpp>

//#include "pattern_generator.hpp"

#include "digitalchannel_manager.hpp"

using namespace std;
using namespace adiscope::m2k;

constexpr int DIGITAL_NR_CHANNELS = 16;

namespace adiscope {

void DIOManager::init()
{

}

DIOManager::DIOManager(struct iio_context *ctx, Filter *filt)
	: m_m2k_context(m2kOpen(ctx, ""))
	, digital(m_m2k_context->getDigital())
{
	outputEnabled = false;

	for (int i = 0; i < DIGITAL_NR_CHANNELS; ++i) {
		digital->setDirection(i, DIO_DIRECTION::DIO_INPUT);
		digital->setValueRaw(i, DIO_LEVEL::LOW);
	}

	direction = gpi = gpo = lockMask = outputEnabled = 0x00;
}

DIOManager::~DIOManager()
{

}

void DIOManager::setOutputMode(int chid, bool mode)
{
	digital->setOutputMode(chid, mode ? DIO_MODE::DIO_OPENDRAIN
					  : DIO_MODE::DIO_PUSHPULL);
}

void DIOManager::setDeviceDirection(int chid, bool force)
{
	if (!isLocked(chid)) {
		const bool output = (outputEnabled & getDirection(chid)) | force;
		digital->setDirection(chid, output ? DIO_DIRECTION::DIO_OUTPUT
						   : DIO_DIRECTION::DIO_INPUT);
	}
}

int DIOManager::getGpo()
{
	return gpo;
}

void DIOManager::setOutRaw(int ch, bool val)
{
	if (val) {
		gpo |= 1<<ch;
	} else {
		gpo &= ( ~ (1 << ch));
	}

	setDeviceOutRaw(ch);
}

bool DIOManager::getOutRaw(int ch)
{
	return gpo & (1 << ch);
}

void DIOManager::setDeviceOutRaw(int ch)
{
	if (outputEnabled) {
		digital->setValueRaw(ch, getOutRaw(ch) ? DIO_LEVEL::HIGH
						       : DIO_LEVEL::LOW);
	}
}

int DIOManager::getGpi()
{
	gpi = 0;

	for (int i = 0; i < DIGITAL_NR_CHANNELS; ++i) {
		gpi |= (getInRaw(i) << i);
	}

	return gpi;
}

bool DIOManager::getInRaw(int ch)
{
	return static_cast<bool>(digital->getValueRaw(ch));

}

void DIOManager::setDirection(int ch, bool output)
{
	if (output) {
		direction |= (1<<ch);
	} else {
		direction &= ( ~ (1 << ch));
	}

	setDeviceDirection(ch, false);
}

bool DIOManager::getDirection(int ch)
{
	return direction & (1 << ch);
}

void DIOManager::setMode(int mask)
{
	for(int i=0; i < DIGITAL_NR_CHANNELS; ++i) {
		setOutputMode(i, mask & 0x01);
		mask = mask >> 1;
	}
}

void DIOManager::lock(int mask)
{
	lockMask = mask;
	int i = 0;

	while (mask) {
		if (mask & 0x01) {
			setDeviceDirection(i, true);
		}

		mask= mask >> 1;
		i++;
	}

	Q_EMIT locked();
}

bool DIOManager::isLocked(int ch)
{
	return lockMask & (1 << ch);
}

int DIOManager::getLockMask()
{
	return lockMask;
}

void DIOManager::unlock()
{
	lockMask = 0;

	for (auto i=0; i < DIGITAL_NR_CHANNELS; ++i) {
		setDeviceDirection(i, false);
	}

	Q_EMIT unlocked();
}

bool DIOManager::getOutputEnabled()
{
	return outputEnabled;
}

void DIOManager::enableOutput(bool output)
{
	if (outputEnabled != output) {
		outputEnabled = output;

		for (auto i=0; i < DIGITAL_NR_CHANNELS; ++i) {
			setDeviceDirection(i, false);
			setDeviceOutRaw(i);
		}
	}
}

}
