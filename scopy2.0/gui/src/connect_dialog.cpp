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

#include "dynamic_widget.hpp"

#include "ui_connect_dialog.h"

#include <iio.h>

#include <QtConcurrentRun>

#include <functional>
#include <iostream>
#include <scopy/gui/connect_dialog.hpp>

using namespace scopy::gui;

ConnectDialog::ConnectDialog(QWidget* widget)
	: QWidget(widget)
	, ui(new Ui::Connect)
	, connected(false)
{
	ui->setupUi(this);
	ui->btnConnect->setText(tr("Connect"));

	// The connect button will be disabled untill we write something in the lineEditHostName
	ui->btnConnect->setDisabled(true);

	connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(btnClicked()));
	connect(ui->lineEditHostName, SIGNAL(returnPressed()), this, SLOT(btnClicked()));
	connect(ui->lineEditHostName, SIGNAL(textChanged(const QString&)), this, SLOT(discardSettings()));
	connect(this, SIGNAL(finished(struct iio_context*)), this, SLOT(updatePopUp(struct iio_context*)));
	ui->lineEditHostName->activateWindow();
	ui->lineEditHostName->setFocus();
	DynamicWidget::setDynamicProperty(ui->lineEditHostName, "invalid", false);
	DynamicWidget::setDynamicProperty(ui->lineEditHostName, "valid", false);
	ui->lblInfoSection->hide();
}

ConnectDialog::~ConnectDialog() { delete ui; }

QString ConnectDialog::URIstringParser(QString uri)
{
	QStringList prefixes = {"usb", "xml", "local", "serial", "ip"};

	bool prependIP = true;
	for (auto pre : prefixes) {
		if (uri.startsWith(pre) == true)
			prependIP = false;
	}

	return ((prependIP) ? "ip:" : "") + uri;
}

void ConnectDialog::btnClicked()
{
	if (!ui->btnConnect->isEnabled()) {
		return;
	} else {
		ui->lblInfoSection->setText(tr("Waiting for connection ..."));
	}

	if (connected) {
		QString new_uri = URIstringParser(ui->lineEditHostName->text());
		Q_EMIT newContext(new_uri);
	} else {
		validateInput();
	}
}

void ConnectDialog::discardSettings()
{
	if (!ui->lineEditHostName->text().isEmpty()) {
		ui->btnConnect->setDisabled(false);
	} else {
		ui->btnConnect->setDisabled(true);
	}

	ui->lblInfoSection->setText(tr("Context info"));
	DynamicWidget::setDynamicProperty(ui->btnConnect, "failed", false);
	DynamicWidget::setDynamicProperty(ui->lineEditHostName, "invalid", false);
	DynamicWidget::setDynamicProperty(ui->lineEditHostName, "valid", false);
	ui->btnConnect->setText(tr("Connect"));

	ui->lblInfoSection->hide();
	ui->lblDescription->setText(QString(""));
	this->connected = false;
}

void ConnectDialog::validateInput()
{
	ui->btnConnect->setDisabled(true);

	QString new_uri = URIstringParser(ui->lineEditHostName->text());
	this->ui->lineEditHostName->setDisabled(true);
	QtConcurrent::run(std::bind(&ConnectDialog::createContext, this, new_uri));
}

void ConnectDialog::createContext(const QString& uri)
{

	this->parent()->installEventFilter(this);

	struct iio_context* ctx_from_uri = iio_create_context_from_uri(uri.toStdString().c_str());

	this->parent()->removeEventFilter(this);

	Q_EMIT finished(ctx_from_uri);
}
void ConnectDialog::updatePopUp(struct iio_context* ctx)
{
	this->ui->lineEditHostName->setDisabled(false);
	this->connected = !!ctx;

	ui->lblInfoSection->show();
	if (!!ctx) {
		ui->btnConnect->setDisabled(false);
		QString lblDescription(iio_context_get_description(ctx));
		ui->lblDescription->setText(lblDescription);
		ui->lblInfoSection->setText(tr("Context info"));
		DynamicWidget::setDynamicProperty(ui->lineEditHostName, "invalid", false);
		DynamicWidget::setDynamicProperty(ui->lineEditHostName, "valid", true);
		ui->btnConnect->setText(tr("Add"));

		iio_context_destroy(ctx);
	} else {
		DynamicWidget::setDynamicProperty(ui->lineEditHostName, "valid", false);
		DynamicWidget::setDynamicProperty(ui->lineEditHostName, "invalid", true);
		ui->lblInfoSection->setText(tr("Warning"));
		ui->lblDescription->setText(tr("Error: Unable to find host: No such host is known!"));
	}
}
bool ConnectDialog::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick ||
	    event->type() == QEvent::MouseButtonRelease) {
		return true;
	}

	return false;
}
