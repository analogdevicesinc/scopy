/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "max14906/diodigitalchannel.h"

#include <QDebug>
#include <qwt_axis.h>
#include <gui/sismograph.hpp>
#include <gui/customSwitch.h>

using namespace scopy::swiotrefactor;

#define SISMOGRAPH_INITIAL_SIZE 10

DioDigitalChannel::DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DioDigitalChannel())
	, m_deviceName(deviceName)
	, m_deviceType(deviceType)
{
	this->ui->setupUi(this);
	this->connectSignalsAndSlots();

	this->ui->m_channelName->setText(deviceName);
	this->ui->m_channelType->setText(deviceType);

	this->ui->customSwitch->setOnText("1");
	this->ui->customSwitch->setOffText("0");

	this->ui->channelTitleLineEdit->setPlaceholderText("Enter title here...");

	if(deviceType == "INPUT") {
		this->ui->customSwitch->setVisible(false);
		this->ui->currentLimitValues->hide();
		this->ui->currentLimitLabel->hide();
		this->ui->setValueLabel->hide();
	} else {
		this->ui->customSwitch->setChecked(false);
	}

	this->ui->sismograph->setAxisTitle(QwtAxis::YLeft, "");		   // clear title
	this->ui->sismograph->setAxisTitle(QwtAxis::XBottom, "");	   // clear title
	this->ui->sismograph->setAxisScale(0, 0, 1, 1);			   // y axis
	this->ui->sismograph->setPlotDirection(Sismograph::RIGHT_TO_LEFT); // plot from right to left
	this->ui->sismograph->setColor(QColor(0xff, 0x72, 0x00));	   // #ff7200
	this->ui->sismograph->setContentsMargins(9, 9, 9, 9);
	this->ui->sismograph->setUpdatesEnabled(true);
	this->ui->sismograph->setAutoscale(false);
	this->ui->sismograph->setAxisScale(QwtAxis::YRight, 0.0, 1.0);
	this->ui->sismograph->setNumSamples(10);
	this->ui->sismograph->setSampleRate(1);
	this->ui->sismograph->updateYScale(SISMOGRAPH_INITIAL_SIZE,
					   0); // FIXME: sismograph typo, actually updates XBottom axis
	this->ui->sismograph->replot();

	this->ui->lcdNumber->setPrecision(0);

	QString modeLabel = deviceType.toLower();
	modeLabel[0] = modeLabel[0].toUpper();
	this->ui->modeLabel->setText(modeLabel + " mode");
}

DioDigitalChannel::~DioDigitalChannel() { delete ui; }

void DioDigitalChannel::connectSignalsAndSlots()
{
	connect(this->ui->customSwitch, &CustomSwitch::toggled, this, [this]() {
		bool isChecked = this->ui->customSwitch->isChecked();

		Q_EMIT this->outputValueChanged(isChecked);
	});
}

void DioDigitalChannel::updateTimeScale(double newMax)
{
	this->ui->sismograph->updateYScale(newMax, 0); // in this case, data always comes towards index 0
	this->ui->sismograph->setNumSamples((int)(newMax));

	this->ui->sismograph->reset(); // apply changes by plotting again
}

void DioDigitalChannel::addDataSample(double value)
{
	this->ui->sismograph->plot(value);

	this->ui->lcdNumber->display(value);
}

const std::vector<std::string> DioDigitalChannel::getConfigModes() const { return m_configModes; }

void DioDigitalChannel::setConfigModes(std::vector<std::string> &configModes)
{
	m_configModes = configModes;

	this->ui->configModes->clear();
	for(const std::string &item : m_configModes) {
		this->ui->configModes->addItem(QString::fromStdString(item));
	}
}

const QString &DioDigitalChannel::getSelectedConfigMode() const { return m_selectedConfigMode; }

void DioDigitalChannel::setSelectedConfigMode(const QString &selectedConfigMode)
{
	m_selectedConfigMode = selectedConfigMode;

	int index = this->ui->configModes->findText(selectedConfigMode);
	this->ui->configModes->setCurrentIndex(index);
	qDebug() << "The channel " << this->m_deviceName << " read selected config mode " << selectedConfigMode;
}

void DioDigitalChannel::resetSismograph() { this->ui->sismograph->reset(); }
