/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "ad9084channel.h"
#include <gui/widgets/menusectionwidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/datastrategy/channelattrdatastrategy.h>
#include <style.h>
#include <QLabel>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::ad9084;

#define MAX_ATTR_SIZE 1024
#define AD9084_MAX_ADC_FREQ_HZ 20000000000LL
#define AD9084_MAX_DAC_FREQ_HZ 28000000000LL
#define PHASE_RANGE "[-180 1 180]"
#define SCALE_RANGE "[0.0 0.01 1.0]"

Ad9084Channel::Ad9084Channel(iio_channel *chn, unsigned int chnIdx, QWidget *parent)
	: QWidget(parent)
	, m_channel(chn)
	, m_device(nullptr)
	, m_input(true)
	, m_chnIdx(++chnIdx)
	, m_channelLabel("")
{
	QVBoxLayout *lay = new QVBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(0);
	this->setLayout(lay);

	m_device = const_cast<iio_device *>(iio_channel_get_device(m_channel));
	m_input = !iio_channel_is_output(m_channel);
	char *label = new char[MAX_ATTR_SIZE];
	iio_channel_attr_read(m_channel, "label", label, MAX_ATTR_SIZE);
	m_channelLabel = QString(label);
	Style::setBackgroundColor(this, json::theme::background_primary);

	connect(this, &Ad9084Channel::requestEnableUi, this, [this](bool en) {
		if(en) {
			Style::setStyle(m_titleLbl, style::properties::label::subtle, false);
			Style::setStyle(m_titleLbl, style::properties::label::menuBig, true);
		} else {
			Style::setStyle(m_titleLbl, style::properties::label::menuBig, false);
			Style::setStyle(m_titleLbl, style::properties::label::subtle, true);
		}
	});
}

Ad9084Channel::~Ad9084Channel() {}

bool Ad9084Channel::isInput() const { return m_input; }

QString Ad9084Channel::label() const { return m_channelLabel; }

void Ad9084Channel::setupFrequency()
{
	if(m_input) {
		m_frequencyWidget = IIOWidgetBuilder(this)
					    .channel(m_channel)
					    .attribute("adc_frequency")
					    .uiStrategy(IIOWidgetBuilder::EditableUi)
					    .title("ADC Frequency")
					    .parent(this)
					    .buildSingle();
		m_iioWidgetGroupList.value(ADC_FREQUENCY)->add(m_frequencyWidget);
	} else {
		m_frequencyWidget = IIOWidgetBuilder(this)
					    .channel(m_channel)
					    .attribute("dac_frequency")
					    .uiStrategy(IIOWidgetBuilder::EditableUi)
					    .title("DAC Frequency")
					    .parent(this)
					    .buildSingle();
		m_iioWidgetGroupList.value(DAC_FREQUENCY)->add(m_frequencyWidget);
	}
	m_frequencyWidget->setUItoDataConversion(
		std::bind(&Ad9084Channel::frequencyUItoDS, this, std::placeholders::_1));
	m_frequencyWidget->setDataToUIConversion(
		std::bind(&Ad9084Channel::frequencyDStoUI, this, std::placeholders::_1));

	m_iioWidgets.push_back(m_frequencyWidget);
}

void Ad9084Channel::setupChannelNco(QLayout *lay)
{
	auto chnNcoFreq = IIOWidgetBuilder(this)
				  .channel(m_channel)
				  .attribute("channel_nco_frequency")
				  .uiStrategy(IIOWidgetBuilder::RangeUi)
				  .optionsAttribute("channel_nco_frequency_available")
				  .title("NCO Frequency")
				  .parent(this)
				  .buildSingle();
	chnNcoFreq->setUItoDataConversion(std::bind(&Ad9084Channel::frequencyUItoDS, this, std::placeholders::_1));
	chnNcoFreq->setDataToUIConversion(std::bind(&Ad9084Channel::frequencyDStoUI, this, std::placeholders::_1));

	auto chnNcoPhase = IIOWidgetBuilder(this)
				   .channel(m_channel)
				   .attribute("channel_nco_phase")
				   .uiStrategy(IIOWidgetBuilder::RangeUi)
				   .optionsValues(PHASE_RANGE)
				   .title("NCO Phase")
				   .parent(this)
				   .buildSingle();
	chnNcoPhase->setUItoDataConversion(std::bind(&Ad9084Channel::phaseUItoDS, this, std::placeholders::_1));
	chnNcoPhase->setDataToUIConversion(std::bind(&Ad9084Channel::phaseDStoUI, this, std::placeholders::_1));

	lay->addWidget(chnNcoFreq);
	lay->addWidget(chnNcoPhase);

	m_iioWidgets.push_back(chnNcoFreq);
	m_iioWidgets.push_back(chnNcoPhase);
}

void Ad9084Channel::setupMainNco(QLayout *lay)
{
	auto mainNcoFreq = IIOWidgetBuilder(this)
				   .channel(m_channel)
				   .attribute("main_nco_frequency")
				   .uiStrategy(IIOWidgetBuilder::RangeUi)
				   .optionsAttribute("main_nco_frequency_available")
				   .title("Main NCO Frequency")
				   .parent(this)
				   .buildSingle();
	mainNcoFreq->setUItoDataConversion(std::bind(&Ad9084Channel::frequencyUItoDS, this, std::placeholders::_1));
	mainNcoFreq->setDataToUIConversion(std::bind(&Ad9084Channel::frequencyDStoUI, this, std::placeholders::_1));

	auto mainNcoPhase = IIOWidgetBuilder(this)
				    .channel(m_channel)
				    .attribute("main_nco_phase")
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .optionsValues(PHASE_RANGE)
				    .title("Main NCO Phase")
				    .parent(this)
				    .buildSingle();
	mainNcoPhase->setUItoDataConversion(std::bind(&Ad9084Channel::phaseUItoDS, this, std::placeholders::_1));
	mainNcoPhase->setDataToUIConversion(std::bind(&Ad9084Channel::phaseDStoUI, this, std::placeholders::_1));

	lay->addWidget(mainNcoFreq);
	lay->addWidget(mainNcoPhase);

	m_iioWidgets.push_back(mainNcoFreq);
	m_iioWidgets.push_back(mainNcoPhase);
}

void Ad9084Channel::setupInChannelsAttrs(QLayout *lay)
{
	auto testMode = IIOWidgetBuilder(this)
				.channel(m_channel)
				.attribute("test_mode")
				.uiStrategy(IIOWidgetBuilder::ComboUi)
				.optionsAttribute("test_mode_available")
				.title("Test Mode")
				.parent(this)
				.buildSingle();

	auto nyquistZone = IIOWidgetBuilder(this)
				   .channel(m_channel)
				   .attribute("nyquist_zone")
				   .uiStrategy(IIOWidgetBuilder::ComboUi)
				   .optionsAttribute("nyquist_zone_available")
				   .title("Nyquist Zone")
				   .parent(this)
				   .buildSingle();

	auto loopback = IIOWidgetBuilder(this)
				.channel(m_channel)
				.attribute("loopback")
				.uiStrategy(IIOWidgetBuilder::ComboUi)
				.optionsAttribute("loopback_available")
				.title("Loopback Mode")
				.parent(this)
				.buildSingle();

	lay->addWidget(testMode);
	lay->addWidget(nyquistZone);
	lay->addWidget(loopback);

	m_iioWidgets.push_back(testMode);
	m_iioWidgets.push_back(nyquistZone);
	m_iioWidgets.push_back(loopback);
}

void Ad9084Channel::setupChannelTone(QLayout *lay)
{
	auto chnNcoGainScale = IIOWidgetBuilder(this)
				       .channel(m_channel)
				       .attribute("channel_nco_gain_scale")
				       .uiStrategy(IIOWidgetBuilder::RangeUi)
				       .optionsValues(SCALE_RANGE)
				       .title("Gain Scale")
				       .parent(this)
				       .buildSingle();

	auto testToneScale = IIOWidgetBuilder(this)
				     .channel(m_channel)
				     .attribute("channel_nco_test_tone_scale")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues(SCALE_RANGE)
				     .title("Test Tone Scale")
				     .parent(this)
				     .buildSingle();

	auto chnNcoTestToneEn = IIOWidgetBuilder(this)
					.channel(m_channel)
					.attribute("channel_nco_test_tone_en")
					.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					.title("Test Tone Enable")
					.parent(this)
					.buildSingle();
	chnNcoTestToneEn->showProgressBar(false);

	lay->addWidget(chnNcoGainScale);
	lay->addWidget(testToneScale);
	lay->addWidget(chnNcoTestToneEn);

	m_iioWidgets.push_back(chnNcoGainScale);
	m_iioWidgets.push_back(testToneScale);
	m_iioWidgets.push_back(chnNcoTestToneEn);
}

void Ad9084Channel::setupMainTone(QLayout *lay)
{
	auto mainTestToneScale = IIOWidgetBuilder(this)
					 .channel(m_channel)
					 .attribute("main_nco_test_tone_scale")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .optionsValues(SCALE_RANGE)
					 .title("Main Test Tone Scale")
					 .parent(this)
					 .buildSingle();

	auto mainTestToneEn = IIOWidgetBuilder(this)
				      .channel(m_channel)
				      .attribute("main_nco_test_tone_en")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("Test Tone Enable")
				      .parent(this)
				      .buildSingle();
	mainTestToneEn->showProgressBar(false);

	lay->addWidget(mainTestToneScale);
	lay->addWidget(mainTestToneEn);
	m_iioWidgets.push_back(mainTestToneScale);
	m_iioWidgets.push_back(mainTestToneEn);
}

void Ad9084Channel::init()
{
	m_chnSection = new MenuSectionWidget(this);
	m_chnSection->contentLayout()->setSpacing(10);
	Style::setStyle(m_chnSection, style::properties::widget::border_interactive);

	// Setup channel name and label
	QString chnId = QString(iio_channel_get_id(m_channel));
	chnId = chnId.remove("voltage");
	QString title = QString("Channel %1 [ %2 ]").arg(QString::number(m_chnIdx), m_channelLabel);
	m_titleLbl = new QLabel(title);
	Style::setStyle(m_titleLbl, style::properties::label::menuBig);

	// Setup frequency widget

	setupFrequency();

	// Setup IIO Widgets
	QHBoxLayout *channelNcoLay = new QHBoxLayout();
	QHBoxLayout *mainNcoLay = new QHBoxLayout();
	QHBoxLayout *headerLay = new QHBoxLayout();
	QHBoxLayout *cfirLay = new QHBoxLayout();
	cfirLay->setMargin(0);

	m_iioWidgets.push_back(IIOWidgetBuilder(this)
				       .channel(m_channel)
				       .attribute("cfir_en")
				       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				       .title("CFIR Enable")
				       .parent(this)
				       .buildSingle());
	m_iioWidgets.last()->showProgressBar(false);
	cfirLay->addWidget(m_iioWidgets.last());

	m_iioWidgets.push_back(IIOWidgetBuilder(this)
				       .channel(m_channel)
				       .attribute("cfir_profile_sel")
				       .uiStrategy(IIOWidgetBuilder::ComboUi)
				       .optionsValues("1 2")
				       .title("CFIR Profile")
				       .parent(this)
				       .buildSingle());
	cfirLay->addWidget(m_iioWidgets.last());

	m_iioWidgets.push_back(IIOWidgetBuilder(this)
				       .channel(m_channel)
				       .attribute("en")
				       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				       .compactMode(true)
				       .parent(this)
				       .buildSingle());
	m_iioWidgets.last()->showProgressBar(false);
	connect(dynamic_cast<ChannelAttrDataStrategy *>(m_iioWidgets.last()->getDataStrategy()),
		&ChannelAttrDataStrategy::emitStatus, this,
		[this](QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp) {
			if(retCode < 0) {
				return;
			}
			bool ok;
			int en = newData.toInt(&ok);
			if(!ok) {
				return;
			}
			Q_EMIT requestEnableUi(en);
		});

	headerLay->addWidget(m_titleLbl);
	headerLay->addWidget(m_iioWidgets.last());
	m_chnSection->contentLayout()->addLayout(headerLay);
	m_chnSection->contentLayout()->addLayout(cfirLay);
	m_chnSection->contentLayout()->addWidget(m_frequencyWidget);
	m_chnSection->contentLayout()->addLayout(channelNcoLay);

	setupChannelNco(channelNcoLay);
	setupMainNco(mainNcoLay);

	if(m_input) {
		m_chnSection->contentLayout()->addLayout(mainNcoLay);
		setupInChannelsAttrs(m_chnSection->contentLayout());
	} else {
		QHBoxLayout *channelToneLay = new QHBoxLayout();
		setupChannelTone(channelToneLay);

		// Main attributes section
		QWidget *mainNcoWidget = new QWidget(this);
		Style::setStyle(mainNcoWidget, style::properties::widget::border_interactive);
		mainNcoWidget->setLayout(mainNcoLay);
		setupMainTone(mainNcoLay);

		m_chnSection->contentLayout()->addLayout(channelToneLay);
		m_chnSection->contentLayout()->addWidget(mainNcoWidget);
	}
	this->layout()->addWidget(m_chnSection);
}

void Ad9084Channel::addGroup(SharedAttrCategory attr, IIOWidgetGroup *grp)
{
	if(!m_iioWidgetGroupList.contains(attr)) {
		m_iioWidgetGroupList.insert(attr, grp);
	}
}

void Ad9084Channel::readChannel()
{
	for(int i = 0; i < m_iioWidgets.size(); ++i) {
		m_iioWidgets.at(i)->getDataStrategy()->readAsync();
	}
}

QString Ad9084Channel::phaseUItoDS(QString data)
{
	bool ok = true;
	QString tmp_data = data;
	tmp_data = tmp_data.toLower();
	tmp_data = tmp_data.simplified();
	tmp_data.replace(" ", "");
	double d_data = tmp_data.toDouble(&ok);
	if(!ok) {
		qWarning() << QString("Invalid phase value in UI %1").arg(data);
		return "";
	}
	d_data = d_data * 1000;
	return QString::number(d_data);
}

QString Ad9084Channel::phaseDStoUI(QString data)
{
	bool ok = true;
	double d_data = data.toInt(&ok);
	if(!ok) {
		qWarning() << QString("Invalid phase value on device %1").arg(data);
		return data;
	}
	d_data = d_data / 1000;
	QString s_data = QString::number(d_data);
	return s_data;
}

QString Ad9084Channel::frequencyUItoDS(QString data)
{
	bool ok;
	QString tmp_data = data;
	tmp_data = tmp_data.toLower();
	tmp_data = tmp_data.simplified();
	double d_data = tmp_data.toDouble(&ok);
	if(!ok) {
		qWarning() << QString("Invalid frequency value in UI %1").arg(data);
		return "";
	}
	d_data = d_data * 1e6;
	return QString::number(d_data, 'g', 10);
}

QString Ad9084Channel::frequencyDStoUI(QString data)
{
	bool ok;
	double d_data = data.toDouble(&ok);
	if(!ok) {
		qWarning() << QString("Invalid frequency value on device %1").arg(data);
		return data;
	}
	d_data = d_data / 1e6;
	QString s_data = QString::number(d_data, 'g', 10);
	return s_data;
}
