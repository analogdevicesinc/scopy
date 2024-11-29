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

#include "deviceiconimpl.h"

#include "gui/dynamicWidget.h"

#include "ui_devicebutton.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <hoverwidget.h>
#include <style.h>

using namespace scopy;
DeviceIconImpl::DeviceIconImpl(Device *d, QWidget *parent)
	: DeviceIcon{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	ui->description->setText(d->displayParam());
	ui->name->setText(d->displayName());
	ui->name->setStyleSheet("border: none;");
	ui->name->setReadOnly(true);
	ui->name->home(false);
	connect(ui->name, &QLineEdit::editingFinished, this, &DeviceIconImpl::onEditFinished);
	createPenBtn();

	ui->iconPlaceHolder->layout()->addWidget(d->icon());
	ui->iconPlaceHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setStyleSheet("QWidget { background-color: transparent; }");
	Style::setStyle(this, style::properties::widget::deviceIcon, true, true);

	setCheckable(true);
}

DeviceIconImpl::~DeviceIconImpl() {}

void DeviceIconImpl::setConnecting(bool val)
{
	setDynamicProperty(ui->line, "connected", false);
	setDynamicProperty(ui->line, "connecting", val);
	//	ensurePolished();
}

void DeviceIconImpl::setConnected(bool val)
{
	setDynamicProperty(ui->line, "connecting", false);
	setDynamicProperty(ui->line, "connected", val);
}

void DeviceIconImpl::createPenBtn()
{
	QPushButton *penBtn = new QPushButton();
	penBtn->setMaximumSize(20, 20);
	penBtn->setStyleSheet("QPushButton {"
			      "background-color: transparent;"
			      "border: 0px;"
			      "}"
			      "QPushButton:hover {"
			      "border-image: url(:/gui/icons/edit_pen.svg);"
			      "}");
	connect(penBtn, &QPushButton::clicked, this, &DeviceIconImpl::onPenBtnPressed);
	HoverWidget *penHover = new HoverWidget(penBtn, ui->name, this);
	penHover->setStyleSheet("background-color: transparent; border: 0px;");
	penHover->setAnchorPos(HoverPosition::HP_RIGHT);
	penHover->setContentPos(HoverPosition::HP_RIGHT);
	penHover->setVisible(true);
	penHover->raise();
}

void DeviceIconImpl::onPenBtnPressed()
{
	ui->name->setReadOnly(false);
	ui->name->end(false);
	// ui->name->setEnabled(true);
	ui->name->setFocus();
}

void DeviceIconImpl::onEditFinished()
{
	ui->name->setReadOnly(true);
	ui->name->end(false);
	// ui->name->setEnabled(false);
	Q_EMIT displayNameChanged(ui->name->text());
}

#include "moc_deviceiconimpl.cpp"
