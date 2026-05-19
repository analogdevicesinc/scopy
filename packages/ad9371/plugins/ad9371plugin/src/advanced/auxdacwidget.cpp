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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <QCheckBox>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <gui/widgets/menucollapsesection.h>
#include <gui/style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_AUX_DAC, "AD9371_AUX_DAC")

using namespace scopy;
using namespace scopy::ad9371;

AuxDacWidget::AuxDacWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_AUX_DAC) << "No device provided to AUX DAC";
		return;
	}

	setupUi();
	readEnableMaskFromDevice();
}

AuxDacWidget::~AuxDacWidget() {}

void AuxDacWidget::setupUi()
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

	// Single combined AUX DAC section
	contentLayout->addWidget(createAuxDacSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	// Connect readRequested to refresh enable mask checkboxes
	connect(this, &AuxDacWidget::readRequested, this, &AuxDacWidget::readEnableMaskFromDevice);
}

QWidget *AuxDacWidget::createAuxDacSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("AUX DAC", MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QGridLayout *configGrid = new QGridLayout(widget);
	configGrid->setContentsMargins(10, 10, 10, 10);
	configGrid->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Table headers
	configGrid->addWidget(new QLabel("DAC"), 0, 0);

	QLabel *enableHeader = new QLabel("ENABLE");
	enableHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(enableHeader, 0, 1);

	QLabel *valueHeader = new QLabel("VALUE");
	valueHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(valueHeader, 0, 2);

	QLabel *slopeHeader = new QLabel("DAC SLOPE");
	slopeHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(slopeHeader, 0, 3);

	QLabel *vrefHeader = new QLabel("VREF");
	vrefHeader->setAlignment(Qt::AlignCenter);
	configGrid->addWidget(vrefHeader, 0, 4);

	// DAC 0-9: enable + value + slope + vref
	for(int i = 0; i < AUX_DAC_COUNT; i++) {
		int row = i + 1;

		// DAC number label
		configGrid->addWidget(new QLabel(QString("DAC %1").arg(i)), row, 0);

		// Enable checkbox
		QCheckBox *cb = new QCheckBox("", widget);
		configGrid->addWidget(cb, row, 1, Qt::AlignCenter);
		m_enableCheckboxes.append(cb);
		connect(cb, &QCheckBox::toggled, this, &AuxDacWidget::onEnableMaskChanged);

		// Value widget - DebugRangeWidget [0,1023,1]
		QString valueAttr = QString("adi,aux-dac-value%1").arg(i);
		IIOWidget *valueWidget =
			Ad9371WidgetFactory::createDebugRangeWidget(m_device, valueAttr, "[0 1 1023]", "VALUE", this);
		configGrid->addWidget(valueWidget, row, 2);
		if(valueWidget) {
			if(m_widgetGroup)
				m_widgetGroup->add(valueWidget);
			m_widgets.append(valueWidget);
			connect(this, &AuxDacWidget::readRequested, valueWidget, &IIOWidget::readAsync);
		}

		// Slope widget - DebugCustomComboWidget
		QString slopeAttr = QString("adi,aux-dac-slope%1").arg(i);
		QMap<QString, QString> slopeMap;
		slopeMap["0"] = "DAC SLOPE 1.404mV";
		slopeMap["1"] = "DAC SLOPE 0.705mV";
		IIOWidget *slopeWidget = Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, slopeAttr,
											   slopeMap, "DAC SLOPE", this);
		configGrid->addWidget(slopeWidget, row, 3);
		if(slopeWidget) {
			if(m_widgetGroup)
				m_widgetGroup->add(slopeWidget);
			m_widgets.append(slopeWidget);
			connect(this, &AuxDacWidget::readRequested, slopeWidget, &IIOWidget::readAsync);
		}

		// VRef widget - DebugCustomComboWidget
		QString vrefAttr = QString("adi,aux-dac-vref%1").arg(i);
		QMap<QString, QString> vrefMap;
		vrefMap["0"] = "VREF 1 V";
		vrefMap["1"] = "VREF 1.5 V";
		vrefMap["2"] = "VREF 2 V";
		vrefMap["3"] = "VREF 2.5 V";
		IIOWidget *vrefWidget =
			Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, vrefAttr, vrefMap, "VREF", this);
		configGrid->addWidget(vrefWidget, row, 4);
		if(vrefWidget) {
			if(m_widgetGroup)
				m_widgetGroup->add(vrefWidget);
			m_widgets.append(vrefWidget);
			connect(this, &AuxDacWidget::readRequested, vrefWidget, &IIOWidget::readAsync);
		}
	}

	return section;
}

void AuxDacWidget::onEnableMaskChanged() { writeEnableMaskToDevice(); }

void AuxDacWidget::readEnableMaskFromDevice()
{
	if(!m_device || m_enableCheckboxes.size() != AUX_DAC_COUNT) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,aux-dac-enable-mask", &mask);
	if(ret < 0) {
		qDebug(CAT_AD9371_AUX_DAC) << "Failed to read aux-dac-enable-mask, using defaults";
		mask = 0;
	}

	// Block signals while updating checkboxes to prevent triggering writes
	for(int i = 0; i < AUX_DAC_COUNT; i++) {
		m_enableCheckboxes[i]->blockSignals(true);
		m_enableCheckboxes[i]->setChecked((mask & (1LL << i)) != 0);
		m_enableCheckboxes[i]->blockSignals(false);
	}
}

void AuxDacWidget::writeEnableMaskToDevice()
{
	if(!m_device || m_enableCheckboxes.size() != AUX_DAC_COUNT) {
		return;
	}

	// Read-modify-write: preserve bits not managed by this UI
	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,aux-dac-enable-mask", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_AUX_DAC) << "Failed to read aux-dac-enable-mask before write, error:" << ret;
		return;
	}

	// Clear only the 10 managed bits (0-9), then apply UI state
	for(int i = 0; i < AUX_DAC_COUNT; i++) {
		mask &= ~(1LL << i);
	}

	for(int i = 0; i < AUX_DAC_COUNT; i++) {
		if(m_enableCheckboxes[i]->isChecked()) {
			mask |= (1LL << i);
		}
	}

	ret = iio_device_debug_attr_write_longlong(m_device, "adi,aux-dac-enable-mask", mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_AUX_DAC) << "Failed to write aux-dac-enable-mask, error:" << ret;
	}
}
