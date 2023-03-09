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

#include "device_widget.hpp"
#include "gui/dynamicWidget.hpp"
#include "gui/info_page.hpp"
#include "tool_launcher.hpp"
#include "ui_device.h"

#include <QTimer>

using namespace adiscope;

DeviceWidget::DeviceWidget(QString uri, QString name,
			   ToolLauncher *parent) :
	QWidget(parent),
	m_ui(new Ui::Device()),
	m_connected(false),
	m_selected(false)
{
	m_ui->setupUi(this);
	m_uri = uri;
	m_ui->description->setText(uri);
	m_ui->name->setText(name);

	if (name.compare("M2K") != 0) {
		m_infoPage = InfoPageBuilder::newPage(InfoPageBuilder::GENERIC,
						      m_uri,
						      parent->getPrefPanel(),
						      parent->getPhoneHome());
		connect(m_infoPage->forgetDeviceButton(), SIGNAL(clicked(bool)),
			this, SLOT(forgetDevice_clicked(bool)));
		connect(m_infoPage->identifyDeviceButton(), SIGNAL(clicked(bool)),
			this, SLOT(identifyDevice_clicked(bool)));
	}
}

DeviceWidget::~DeviceWidget()
{
	delete m_infoPage;
	delete m_ui;
}

void DeviceWidget::identifyDevice_clicked(bool pressed)
{
	Q_EMIT identifyDevice(m_uri);
}

void DeviceWidget::forgetDevice_clicked(bool pressed)
{
	Q_EMIT forgetDevice(m_uri);
}

QPushButton* DeviceWidget::deviceButton() const
{
	return m_ui->btn;
}

QPushButton* DeviceWidget::connectButton() const
{
	if (m_infoPage) {
		return m_infoPage->connectButton();
	}
	return nullptr;
}

QPushButton* DeviceWidget::calibrateButton() const
{
	if (m_infoPage) {
		return m_infoPage->calibrateButton();
	}
	return nullptr;
}

void DeviceWidget::on_btn_toggled(bool toggled)
{
	Q_EMIT selected(toggled);
	m_selected = toggled;
	setDynamicProperty(m_ui->widget, "selected",
			   toggled);
	if (toggled) {
		m_infoPage->getDeviceInfo();
	}
}

bool DeviceWidget::connected() const
{
	return m_connected;
}

void DeviceWidget::setConnected(bool connected, bool failed,
				struct iio_context *ctx)
{
	m_connected = connected;
	m_infoPage->setCtx(ctx);
	setDynamicProperty(m_ui->label, "connected", connected);
	setDynamicProperty(m_ui->line, "connected", connected);
	setDynamicProperty(m_ui->label, "failed", failed);
	setDynamicProperty(m_ui->line, "failed", failed);
	setDynamicProperty(connectButton(), "connected", connected);
	calibrateButton()->setEnabled(false);
	m_infoPage->setConnectionStatus(failed);
}

InfoPage *DeviceWidget::infoPage() const
{
	return m_infoPage;
}

void DeviceWidget::setInfoPage(InfoPage *infoPage)
{
	m_infoPage = infoPage;
}

QString DeviceWidget::uri() const
{
	return m_uri;
}

void DeviceWidget::setUri(const QString &uri)
{
	m_uri = uri;
}

void DeviceWidget::highlightDevice()
{
	bool initialBtnState = m_ui->btn->property("checked").toBool();

	QTimer::singleShot(200, [=](){
		setDynamicProperty(m_ui->btn, "checked", !initialBtnState);
	});
	QTimer::singleShot(400, [=](){
		setDynamicProperty(m_ui->btn, "checked", initialBtnState);
	});
	QTimer::singleShot(600, [=](){
		setDynamicProperty(m_ui->btn, "checked", !initialBtnState);
	});
	QTimer::singleShot(800, [=](){
		setDynamicProperty(m_ui->btn, "checked", initialBtnState);
	});
}

void DeviceWidget::setChecked(bool checked)
{
	m_ui->btn->setChecked(checked);
}

bool DeviceWidget::isChecked()
{
	return m_ui->btn->isChecked();
}

void DeviceWidget::click()
{
	m_ui->btn->click();
}

void DeviceWidget::setName(QString name)
{
	m_ui->name->setText(name);
}

M2kDeviceWidget::M2kDeviceWidget(QString uri, QString name, ToolLauncher *parent) :
	DeviceWidget(uri, name, parent)
{
	m_ui->name->setText(tr("M2K"));
	m_infoPage = InfoPageBuilder::newPage(InfoPageBuilder::M2K,
					      m_uri,
					      parent->getPrefPanel(),
					      parent->getPhoneHome(),
					      nullptr);

	connect(m_infoPage->forgetDeviceButton(), SIGNAL(clicked(bool)),
		this, SLOT(forgetDevice_clicked(bool)));
	connect(m_infoPage->identifyDeviceButton(), SIGNAL(clicked(bool)),
		this, SLOT(identifyDevice_clicked(bool)));
}


M2kDeviceWidget::~M2kDeviceWidget()
{
}
