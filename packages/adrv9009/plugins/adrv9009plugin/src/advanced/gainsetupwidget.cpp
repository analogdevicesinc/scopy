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
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_GAINSETUP, "GainSetup")

using namespace scopy;
using namespace scopy::adrv9009;

GainSetupWidget::GainSetupWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_GAINSETUP) << "No device provided to GAIN Setup widget";
		return;
	}

	setupUi();
}

GainSetupWidget::~GainSetupWidget() {}

void GainSetupWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Add RX Gain section
	contentLayout->addWidget(createRxGainSection(contentWidget));

	// Add Observation Gain section
	contentLayout->addWidget(createObservationGainSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_GAINSETUP) << "GAIN Setup widget created with 14 attributes in 2 collapsible sections";
}

QWidget *GainSetupWidget::createRxGainSection(QWidget *parent)
{
	// RX Gain section (7 attributes)
	MenuSectionCollapseWidget *rxGainSection = new MenuSectionCollapseWidget(
		"RX", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Gain Mode - Combobox [0,1,2,3] → [MGC,AGC_FAST,AGC_SLOW,HYBRID]
	QMap<QString, QString> *gainModeOptions = new QMap<QString, QString>();
	gainModeOptions->insert("0", "MGC");
	gainModeOptions->insert("1", "AGC_FAST");
	gainModeOptions->insert("2", "AGC_SLOW");
	gainModeOptions->insert("3", "HYBRID");
	auto gainMode = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,rx-gain-control-gain-mode",
								       gainModeOptions, "Gain Mode");
	if(gainMode) {
		rxGainSection->contentLayout()->addWidget(gainMode);
		connect(this, &GainSetupWidget::readRequested, gainMode, &IIOWidget::readAsync);
	}

	// RX1 Gain Index - Range Widget [0 1 255]
	auto rx1GainIndex = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx1-gain-index",
								     "[0 1 255]", "RX1 Gain Index");
	if(rx1GainIndex) {
		rxGainSection->contentLayout()->addWidget(rx1GainIndex);
		connect(this, &GainSetupWidget::readRequested, rx1GainIndex, &IIOWidget::readAsync);
	}

	// RX2 Gain Index - Range Widget [0 1 255]
	auto rx2GainIndex = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx2-gain-index",
								     "[0 1 255]", "RX2 Gain Index");
	if(rx2GainIndex) {
		rxGainSection->contentLayout()->addWidget(rx2GainIndex);
		connect(this, &GainSetupWidget::readRequested, rx2GainIndex, &IIOWidget::readAsync);
	}

	// RX1 Max Gain Index - Range Widget [0 1 255]
	auto rx1MaxGain = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx1-max-gain-index",
								   "[0 1 255]", "RX1 Max Gain Index");
	if(rx1MaxGain) {
		rxGainSection->contentLayout()->addWidget(rx1MaxGain);
		connect(this, &GainSetupWidget::readRequested, rx1MaxGain, &IIOWidget::readAsync);
	}

	// RX1 Min Gain Index - Range Widget [0 1 255]
	auto rx1MinGain = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx1-min-gain-index",
								   "[0 1 255]", "RX1 Min Gain Index");
	if(rx1MinGain) {
		rxGainSection->contentLayout()->addWidget(rx1MinGain);
		connect(this, &GainSetupWidget::readRequested, rx1MinGain, &IIOWidget::readAsync);
	}

	// RX2 Max Gain Index - Range Widget [0 1 255]
	auto rx2MaxGain = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx2-max-gain-index",
								   "[0 1 255]", "RX2 Max Gain Index");
	if(rx2MaxGain) {
		rxGainSection->contentLayout()->addWidget(rx2MaxGain);
		connect(this, &GainSetupWidget::readRequested, rx2MaxGain, &IIOWidget::readAsync);
	}

	// RX2 Min Gain Index - Range Widget [0 1 255]
	auto rx2MinGain = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rx-gain-control-rx2-min-gain-index",
								   "[0 1 255]", "RX2 Min Gain Index");
	if(rx2MinGain) {
		rxGainSection->contentLayout()->addWidget(rx2MinGain);
		connect(this, &GainSetupWidget::readRequested, rx2MinGain, &IIOWidget::readAsync);
	}

	return rxGainSection;
}

QWidget *GainSetupWidget::createObservationGainSection(QWidget *parent)
{
	// Observation Gain section (7 attributes)
	MenuSectionCollapseWidget *obsGainSection = new MenuSectionCollapseWidget(
		"Observation", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Gain Mode - Combobox [0,1,2,3] → [MGC,AGC_FAST,AGC_SLOW,HYBRID]
	QMap<QString, QString> *gainModeOptions = new QMap<QString, QString>();
	gainModeOptions->insert("0", "MGC");
	gainModeOptions->insert("1", "AGC_FAST");
	gainModeOptions->insert("2", "AGC_SLOW");
	gainModeOptions->insert("3", "HYBRID");
	auto gainMode = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "adi,orx-gain-control-gain-mode",
								       gainModeOptions, "Gain Mode");
	if(gainMode) {
		obsGainSection->contentLayout()->addWidget(gainMode);
		connect(this, &GainSetupWidget::readRequested, gainMode, &IIOWidget::readAsync);
	}

	// ORX1 Gain Index - Range Widget [0 1 255]
	auto orx1GainIndex = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-gain-control-orx1-gain-index",
								      "[0 1 255]", "ORX1 Gain Index");
	if(orx1GainIndex) {
		obsGainSection->contentLayout()->addWidget(orx1GainIndex);
		connect(this, &GainSetupWidget::readRequested, orx1GainIndex, &IIOWidget::readAsync);
	}

	// ORX2 Gain Index - Range Widget [0 1 255]
	auto orx2GainIndex = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,orx-gain-control-orx2-gain-index",
								      "[0 1 255]", "ORX2 Gain Index");
	if(orx2GainIndex) {
		obsGainSection->contentLayout()->addWidget(orx2GainIndex);
		connect(this, &GainSetupWidget::readRequested, orx2GainIndex, &IIOWidget::readAsync);
	}

	// ORX1 Max Gain Index - Range Widget [0 1 255]
	auto orx1MaxGain = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,orx-gain-control-orx1-max-gain-index", "[0 1 255]", "ORX1 Max Gain Index");
	if(orx1MaxGain) {
		obsGainSection->contentLayout()->addWidget(orx1MaxGain);
		connect(this, &GainSetupWidget::readRequested, orx1MaxGain, &IIOWidget::readAsync);
	}

	// ORX1 Min Gain Index - Range Widget [0 1 255]
	auto orx1MinGain = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,orx-gain-control-orx1-min-gain-index", "[0 1 255]", "ORX1 Min Gain Index");
	if(orx1MinGain) {
		obsGainSection->contentLayout()->addWidget(orx1MinGain);
		connect(this, &GainSetupWidget::readRequested, orx1MinGain, &IIOWidget::readAsync);
	}

	// ORX2 Max Gain Index - Range Widget [0 1 255]
	auto orx2MaxGain = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,orx-gain-control-orx2-max-gain-index", "[0 1 255]", "ORX2 Max Gain Index");
	if(orx2MaxGain) {
		obsGainSection->contentLayout()->addWidget(orx2MaxGain);
		connect(this, &GainSetupWidget::readRequested, orx2MaxGain, &IIOWidget::readAsync);
	}

	// ORX2 Min Gain Index - Range Widget [0 1 255]
	auto orx2MinGain = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,orx-gain-control-orx2-min-gain-index", "[0 1 255]", "ORX2 Min Gain Index");
	if(orx2MinGain) {
		obsGainSection->contentLayout()->addWidget(orx2MinGain);
		connect(this, &GainSetupWidget::readRequested, orx2MinGain, &IIOWidget::readAsync);
	}

	return obsGainSection;
}
