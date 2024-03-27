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

#include "configcontroller.h"

using namespace scopy::swiotrefactor;

ConfigController::ConfigController(ConfigChannelView *channelView, ConfigModel *model, int channelId)
	: m_channelsView(channelView)
	, m_model(model)
	, m_channelId(channelId)
{
	this->connectSignalsAndSlots();
	this->initChannelView();
}

ConfigController::~ConfigController()
{
	if(m_channelsView) {
		delete m_channelsView;
		m_channelsView = nullptr;
	}

	if(m_model) {
		delete m_model;
		m_model = nullptr;
	}
}

void ConfigController::connectSignalsAndSlots()
{
	connect(m_model, &ConfigModel::readConfigChannelEnabled, this,
		[=, this](bool enabled) { m_channelsView->setChannelEnabled(enabled); });
	connect(m_model, &ConfigModel::readConfigChannelDevice, this, [=, this](QString device) {
		m_channelsView->setSelectedDevice(device);
		Q_EMIT deviceReadingComplete();
	});
	connect(m_model, &ConfigModel::readConfigChannelFunction, this,
		[=, this](QString function) { m_channelsView->setSelectedFunction(function); });
	connect(m_model, &ConfigModel::readConfigChannelDeviceAvailable, this,
		[=, this](QStringList devicesAvailable) { m_channelsView->setDeviceAvailable(devicesAvailable); });
	connect(m_model, &ConfigModel::readConfigChannelFunctionAvailable, this,
		[=, this](QStringList functionsAvailable) {
			m_channelsView->setFunctionAvailable(functionsAvailable);
			Q_EMIT functionAvailableReadingComplete();
		});
	connect(m_model, &ConfigModel::configChannelDevice, this, [=, this]() {
		Q_EMIT deviceReadingComplete();
		// stop and wait for readConfigChannelDevice to finish in order to continue
	});
	connect(this, &ConfigController::deviceReadingComplete, this, [this]() {
		m_model->readFunctionAvailable();
		// stop and wait for readConfigChannelFunctionAvailable to finish in order to continue
	});
	connect(this, &ConfigController::functionAvailableReadingComplete, this, [this]() { m_model->readFunction(); });

	QObject::connect(m_channelsView, &ConfigChannelView::enabledChanged, this, [this](int index, bool value) {
		if(m_channelId == index) {
			if(value) {
				m_model->writeEnabled("1");
			} else {
				m_model->writeEnabled("0");
			}
		}
	});

	QObject::connect(m_channelsView, &ConfigChannelView::deviceChanged, this,
			 [this](int index, const QString &device) {
				 if(m_channelId == index) {
					 m_model->writeDevice(device);
					 Q_EMIT clearDrawArea();
				 }
			 });

	QObject::connect(m_channelsView, &ConfigChannelView::functionChanged, this,
			 [this](int index, const QString &function) {
				 if(m_channelId == index) {
					 m_model->writeFunction(function);
				 }
			 });
}

void ConfigController::initChannelView()
{
	m_model->readEnabled();
	m_model->readDeviceAvailable();
	m_model->readDevice();
	// the read device function activates a sequence that also reads the "function_available" and "function"
	// attributes
}
