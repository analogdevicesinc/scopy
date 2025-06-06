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
 */

#include <gui/widgets/menusectionwidget.h>
#include <style.h>
#include "jesdstatus.h"
#include "jesdstatusview.h"

using namespace scopy::jesdstatus;

JesdStatus::JesdStatus(QList<struct iio_device *> devLst, QWidget *parent)
	: QWidget(parent)
	, m_deviceList(devLst)
	, m_timer(new QTimer(this))
{
	QVBoxLayout *lay = new QVBoxLayout();
	this->setLayout(lay);
	lay->setMargin(0);

	m_tool = new ToolTemplate(this);
	m_tool->topContainer()->setVisible(false);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->bottomContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(false);
	m_tool->leftContainer()->setVisible(false);
	m_tool->topCentral()->setVisible(false);
	m_tool->centralContainer()->setVisible(true);
	m_tool->setRightContainerWidth(0);
	m_tool->centralContainer()->layout()->setSpacing(10);
	lay->addWidget(m_tool);

	m_jesdDeviceStack = new MapStackedWidget(this);

	MenuSectionWidget *deviceSelectorWidget = new MenuSectionWidget(this);
	m_deviceSelector = new MenuComboWidget("JESD204 Link Layer Device", deviceSelectorWidget);
	Style::setStyle(m_deviceSelector, style::properties::label::menuBig);
	deviceSelectorWidget->contentLayout()->addWidget(m_deviceSelector);
	Style::setStyle(deviceSelectorWidget, style::properties::widget::border_interactive);
	connect(m_deviceSelector->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		[=, this](int idx) { poll(); });

	connect(m_timer, &QTimer::timeout, this, &JesdStatus::poll);

	m_tool->addWidgetToCentralContainerHelper(deviceSelectorWidget);
	m_tool->addWidgetToCentralContainerHelper(m_jesdDeviceStack);

	for(auto dev : qAsConst(m_deviceList)) {
		setupDevice(dev);
	}

	m_jesdDeviceStack->setCurrentIndex(0);
}

JesdStatus::~JesdStatus()
{
	if(m_timer->isActive()) {
		Q_EMIT running(false);
		m_timer->stop();
	}
}

void JesdStatus::runToggled(bool toggled)
{
	if(toggled) {
		m_timer->start(1000);
	} else {
		m_timer->stop();
	}
}

void JesdStatus::setupDevice(struct iio_device *dev)
{
	JesdStatusView *jesdLinkDevView = new JesdStatusView(dev, this);
	QString devLbl = iio_device_get_label(dev);
	m_deviceSelector->combo()->addItem(devLbl);
	m_jesdDeviceStack->add(devLbl, jesdLinkDevView);
	jesdLinkDevView->update();
}

void JesdStatus::poll()
{
	unsigned int idx = m_deviceSelector->combo()->currentIndex();
	QString device = m_deviceSelector->combo()->itemText(idx);
	m_jesdDeviceStack->show(device);
	JesdStatusView *current = dynamic_cast<JesdStatusView *>(m_jesdDeviceStack->get(device));
	if(current) {
		current->update();
	}
}
