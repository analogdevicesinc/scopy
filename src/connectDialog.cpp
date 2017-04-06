/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "connectDialog.hpp"
#include "dynamicWidget.hpp"
#include <QtConcurrentRun>
#include <functional>

#include "ui_connect.h"

#include <iio.h>
#include <iostream>

using namespace adiscope;

ConnectDialog::ConnectDialog(QWidget *widget) : QObject(widget),
	ui(new Ui::Connect), connected(false)
{
	ui->setupUi(widget);
	ui->connectBtn->setText("Connect");
	//The connect button will be disabled untill we write something in the hostname line edit
	ui->connectBtn->setDisabled(true);

	connect(ui->connectBtn, SIGNAL(clicked()), this, SLOT(btnClicked()));
	connect(ui->hostname, SIGNAL(returnPressed()),
	        this, SLOT(btnClicked()));
	connect(ui->hostname, SIGNAL(textChanged(const QString&)),
	        this, SLOT(discardSettings()));
	connect(this,SIGNAL(finished(struct iio_context *)),this,
	        SLOT(updatePopUp(struct iio_context *)));
	ui->hostname->activateWindow();
	ui->hostname->setFocus();

}

ConnectDialog::~ConnectDialog()
{
	delete ui;
}

void ConnectDialog::btnClicked()
{
	if (!ui->connectBtn->isEnabled()) {
		return;
	} else {
		ui->groupBox->setTitle("Waiting for connection...");
	}

	if (connected) {
		QString new_uri = "ip:" + ui->hostname->text();
		Q_EMIT newContext(new_uri);
	} else {
		validateInput();
	}
}

void ConnectDialog::discardSettings()
{
	if (!ui->hostname->text().isEmpty()) {
		ui->connectBtn->setDisabled(false);
	} else {
		ui->connectBtn->setDisabled(true);
	}

	ui->groupBox->setTitle(QString("Context info"));
	setDynamicProperty(ui->connectBtn, "failed", false);
	setDynamicProperty(ui->connectBtn, "connected", false);
	ui->connectBtn->setText("Connect");

	ui->description->setPlainText(QString(""));
	this->connected = false;
}

void ConnectDialog::validateInput()
{
	ui->connectBtn->setDisabled(true);

	QString new_uri = "ip:" + ui->hostname->text();
	QtConcurrent::run(std::bind(&ConnectDialog::createContext,this,new_uri));
}

void ConnectDialog::createContext(const QString& uri)
{

	this->parent()->installEventFilter(this);
	this->ui->hostname->setDisabled(true);

	struct iio_context *ctx_from_uri = iio_create_context_from_uri(
	                uri.toStdString().c_str());

	this->parent()->removeEventFilter(this);
	this->ui->hostname->setDisabled(false);
	Q_EMIT finished(ctx_from_uri);

}
void ConnectDialog::updatePopUp(struct iio_context *ctx)
{
	this->connected = !!ctx;

	if (!!ctx) {
		ui->connectBtn->setDisabled(false);
		QString description(iio_context_get_description(ctx));
		ui->description->setPlainText(description);
		ui->groupBox->setTitle(QString("Context info"));
		setDynamicProperty(ui->connectBtn, "connected", true);
		ui->connectBtn->setText("Add");

		iio_context_destroy(ctx);
	} else {
		setDynamicProperty(ui->connectBtn, "failed", true);
		ui->connectBtn->setText("Failed!");
		ui->groupBox->setTitle(QString("Warning"));
		ui->description->setPlainText("Error: Unable to find host: No such host is known!");
	}
}
bool ConnectDialog::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress ||
	    event->type() == QEvent::MouseButtonDblClick ||
	    event->type() == QEvent::MouseButtonRelease) {
		return true;
	}

	return false;
}
