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

#include "agcsetupwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

AgcSetupWidget::AgcSetupWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	auto addAgcRow = [&](QGridLayout *grid, int row, const QString &label, const char *rxAttr,
			     const char *obsAttr) {
		grid->addWidget(new QLabel(label, widget), row, 0);
		IIOWidget *rxW = makeDebugRange(widget, m_device, rxAttr, "", m_group);
		grid->addWidget(rxW, row, 1);
		allWidgets.append(rxW);
		IIOWidget *obsW = makeDebugRange(widget, m_device, obsAttr, "", m_group);
		grid->addWidget(obsW, row, 2);
		allWidgets.append(obsW);
	};

	// AGC Config section
	QLabel *agcConfigTitle = new QLabel("AGC Config", widget);
	Style::setStyle(agcConfigTitle, style::properties::label::menuBig);
	layout->addWidget(agcConfigTitle);

	QGridLayout *agcGrid = new QGridLayout();
	agcGrid->addWidget(new QLabel("", widget), 0, 0);
	agcGrid->addWidget(new QLabel("RX", widget), 0, 1);
	agcGrid->addWidget(new QLabel("OBS", widget), 0, 2);
	int row = 1;

	addAgcRow(agcGrid, row++, "RX1 Max Gain Index", "adi,rx-agc-conf-agc-rx1-max-gain-index",
		  "adi,obs-agc-conf-agc-rx1-max-gain-index");
	addAgcRow(agcGrid, row++, "RX1 Min Gain Index", "adi,rx-agc-conf-agc-rx1-min-gain-index",
		  "adi,obs-agc-conf-agc-rx1-min-gain-index");
	addAgcRow(agcGrid, row++, "RX2 Max Gain Index", "adi,rx-agc-conf-agc-rx2-max-gain-index",
		  "adi,obs-agc-conf-agc-rx2-max-gain-index");
	addAgcRow(agcGrid, row++, "RX2 Min Gain Index", "adi,rx-agc-conf-agc-rx2-min-gain-index",
		  "adi,obs-agc-conf-agc-rx2-min-gain-index");
	addAgcRow(agcGrid, row++, "OBS RX Max Gain Index", "adi,rx-agc-conf-agc-obs-rx-max-gain-index",
		  "adi,obs-agc-conf-agc-obs-rx-max-gain-index");
	addAgcRow(agcGrid, row++, "OBS RX Min Gain Index", "adi,rx-agc-conf-agc-obs-rx-min-gain-index",
		  "adi,obs-agc-conf-agc-obs-rx-min-gain-index");
	addAgcRow(agcGrid, row++, "Gain Update Counter", "adi,rx-agc-conf-agc-gain-update-counter",
		  "adi,obs-agc-conf-agc-gain-update-counter");
	addAgcRow(agcGrid, row++, "Slow Loop Settling Delay", "adi,rx-agc-conf-agc-slow-loop-settling-delay",
		  "adi,obs-agc-conf-agc-slow-loop-settling-delay");
	addAgcRow(agcGrid, row++, "Peak Wait Time", "adi,rx-agc-conf-agc-peak-wait-time",
		  "adi,obs-agc-conf-agc-peak-wait-time");
	layout->addLayout(agcGrid);

	// Analog Peak Detector
	QLabel *peakTitle = new QLabel("Analog Peak Detector", widget);
	Style::setStyle(peakTitle, style::properties::label::menuBig);
	layout->addWidget(peakTitle);

	QGridLayout *peakGrid = new QGridLayout();
	peakGrid->addWidget(new QLabel("", widget), 0, 0);
	peakGrid->addWidget(new QLabel("RX", widget), 0, 1);
	peakGrid->addWidget(new QLabel("OBS", widget), 0, 2);
	row = 1;

	addAgcRow(peakGrid, row++, "APD High Thresh", "adi,rx-peak-agc-apd-high-thresh",
		  "adi,obs-peak-agc-apd-high-thresh");
	addAgcRow(peakGrid, row++, "APD Low Thresh", "adi,rx-peak-agc-apd-low-thresh",
		  "adi,obs-peak-agc-apd-low-thresh");
	addAgcRow(peakGrid, row++, "APD High Gain Step Attack", "adi,rx-peak-agc-apd-high-gain-step-attack",
		  "adi,obs-peak-agc-apd-high-gain-step-attack");
	addAgcRow(peakGrid, row++, "APD Low Gain Step Recovery", "adi,rx-peak-agc-apd-low-gain-step-recovery",
		  "adi,obs-peak-agc-apd-low-gain-step-recovery");
	addAgcRow(peakGrid, row++, "APD High Exceeded Cnt", "adi,rx-peak-agc-apd-high-thresh-exceeded-cnt",
		  "adi,obs-peak-agc-apd-high-thresh-exceeded-cnt");
	addAgcRow(peakGrid, row++, "APD Low Exceeded Cnt", "adi,rx-peak-agc-apd-low-thresh-exceeded-cnt",
		  "adi,obs-peak-agc-apd-low-thresh-exceeded-cnt");
	addAgcRow(peakGrid, row++, "HB2 High Thresh", "adi,rx-peak-agc-hb2-high-thresh",
		  "adi,obs-peak-agc-hb2-high-thresh");
	addAgcRow(peakGrid, row++, "HB2 Low Thresh", "adi,rx-peak-agc-hb2-low-thresh",
		  "adi,obs-peak-agc-hb2-low-thresh");
	addAgcRow(peakGrid, row++, "HB2 Very Low Thresh", "adi,rx-peak-agc-hb2-very-low-thresh",
		  "adi,obs-peak-agc-hb2-very-low-thresh");
	addAgcRow(peakGrid, row++, "HB2 High Gain Step Attack", "adi,rx-peak-agc-hb2-high-gain-step-attack",
		  "adi,obs-peak-agc-hb2-high-gain-step-attack");
	addAgcRow(peakGrid, row++, "HB2 Low Gain Step Recovery", "adi,rx-peak-agc-hb2-low-gain-step-recovery",
		  "adi,obs-peak-agc-hb2-low-gain-step-recovery");
	addAgcRow(peakGrid, row++, "HB2 Very Low Gain Step Recovery", "adi,rx-peak-agc-hb2-very-low-gain-step-recovery",
		  "adi,obs-peak-agc-hb2-very-low-gain-step-recovery");
	addAgcRow(peakGrid, row++, "HB2 High Exceeded Cnt", "adi,rx-peak-agc-hb2-high-thresh-exceeded-cnt",
		  "adi,obs-peak-agc-hb2-high-thresh-exceeded-cnt");
	addAgcRow(peakGrid, row++, "HB2 Low Exceeded Cnt", "adi,rx-peak-agc-hb2-low-thresh-exceeded-cnt",
		  "adi,obs-peak-agc-hb2-low-thresh-exceeded-cnt");
	addAgcRow(peakGrid, row++, "HB2 Very Low Exceeded Cnt", "adi,rx-peak-agc-hb2-very-low-thresh-exceeded-cnt",
		  "adi,obs-peak-agc-hb2-very-low-thresh-exceeded-cnt");
	addAgcRow(peakGrid, row++, "HB2 Overload Duration Cnt", "adi,rx-peak-agc-hb2-overload-duration-cnt",
		  "adi,obs-peak-agc-hb2-overload-duration-cnt");
	addAgcRow(peakGrid, row++, "HB2 Overload Thresh Cnt", "adi,rx-peak-agc-hb2-overload-thresh-cnt",
		  "adi,obs-peak-agc-hb2-overload-thresh-cnt");
	layout->addLayout(peakGrid);

	// Power Measurement Detector
	QLabel *pwrTitle = new QLabel("Power Measurement Detector", widget);
	Style::setStyle(pwrTitle, style::properties::label::menuBig);
	layout->addWidget(pwrTitle);

	QGridLayout *pwrGrid = new QGridLayout();
	pwrGrid->addWidget(new QLabel("", widget), 0, 0);
	pwrGrid->addWidget(new QLabel("RX", widget), 0, 1);
	pwrGrid->addWidget(new QLabel("OBS", widget), 0, 2);
	row = 1;

	addAgcRow(pwrGrid, row++, "PMD Upper High Thresh", "adi,rx-pwr-agc-pmd-upper-high-thresh",
		  "adi,obs-pwr-agc-pmd-upper-high-thresh");
	addAgcRow(pwrGrid, row++, "PMD Upper Low Thresh", "adi,rx-pwr-agc-pmd-upper-low-thresh",
		  "adi,obs-pwr-agc-pmd-upper-low-thresh");
	addAgcRow(pwrGrid, row++, "PMD Lower High Thresh", "adi,rx-pwr-agc-pmd-lower-high-thresh",
		  "adi,obs-pwr-agc-pmd-lower-high-thresh");
	addAgcRow(pwrGrid, row++, "PMD Lower Low Thresh", "adi,rx-pwr-agc-pmd-lower-low-thresh",
		  "adi,obs-pwr-agc-pmd-lower-low-thresh");
	addAgcRow(pwrGrid, row++, "PMD Upper High Gain Step Attack", "adi,rx-pwr-agc-pmd-upper-high-gain-step-attack",
		  "adi,obs-pwr-agc-pmd-upper-high-gain-step-attack");
	addAgcRow(pwrGrid, row++, "PMD Upper Low Gain Step Attack", "adi,rx-pwr-agc-pmd-upper-low-gain-step-attack",
		  "adi,obs-pwr-agc-pmd-upper-low-gain-step-attack");
	addAgcRow(pwrGrid, row++, "PMD Lower High Gain Step Recovery",
		  "adi,rx-pwr-agc-pmd-lower-high-gain-step-recovery",
		  "adi,obs-pwr-agc-pmd-lower-high-gain-step-recovery");
	addAgcRow(pwrGrid, row++, "PMD Lower Low Gain Step Recovery", "adi,rx-pwr-agc-pmd-lower-low-gain-step-recovery",
		  "adi,obs-pwr-agc-pmd-lower-low-gain-step-recovery");
	addAgcRow(pwrGrid, row++, "PMD Meas Duration", "adi,rx-pwr-agc-pmd-meas-duration",
		  "adi,obs-pwr-agc-pmd-meas-duration");
	layout->addLayout(pwrGrid);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &AgcSetupWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

AgcSetupWidget::~AgcSetupWidget() {}
