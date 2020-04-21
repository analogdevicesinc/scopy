/*
 * Copyright 2018 Analog Devices, Inc.
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

#include "runsinglewidget.h"

#include "customPushButton.hpp"
#include "dynamicWidget.hpp"
#include "utils.h"

#include "ui_runsinglewidget.h"

using namespace adiscope;

RunSingleWidget::RunSingleWidget(QWidget *parent)
	: QWidget(parent), d_ui(new Ui::RunSingleWidget), d_singleButtonEnabled(true) {
	d_ui->setupUi(this);

	connect(d_ui->runButton, &QPushButton::toggled, this, &RunSingleWidget::_toggle);
	connect(d_ui->singleButton, &QPushButton::toggled, this, &RunSingleWidget::_toggle);
}

RunSingleWidget::~RunSingleWidget() { delete d_ui; }

void RunSingleWidget::enableSingleButton(bool enable) {
	d_ui->singleButton->setEnabled(enable);
	d_ui->singleButton->setVisible(enable);
	d_singleButtonEnabled = enable;
}

bool RunSingleWidget::singleButtonEnabled() const { return d_singleButtonEnabled; }

bool RunSingleWidget::singleButtonChecked() const { return d_ui->singleButton->isChecked(); }

bool RunSingleWidget::runButtonChecked() const { return d_ui->runButton->isChecked(); }

void RunSingleWidget::toggle(bool checked) {
	if (!checked) {
		QSignalBlocker blockerRunButton(d_ui->runButton);
		QSignalBlocker blockerSingleButton(d_ui->singleButton);
		d_ui->runButton->setChecked(false);
		d_ui->singleButton->setChecked(false);
		setDynamicProperty(d_ui->runButton, "running", false);
		setDynamicProperty(d_ui->singleButton, "running", false);

	} else if (!d_ui->singleButton->isChecked()) {
		QSignalBlocker blockerRunButton(d_ui->runButton);
		d_ui->runButton->setChecked(true);
		setDynamicProperty(d_ui->runButton, "running", true);
	}

	Q_EMIT toggled(checked);
}

void RunSingleWidget::single() { d_ui->singleButton->setChecked(true); }

void RunSingleWidget::_toggle(bool checked) {
	QPushButton *btn = dynamic_cast<QPushButton *>(QObject::sender());
	setDynamicProperty(btn, "running", checked);

	if (btn == d_ui->singleButton && d_ui->runButton->isChecked()) {
		QSignalBlocker blocker(d_ui->runButton);
		d_ui->runButton->setChecked(false);
		setDynamicProperty(d_ui->runButton, "running", false);
	} else if (btn == d_ui->runButton && d_ui->singleButton->isChecked()) {
		QSignalBlocker blocker(d_ui->singleButton);
		d_ui->singleButton->setChecked(false);
		setDynamicProperty(d_ui->singleButton, "running", false);
	} else {
		Q_EMIT toggled(checked);
	}
}
