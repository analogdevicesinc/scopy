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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include <QDebug>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>

#include "digitalio.hpp"
#include "gui/dynamicWidget.h"
#include "digitalio_api.hpp"

// Generated UI
#include "ui_digitalio.h"
#include "ui_digitalIoElement.h"
#include "ui_digitalIoChannel.h"
#include "pluginbase/scopyjs.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2K_DIGITAL_IO,"M2kDigitalIo")

using namespace std;
using namespace adiscope::m2k;

namespace adiscope {

DigitalIoGroup::DigitalIoGroup(QString label, int ch_mask,int io_mask,
                               DigitalIO *dio, QWidget *parent) : QWidget(parent), ch_mask(ch_mask),
	io_mask(io_mask), dio(dio)
{
	ui = new Ui::dioElement();
	ui->setupUi(this);

	auto ch_mask_temp = ch_mask;
	auto j=0;
	nr_of_channels = 0;

	while (ch_mask_temp) {
		if (ch_mask_temp&0x01) {
			nr_of_channels ++;
			QWidget *wid = new QWidget(this);
			Ui::dioChannel *dioUi = new Ui::dioChannel;
			dioUi->setupUi(wid);
			ui->horizontalLayout_4->addWidget(wid);
			wid->setProperty("dio",j);
			dioUi->label->setText(QString::number(j));
			wid->setProperty("locked",false);
			chui.append(new QPair<QWidget *,Ui::dioChannel *>(wid,dioUi));
			connect(chui.last()->second->inout,SIGNAL(clicked()),dio,SLOT(setDirection()));
			connect(chui.last()->second->output,SIGNAL(clicked()),dio,SLOT(setOutput()));
		}

		ch_mask_temp>>=1;
		j++;
	}
	connect(ui->inout,SIGNAL(clicked()),this,SLOT(changeDirection()));
	connect(this,SIGNAL(slider(int)),dio,SLOT(setSlider(int)));
	ui->label_2->setText(label);
	auto max = (1<<nr_of_channels) -1;
	ui->lineEdit->setText(QString::number(max/2));
	ui->horizontalSlider->setValue(max/2);	
	ui->comboBox->setCurrentIndex(0);
	ui->stackedWidget->setCurrentIndex(0);
}
DigitalIoGroup::~DigitalIoGroup()
{
	for (auto it = chui.begin(); it != chui.end(); ++it) {
		//delete Ui::dioChannel from pair<QWidget*, Ui::dioChannel>
		delete (*it)->second;
		//delete the pair<QWidget*, Ui::dioChannel>
		delete *it;
	}
	delete ui;
}

void DigitalIO::setDirection(int ch, int direction)
{
	if (!offline_mode) {
		diom->setDirection(ch,direction);
	}
}

void DigitalIO::setDirection()
{
	auto direction = static_cast<QPushButton *>(sender())->isChecked();
	auto ch = sender()->parent()->property("dio").toInt();
	setDirection(ch,direction);
}

void DigitalIO::setOutput(int ch, int out)
{
	if (!offline_mode) {
		diom->setOutRaw(ch,out);
	}
}

void DigitalIO::setVisible(bool visible)
{
	if (visible)
		poll->start(polling_rate);
	else
		poll->stop();
	M2kTool::setVisible(visible);
}

void DigitalIO::setSlider(int val)
{
	auto grp = static_cast<DigitalIoGroup *>(QObject::sender());
	auto v = groups.indexOf(grp);
	auto tempval = val;

	for (auto i=0; i<8; i++) {
		auto output = tempval & 1;
		tempval>>=1;
		setOutput(i+v*8, output);
		findIndividualUi(i+v*8)->second->output->setChecked(output);
	}
}

void DigitalIO::setOutput()
{
	auto output = static_cast<QPushButton *>(sender())->isChecked();
	auto ch = sender()->parent()->property("dio").toInt();
	setOutput(ch, output);

	if (diom->getDirection(ch) && diom->getOutputEnabled()) { // only if output
		setDynamicProperty(findIndividualUi(ch)->second->input,"high",output);
	}
}

DigitalIO::DigitalIO(struct iio_context *ctx, Filter *filt, ToolMenuEntry *toolMenuItem,
                     DIOManager *diom, QJSEngine *engine,
		     QWidget *parent, bool offline_mode) :
	M2kTool(ctx, toolMenuItem, new DigitalIO_API(this), "Digital IO", parent),
	ui(new Ui::DigitalIO),
	offline_mode(offline_mode),
	diom(diom)
{

	// UI
	ui->setupUi(this);
	groups.append(new DigitalIoGroup("DIO 0 - 7 ",0xff,0xff,this,ui->dioContainer));
	ui->containerLayout->addWidget(groups.last());
	groups.append(new DigitalIoGroup("DIO 8 - 15",0xff00,0xff00,this,
	                                 ui->dioContainer));
	ui->containerLayout->addWidget(groups.last());

	connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
	connect(tme, SIGNAL(runToggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), tme, SLOT(setRunning(bool)));

	if (!offline_mode) {
		connect(diom,SIGNAL(locked()),this,SLOT(lockUi()));
		connect(diom,SIGNAL(unlocked()),this,SLOT(lockUi()));
	}

	poll = new QTimer(this);
	connect(poll,SIGNAL(timeout()),this,SLOT(updateUi()));

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                               TOOL_DIGITALIO)));
	ScopyJS::GetInstance()->registerApi(api);
	updateUi();
	readPreferences();

	ui->btnHelp->setUrl("https://wiki.analog.com/university/tools/m2k/scopy/digitalio");

}

void DigitalIO::readPreferences()
{
	Preferences *p = Preferences::GetInstance();
	ui->instrumentNotes->setVisible(p->get("m2k_instrument_notes_active").toBool());
}

DigitalIO::~DigitalIO()
{
	delete api;
	delete ui;
}

QPair<QWidget *,Ui::dioChannel *> *DigitalIO::findIndividualUi(int ch)
{

	for (auto &&group : groups) {
		auto i=0;

		for (auto wid : qAsConst(group->chui)) {
			if (wid->first->property("dio")==ch) {
				return group->chui[i];
			}

			i++;
		}
	}

	return nullptr;
}

void DigitalIO::updateUi()
{
	if (!offline_mode) {
		auto gpi = diom->getGpi();
		auto gpigrp1 = gpi & 0xff;
		auto gpigrp2 = (gpi & 0xff00) >> 8;

		for (auto i=0; i<16; i++) {
			Ui::dioChannel *chui = findIndividualUi(i)->second;
			bool chk = gpi&0x01;
			gpi >>= 1;

			setDynamicProperty(chui->input,"high",chk);

			auto isLocked = diom->getLockMask() & 1<<i;
			auto isOutput = diom->getDirection(i);

			if (!isLocked && isOutput && diom->getOutputEnabled())
				if (chk != diom->getOutRaw(i)) {
					setDynamicProperty(chui->input,"short",true);
				} else {
					setDynamicProperty(chui->input,"short",false);
				}
			else {
				setDynamicProperty(chui->input,"short",false);
			}
		}

		if (groups[0]->ui->inout->isChecked()) {
			groups[0]->ui->horizontalSlider->setValue(gpigrp1);
		}

		if (groups[1]->ui->inout->isChecked()) {
			groups[1]->ui->horizontalSlider->setValue(gpigrp2);
		}
	}
}

void DigitalIoGroup::changeDirection()
{
	qDebug(CAT_M2K_DIGITAL_IO)<<"PB";
	auto chk = ui->inout->isChecked();

	ui->lineEdit->setEnabled(!chk);
	ui->horizontalSlider->setEnabled(!chk);

	auto val = 0;
	auto i=0;

	for (auto ch:qAsConst(chui)) {
		ch->second->inout->setChecked(!chk);
		auto channel = ch->first->property("dio").toInt();
		dio->setDirection(channel,!chk);
		auto bit = ch->second->output->isChecked();
		val = val | (bit << i);
		i++;

	}

	if (!chk) {
		ui->horizontalSlider->setValue(val);
	}
}

void DigitalIoGroup::on_horizontalSlider_valueChanged(int value)
{
	qDebug(CAT_M2K_DIGITAL_IO)<<"horizontalSlider";

	if (ui->horizontalSlider->hasTracking()) {
		ui->lineEdit->setText(QString::number(value));

		if (!ui->inout->isChecked() && ui->inout->isEnabled()) {
			auto val = ui->lineEdit->text().toInt();
			Q_EMIT slider(val);

		}
	}
}

void DigitalIoGroup::on_lineEdit_editingFinished()
{
	qDebug(CAT_M2K_DIGITAL_IO)<<"lineedit";
	auto max = (1<<nr_of_channels) -1;
	auto nr = ui->lineEdit->text().toInt();
	if(nr > max)
	{
		nr = max;
	}
	ui->horizontalSlider->setValue(nr);
}

void DigitalIoGroup::on_comboBox_activated(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
	changeDirection();
}

void DigitalIO::lockUi()
{
	auto lockmask = diom->getLockMask();
	bool g0Lock = false;
	bool g1Lock = false;

	if (lockmask&0xff) {
		g0Lock = true;
	}

	groups[0]->ui->inout->setDisabled(g0Lock);
	groups[0]->ui->horizontalSlider->setDisabled(g0Lock);
	setDynamicProperty(groups[0]->ui->stackedWidgetPage1,"locked",g0Lock);

	if (lockmask&0xff00) {
		g1Lock = true;
	}

	groups[1]->ui->inout->setDisabled(g1Lock);
	groups[1]->ui->horizontalSlider->setDisabled(g1Lock);
	setDynamicProperty(groups[1]->ui->stackedWidgetPage1,"locked",g1Lock);

	for (auto i=0; i<16; i++) {
		auto wid = findIndividualUi(i)->first;
		bool locked = static_cast<bool>(lockmask&0x01);

		setDynamicProperty(wid,"locked",locked);
		wid->setDisabled(locked);
		lockmask>>=1;
	}

}

void DigitalIO::run()
{
	startStop(true);
}
void DigitalIO::stop()
{
	startStop(false);
}

void DigitalIO::startStop(bool checked)
{
	if (checked) {
		ui->btnRunStop->setText(tr("Stop"));
		diom->enableOutput(true);
	} else {
		ui->btnRunStop->setText(tr("Run"));
		diom->enableOutput(false);
	}
}
}

