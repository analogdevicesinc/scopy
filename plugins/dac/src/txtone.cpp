/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#include "txtone.h"
#include "txnode.h"
#include "dac_logging_categories.h"
#include "dacutils.h"

#include <float.h>
#include <menuheader.h>

#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/datastrategy/channelattrdatastrategy.h>

using namespace scopy;
using namespace scopy::dac;

TxTone::TxTone(TxNode *node, unsigned int idx, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
	, m_pairedTone(nullptr)
	, m_idx(idx)
{
	QVBoxLayout *toneLay = new QVBoxLayout(this);
	toneLay->setMargin(0);
	toneLay->setSpacing(10);
	this->setLayout(toneLay);

	QWidget *headerWidget = new QWidget(this);
	QHBoxLayout *headerLay = new QHBoxLayout(headerWidget);
	headerLay->setMargin(0);
	headerLay->setSpacing(0);
	headerWidget->setLayout(headerLay);

	QLabel *name = new QLabel(this);
	name->setText("Tone " + QString::number(idx) + " : " + m_node->getUuid());
	StyleHelper::MenuComboLabel(name);
	StyleHelper::BackgroundWidget(this);

	m_frequency = IIOWidgetBuilder(this)
			      .channel(m_node->getChannel())
			      .attribute("frequency")
			      .uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			      .parent(this)
			      .buildSingle();
	m_frequency->setUItoDataConversion(std::bind(&TxTone::frequencyUItoDS, this, std::placeholders::_1));
	m_frequency->setDataToUIConversion(std::bind(&TxTone::frequencyDStoUI, this, std::placeholders::_1));

	m_scale = IIOWidgetBuilder(this)
			  .channel(m_node->getChannel())
			  .attribute("scale")
			  .uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			  .parent(this)
			  .buildSingle();
	m_scale->setUItoDataConversion(std::bind(&TxTone::scaleUItoDS, std::placeholders::_1));
	m_scale->setDataToUIConversion(std::bind(&TxTone::scaleDStoUI, std::placeholders::_1));

	m_phase = IIOWidgetBuilder(this)
			  .channel(m_node->getChannel())
			  .attribute("phase")
			  .uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			  .parent(this)
			  .buildSingle();

	m_phase->setUItoDataConversion(std::bind(&TxTone::phaseUItoDS, this, std::placeholders::_1));
	m_phase->setDataToUIConversion(std::bind(&TxTone::phaseDStoUI, this, std::placeholders::_1));

	connect(dynamic_cast<ChannelAttrDataStrategy *>(m_frequency->getDataStrategy()),
		&ChannelAttrDataStrategy::emitStatus, this, &TxTone::forwardFreqChange);

	connect(dynamic_cast<ChannelAttrDataStrategy *>(m_scale->getDataStrategy()),
		&ChannelAttrDataStrategy::emitStatus, this, &TxTone::forwardScaleChange);

	connect(dynamic_cast<ChannelAttrDataStrategy *>(m_phase->getDataStrategy()),
		&ChannelAttrDataStrategy::emitStatus, this, &TxTone::forwardPhaseChange);

	headerLay->addWidget(name);
	headerLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));

	toneLay->addWidget(headerWidget);
	toneLay->addWidget(m_frequency);
	toneLay->addWidget(m_scale);
	toneLay->addWidget(m_phase);
}

TxTone::~TxTone() {}

void TxTone::updateFrequency(QString frequency)
{
	if(m_frequency->read().first == frequency) {
		return;
	}
	qDebug(CAT_DAC_DDS) << "Tone " << m_node->getUuid() << "frequency update.";
	m_frequency->getDataStrategy()->write(frequency);
	m_frequency->getUiStrategy()->requestData();
}

void TxTone::updateScale(QString scale)
{
	if(m_scale->read().first == scale) {
		return;
	}
	m_scale->getDataStrategy()->write(scale);
	m_scale->getUiStrategy()->requestData();
}

void TxTone::updatePhase(QString phase)
{
	if(m_phase->read().first == phase) {
		return;
	}
	m_phase->getDataStrategy()->write(phase);
	m_phase->getUiStrategy()->requestData();
}

void TxTone::forwardScaleChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	qDebug(CAT_DAC_DDS) << "Tone " << m_node->getUuid() << "emit scale updated.";
	Q_EMIT scaleUpdated(m_idx, oldData, newData);
}

void TxTone::forwardPhaseChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	qDebug(CAT_DAC_DDS) << "Tone " << m_node->getUuid() << "emit phase updated.";
	Q_EMIT phaseUpdated(m_idx, newData);
}

void TxTone::forwardFreqChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	qDebug(CAT_DAC_DDS) << "Tone " << m_node->getUuid() << "emit frequency updated.";
	Q_EMIT frequencyUpdated(m_idx, newData);
}

void TxTone::read()
{
	m_frequency->getUiStrategy()->requestData();
	m_scale->getUiStrategy()->requestData();
	m_phase->getUiStrategy()->requestData();
}

QString TxTone::toneUuid() const { return m_node->getUuid(); }

QString TxTone::frequency() { return m_frequency->read().first; }

QString TxTone::phase() { return m_phase->read().first; }

QString TxTone::scale() { return m_scale->read().first; }

void TxTone::setPairedTone(TxTone *tone)
{
	m_pairedTone = tone;
	if(m_pairedTone) {
		qDebug(CAT_DAC_DDS) << QString("Paired tone %1 to tone %2").arg(toneUuid()).arg(tone->toneUuid());
		connect(this, &TxTone::phaseUpdated, [=, this](int toneIdx, QString phase) {
			QString emitterFreq = frequency();
			QString ph2 = computePairedPhase(emitterFreq, phase);
			m_pairedTone->updatePhase(ph2);
		});
		connect(this, &TxTone::frequencyUpdated, [=, this](int toneIdx, QString frequency) {
			m_pairedTone->updateFrequency(frequency);
			QString emitterPhase = phase();
			QString ph2 = computePairedPhase(frequency, emitterPhase);
			m_pairedTone->updatePhase(ph2);
		});
		connect(this, &TxTone::scaleUpdated,
			[=, this](int toneIdx, QString oldScale, QString scale) { m_pairedTone->updateScale(scale); });
	}
}

QString TxTone::computePairedPhase(QString frequency, QString phase)
{
	bool ok;
	int freq1 = frequency.toInt(&ok);
	if(!ok) {
		return "";
	}
	int ph1 = phaseDStoUI(phase).toInt(&ok);
	if(!ok) {
		return "";
	}
	double inc1 = 0;

	if(freq1 >= 0) {
		inc1 = 90;
	} else {
		inc1 = 270;
	}
	if((ph1 - inc1) < 0) {
		ph1 += 360;
	}
	ph1 = ph1 - inc1;
	return phaseUItoDS(QString::number(ph1));
}

QString TxTone::phaseUItoDS(QString data)
{
	bool ok = true;
	QString tmp_data = data;
	tmp_data = tmp_data.toLower();
	tmp_data = tmp_data.simplified();
	tmp_data.replace(" ", "");
	double d_data = tmp_data.toDouble(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid phase value in UI %1").arg(data);
		return "";
	}
	d_data = d_data * 1000;
	return QString::number(d_data);
}

QString TxTone::phaseDStoUI(QString data)
{
	bool ok = true;
	double d_data = data.toInt(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid phase value on device %1").arg(data);
		return data;
	}
	d_data = d_data / 1000;
	QString s_data = QString::number(d_data);
	return s_data;
}

QString TxTone::scaleUItoDS(QString data)
{
	bool ok;
	QString tmp_data = data;
	tmp_data = tmp_data.toLower();
	tmp_data = tmp_data.simplified();
	tmp_data.replace("db", "");
	tmp_data.replace(" ", "");
	double d_data = tmp_data.toDouble(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid scale value in UI %1").arg(data);
		d_data = 0;
	}
	d_data = DacUtils::dbFullScaleConvert(d_data, false);
	return QString::number(d_data);
}

QString TxTone::scaleDStoUI(QString data)
{
	bool ok;
	double d_data = data.toDouble(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid scale value on device %1").arg(data);
		return data;
	}
	d_data = DacUtils::dbFullScaleConvert(d_data, true);
	QString s_data;
	if(d_data == -DBL_MAX) {
		s_data = "-Inf";
	} else {
		s_data = QString::number(d_data);
	}
	s_data += " dB";
	return s_data;
}

QString TxTone::frequencyUItoDS(QString data)
{
	bool ok;
	QString tmp_data = data;
	tmp_data = tmp_data.toLower();
	tmp_data = tmp_data.simplified();
	tmp_data.replace("mhz", "");
	tmp_data.replace(" ", "");
	double d_data = tmp_data.toDouble(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid frequency value in UI %1").arg(data);
		return "";
	}
	d_data = d_data * 1e6;
	return QString::number(d_data, 'g', 10);
}

QString TxTone::frequencyDStoUI(QString data)
{
	bool ok;
	double d_data = data.toDouble(&ok);
	if(!ok) {
		qDebug(CAT_DAC_DDS) << QString("Invalid frequency value on device %1").arg(data);
		return data;
	}
	d_data = d_data / 1e6;
	QString s_data = QString::number(d_data, 'g', 10);
	s_data += " MHz";
	return s_data;
}
