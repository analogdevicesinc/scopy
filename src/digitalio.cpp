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
#include "dynamicWidget.hpp"

// Generated UI
#include "ui_digitalio.h"
#include "ui_digitalIoElement.h"
#include "ui_digitalIoChannel.h"
#include "ui_digitaliomenu.h"

using namespace std;
using namespace adiscope;

namespace adiscope {

DigitalIoGroup::DigitalIoGroup(QString label, int ch_mask,int io_mask,
                               DigitalIO *dio, QWidget *parent) : QWidget(parent), ch_mask(ch_mask),
	io_mask(io_mask), dio(dio)
{
	ui = new Ui::dioElement();
	ui->setupUi(this);

	auto ch_mask_temp = ch_mask;
	auto j=0;

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

	connect(this,SIGNAL(slider(int)),dio,SLOT(setSlider(int)));
	ui->label_2->setText(label);
	auto max = (1<<nr_of_channels) -1;
	ui->lineEdit->setValidator(new QIntValidator(0, max, this));
	ui->lineEdit->setText(QString::number(max/2));
	ui->horizontalSlider->setValue(max/2);

}
DigitalIoGroup::~DigitalIoGroup()
{
	delete ui;
}

void DigitalIO::setDirection(int ch, int direction)
{
	if (!offline_mode) {
		diom->setDirection(ch,direction);
		updateUi();
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
	Tool::setVisible(visible);
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

DigitalIO::DigitalIO(struct iio_context *ctx, Filter *filt, QPushButton *runBtn,
                     DIOManager *diom, QJSEngine *engine,
                     ToolLauncher *parent, bool offline_mode) :
	Tool(ctx, runBtn, new DigitalIO_API(this), "Digital IO", parent),
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

	connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(btnRunStop_toggled()));
	connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));

	if (!offline_mode) {
		connect(diom,SIGNAL(locked()),this,SLOT(lockUi()));
		connect(diom,SIGNAL(unlocked()),this,SLOT(lockUi()));
	}

	poll = new QTimer(this);
	connect(poll,SIGNAL(timeout()),this,SLOT(updateUi()));

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                               TOOL_DIGITALIO)));
	api->load(*settings);
	api->js_register(engine);

}

DigitalIO::~DigitalIO()
{
	if (!offline_mode) {
	}

	if (saveOnExit) {
		api->save(*settings);
	}

	delete api;
	delete ui;
}

QPair<QWidget *,Ui::dioChannel *> *DigitalIO::findIndividualUi(int ch)
{

	for (auto &&group : groups) {
		auto i=0;

		for (auto wid : group->chui) {
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
			auto chk = gpi&0x01;
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

void adiscope::DigitalIoGroup::on_inout_clicked()
{
	qDebug()<<"PB";
	auto chk = ui->inout->isChecked();

	ui->lineEdit->setEnabled(!chk);
	ui->horizontalSlider->setEnabled(!chk);

	auto val = 0;
	auto i=0;

	for (auto ch:chui) {
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

void adiscope::DigitalIoGroup::on_horizontalSlider_valueChanged(int value)
{
	qDebug()<<"horizontalSlider";

	if (ui->horizontalSlider->hasTracking()) {
		ui->lineEdit->setText(QString::number(value));

		if (!ui->inout->isChecked() && ui->inout->isEnabled()) {
			auto val = ui->lineEdit->text().toInt();
			Q_EMIT slider(val);

		}
	}
}

void adiscope::DigitalIoGroup::on_lineEdit_editingFinished()
{
	qDebug()<<"lineedit";
	ui->horizontalSlider->setValue(ui->lineEdit->text().toInt());
}

void adiscope::DigitalIoGroup::on_comboBox_activated(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
	on_inout_clicked();
}

void adiscope::DigitalIO::lockUi()
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

void adiscope::DigitalIO::btnRunStop_toggled()
{
	if (ui->btnRunStop->isChecked()) {
		ui->btnRunStop->setText("Stop");
		diom->enableOutput(true);
	} else {
		ui->btnRunStop->setText("Run");
		diom->enableOutput(false);
	}
}

QList<bool> DigitalIO_API::direction() const
{
	QList<bool> list;
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		list.append(ch->second->inout->isChecked());
	}

	return list;
}

void DigitalIO_API::setDirection(const QList<bool>& list)
{
	unsigned int i;

	for (i=0; i<16; i++) {
		auto ch = dio->findIndividualUi(i);
		ch->second->inout->setChecked(list.at(i));
		dio->setDirection(i,list.at(i));
	}
}


QList<bool> DigitalIO_API::output() const
{
	QList<bool> list;
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		list.append(ch->second->output->isChecked());
	}

	return list;
}

void DigitalIO_API::setOutput(const QList<bool>& list)
{
	unsigned int i;

	for (i=0; i<16; i++) {
		auto ch = dio->findIndividualUi(i);
		ch->second->output->setChecked(list.at(i));
		dio->setOutput(i,list.at(i));
	}
}

QList<bool> DigitalIO_API::grouped() const
{

	QList<bool> list;
	for (int i = 0; i < dio->groups.size(); ++i) {
		bool grouped = dio->groups[i]->ui->stackedWidget->currentIndex() == 1;
		list.push_back(grouped);
	}
	return list;
}

void DigitalIO_API::setGrouped(const QList<bool> &grouped)
{
	for (int i = 0; i < grouped.size(); ++i) {
		int index = grouped[i] ? 1 : 0;
		dio->groups[i]->ui->stackedWidget->setCurrentIndex(index);
		dio->groups[i]->ui->comboBox->setCurrentIndex(index);
		dio->groups[i]->on_inout_clicked();
	}

}
}

