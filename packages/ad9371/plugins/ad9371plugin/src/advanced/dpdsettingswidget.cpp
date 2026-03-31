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

#include "advanced/dpdsettingswidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <gui/widgets/menucollapsesection.h>
#include <gui/style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_DPD_SETTINGS, "AD9371_DPD_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

DpdSettingsWidget::DpdSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_DPD_SETTINGS) << "No device provided to DPD Settings";
		return;
	}

	setupUi();
}

DpdSettingsWidget::~DpdSettingsWidget() {}

void DpdSettingsWidget::setupUi()
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

	contentLayout->addWidget(createDpdSettingsSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);
}

QWidget *DpdSettingsWidget::createDpdSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"DPD Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// DPD Damping - RangeUi [0,255,1]
	auto dpdDampingWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-damping", "[0 1 255]",
									    "DPD Damping", this);
	if(dpdDampingWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdDampingWidget);
		layout->addWidget(dpdDampingWidget);
		m_widgets.append(dpdDampingWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdDampingWidget, &IIOWidget::readAsync);
	}

	// DPD Num Weights - RangeUi [0,3,1]
	auto dpdNumWeightsWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-num-weights",
									       "[0 1 3]", "DPD Num Weights", this);
	if(dpdNumWeightsWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdNumWeightsWidget);
		layout->addWidget(dpdNumWeightsWidget);
		m_widgets.append(dpdNumWeightsWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdNumWeightsWidget, &IIOWidget::readAsync);
	}

	// DPD Model Version - RangeUi [0,3,1]
	auto dpdModelVersionWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-model-version",
										 "[0 1 3]", "DPD Model Version", this);
	if(dpdModelVersionWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdModelVersionWidget);
		layout->addWidget(dpdModelVersionWidget);
		m_widgets.append(dpdModelVersionWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdModelVersionWidget, &IIOWidget::readAsync);
	}

	// DPD High Power Model Update - CheckBoxUi
	auto dpdHighPowerWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,dpd-high-power-model-update", "DPD High Power Model Update", this);
	if(dpdHighPowerWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdHighPowerWidget);
		layout->addWidget(dpdHighPowerWidget);
		m_widgets.append(dpdHighPowerWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdHighPowerWidget, &IIOWidget::readAsync);
	}

	// DPD Model Prior Weight - RangeUi [0,32,1]
	auto dpdModelPriorWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,dpd-model-prior-weight", "[0 1 32]", "DPD Model Prior Weight", this);
	if(dpdModelPriorWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdModelPriorWidget);
		layout->addWidget(dpdModelPriorWidget);
		m_widgets.append(dpdModelPriorWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdModelPriorWidget, &IIOWidget::readAsync);
	}

	// DPD Robust Modeling - CheckBoxUi
	auto dpdRobustWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "adi,dpd-robust-modeling",
									      "DPD Robust Modeling", this);
	if(dpdRobustWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdRobustWidget);
		layout->addWidget(dpdRobustWidget);
		m_widgets.append(dpdRobustWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdRobustWidget, &IIOWidget::readAsync);
	}

	// DPD Samples - RangeUi [0,65535,1]
	auto dpdSamplesWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-samples", "[0 1 65535]",
									    "DPD Samples", this);
	if(dpdSamplesWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdSamplesWidget);
		layout->addWidget(dpdSamplesWidget);
		m_widgets.append(dpdSamplesWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdSamplesWidget, &IIOWidget::readAsync);
	}

	// DPD Outlier Threshold - RangeUi [0,65535,1]
	auto dpdOutlierWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,dpd-outlier-threshold", "[0 1 65535]", "DPD Outlier Threshold", this);
	if(dpdOutlierWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdOutlierWidget);
		layout->addWidget(dpdOutlierWidget);
		m_widgets.append(dpdOutlierWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdOutlierWidget, &IIOWidget::readAsync);
	}

	// DPD Additional Delay Offset - RangeUi [0,63,1]
	auto dpdDelayOffsetWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,dpd-additional-delay-offset", "[0 1 63]", "DPD Additional Delay Offset", this);
	if(dpdDelayOffsetWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdDelayOffsetWidget);
		layout->addWidget(dpdDelayOffsetWidget);
		m_widgets.append(dpdDelayOffsetWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdDelayOffsetWidget, &IIOWidget::readAsync);
	}

	// DPD Path Delay PN Seq Level - RangeUi [0,255,1]
	auto dpdPathDelayWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,dpd-path-delay-pn-seq-level", "[0 1 255]", "DPD Path Delay PN Seq Level", this);
	if(dpdPathDelayWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdPathDelayWidget);
		layout->addWidget(dpdPathDelayWidget);
		m_widgets.append(dpdPathDelayWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdPathDelayWidget, &IIOWidget::readAsync);
	}

	// DPD Weights0 Real - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW0RealWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights0-real",
									   "[0 1 255]", "DPD Weights0 Real", this);
	if(dpdW0RealWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW0RealWidget);
		layout->addWidget(dpdW0RealWidget);
		m_widgets.append(dpdW0RealWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW0RealWidget, &IIOWidget::readAsync);
	}

	// DPD Weights0 Imag - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW0ImagWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights0-imag",
									   "[0 1 255]", "DPD Weights0 Imag", this);
	if(dpdW0ImagWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW0ImagWidget);
		layout->addWidget(dpdW0ImagWidget);
		m_widgets.append(dpdW0ImagWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW0ImagWidget, &IIOWidget::readAsync);
	}

	// DPD Weights1 Real - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW1RealWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights1-real",
									   "[0 1 255]", "DPD Weights1 Real", this);
	if(dpdW1RealWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW1RealWidget);
		layout->addWidget(dpdW1RealWidget);
		m_widgets.append(dpdW1RealWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW1RealWidget, &IIOWidget::readAsync);
	}

	// DPD Weights1 Imag - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW1ImagWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights1-imag",
									   "[0 1 255]", "DPD Weights1 Imag", this);
	if(dpdW1ImagWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW1ImagWidget);
		layout->addWidget(dpdW1ImagWidget);
		m_widgets.append(dpdW1ImagWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW1ImagWidget, &IIOWidget::readAsync);
	}

	// DPD Weights2 Real - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW2RealWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights2-real",
									   "[0 1 255]", "DPD Weights2 Real", this);
	if(dpdW2RealWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW2RealWidget);
		layout->addWidget(dpdW2RealWidget);
		m_widgets.append(dpdW2RealWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW2RealWidget, &IIOWidget::readAsync);
	}

	// DPD Weights2 Imag - RangeUi [-128,127,1] (signed 8-bit)
	auto dpdW2ImagWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,dpd-weights2-imag",
									   "[0 1 255]", "DPD Weights2 Imag", this);
	if(dpdW2ImagWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(dpdW2ImagWidget);
		layout->addWidget(dpdW2ImagWidget);
		m_widgets.append(dpdW2ImagWidget);
		connect(this, &DpdSettingsWidget::readRequested, dpdW2ImagWidget, &IIOWidget::readAsync);
	}

	return section;
}
