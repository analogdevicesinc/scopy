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
 *
 */

#include "bistwidget.h"

#include <iiowidgetutils.h>
#include <style.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

BistWidget::BistWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("BIST", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QHBoxLayout *hLayout1 = new QHBoxLayout();

	// bist_tone
	m_bistTone = new MenuComboWidget("Bist TONE", widget);
	m_bistTone->combo()->addItem("Disable", 0);
	m_bistTone->combo()->addItem("Injection Point TX", 1);
	m_bistTone->combo()->addItem("Injection Point RX", 2);
	hLayout1->addWidget(m_bistTone);

	// bist_prbs
	QMap<QString, QString> *bistOptions = new QMap<QString, QString>();
	bistOptions->insert("0", "Disable");
	bistOptions->insert("1", "Injection_Point_TX");
	bistOptions->insert("2", "Injection_Point_RX");

	auto bistValues = bistOptions->values();
	QString bistOptionasData = "";
	for(int i = 0; i < bistValues.size(); i++) {
		bistOptionasData += " " + bistValues.at(i);
	}

	IIOWidget *bistPrbs = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("bist_prbs")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .optionsValues(bistOptionasData)
				      .title("Bist PRBS")
				      .buildSingle();
	hLayout1->addWidget(bistPrbs);

	bistPrbs->setUItoDataConversion([this, bistOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, bistOptions);
	});
	bistPrbs->setDataToUIConversion([this, bistOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, bistOptions);
	});

	// loopback
	QMap<QString, QString> *loopbackOptions = new QMap<QString, QString>();
	loopbackOptions->insert("0", "Disable");
	loopbackOptions->insert("1", "Digital_TX→Digital_RX");
	loopbackOptions->insert("2", "RF_RX→RF_TX");

	auto loopbackValues = loopbackOptions->values();
	QString loopbackOptionasData = "";
	for(int i = 0; i < loopbackValues.size(); i++) {
		loopbackOptionasData += " " + loopbackValues.at(i);
	}
	IIOWidget *loopback = IIOWidgetBuilder(widget)
				      .device(m_device)
				      .attribute("loopback")
				      .uiStrategy(IIOWidgetBuilder::ComboUi)
				      .optionsValues(loopbackOptionasData)
				      .title("Loopback")
				      .buildSingle();
	hLayout1->addWidget(loopback);

	loopback->setUItoDataConversion([this, loopbackOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, loopbackOptions);
	});
	loopback->setDataToUIConversion([this, loopbackOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, loopbackOptions);
	});

	layout->addLayout(hLayout1);

	// tone_level
	m_toneLevel = new MenuComboWidget("Level", widget);
	m_toneLevel->combo()->addItem("0dB", 0);
	m_toneLevel->combo()->addItem("-6dB", 6);
	m_toneLevel->combo()->addItem("-12dB", 12);
	m_toneLevel->combo()->addItem("-18dB", 18);
	layout->addWidget(m_toneLevel);

	// bist_tone_frequency
	m_toneFrequency = new MenuComboWidget("Frequency", widget);
	m_toneFrequency->combo()->addItem("Fsamp / 32");
	m_toneFrequency->combo()->addItem("Fsamp / 16");
	m_toneFrequency->combo()->addItem("Fsamp * 3 / 32");
	m_toneFrequency->combo()->addItem("Fsamp / 8");
	layout->addWidget(m_toneFrequency);

	layout->addWidget(new QLabel("Channel Mask", widget));

	QHBoxLayout *hLayout2 = new QHBoxLayout();
	// C2-Q
	m_c2q = new MenuOnOffSwitch("C2-Q", widget, false);
	m_c2q->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	hLayout2->addWidget(m_c2q);

	//  C2-I
	m_c2i = new MenuOnOffSwitch("C2-I", widget, false);
	m_c2i->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	hLayout2->addWidget(m_c2i);

	//  C1-Q
	m_c1q = new MenuOnOffSwitch("C1-Q", widget, false);
	m_c1q->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	hLayout2->addWidget(m_c1q);

	//  C1-I
	m_c1i = new MenuOnOffSwitch("C1-I", widget, false);
	m_c1i->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	hLayout2->addWidget(m_c1i);

	hLayout2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	connect(m_bistTone->combo(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		[this](int index) { updateBistTone(); });
	connect(m_toneFrequency->combo(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		[this](int index) { updateBistTone(); });
	connect(m_toneLevel->combo(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		[this](int index) { updateBistTone(); });
	connect(m_c2q->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c2i->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c1q->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c1i->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);

	layout->addLayout(hLayout2);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &BistWidget::readRequested, this, [=, this]() {
		bistPrbs->readAsync();
		loopback->readAsync();
	});
}

scopy::pluto::BistWidget::~BistWidget() { ConnectionProvider::close(m_uri); }

void BistWidget::updateBistTone()
{

	int c2qVal = m_c2q->onOffswitch()->isChecked() ? 1 : 0;
	int c2iVal = m_c2i->onOffswitch()->isChecked() ? 1 : 0;
	int c1qVal = m_c1q->onOffswitch()->isChecked() ? 1 : 0;
	int c1iVal = m_c1i->onOffswitch()->isChecked() ? 1 : 0;

	unsigned int bitmask = (c2qVal << 3) | (c2iVal << 2) | (c1qVal << 1) | c1iVal;

	auto tone = QString::number(m_bistTone->combo()->currentData().toInt());
	auto freq = m_toneFrequency->combo()->currentText();
	auto level = QString::number(m_toneLevel->combo()->currentData().toInt() * 6 * (-1));

	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	iio_device *m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	// Format the configuration string
	QString config = QString("%1 %2 %3 %4").arg(tone).arg(freq).arg(level).arg(bitmask);

	int ret = iio_device_debug_attr_write(m_device, "bist_tone", config.toStdString().c_str());
	if(ret < 0) {
		qWarning() << "BIST Tone configuration failed" << QString::fromLocal8Bit(strerror(ret * (-1)));
	}
	// if (m_dev2)
	// 	iio_device_debug_attr_write(m_dev2, "bist_tone", temp);

	Q_EMIT bistToneUpdated();
}
