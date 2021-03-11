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

#include "ui_run_single_widget.h"

#include <scopy/gui/run_single_widget.hpp>

using namespace scopy::gui;

RunSingleWidget::RunSingleWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::RunSingleWidget)
	, m_singleButtonEnabled(true)
	, m_runButtonEnabled(true)
{
	m_ui->setupUi(this);

	connect(m_ui->btnRun, &QPushButton::toggled, this, &RunSingleWidget::_toggle);
	connect(m_ui->btnSingle, &QPushButton::toggled, this, &RunSingleWidget::_toggle);
}

RunSingleWidget::~RunSingleWidget() { delete m_ui; }

void RunSingleWidget::enableSingleButton(bool enable)
{
	m_ui->btnSingle->setEnabled(enable);
	m_ui->btnSingle->setVisible(enable);
	m_singleButtonEnabled = enable;
	if (!enable) {
		setMinimumSize(150, 0);
	}
}

bool RunSingleWidget::singleButtonEnabled() const { return m_singleButtonEnabled; }

void RunSingleWidget::enableRunButton(bool enable)
{
	m_ui->btnRun->setEnabled(enable);
	m_ui->btnRun->setVisible(enable);
	m_runButtonEnabled = enable;
}

bool RunSingleWidget::runButtonEnabled() const { return m_runButtonEnabled; }

bool RunSingleWidget::singleButtonChecked() const { return m_ui->btnSingle->isChecked(); }

bool RunSingleWidget::runButtonChecked() const { return m_ui->btnRun->isChecked(); }

QPushButton* RunSingleWidget::getRunButton() { return m_ui->btnRun; }

QPushButton* RunSingleWidget::getSingleButton() { return m_ui->btnSingle; }

void RunSingleWidget::toggle(bool checked)
{
	if (!checked) {
		QSignalBlocker blockerRunButton(m_ui->btnRun);
		QSignalBlocker blockerSingleButton(m_ui->btnSingle);
		m_ui->btnRun->setChecked(false);
		m_ui->btnSingle->setChecked(false);
		DynamicWidget::setDynamicProperty(m_ui->btnRun, "running", false);
		DynamicWidget::setDynamicProperty(m_ui->btnSingle, "running", false);
		m_ui->btnRun->setText(tr("Run"));
		m_ui->btnSingle->setText(tr("Single"));

	} else if (!m_ui->btnSingle->isChecked()) {
		QSignalBlocker blockerRunButton(m_ui->btnRun);
		if (runButtonEnabled()) {
			m_ui->btnRun->setChecked(true);
			m_ui->btnRun->setText(tr("Stop"));
		} else if (singleButtonEnabled()) {
			m_ui->btnSingle->setChecked(true);
			m_ui->btnSingle->setText(tr("Stop"));
		}
		DynamicWidget::setDynamicProperty(m_ui->btnRun, "running", true);
		m_ui->btnRun->setText(tr("Stop"));
	}

	Q_EMIT toggled(checked);
}

void RunSingleWidget::single() { m_ui->btnSingle->setChecked(true); }

void RunSingleWidget::_toggle(bool checked)
{
	QPushButton* btn = dynamic_cast<QPushButton*>(QObject::sender());
	DynamicWidget::setDynamicProperty(btn, "running", checked);

	if (btn == m_ui->btnSingle) {
		btn->setText(checked ? tr("Stop") : tr("Single"));
	} else if (btn == m_ui->btnRun) {
		btn->setText(checked ? tr("Stop") : tr("Run"));
	}

	if (btn == m_ui->btnSingle && m_ui->btnRun->isChecked()) {
		QSignalBlocker blocker(m_ui->btnRun);
		m_ui->btnRun->setChecked(false);
		DynamicWidget::setDynamicProperty(m_ui->btnRun, "running", false);
		m_ui->btnRun->setText(tr("Run"));
	} else if (btn == m_ui->btnRun && m_ui->btnSingle->isChecked()) {
		QSignalBlocker blocker(m_ui->btnSingle);
		m_ui->btnSingle->setChecked(false);
		DynamicWidget::setDynamicProperty(m_ui->btnSingle, "running", false);
		m_ui->btnSingle->setText(tr("Single"));
	} else {
		Q_EMIT toggled(checked);
	}
}
