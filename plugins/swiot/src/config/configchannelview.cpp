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

ConfigChannelView::ConfigChannelView(int channelIndex, QWidget *parent)
	: QFrame(parent)
	, ui(new Ui::ConfigChannelView())
	, m_channelEnabled(false)
	, m_channelIndex(channelIndex)
{
	this->installEventFilter(this);
	this->ui->setupUi(this);

	this->setAttribute(Qt::WA_Hover, true);

	this->ui->channelLabel->setText(QString::fromStdString("Channel ") + QString::number(m_channelIndex + 1));
	this->ui->enabledCheckBox->toggled(true);
	this->connectSignalsAndSlots();

	this->initTutorialProperties();
}

ConfigChannelView::~ConfigChannelView() { delete ui; }

void ConfigChannelView::connectSignalsAndSlots()
{
	QObject::connect(this->ui->enabledCheckBox, &QCheckBox::stateChanged, this, [=, this](int state) {
		this->setChannelEnabled(state);
		if(m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		} else {
			Q_EMIT hidePaths();
		}
		Q_EMIT enabledChanged(m_channelIndex, state);
	});

	QObject::connect(this->ui->deviceOptions, &QComboBox::textActivated, this, [this](const QString &text) {
		m_selectedDevice = text;
		Q_EMIT deviceChanged(m_channelIndex, this->ui->deviceOptions->currentText());
	});

	QObject::connect(this->ui->functionOptions, &QComboBox::textActivated, this, [=, this](const QString &text) {
		m_selectedFunction = text;
		Q_EMIT functionChanged(m_channelIndex, this->ui->functionOptions->currentText());
	});
}

bool ConfigChannelView::isChannelEnabled() const { return m_channelEnabled; }

void ConfigChannelView::setChannelEnabled(bool mChannelEnabled)
{
	m_channelEnabled = mChannelEnabled;

	if(m_channelEnabled) {
		this->ui->enabledCheckBox->setChecked(true);
		this->ui->deviceOptions->setEnabled(true);
		this->ui->functionOptions->setEnabled(true);
	} else {
		this->ui->enabledCheckBox->setChecked(false);
		this->ui->deviceOptions->setEnabled(false);
		this->ui->functionOptions->setEnabled(false);
	}
}

const QString &ConfigChannelView::getSelectedDevice() const { return m_selectedDevice; }

void ConfigChannelView::setSelectedDevice(const QString &mSelectedDevice)
{
	m_selectedDevice = mSelectedDevice;
	int index = this->ui->deviceOptions->findText(m_selectedDevice);
	this->ui->deviceOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getDeviceAvailable() const { return m_deviceAvailable; }

void ConfigChannelView::setDeviceAvailable(const QStringList &mDeviceAvailable)
{
	m_deviceAvailable = mDeviceAvailable;
	this->ui->deviceOptions->clear();
	for(const QString &device : qAsConst(m_deviceAvailable)) {
		this->ui->deviceOptions->addItem(device);
	}
}

const QString &ConfigChannelView::getSelectedFunction() const { return m_selectedFunction; }

void ConfigChannelView::setSelectedFunction(const QString &mSelectedFunction)
{
	m_selectedFunction = mSelectedFunction;
	if(mSelectedFunction.isEmpty()) {
		this->ui->functionOptions->setCurrentIndex(0);
	} else {
		int index = this->ui->functionOptions->findText(m_selectedFunction);
		if(index >= 0) {
			this->ui->functionOptions->setCurrentIndex(index);
		}
	}
}

const QStringList &ConfigChannelView::getFunctionAvailable() const { return m_functionAvailable; }

void ConfigChannelView::setFunctionAvailable(const QStringList &mFunctionAvailable)
{
	m_functionAvailable = mFunctionAvailable;
	this->ui->functionOptions->clear();
	for(const QString &device : qAsConst(m_functionAvailable)) {
		this->ui->functionOptions->addItem(device);
	}
}

bool ConfigChannelView::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::Enter) {
		this->setStyleSheet("background-color: #272730;");
		this->ensurePolished();
		if(m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		}
	}

	if(event->type() == QEvent::Leave) {
		this->setStyleSheet("");
		this->ensurePolished();
		Q_EMIT hidePaths();
	}

	return QFrame::event(event);
}

void ConfigChannelView::initTutorialProperties()
{
	if(m_channelIndex == 0) {
		this->setProperty("tutorial_name", "CHANNEL_WIDGET_1");
		this->ui->enabledCheckBox->setProperty("tutorial_name", "CHANNEL_ENABLE_1");
		this->ui->deviceOptions->setProperty("tutorial_name", "CHANNEL_DEVICE_1");
		this->ui->functionOptions->setProperty("tutorial_name", "CHANNEL_FUNCTION_1");
	}
}

#include "moc_configchannelview.cpp"
