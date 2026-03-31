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

#include "advanced/agcsetupwidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QLoggingCategory>
#include <iio-widgets/iiowidget.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_AGC_SETUP, "AD9371_AGC_SETUP")

using namespace scopy;
using namespace scopy::ad9371;

AgcSetupWidget::AgcSetupWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_AGC_SETUP) << "No device provided to AGC Setup";
		return;
	}

	setupUi();
}

AgcSetupWidget::~AgcSetupWidget() {}

void AgcSetupWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	contentLayout->addWidget(createPeakAgcSection("Analog Peak Detector - RX", "rx-peak-agc", contentWidget));
	contentLayout->addWidget(
		createPeakAgcSection("Analog Peak Detector - Observation", "obs-peak-agc", contentWidget));
	contentLayout->addWidget(createPowerAgcSection("Power Measurement - RX", "rx-pwr-agc", contentWidget));
	contentLayout->addWidget(
		createPowerAgcSection("Power Measurement - Observation", "obs-pwr-agc", contentWidget));
	contentLayout->addWidget(createRxAgcConfigSection(contentWidget));
	contentLayout->addWidget(createObsAgcConfigSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_AGC_SETUP) << "AGC Setup widget created with 81 debug attributes in 6 collapsible sections";
}

void AgcSetupWidget::addDebugRange(QVBoxLayout *layout, const QString &attr, const QString &range, const QString &title)
{
	IIOWidget *w = Ad9371WidgetFactory::createDebugRangeWidget(m_device, attr, range, title);
	if(w) {
		if(m_widgetGroup)
			m_widgetGroup->add(w);
		layout->addWidget(w);
		m_widgets.append(w);
		connect(this, &AgcSetupWidget::readRequested, w, &IIOWidget::readAsync);
	}
}

void AgcSetupWidget::addDebugCheckbox(QVBoxLayout *layout, const QString &attr, const QString &title)
{
	IIOWidget *w = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, attr, title);
	if(w) {
		if(m_widgetGroup)
			m_widgetGroup->add(w);
		layout->addWidget(w);
		w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		m_widgets.append(w);
		connect(this, &AgcSetupWidget::readRequested, w, &IIOWidget::readAsync);
	}
}

void AgcSetupWidget::addDebugCombo(QVBoxLayout *layout, const QString &attr, const QMap<QString, QString> &options,
				   const QString &title)
{
	IIOWidget *w = Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, attr, options, title);
	if(w) {
		if(m_widgetGroup)
			m_widgetGroup->add(w);
		layout->addWidget(w);
		m_widgets.append(w);
		connect(this, &AgcSetupWidget::readRequested, w, &IIOWidget::readAsync);
	}
}

QWidget *AgcSetupWidget::createPeakAgcSection(const QString &title, const QString &prefix, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QString p = "adi," + prefix;

	// 15 Range Widgets - thresholds and counts
	addDebugRange(layout, p + "-apd-high-thresh", "[0 1 63]", "APD HIGH THRESH");
	addDebugRange(layout, p + "-apd-low-thresh", "[0 1 63]", "APD LOW THRESH");
	addDebugRange(layout, p + "-hb2-high-thresh", "[0 1 255]", "HB2 HIGH THRESH");
	addDebugRange(layout, p + "-hb2-low-thresh", "[0 1 255]", "HB2 LOW THRESH");
	addDebugRange(layout, p + "-hb2-very-low-thresh", "[0 1 255]", "HB2 VERY LOW THRESH");
	addDebugRange(layout, p + "-apd-high-thresh-exceeded-cnt", "[0 1 255]", "APD HIGH THRESH EXCEEDED CNT");
	addDebugRange(layout, p + "-apd-low-thresh-exceeded-cnt", "[0 1 255]", "APD LOW THRESH EXCEEDED CNT");
	addDebugRange(layout, p + "-hb2-high-thresh-exceeded-cnt", "[0 1 255]", "HB2 HIGH THRESH EXCEEDED CNT");
	addDebugRange(layout, p + "-hb2-low-thresh-exceeded-cnt", "[0 1 255]", "HB2 LOW THRESH EXCEEDED CNT");
	addDebugRange(layout, p + "-hb2-very-low-thresh-exceeded-cnt", "[0 1 255]", "HB2 VERY LOW THRESH EXCEEDED CNT");

	// 5 Range Widgets - gain steps
	addDebugRange(layout, p + "-apd-high-gain-step-attack", "[0 1 31]", "APD HIGH GAIN STEP ATTACK");
	addDebugRange(layout, p + "-apd-low-gain-step-recovery", "[0 1 31]", "APD LOW GAIN STEP RECOVERY");
	addDebugRange(layout, p + "-hb2-high-gain-step-attack", "[0 1 31]", "HB2 HIGH GAIN STEP ATTACK");
	addDebugRange(layout, p + "-hb2-low-gain-step-recovery", "[0 1 31]", "HB2 LOW GAIN STEP RECOVERY");
	addDebugRange(layout, p + "-hb2-very-low-gain-step-recovery", "[0 1 31]", "HB2 VERY LOW GAIN STEP RECOVERY");

	// 3 Checkboxes
	addDebugCheckbox(layout, p + "-apd-fast-attack", "APD FAST ATTACK");
	addDebugCheckbox(layout, p + "-hb2-fast-attack", "HB2 FAST ATTACK");
	addDebugCheckbox(layout, p + "-hb2-overload-detect-enable", "HB2 OVERLOAD DETECT ENABLE");

	// 2 More Range Widgets
	addDebugRange(layout, p + "-hb2-overload-duration-cnt", "[0 1 6]", "HB2 OVERLOAD DURATION CNT");
	addDebugRange(layout, p + "-hb2-overload-thresh-cnt", "[0 1 15]", "HB2 OVERLOAD THRESH CNT");

	return section;
}

QWidget *AgcSetupWidget::createPowerAgcSection(const QString &title, const QString &prefix, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QString p = "adi," + prefix;

	// 4 Threshold Range Widgets
	addDebugRange(layout, p + "-pmd-upper-high-thresh", "[0 1 15]", "PMD UPPER HIGH THRESH");
	addDebugRange(layout, p + "-pmd-upper-low-thresh", "[0 1 127]", "PMD UPPER LOW THRESH");
	addDebugRange(layout, p + "-pmd-lower-high-thresh", "[0 1 127]", "PMD LOWER HIGH THRESH");
	addDebugRange(layout, p + "-pmd-lower-low-thresh", "[0 1 15]", "PMD LOWER LOW THRESH");

	// 4 Gain Step Range Widgets
	addDebugRange(layout, p + "-pmd-upper-high-gain-step-attack", "[0 1 31]", "PMD UPPER HIGH GAIN STEP ATTACK");
	addDebugRange(layout, p + "-pmd-upper-low-gain-step-attack", "[0 1 31]", "PMD UPPER LOW GAIN STEP ATTACK");
	addDebugRange(layout, p + "-pmd-lower-high-gain-step-recovery", "[0 1 31]",
		      "PMD LOWER HIGH GAIN STEP RECOVERY");
	addDebugRange(layout, p + "-pmd-lower-low-gain-step-recovery", "[0 1 31]", "PMD LOWER LOW GAIN STEP RECOVERY");

	// Measurement Duration
	addDebugRange(layout, p + "-pmd-meas-duration", "[0 1 15]", "PMD MEAS DURATION");

	// Measurement Config - CustomCombo
	QMap<QString, QString> measConfigOptions;
	measConfigOptions.insert("0", "DISABLED");
	measConfigOptions.insert("1", "HB2");
	measConfigOptions.insert("2", "RFIR");
	measConfigOptions.insert("3", "PMD");
	addDebugCombo(layout, p + "-pmd-meas-config", measConfigOptions, "PMD MEAS CONFIG");

	return section;
}

QWidget *AgcSetupWidget::createRxAgcConfigSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"AGC Config - RX", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// RX1/RX2 Gain Index Range Widgets
	addDebugRange(layout, "adi,rx-agc-conf-agc-rx1-max-gain-index", "[0 1 255]", "AGC RX MAX GAIN INDEX");
	addDebugRange(layout, "adi,rx-agc-conf-agc-rx1-min-gain-index", "[0 1 255]", "AGC RX MIN GAIN INDEX");
	addDebugRange(layout, "adi,rx-agc-conf-agc-rx2-max-gain-index", "[0 1 255]", "AGC RX2 MAX GAIN INDEX");
	addDebugRange(layout, "adi,rx-agc-conf-agc-rx2-min-gain-index", "[0 1 255]", "AGC RX2 MIN GAIN INDEX");

	// Checkboxes
	addDebugCheckbox(layout, "adi,rx-agc-conf-agc-peak-threshold-mode", "AGC PEAK THRESHOLD MODE");
	addDebugCheckbox(layout, "adi,rx-agc-conf-agc-low-ths-prevent-gain-increase",
			 "AGC LOW THS PREVENT GAIN INCREASE");

	// Range Widgets
	addDebugRange(layout, "adi,rx-agc-conf-agc-gain-update-counter", "[1 1 4194303]", "AGC GAIN UPDATE COUNTER");
	addDebugRange(layout, "adi,rx-agc-conf-agc-slow-loop-settling-delay", "[0 1 127]",
		      "AGC SLOW LOOP SETTLING DELAY");
	addDebugRange(layout, "adi,rx-agc-conf-agc-peak-wait-time", "[2 1 31]", "AGC PEAK WAIT TIME");

	// More Checkboxes
	addDebugCheckbox(layout, "adi,rx-agc-conf-agc-reset-on-rx-enable", "AGC RESET ON RX ENABLE");
	addDebugCheckbox(layout, "adi,rx-agc-conf-agc-enable-sync-pulse-for-gain-counter",
			 "AGC ENABLE SYNC PULSE FOR GAIN COUNTER");

	return section;
}

QWidget *AgcSetupWidget::createObsAgcConfigSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("AGC Config - Observation", MenuCollapseSection::MHCW_ARROW,
					      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// OBS RX Gain Index Range Widgets
	addDebugRange(layout, "adi,obs-agc-conf-agc-obs-rx-max-gain-index", "[0 1 255]", "AGC OBS RX MAX GAIN INDEX");
	addDebugRange(layout, "adi,obs-agc-conf-agc-obs-rx-min-gain-index", "[0 1 255]", "AGC OBS RX MIN GAIN INDEX");

	// Checkboxes
	addDebugCheckbox(layout, "adi,obs-agc-conf-agc-obs-rx-select", "AGC OBS RX SELECT");
	addDebugCheckbox(layout, "adi,obs-agc-conf-agc-peak-threshold-mode", "AGC PEAK THRESHOLD MODE");
	addDebugCheckbox(layout, "adi,obs-agc-conf-agc-low-ths-prevent-gain-increase",
			 "AGC LOW THS PREVENT GAIN INCREASE");

	// Range Widgets
	addDebugRange(layout, "adi,obs-agc-conf-agc-gain-update-counter", "[1 1 4194303]", "AGC GAIN UPDATE COUNTER");
	addDebugRange(layout, "adi,obs-agc-conf-agc-slow-loop-settling-delay", "[0 1 127]",
		      "AGC SLOW LOOP SETTLING DELAY");
	addDebugRange(layout, "adi,obs-agc-conf-agc-peak-wait-time", "[2 1 31]", "AGC PEAK WAIT TIME");

	// More Checkboxes
	addDebugCheckbox(layout, "adi,obs-agc-conf-agc-reset-on-rx-enable", "AGC RESET ON RX ENABLE");
	addDebugCheckbox(layout, "adi,obs-agc-conf-agc-enable-sync-pulse-for-gain-counter",
			 "AGC ENABLE SYNC PULSE FOR GAIN COUNTER");

	return section;
}
