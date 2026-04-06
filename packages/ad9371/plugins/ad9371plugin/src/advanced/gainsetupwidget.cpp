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

#include "advanced/gainsetupwidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_GAIN_SETUP, "AD9371_GAIN_SETUP")

using namespace scopy;
using namespace scopy::ad9371;

GainSetupWidget::GainSetupWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_GAIN_SETUP) << "No device provided to GAIN Setup";
		return;
	}

	setupUi();
}

GainSetupWidget::~GainSetupWidget() {}

void GainSetupWidget::setupUi()
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

	// Add RX Gain section
	contentLayout->addWidget(createRxGainSection(contentWidget));

	// Add ORX Gain section
	contentLayout->addWidget(createOrxGainSection(contentWidget));

	// Add Sniffer Gain section
	contentLayout->addWidget(createSnifferGainSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_GAIN_SETUP) << "GAIN Setup widget created with 16 attributes in 3 collapsible sections";
}

QWidget *GainSetupWidget::createRxGainSection(QWidget *parent)
{
	MenuSectionCollapseWidget *rxGainSection = new MenuSectionCollapseWidget(
		"RX", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	rxGainSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// 1. RX Gain Mode - Custom Combo {0:"MGC", 2:"AGC", 3:"HYBRID"} (non-sequential LUT)
	QMap<QString, QString> rxGainModeOptions;
	rxGainModeOptions.insert("0", "MGC");
	rxGainModeOptions.insert("2", "AGC");
	rxGainModeOptions.insert("3", "HYBRID");
	auto rxGainMode = Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, "adi,rx-gain-mode",
									    rxGainModeOptions, "GAIN CONTROL MODE");
	if(rxGainMode) {
		if(m_widgetGroup)
			m_widgetGroup->add(rxGainMode);
		layout->addWidget(rxGainMode);
		connect(this, &GainSetupWidget::readRequested, rxGainMode, &IIOWidget::readAsync);
	}

	// 2. RX1 Gain Index - Range [0,255,1]
	auto rx1GainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx1-gain-index", "[0 1 255]",
									"RX1 GAIN INDEX");
	if(rx1GainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx1GainIndex);
		layout->addWidget(rx1GainIndex);
		connect(this, &GainSetupWidget::readRequested, rx1GainIndex, &IIOWidget::readAsync);
	}

	// 3. RX2 Gain Index - Range [0,255,1]
	auto rx2GainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx2-gain-index", "[0 1 255]",
									"RX2 GAIN INDEX");
	if(rx2GainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx2GainIndex);
		layout->addWidget(rx2GainIndex);
		connect(this, &GainSetupWidget::readRequested, rx2GainIndex, &IIOWidget::readAsync);
	}

	// 4. RX1 Max Gain Index - Range [0,255,1]
	auto rx1MaxGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx1-max-gain-index",
									   "[0 1 255]", "RX1 MAX GAIN INDEX");
	if(rx1MaxGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx1MaxGainIndex);
		layout->addWidget(rx1MaxGainIndex);
		connect(this, &GainSetupWidget::readRequested, rx1MaxGainIndex, &IIOWidget::readAsync);
	}

	// 5. RX1 Min Gain Index - Range [0,255,1]
	auto rx1MinGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx1-min-gain-index",
									   "[0 1 255]", "RX1 MIN GAIN INDEX");
	if(rx1MinGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx1MinGainIndex);
		layout->addWidget(rx1MinGainIndex);
		connect(this, &GainSetupWidget::readRequested, rx1MinGainIndex, &IIOWidget::readAsync);
	}

	// 6. RX2 Max Gain Index - Range [0,255,1]
	auto rx2MaxGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx2-max-gain-index",
									   "[0 1 255]", "RX2 MAX GAIN INDEX");
	if(rx2MaxGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx2MaxGainIndex);
		layout->addWidget(rx2MaxGainIndex);
		connect(this, &GainSetupWidget::readRequested, rx2MaxGainIndex, &IIOWidget::readAsync);
	}

	// 7. RX2 Min Gain Index - Range [0,255,1]
	auto rx2MinGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,rx2-min-gain-index",
									   "[0 1 255]", "RX2 MIN GAIN INDEX");
	if(rx2MinGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(rx2MinGainIndex);
		layout->addWidget(rx2MinGainIndex);
		connect(this, &GainSetupWidget::readRequested, rx2MinGainIndex, &IIOWidget::readAsync);
	}

	return rxGainSection;
}

QWidget *GainSetupWidget::createOrxGainSection(QWidget *parent)
{
	MenuSectionCollapseWidget *orxGainSection = new MenuSectionCollapseWidget(
		"Observation", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	orxGainSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// 1. ORX Gain Mode - Custom Combo {0:"MGC", 2:"AGC", 3:"HYBRID"} (non-sequential LUT)
	QMap<QString, QString> orxGainModeOptions;
	orxGainModeOptions.insert("0", "MGC");
	orxGainModeOptions.insert("2", "AGC");
	orxGainModeOptions.insert("3", "HYBRID");
	auto orxGainMode = Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, "adi,orx-gain-mode",
									     orxGainModeOptions, "GAIN CONTROL MODE");
	if(orxGainMode) {
		if(m_widgetGroup)
			m_widgetGroup->add(orxGainMode);
		layout->addWidget(orxGainMode);
		connect(this, &GainSetupWidget::readRequested, orxGainMode, &IIOWidget::readAsync);
	}

	// 2. ORX1 Gain Index - Range [0,255,1]
	auto orx1GainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,orx1-gain-index", "[0 1 255]",
									 "ORX1 GAIN INDEX");
	if(orx1GainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(orx1GainIndex);
		layout->addWidget(orx1GainIndex);
		connect(this, &GainSetupWidget::readRequested, orx1GainIndex, &IIOWidget::readAsync);
	}

	// 3. ORX2 Gain Index - Range [0,255,1]
	auto orx2GainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,orx2-gain-index", "[0 1 255]",
									 "ORX2 GAIN INDEX");
	if(orx2GainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(orx2GainIndex);
		layout->addWidget(orx2GainIndex);
		connect(this, &GainSetupWidget::readRequested, orx2GainIndex, &IIOWidget::readAsync);
	}

	// 4. ORX Max Gain Index - Range [0,255,1]
	auto orxMaxGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,orx-max-gain-index",
									   "[0 1 255]", "MAX GAIN INDEX");
	if(orxMaxGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(orxMaxGainIndex);
		layout->addWidget(orxMaxGainIndex);
		connect(this, &GainSetupWidget::readRequested, orxMaxGainIndex, &IIOWidget::readAsync);
	}

	// 5. ORX Min Gain Index - Range [0,255,1]
	auto orxMinGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,orx-min-gain-index",
									   "[0 1 255]", "MIN GAIN INDEX");
	if(orxMinGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(orxMinGainIndex);
		layout->addWidget(orxMinGainIndex);
		connect(this, &GainSetupWidget::readRequested, orxMinGainIndex, &IIOWidget::readAsync);
	}

	return orxGainSection;
}

QWidget *GainSetupWidget::createSnifferGainSection(QWidget *parent)
{
	MenuSectionCollapseWidget *snifferGainSection = new MenuSectionCollapseWidget(
		"Sniffer", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	snifferGainSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// 1. Sniffer Gain Mode - Custom Combo {0:"MGC", 2:"AGC", 3:"HYBRID"} (non-sequential LUT)
	QMap<QString, QString> snifferGainModeOptions;
	snifferGainModeOptions.insert("0", "MGC");
	snifferGainModeOptions.insert("2", "AGC");
	snifferGainModeOptions.insert("3", "HYBRID");
	auto snifferGainMode = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,sniffer-gain-mode", snifferGainModeOptions, "GAIN CONTROL MODE");
	if(snifferGainMode) {
		if(m_widgetGroup)
			m_widgetGroup->add(snifferGainMode);
		layout->addWidget(snifferGainMode);
		connect(this, &GainSetupWidget::readRequested, snifferGainMode, &IIOWidget::readAsync);
	}

	// 2. Sniffer Gain Index - Range [0,255,1]
	auto snifferGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-gain-index",
									    "[0 1 255]", "GAIN INDEX");
	if(snifferGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(snifferGainIndex);
		layout->addWidget(snifferGainIndex);
		connect(this, &GainSetupWidget::readRequested, snifferGainIndex, &IIOWidget::readAsync);
	}

	// 3. Sniffer Max Gain Index - Range [0,255,1]
	auto snifferMaxGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-max-gain-index",
									       "[0 1 255]", "MAX GAIN INDEX");
	if(snifferMaxGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(snifferMaxGainIndex);
		layout->addWidget(snifferMaxGainIndex);
		connect(this, &GainSetupWidget::readRequested, snifferMaxGainIndex, &IIOWidget::readAsync);
	}

	// 4. Sniffer Min Gain Index - Range [0,255,1]
	auto snifferMinGainIndex = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-min-gain-index",
									       "[0 1 255]", "MIN GAIN INDEX");
	if(snifferMinGainIndex) {
		if(m_widgetGroup)
			m_widgetGroup->add(snifferMinGainIndex);
		layout->addWidget(snifferMinGainIndex);
		connect(this, &GainSetupWidget::readRequested, snifferMinGainIndex, &IIOWidget::readAsync);
	}

	return snifferGainSection;
}
