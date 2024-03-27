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
#include <gui/stylehelper.h>

using namespace scopy::swiotrefactor;

ConfigChannelView::ConfigChannelView(int channelIndex, QWidget *parent)
	: QWidget(parent)
	, m_channelEnabled(false)
	, m_channelIndex(channelIndex)
{
	installEventFilter(this);
	setHighlightPalette();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_chnlLabel = new QLabel(this);
	m_chnlLabel->setText(QString::fromStdString("Channel ") + QString::number(m_channelIndex + 1));
	StyleHelper::MenuSmallLabel(m_chnlLabel);

	m_chnlCheck = new QCheckBox(this);
	StyleHelper::BlueSquareCheckbox(m_chnlCheck);
	Q_EMIT m_chnlCheck->toggled(true);

	m_deviceOptions = new QComboBox(this);
	m_deviceOptions->setFixedWidth(DEVICE_COMBO_WIDTH);
	StyleHelper::MenuComboBox(m_deviceOptions);

	m_functionOptions = new QComboBox(this);
	m_functionOptions->setFixedWidth(FUNCTION_COMBO_WIDTH);
	StyleHelper::MenuComboBox(m_functionOptions);

	layout->addWidget(m_chnlLabel);
	layout->addWidget(m_chnlCheck);
	layout->addWidget(m_deviceOptions);
	layout->addWidget(m_functionOptions);

	setAttribute(Qt::WA_Hover, true);
	connectSignalsAndSlots();
	initTutorialProperties();
}

ConfigChannelView::~ConfigChannelView() {}

void ConfigChannelView::connectSignalsAndSlots()
{
	QObject::connect(m_chnlCheck, &QCheckBox::stateChanged, this, [=, this](int state) {
		this->setChannelEnabled(state);
		if(m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		} else {
			Q_EMIT hidePaths();
		}
		Q_EMIT enabledChanged(m_channelIndex, state);
	});

	QObject::connect(m_deviceOptions, &QComboBox::textActivated, this, [this](const QString &text) {
		m_selectedDevice = text;
		Q_EMIT deviceChanged(m_channelIndex, m_deviceOptions->currentText());
	});

	QObject::connect(m_functionOptions, &QComboBox::textActivated, this, [=, this](const QString &text) {
		m_selectedFunction = text;
		Q_EMIT functionChanged(m_channelIndex, m_functionOptions->currentText());
	});
}

void ConfigChannelView::setHighlightPalette()
{
	QString highlightColor = StyleHelper::getColor("UIElementHighlight");
	QPalette newPalette = QPalette(palette());
	newPalette.setColor(QPalette::Highlight, QColor(highlightColor));
	setPalette(newPalette);
}

bool ConfigChannelView::isChannelEnabled() const { return m_channelEnabled; }

void ConfigChannelView::setChannelEnabled(bool mChannelEnabled)
{
	m_channelEnabled = mChannelEnabled;

	if(m_channelEnabled) {
		m_chnlCheck->setChecked(true);
		m_deviceOptions->setEnabled(true);
		m_functionOptions->setEnabled(true);
	} else {
		m_chnlCheck->setChecked(false);
		m_deviceOptions->setEnabled(false);
		m_functionOptions->setEnabled(false);
	}
}

const QString &ConfigChannelView::getSelectedDevice() const { return m_selectedDevice; }

void ConfigChannelView::setSelectedDevice(const QString &mSelectedDevice)
{
	m_selectedDevice = mSelectedDevice;
	int index = m_deviceOptions->findText(m_selectedDevice);
	m_deviceOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getDeviceAvailable() const { return m_deviceAvailable; }

void ConfigChannelView::setDeviceAvailable(const QStringList &mDeviceAvailable)
{
	m_deviceAvailable = mDeviceAvailable;
	m_deviceOptions->clear();
	for(const QString &device : qAsConst(m_deviceAvailable)) {
		m_deviceOptions->addItem(device);
	}
}

const QString &ConfigChannelView::getSelectedFunction() const { return m_selectedFunction; }

void ConfigChannelView::setSelectedFunction(const QString &mSelectedFunction)
{
	m_selectedFunction = mSelectedFunction;
	if(mSelectedFunction.isEmpty()) {
		m_functionOptions->setCurrentIndex(0);
	} else {
		int index = m_functionOptions->findText(m_selectedFunction);
		if(index >= 0) {
			m_functionOptions->setCurrentIndex(index);
		}
	}
}

const QStringList &ConfigChannelView::getFunctionAvailable() const { return m_functionAvailable; }

void ConfigChannelView::setFunctionAvailable(const QStringList &mFunctionAvailable)
{
	m_functionAvailable = mFunctionAvailable;
	m_functionOptions->clear();
	for(const QString &device : qAsConst(m_functionAvailable)) {
		m_functionOptions->addItem(device);
	}
}

bool ConfigChannelView::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::Enter) {
		setBackgroundRole(QPalette::Highlight);
		setAutoFillBackground(true);
		if(m_channelEnabled) {
			Q_EMIT showPath(m_channelIndex, m_selectedDevice);
		}
	}

	if(event->type() == QEvent::Leave) {
		setBackgroundRole(QPalette::Window);
		setAutoFillBackground(false);
		Q_EMIT hidePaths();
	}

	return QWidget::event(event);
}

void ConfigChannelView::initTutorialProperties()
{
	if(m_channelIndex == 0) {
		setProperty("tutorial_name", "CHANNEL_WIDGET_1");
		m_chnlCheck->setProperty("tutorial_name", "CHANNEL_ENABLE_1");
		m_deviceOptions->setProperty("tutorial_name", "CHANNEL_DEVICE_1");
		m_functionOptions->setProperty("tutorial_name", "CHANNEL_FUNCTION_1");
	}
}
