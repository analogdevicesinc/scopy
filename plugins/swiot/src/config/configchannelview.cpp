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


#include "configchannelview.h"
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

ConfigChannelView::ConfigChannelView(int channelIndex, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ConfigChannelView()),
	m_channelEnabled(false),
	m_channelIndex(channelIndex)
{
	this->installEventFilter(this);
	this->ui->setupUi(this);

	this->setAttribute(Qt::WA_Hover, true);

	this->ui->enabledCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->ui->channelLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->ui->channelLabel->setText(QString::fromStdString("Channel ") + QString::number(m_channelIndex + 1));
	this->ui->enabledCheckBox->toggled(true);
	this->connectSignalsAndSlots();
}

ConfigChannelView::~ConfigChannelView() {
	delete ui;
}

void ConfigChannelView::connectSignalsAndSlots() {
	QObject::connect(this->ui->enabledCheckBox, &QCheckBox::stateChanged, this, [this]() {
		this->setChannelEnabled(this->ui->enabledCheckBox->isChecked());
		if (m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		} else {
			Q_EMIT hidePaths();
		}
		Q_EMIT enabledChanged(m_channelIndex, this->ui->enabledCheckBox->isChecked());
	});

	QObject::connect(this->ui->deviceOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		Q_EMIT deviceChanged(m_channelIndex, this->ui->deviceOptions->currentText());
	});

	QObject::connect(this->ui->functionOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		Q_EMIT functionChanged(m_channelIndex, this->ui->functionOptions->currentText());
	});
}

bool ConfigChannelView::isChannelEnabled() const {
	return m_channelEnabled;
}

void ConfigChannelView::setChannelEnabled(bool mChannelEnabled) {
	m_channelEnabled = mChannelEnabled;
	this->ui->enabledCheckBox->setChecked(m_channelEnabled);

	if (m_channelEnabled) {
		this->ui->deviceOptions->setEnabled(true);
		this->ui->functionOptions->setEnabled(true);
	} else {
		this->ui->deviceOptions->setEnabled(false);
		this->ui->functionOptions->setEnabled(false);
	}
}

const QString &ConfigChannelView::getSelectedDevice() const {
	return m_selectedDevice;
}

void ConfigChannelView::setSelectedDevice(const QString &mSelectedDevice) {
	m_selectedDevice = mSelectedDevice;
	int index = this->ui->deviceOptions->findText(m_selectedDevice);
	this->ui->deviceOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getDeviceAvailable() const {
	return m_deviceAvailable;
}

void ConfigChannelView::setDeviceAvailable(const QStringList &mDeviceAvailable) {
	m_deviceAvailable = mDeviceAvailable;
	this->ui->deviceOptions->clear();
	for (const QString &device: m_deviceAvailable) {
		this->ui->deviceOptions->addItem(device);
	}
}

const QString &ConfigChannelView::getSelectedFunction() const {
	return m_selectedFunction;
}

void ConfigChannelView::setSelectedFunction(const QString &mSelectedFunction) {
	m_selectedFunction = mSelectedFunction;
	int index = this->ui->functionOptions->findText(m_selectedFunction);
	this->ui->functionOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getFunctionAvailable() const {
	return m_functionAvailable;
}

void ConfigChannelView::setFunctionAvailable(const QStringList &mFunctionAvailable) {
	m_functionAvailable = mFunctionAvailable;
	this->ui->functionOptions->clear();
	for (const QString &device: m_functionAvailable) {
		this->ui->functionOptions->addItem(device);
	}
}

bool ConfigChannelView::eventFilter(QObject* object, QEvent *event) {
	if (event->type() == QEvent::HoverEnter) {
		this->setStyleSheet("background-color: #272730;");
		if (m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		}
	}

	if (event->type() == QEvent::HoverLeave) {
		this->setStyleSheet("");
		Q_EMIT hidePaths();
	}

	return QFrame::event(event);
}

#include "moc_configchannelview.cpp"
