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

#include "rxsettingswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

RxSettingsWidget::RxSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *configTitle = new QLabel("RX Config", widget);
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

	addRange("adi,rx-channel-enable", "RX Channel Enable");
	addRange("adi,rx-pll-lo-frequency-hz", "RX PLL LO Frequency (Hz)");
	addCheck("adi,rx-pll-use-external-lo", "RX PLL Use External LO");
	addCheck("adi,rx-use-real-if-data", "Use Real IF Data");

	QLabel *profileTitle = new QLabel("RX Profile", widget);
	Style::setStyle(profileTitle, style::properties::label::menuBig);
	layout->addWidget(profileTitle);

	addRange("adi,rx-profile-adc-div", "ADC Div");
	addRange("adi,rx-profile-rx-fir", "RX FIR");
	addRange("adi,rx-profile-rx-dec5-decimation", "RX Dec5 Decimation");
	addCheck("adi,rx-profile-en-high-rej-dec5", "EN High Rej Dec5");
	addRange("adi,rx-profile-rhb1-decimation", "RHB1 Decimation");
	addRange("adi,rx-profile-iq-rate-khz", "IQ Rate (kHz)");
	addRange("adi,rx-profile-rf-bandwidth-hz", "RF Bandwidth (Hz)");
	addRange("adi,rx-profile-rx-bbf3db-corner-khz", "RX BBF 3dB Corner (kHz)");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &RxSettingsWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

RxSettingsWidget::~RxSettingsWidget() {}
