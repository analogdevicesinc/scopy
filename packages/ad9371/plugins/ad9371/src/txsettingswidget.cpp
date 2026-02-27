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

#include "txsettingswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

TxSettingsWidget::TxSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: m_device(device)
	, m_group(group)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	m_layout->addWidget(widget);

	QLabel *configTitle = new QLabel("TX Config", widget);
	Style::setStyle(configTitle, style::properties::label::menuBig);
	layout->addWidget(configTitle);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};
	auto addCheck = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugCheck(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	addRange("adi,tx-channel-enable", "TX Channel Enable");
	addRange("adi,tx-pll-lo-frequency-hz", "TX PLL LO Frequency (Hz)");
	addCheck("adi,tx-pll-use-external-lo", "TX PLL Use External LO");
	addRange("adi,tx-atten-step-size", "TX Atten Step Size");
	addRange("adi,tx1-atten-mdb", "TX1 Atten (mdB)");
	addRange("adi,tx2-atten-mdb", "TX2 Atten (mdB)");

	QLabel *profileTitle = new QLabel("TX Profile", widget);
	Style::setStyle(profileTitle, style::properties::label::menuBig);
	layout->addWidget(profileTitle);

	addRange("adi,tx-profile-dac-div", "DAC Div");
	addRange("adi,tx-profile-tx-fir", "TX FIR");
	addRange("adi,tx-profile-thb1-interpolation", "THB1 Interpolation");
	addRange("adi,tx-profile-thb2-interpolation", "THB2 Interpolation");
	addRange("adi,tx-profile-tx-input-hb-interpolation", "TX Input HB Interpolation");
	addRange("adi,tx-profile-iq-rate-khz", "IQ Rate (kHz)");
	addRange("adi,tx-profile-primary-sig-bandwidth-hz", "Primary Sig BW (Hz)");
	addRange("adi,tx-profile-rf-bandwidth-hz", "RF Bandwidth (Hz)");
	addRange("adi,tx-profile-tx-dac3db-corner-khz", "TX DAC 3dB Corner (kHz)");
	addRange("adi,tx-profile-tx-bbf3db-corner-khz", "TX BBF 3dB Corner (kHz)");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &TxSettingsWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

TxSettingsWidget::~TxSettingsWidget() {}
