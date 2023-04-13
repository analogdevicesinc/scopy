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

#include "runsinglewidget.h"
#include "ui_runsinglewidget.h"

#include "utils.h"
#include "dynamicWidget.h"
#include "customPushButton.h"

using namespace scopy;

RunSingleWidget::RunSingleWidget(QWidget *parent) :
	QWidget(parent),
	d_ui(new Ui::RunSingleWidget),
	d_singleButtonEnabled(true),
	d_runButtonEnabled(true)
{
	d_ui->setupUi(this);

	connect(d_ui->runButton, &QPushButton::toggled,
		this, &RunSingleWidget::_toggle);
	connect(d_ui->singleButton, &QPushButton::toggled,
		this, &RunSingleWidget::_toggle);

}

RunSingleWidget::~RunSingleWidget()
{
	delete d_ui;
}

void RunSingleWidget::enableSingleButton(bool enable)
{
	d_ui->singleButton->setEnabled(enable);
	d_ui->singleButton->setVisible(enable);
	d_singleButtonEnabled = enable;
	if(!enable) {
		setMinimumSize(150, 0);
	}
}

bool RunSingleWidget::singleButtonEnabled() const
{
	return d_singleButtonEnabled;
}

void RunSingleWidget::enableRunButton(bool enable)
{
	d_ui->runButton->setEnabled(enable);
	d_ui->runButton->setVisible(enable);
	d_runButtonEnabled = enable;
}

bool RunSingleWidget::runButtonEnabled() const
{
	return d_runButtonEnabled;
}

bool RunSingleWidget::singleButtonChecked() const
{
	return d_ui->singleButton->isChecked();
}

bool RunSingleWidget::runButtonChecked() const
{
	return d_ui->runButton->isChecked();
}

QPushButton* RunSingleWidget::getRunButton() { return d_ui->runButton; }

QPushButton* RunSingleWidget::getSingleButton() { return d_ui->singleButton; }

void RunSingleWidget::toggle(bool checked)
{
	if (!checked) {
		QSignalBlocker blockerRunButton(d_ui->runButton);
		QSignalBlocker blockerSingleButton(d_ui->singleButton);
		d_ui->runButton->setChecked(false);
		d_ui->singleButton->setChecked(false);
		setDynamicProperty(d_ui->runButton, "running", false);
		setDynamicProperty(d_ui->singleButton, "running", false);
		d_ui->runButton->setText(tr("Run"));
		d_ui->singleButton->setText(tr("Single"));

	} else if (!d_ui->singleButton->isChecked()) {
		QSignalBlocker blockerRunButton(d_ui->runButton);
		if (runButtonEnabled()) {
			d_ui->runButton->setChecked(true);
			d_ui->runButton->setText(tr("Stop"));
		} else if (singleButtonEnabled()) {
			d_ui->singleButton->setChecked(true);
			d_ui->singleButton->setText(tr("Stop"));
		}
		setDynamicProperty(d_ui->runButton, "running", true);
		d_ui->runButton->setText(tr("Stop"));
	}

	Q_EMIT toggled(checked);
}

void RunSingleWidget::single()
{
	d_ui->singleButton->setChecked(true);
}

void RunSingleWidget::_toggle(bool checked)
{
	QPushButton *btn = dynamic_cast<QPushButton *>(QObject::sender());
	setDynamicProperty(btn, "running", checked);

	if (btn == d_ui->singleButton) {
		btn->setText(checked ? tr("Stop") : tr("Single"));
	} else if (btn == d_ui->runButton) {
		btn->setText(checked ? tr("Stop") : tr("Run"));
	}

	if (btn == d_ui->singleButton && d_ui->runButton->isChecked()) {
		QSignalBlocker blocker(d_ui->runButton);
		d_ui->runButton->setChecked(false);
		setDynamicProperty(d_ui->runButton, "running", false);
		d_ui->runButton->setText(tr("Run"));
	} else if (btn == d_ui->runButton && d_ui->singleButton->isChecked()) {
		QSignalBlocker blocker(d_ui->singleButton);
		d_ui->singleButton->setChecked(false);
		setDynamicProperty(d_ui->singleButton, "running", false);
		d_ui->singleButton->setText(tr("Single"));
	} else {
		Q_EMIT toggled(checked);
	}
}

#include "moc_runsinglewidget.cpp"
