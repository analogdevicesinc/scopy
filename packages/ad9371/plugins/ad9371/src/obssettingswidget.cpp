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

#include "obssettingswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

ObsSettingsWidget::ObsSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *configTitle = new QLabel("OBS Config", widget);
	Style::setStyle(configTitle, style::properties::label::menuBig);
	layout->addWidget(configTitle);

	addRange("adi,obs-settings-sniffer-pll-lo-frequency-hz", "Sniffer PLL LO Frequency (Hz)");
	addRange("adi,obs-rx-lo-source", "OBS RX LO Source");
	addRange("adi,obs-rx-real-if-data", "OBS RX Real IF Data");

	QLabel *obsProfileTitle = new QLabel("Observation Profile", widget);
	Style::setStyle(obsProfileTitle, style::properties::label::menuBig);
	layout->addWidget(obsProfileTitle);

	addRange("adi,obs-profile-adc-div", "ADC Div");
	addRange("adi,obs-profile-rx-fir", "RX FIR");
	addRange("adi,obs-profile-rx-dec5-decimation", "RX Dec5 Decimation");
	addCheck("adi,obs-profile-en-high-rej-dec5", "EN High Rej Dec5");
	addRange("adi,obs-profile-rhb1-decimation", "RHB1 Decimation");
	addRange("adi,obs-profile-iq-rate-khz", "IQ Rate (kHz)");
	addRange("adi,obs-profile-rf-bandwidth-hz", "RF Bandwidth (Hz)");
	addRange("adi,obs-profile-rx-bbf3db-corner-khz", "RX BBF 3dB Corner (kHz)");

	QLabel *snifferTitle = new QLabel("Sniffer Profile", widget);
	Style::setStyle(snifferTitle, style::properties::label::menuBig);
	layout->addWidget(snifferTitle);

	addRange("adi,sniffer-profile-adc-div", "ADC Div");
	addRange("adi,sniffer-profile-rx-fir", "RX FIR");
	addRange("adi,sniffer-profile-rx-dec5-decimation", "RX Dec5 Decimation");
	addRange("adi,sniffer-profile-rhb1-decimation", "RHB1 Decimation");
	addRange("adi,sniffer-profile-iq-rate-khz", "IQ Rate (kHz)");
	addRange("adi,sniffer-profile-rf-bandwidth-hz", "RF Bandwidth (Hz)");
	addRange("adi,sniffer-profile-rx-bbf3db-corner-khz", "RX BBF 3dB Corner (kHz)");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &ObsSettingsWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

ObsSettingsWidget::~ObsSettingsWidget() {}
