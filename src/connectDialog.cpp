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

#include "connectDialog.hpp"
#include "dynamicWidget.hpp"
#include <QtConcurrentRun>
#include <functional>

#include "ui_connect.h"

#include <iio.h>
#include <iostream>
#include <QCoreApplication>

using namespace adiscope;

ConnectDialog::ConnectDialog(QWidget *widget) : QWidget(widget),
	ui(new Ui::Connect), connected(false), enableDemo(false)
{
	ui->setupUi(this);
	ui->connectBtn->setText(tr("Connect"));
	//The connect button will be disabled untill we write something in the hostname line edit
	ui->connectBtn->setDisabled(true);

	connect(ui->connectBtn, SIGNAL(clicked()), this, SLOT(btnClicked()));
	connect(ui->enableDemoBtn, SIGNAL(clicked()), this, SLOT(enableDemoBtn()));
	connect(ui->hostname, SIGNAL(returnPressed()),
	        this, SLOT(btnClicked()));
	connect(ui->hostname, SIGNAL(textChanged(const QString&)),
	        this, SLOT(discardSettings()));
	connect(this,SIGNAL(finished(struct iio_context *)),this,
	        SLOT(updatePopUp(struct iio_context *)));
	ui->hostname->activateWindow();
	ui->hostname->setFocus();
	setDynamicProperty(ui->hostname, "invalid", false);
	setDynamicProperty(ui->hostname, "valid", false);
	ui->infoSection->hide();

	ui->demoDevicesComboBox->addItem("adalm2000");
	ui->demoDevicesComboBox->setDisabled(false);
	process = new QProcess(this);
}

ConnectDialog::~ConnectDialog()
{
	delete ui;
}

QString ConnectDialog::URIstringParser(QString uri)
{
	QStringList prefixes =
	{
		"usb","xml","local","serial","ip"
	};

	bool prependIP = true;
	for( auto pre:prefixes)
	{
		if(uri.startsWith(pre) == true)
			prependIP = false;
	}

	return ((prependIP) ? "ip:" : "") + uri;
}

void ConnectDialog::btnClicked()
{
	if (!ui->connectBtn->isEnabled()) {
		return;
	} else {
		ui->infoSection->setText(tr("Waiting for connection ..."));
	}

	if (connected) {
		QString new_uri = URIstringParser(ui->hostname->text());
		Q_EMIT newContext(new_uri);
	} else {
		validateInput();
	}
}

void ConnectDialog::enableDemoBtn()
{
	if (!enableDemo) {
		QProcess killProcess;
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
		killProcess.start("taskkill /im iio-emu /f");
#else
		killProcess.start("pkill iio-emu");
#endif
		killProcess.waitForFinished();

		// iio-emu found in system
		QString dirPath = QCoreApplication::applicationDirPath();
		QString program = dirPath + "/iio-emu";
		QStringList arguments;
		arguments.append(ui->demoDevicesComboBox->currentText());
		process->setProgram(program);
		process->setArguments(arguments);
		process->start();
		auto started = process->waitForStarted();
		if (!started) {
			// retry to start the process
			// path for iio-emu when Scopy is built manually
			program = dirPath + "/iio-emu/iio-emu";
			process->setProgram(program);
			process->start();
			started = process->waitForStarted();
			if (!started) {
				ui->description->setText("Server failed to start");
				qDebug() << "Process failed to start";
				return;
			}
		}

		ui->enableDemoBtn->setChecked(true);
		ui->enableDemoBtn->setText("Disable Demo");
		ui->hostname->setText("127.0.0.1");

		enableDemo = true;
	} else {
		process->kill();
		ui->enableDemoBtn->setChecked(false);
		ui->enableDemoBtn->setText("Enable Demo");
		ui->hostname->setText("");

		enableDemo = false;
	}

}

void ConnectDialog::discardSettings()
{
	if (!ui->hostname->text().isEmpty()) {
		ui->connectBtn->setDisabled(false);
	} else {
		ui->connectBtn->setDisabled(true);
	}

	ui->infoSection->setText(tr("Context info"));
	setDynamicProperty(ui->connectBtn, "failed", false);
	setDynamicProperty(ui->hostname, "invalid", false);
	setDynamicProperty(ui->hostname, "valid", false);
	ui->connectBtn->setText(tr("Connect"));

	ui->infoSection->hide();
	ui->description->setText(QString(""));
	this->connected = false;
}

void ConnectDialog::validateInput()
{
	ui->connectBtn->setDisabled(true);

	QString new_uri = URIstringParser(ui->hostname->text());
	this->ui->hostname->setDisabled(true);
	QtConcurrent::run(std::bind(&ConnectDialog::createContext,this,new_uri));
}

void ConnectDialog::createContext(const QString& uri)
{

	this->parent()->installEventFilter(this);

	struct iio_context *ctx_from_uri = iio_create_context_from_uri(
	                uri.toStdString().c_str());

	this->parent()->removeEventFilter(this);

	Q_EMIT finished(ctx_from_uri);

}
void ConnectDialog::updatePopUp(struct iio_context *ctx)
{
	this->ui->hostname->setDisabled(false);
	this->connected = !!ctx;

	ui->infoSection->show();
	if (!!ctx) {
		ui->connectBtn->setDisabled(false);
		QString description(iio_context_get_description(ctx));
		ui->description->setText(description);
		ui->infoSection->setText(tr("Context info"));
		setDynamicProperty(ui->hostname, "invalid", false);
		setDynamicProperty(ui->hostname, "valid", true);
		ui->connectBtn->setText(tr("Add"));

		iio_context_destroy(ctx);
	} else {
		setDynamicProperty(ui->hostname, "valid", false);
		setDynamicProperty(ui->hostname, "invalid", true);
		ui->infoSection->setText(tr("Warning"));
		ui->description->setText(tr("Error: Unable to find host: No such host is known!"));
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
