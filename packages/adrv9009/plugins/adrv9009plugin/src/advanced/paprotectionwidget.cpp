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

#include "advanced/paprotectionwidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_PAPROTECTION, "PAProtection")

using namespace scopy;
using namespace scopy::adrv9009;

PaProtectionWidget::PaProtectionWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_PAPROTECTION) << "No device provided to PA Protection widget";
		return;
	}

	setupUi();
}

PaProtectionWidget::~PaProtectionWidget() {}

void PaProtectionWidget::setupUi()
{
	// Create main section with collapse functionality
	m_section = new MenuSectionCollapseWidget("PA Protection", MenuCollapseSection::MHCW_ARROW,
						  MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *widget = new QWidget(m_section);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	m_section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_section);

	// Add spacer to push MenuSectionCollapseWidget to top
	mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Exact iio-osc order: AVG DURATION → TX ATTEN STEP → TX1 POWER → TX2 POWER → PEAK COUNT → TX1 PEAK → TX2 PEAK

	// 1. Average Duration - Range Widget [0 1 14]
	auto avgDuration = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-pa-protection-avg-duration",
								    "[0 1 14]", "Average Duration");
	if(avgDuration) {
		layout->addWidget(avgDuration);
		connect(this, &PaProtectionWidget::readRequested, avgDuration, &IIOWidget::readAsync);
	}

	// 2. TX Attenuation Step - Range Widget [0 1 127]
	auto txAttenStep = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-pa-protection-tx-atten-step",
								    "[0 1 127]", "TX Attenuation Step");
	if(txAttenStep) {
		layout->addWidget(txAttenStep);
		connect(this, &PaProtectionWidget::readRequested, txAttenStep, &IIOWidget::readAsync);
	}

	// 3. TX1 Power Threshold - Range Widget [1 1 8191]
	auto tx1PowerThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,tx-pa-protection-tx1-power-threshold", "[1 1 8191]", "TX1 Power Threshold");
	if(tx1PowerThresh) {
		layout->addWidget(tx1PowerThresh);
		connect(this, &PaProtectionWidget::readRequested, tx1PowerThresh, &IIOWidget::readAsync);
	}

	// 4. TX2 Power Threshold - Range Widget [1 1 8191]
	auto tx2PowerThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,tx-pa-protection-tx2-power-threshold", "[1 1 8191]", "TX2 Power Threshold");
	if(tx2PowerThresh) {
		layout->addWidget(tx2PowerThresh);
		connect(this, &PaProtectionWidget::readRequested, tx2PowerThresh, &IIOWidget::readAsync);
	}

	// 5. Peak Count - Range Widget [0 1 31] - Critical: After power thresholds!
	auto peakCount = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,tx-pa-protection-peak-count",
								  "[0 1 31]", "Peak Count");
	if(peakCount) {
		layout->addWidget(peakCount);
		connect(this, &PaProtectionWidget::readRequested, peakCount, &IIOWidget::readAsync);
	}

	// 6. TX1 Peak Threshold - Range Widget [1 1 255]
	auto tx1PeakThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,tx-pa-protection-tx1-peak-threshold", "[1 1 255]", "TX1 Peak Threshold");
	if(tx1PeakThresh) {
		layout->addWidget(tx1PeakThresh);
		connect(this, &PaProtectionWidget::readRequested, tx1PeakThresh, &IIOWidget::readAsync);
	}

	// 7. TX2 Peak Threshold - Range Widget [1 1 255]
	auto tx2PeakThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,tx-pa-protection-tx2-peak-threshold", "[1 1 255]", "TX2 Peak Threshold");
	if(tx2PeakThresh) {
		layout->addWidget(tx2PeakThresh);
		connect(this, &PaProtectionWidget::readRequested, tx2PeakThresh, &IIOWidget::readAsync);
	}

	qDebug(CAT_PAPROTECTION) << "PA Protection widget created with 7 attributes";
}

MenuSectionCollapseWidget *PaProtectionWidget::section() const { return m_section; }
