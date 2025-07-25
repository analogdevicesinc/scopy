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

#include "miscwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace ad936x;

MiscWidget::MiscWidget(iio_device *device, QWidget *parent)
	: m_device(device)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *dcOffsetTracking = new QWidget(this);
	Style::setStyle(dcOffsetTracking, style::properties::widget::border_interactive);

	QGridLayout *dcOffsetTrackingLayout = new QGridLayout(dcOffsetTracking);
	dcOffsetTrackingLayout->setSpacing(10);
	dcOffsetTracking->setLayout(dcOffsetTrackingLayout);

	dcOffsetTrackingLayout->addWidget(new QLabel("RX Frequency", dcOffsetTracking), 0, 0);
	dcOffsetTrackingLayout->addWidget(new QLabel("> 4 GHz", dcOffsetTracking), 0, 1);
	dcOffsetTrackingLayout->addWidget(new QLabel("< 4 GHz", dcOffsetTracking), 0, 2);

	// attenuation
	dcOffsetTrackingLayout->addWidget(new QLabel("Attenuation", dcOffsetTracking), 1, 0);

	// adi,dc-offset-attenuation-high-range
	IIOWidget *attenuationHighRange = IIOWidgetBuilder(dcOffsetTracking)
						  .device(m_device)
						  .attribute("adi,dc-offset-attenuation-high-range")
						  .uiStrategy(IIOWidgetBuilder::RangeUi)
						  .optionsValues("[-16 1 15]")
						  .title("")
						  .infoMessage("RX LO > 4 GHz: These bits control the attenuator for "
							       "the initialization and tracking RF DC offset "
							       "calibrations. The integrated data shifts by this twos "
							       "complement value and ranges from -16 to +15.")
						  .buildSingle();
	dcOffsetTrackingLayout->addWidget(attenuationHighRange, 1, 1);

	// adi,dc-offset-attenuation-low-range
	IIOWidget *attenuationLowRange = IIOWidgetBuilder(dcOffsetTracking)
						 .device(m_device)
						 .attribute("adi,dc-offset-attenuation-low-range")
						 .uiStrategy(IIOWidgetBuilder::RangeUi)
						 .optionsValues("[-16 1 15]")
						 .title("")
						 .infoMessage("RX LO < 4 GHz: These bits control the attenuator for "
							      "the initialization and tracking RF DC offset "
							      "calibrations. The integrated data shifts by this twos "
							      "complement value and ranges from -16 to +15.")
						 .buildSingle();
	dcOffsetTrackingLayout->addWidget(attenuationLowRange, 1, 2);

	// count
	dcOffsetTrackingLayout->addWidget(new QLabel("Count", dcOffsetTracking), 2, 0);

	// adi,dc-offset-count-high-range
	IIOWidget *countHighRange =
		IIOWidgetBuilder(dcOffsetTracking)
			.device(m_device)
			.attribute("adi,dc-offset-count-high-range")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 255]")
			.title("")
			.infoMessage("RX LO > 4 GHz: This value affects both RF DC offset initialization and tracking "
				     "and it sets the "
				     "number of integrated samples and the loop gain. The number of samples equals 256 "
				     "× RF DC Offset "
				     "Count[7:0] in ClkRF cycles. Increasing this value increases loop gain.")
			.buildSingle();
	dcOffsetTrackingLayout->addWidget(countHighRange, 2, 1);

	// adi,dc-offset-count-low-range
	IIOWidget *countLowRange =
		IIOWidgetBuilder(dcOffsetTracking)
			.device(m_device)
			.attribute("adi,dc-offset-count-low-range")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 255]")
			.title("")
			.infoMessage("RX LO < 4 GHz: This value affects both RF DC offset initialization and tracking "
				     "and it sets the "
				     "number of integrated samples and the loop gain. The number of samples equals 256 "
				     "× RF DC Offset "
				     "Count[7:0] in ClkRF cycles. Increasing this value increases loop gain.")
			.buildSingle();
	dcOffsetTrackingLayout->addWidget(countLowRange, 2, 2);

	// adi,dc-offset-tracking-update-event-mask
	dcOffsetTrackingLayout->addWidget(new QLabel("Update Event Mask", dcOffsetTracking), 3, 0);
	IIOWidget *updateEventMask =
		IIOWidgetBuilder(dcOffsetTracking)
			.device(m_device)
			.attribute("adi,dc-offset-tracking-update-event-mask")
			.uiStrategy(IIOWidgetBuilder::RangeUi)
			.optionsValues("[0 1 7]")
			.title("")
			.infoMessage("BIT(0) Apply a new tracking word when a gain change occurs. BIT(1) Apply a new "
				     "tracking word when the received signal is less than the SOI Threshold. BIT(2) "
				     "Apply a new tracking word after the device exits the receive state")
			.buildSingle();
	dcOffsetTrackingLayout->addWidget(updateEventMask, 3, 1);

	m_layout->addWidget(dcOffsetTracking);

	// adi,qec-tracking-slow-mode-enable
	QWidget *qecTracking = new QWidget(this);
	Style::setStyle(qecTracking, style::properties::widget::border_interactive);

	QVBoxLayout *qecTrackingLayout = new QVBoxLayout(qecTracking);
	qecTracking->setLayout(qecTrackingLayout);

	QLabel *title = new QLabel("QEC Tracking", qecTracking);
	Style::setStyle(title, style::properties::label::menuBig);
	qecTrackingLayout->addWidget(title);

	// adi,qec-tracking-slow-mode-enable
	IIOWidget *qecTrackingIio =
		IIOWidgetBuilder(qecTracking)
			.device(m_device)
			.attribute("adi,qec-tracking-slow-mode-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("Slow QEC")
			.infoMessage("Improved RX QEC tracking in case signal of interest is close to DC/LO")
			.buildSingle();
	qecTrackingLayout->addWidget(qecTrackingIio);
	qecTrackingIio->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	qecTrackingIio->showProgressBar(false);

	m_layout->addWidget(qecTracking);
	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &MiscWidget::readRequested, this, [=, this]() {
		attenuationLowRange->readAsync();
		attenuationHighRange->readAsync();
		countLowRange->readAsync();
		countHighRange->readAsync();
		updateEventMask->readAsync();
		qecTrackingIio->readAsync();
	});
}

MiscWidget::~MiscWidget() {}
