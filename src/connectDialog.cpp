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

#include "ui_connect.h"

#include <iio.h>

using namespace adiscope;

ConnectDialog::ConnectDialog(QWidget *widget) : QObject(widget),
    ui(new Ui::Connect), connected(false), signalsBlocked(false)
{
    //returnBlocked == false => returnPressed() signal works , otherwise it's blocked
	ui->setupUi(widget);
	ui->connectBtn->setText("Connect");
    //We force the cursor on the line edit
    forceFocus(ui->hostname);
    //We direct the keyboard input to the line edit
    ui->hostname->grabKeyboard();


    connect(ui->connectBtn, SIGNAL(clicked()), this, SLOT(btnClicked()));
	connect(ui->hostname, SIGNAL(returnPressed()),
			this, SLOT(btnClicked()));
	connect(ui->hostname, SIGNAL(textChanged(const QString&)),
			this, SLOT(discardSettings()));
}

ConnectDialog::~ConnectDialog()
{
    //We release the keyboard from the line edit
    ui->hostname->releaseKeyboard();
	delete ui;
}

void ConnectDialog::btnClicked()
{   if (!signalsBlocked){
        if (connected)
            Q_EMIT newContext(uri);
        else{
            //
            signalsBlocked = true;
            validateInput();
            //
        }
    }
}

void ConnectDialog::discardSettings()
{
	setDynamicProperty(ui->connectBtn, "failed", false);
	setDynamicProperty(ui->connectBtn, "connected", false);
	ui->connectBtn->setText("Connect");

    //when we discard the settings we unlock the signals
    ui->groupBox->setTitle(QApplication::translate("Connect", "Context info", Q_NULLPTR));
    ui->description->setPlainText(QString(""));
	this->connected = false;

    signalsBlocked = false;
}

void ConnectDialog::validateInput()
{
	QString new_uri = "ip:" + ui->hostname->text();
	struct iio_context *ctx = iio_create_context_from_uri(
			new_uri.toStdString().c_str());

	this->connected = !!ctx;
	if (!!ctx) {
		QString description(iio_context_get_description(ctx));
		ui->description->setPlainText(description);

		setDynamicProperty(ui->connectBtn, "connected", true);
		ui->connectBtn->setText("Add");
		this->uri = new_uri;

		iio_context_destroy(ctx);
	} else {
		setDynamicProperty(ui->connectBtn, "failed", true);
		ui->connectBtn->setText("Failed!");
        //Show a warning instead of a Context info when we can't connect
        ui->groupBox->setTitle(QApplication::translate("Connect", "Warning", Q_NULLPTR));
        ui->description->setPlainText("Unable to find host: No such host is known!");
	}
}

//Method to focus on a widget
void ConnectDialog::forceFocus(QWidget *widget){
     widget->activateWindow();
     QFocusEvent *eventFocus = new QFocusEvent(QEvent::FocusIn);
     qApp->postEvent(widget,(QEvent *)eventFocus,Qt::LowEventPriority);
}

//
