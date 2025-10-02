/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "buffermenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <menuonoffswitch.h>

using namespace scopy::extprocplugin;

BufferMenu::BufferMenu(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
}

BufferMenu::~BufferMenu() {}

void BufferMenu::setupUI()
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	// Buffer size
	m_bufferSizeSpin = new MenuSpinbox("Buffer Size", 16, "samples", 16, 4000000, true, false);
	m_bufferSizeSpin->setScaleRange(1, 1e6);
	layout->addWidget(m_bufferSizeSpin);

	// Device selection
	m_deviceCombo = new MenuCombo("Device");
	layout->addWidget(m_deviceCombo);

	// Channel list
	QLabel *chnlsLabel = new QLabel("Channels:");
	layout->addWidget(chnlsLabel);
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setMinimumHeight(80);
	m_chnList = new QWidget();
	m_chnList->setLayout(new QVBoxLayout());
	scrollArea->setWidget(m_chnList);
	scrollArea->setWidgetResizable(true);
	layout->addWidget(scrollArea);

	// Connections
	connect(m_deviceCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BufferMenu::updateChnList);
	connect(m_deviceCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BufferMenu::onParamsChanged);
	connect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, this, &BufferMenu::onParamsChanged);
	connect(this, &BufferMenu::channelsAvailable, this, &BufferMenu::updateChnList);
	connect(this, &BufferMenu::channelsAvailable, this,
		[this, chnlsLabel, scrollArea]() { updateWidgetVisibility(chnlsLabel, scrollArea); });
}

QStringList BufferMenu::getEnChannels()
{
	QStringList result;
	QString currentDevice = m_deviceCombo->combo()->currentText();
	QList<ChannelInfo> &channels = m_availableChannels[currentDevice];
	for(ChannelInfo &channel : channels) {
		if(channel.enable) {
			result.push_back(channel.name);
		}
	}
	return result;
}

void BufferMenu::setAvailableChannels(const QMap<QString, QList<ChannelInfo>> &channels)
{
	m_availableChannels.clear();
	m_availableChannels = channels;
	m_deviceCombo->combo()->clear();
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		m_deviceCombo->combo()->addItem(it.key());
	}
	Q_EMIT channelsAvailable();
}

void BufferMenu::updateChnList()
{
	// Clear existing checkboxes
	QLayout *layout = m_chnList->layout();
	while(QLayoutItem *item = layout->takeAt(0)) {
		if(item->widget()) {
			item->widget()->deleteLater();
		}
		delete item;
	}
	// Add new checkboxes for current device
	QString currentDevice = m_deviceCombo->combo()->currentText();
	if(m_availableChannels.contains(currentDevice)) {
		for(int i = 0; i < m_availableChannels[currentDevice].size(); i++) {
			ChannelInfo chInfo = m_availableChannels[currentDevice].at(i);
			MenuOnOffSwitch *onOffSwitch = new MenuOnOffSwitch(chInfo.name);
			layout->addWidget(onOffSwitch);
			connect(onOffSwitch->onOffswitch(), &QCheckBox::toggled, this,
				[this, i, currentDevice](bool checked) {
					m_availableChannels[currentDevice][i].enable = checked;
					onParamsChanged();
				});
			onOffSwitch->onOffswitch()->setChecked(chInfo.enable);
		}
	}
}

void BufferMenu::onParamsChanged()
{
	BufferParams params;
	params.samplesCount = m_bufferSizeSpin->value();
	params.deviceName = m_deviceCombo->combo()->currentText();
	params.enChnls = getEnChannels();

	Q_EMIT bufferParamsChanged(params);
}

void BufferMenu::updateWidgetVisibility(QLabel *channelsLabel, QScrollArea *scrollArea)
{
	bool hasValidChannels = !m_availableChannels.isEmpty() &&
		!(m_availableChannels.size() == 1 && m_availableChannels.first().isEmpty());

	channelsLabel->setVisible(hasValidChannels);
	scrollArea->setVisible(hasValidChannels);
	m_deviceCombo->setVisible(hasValidChannels);
}
