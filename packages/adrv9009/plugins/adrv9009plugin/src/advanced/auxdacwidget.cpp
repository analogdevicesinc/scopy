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

#include "advanced/auxdacwidget.h"
#include "adrv9009widgetfactory.h"

#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>

#include <iio-widgets/iiowidget.h>
#include <gui/style.h>
#include <gui/widgets/menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_AUX_DAC, "AUX DAC")

using namespace scopy::adrv9009;
using namespace scopy;

AuxDacWidget::AuxDacWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_AUX_DAC) << "No device provided to Calibrations widget";
		return;
	}

	setupUi();
}

void AuxDacWidget::setupUi()
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

	// Create main section with collapse functionality
	m_section = new MenuSectionCollapseWidget("AUX DAC", MenuCollapseSection::MHCW_ARROW,
						  MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *widget = new QWidget(m_section);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	m_section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Add CLK Settings section
	contentLayout->addWidget(m_section);

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	// Create table layout with proper headers
	QGridLayout *configGrid = new QGridLayout();
	configGrid->setHorizontalSpacing(15);
	configGrid->setVerticalSpacing(5);

	// Table headers
	configGrid->addWidget(new QLabel("DAC"), 0, 0);
	auto *valueHeader = new QLabel("VALUE");
	valueHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(valueHeader, 0, 1);

	auto *resHeader = new QLabel("RES");
	resHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(resHeader, 0, 2);

	auto *refHeader = new QLabel("REF");
	refHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(refHeader, 0, 3);

	// DAC 0-9: Full configuration (value + resolution + vref)
	for(int i = 0; i < 10; i++) {
		int row = i + 1;

		// DAC number label
		auto *dacLabel = new QLabel(QString("DAC %1").arg(i));
		configGrid->addWidget(dacLabel, row, 0);

		// Value widget - use CORRECT attribute name following iio-oscilloscope reference
		QString valueAttr = QString("adi,aux-dac-values%1").arg(i);
		IIOWidget *valueWidget = Adrv9009WidgetFactory::createDebugRangeWidget(
			m_device, valueAttr, "[0 1023 1]", QString("DAC %1 Value").arg(i), this);
		configGrid->addWidget(valueWidget, row, 1);
		if(valueWidget) {
			connect(this, &AuxDacWidget::readRequested, valueWidget, &IIOWidget::readAsync);
		}

		// Resolution widget - use CORRECT attribute name and proper Template 2B pattern
		QString resolutionAttr = QString("adi,aux-dac-resolution%1").arg(i);
		QMap<QString, QString> *resolutionMap = new QMap<QString, QString>();
		(*resolutionMap)["0"] = "8_bit";
		(*resolutionMap)["1"] = "10_bit";
		(*resolutionMap)["2"] = "12_bit";
		IIOWidget *resolutionWidget = Adrv9009WidgetFactory::createDebugCustomComboWidget(
			m_device, resolutionAttr, resolutionMap, QString("DAC %1 Resolution").arg(i), this);
		configGrid->addWidget(resolutionWidget, row, 2);
		if(resolutionWidget) {
			connect(this, &AuxDacWidget::readRequested, resolutionWidget, &IIOWidget::readAsync);
		}

		// Vref widget - use CORRECT attribute name and proper Template 2B pattern
		QString vrefAttr = QString("adi,aux-dac-vref%1").arg(i);
		QMap<QString, QString> *vrefMap = new QMap<QString, QString>();
		(*vrefMap)["0"] = "Internal_1.25V";
		(*vrefMap)["1"] = "External_Vref";
		(*vrefMap)["2"] = "Internal_2.5V";
		(*vrefMap)["3"] = "Reserved";
		IIOWidget *vrefWidget = Adrv9009WidgetFactory::createDebugCustomComboWidget(
			m_device, vrefAttr, vrefMap, QString("DAC %1 Vref").arg(i), this);
		configGrid->addWidget(vrefWidget, row, 3);
		if(vrefWidget) {
			connect(this, &AuxDacWidget::readRequested, vrefWidget, &IIOWidget::readAsync);
		}
	}

	// DAC 10-11: Value only (no resolution/vref controls per iio-oscilloscope reference)
	for(int i = 10; i < 12; i++) {
		int row = i + 1;

		auto *dacLabel = new QLabel(QString("DAC %1").arg(i));
		configGrid->addWidget(dacLabel, row, 0);

		// Value widget only for DAC 10-11
		QString valueAttr = QString("adi,aux-dac-values%1").arg(i);
		IIOWidget *valueWidget = Adrv9009WidgetFactory::createDebugRangeWidget(
			m_device, valueAttr, "[0 1023 1]", QString("DAC %1 Value").arg(i), this);
		configGrid->addWidget(valueWidget, row, 1);
		if(valueWidget) {
			connect(this, &AuxDacWidget::readRequested, valueWidget, &IIOWidget::readAsync);
		}

		// Empty cells for consistency
		configGrid->addWidget(new QLabel("N/A"), row, 2);
		configGrid->addWidget(new QLabel("N/A"), row, 3);
	}

	layout->addLayout(configGrid);
}
