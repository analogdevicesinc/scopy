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

#include "elnawidget.h"

#include <style.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

ElnaWidget::ElnaWidget(iio_device *device, QWidget *parent)
	: m_device(device)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	widget->setLayout(layout);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("eLNA", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// adi,elna-gain-mdB
	IIOWidget *gain = IIOWidgetBuilder(widget)
				  .device(m_device)
				  .attribute("adi,elna-gain-mdB")
				  .uiStrategy(IIOWidgetBuilder::RangeUi)
				  .optionsValues("[0 500 31500]")
				  .title("LNA Gain (mdB)")
				  .infoMessage("These options must have non-zero values only if (1) an external LNA is "
					       "used and (2) the “Ext LNA "
					       "ctrl” bits in the Gain Table have been programmed. For a fixed-gain "
					       "LNA, set elna-gain-mdB to the "
					       "gain of the LNA and leave register elna-bypass-loss-mdB at its default "
					       "of 0. For an external LNA with "
					       "a bypass mode, program elna-gain-mdB with the “high gain” (non-bypass) "
					       "value and program "
					       "elna-bypass-loss-mdB with the “low gain” (bypass) value. The part "
					       "considers both values to represent "
					       "positive gain in the front end prior to the AD9361. Both registers "
					       "range from 0 to 31500mdB in 500mdB "
					       "steps. See elna-rx[1|2]-gpo[0|1]-control-enable to route the external "
					       "LNA gain table bits to the GPO "
					       "pins.")
				  .buildSingle();
	layout->addWidget(gain);

	// adi,elna-bypass-loss-mdB
	IIOWidget *bypassLoss = IIOWidgetBuilder(widget)
					.device(m_device)
					.attribute("adi,elna-bypass-loss-mdB")
					.uiStrategy(IIOWidgetBuilder::RangeUi)
					.optionsValues("[0 500 31500]")
					.title("LNA Bypass Loss (mdB)")
					.infoMessage("These options must have non-zero values only if (1) an external "
						     "LNA is used and (2) the “Ext LNA "
						     "ctrl” bits in the Gain Table have been programmed. For a "
						     "fixed-gain LNA, set elna-gain-mdB to the "
						     "gain of the LNA and leave register elna-bypass-loss-mdB at its "
						     "default of 0. For an external LNA with "
						     "a bypass mode, program elna-gain-mdB with the “high gain” "
						     "(non-bypass) value and program "
						     "elna-bypass-loss-mdB with the “low gain” (bypass) value. The "
						     "part considers both values to represent "
						     "positive gain in the front end prior to the AD9361. Both "
						     "registers range from 0 to 31500mdB in 500mdB "
						     "steps. See elna-rx[1|2]-gpo[0|1]-control-enable to route the "
						     "external LNA gain table bits to the GPO "
						     "pins.")
					.buildSingle();
	layout->addWidget(bypassLoss);

	// adi,elna-settling-delay-ns
	IIOWidget *settlingDelay = IIOWidgetBuilder(widget)
					   .device(m_device)
					   .attribute("adi,elna-settling-delay-ns")
					   .uiStrategy(IIOWidgetBuilder::RangeUi)
					   .optionsValues("[0 1 20000]")
					   .title("Settling Delay (ns)")
					   .infoMessage("Settling delay of external LNA in ns")
					   .buildSingle();
	layout->addWidget(settlingDelay);

	// adi,elna-rx1-gpo0-control-enable
	IIOWidget *rx1GPO0Control =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,elna-rx1-gpo0-control-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("RX1 GPO0")
			.infoMessage("When set, the “Ext LNA Ctrl” bit in the Rx1 gain table sets the GPO0 state")
			.buildSingle();
	layout->addWidget(rx1GPO0Control);
	rx1GPO0Control->showProgressBar(false);
	rx1GPO0Control->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	// adi,elna-rx2-gpo1-control-enable
	IIOWidget *rx2GPO0Control =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,elna-rx2-gpo1-control-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("RX2 GPO0")
			.infoMessage("When set, the “Ext LNA Ctrl” bit in the Rx2 gain table sets the GPO1 state")
			.buildSingle();
	layout->addWidget(rx2GPO0Control);
	rx2GPO0Control->showProgressBar(false);
	rx2GPO0Control->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	// adi,elna-gaintable-all-index-enable
	IIOWidget *gaintableAllIndex =
		IIOWidgetBuilder(widget)
			.device(m_device)
			.attribute("adi,elna-gaintable-all-index-enable")
			.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
			.title("External LNA enabled for all gain indexes")
			.infoMessage("The external LNA control bit in the gain tables is set for all indexes")
			.buildSingle();
	layout->addWidget(gaintableAllIndex);
	gaintableAllIndex->showProgressBar(false);
	gaintableAllIndex->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &ElnaWidget::readRequested, this, [=, this]() {
		gain->readAsync();
		bypassLoss->readAsync();
		settlingDelay->readAsync();
		rx1GPO0Control->readAsync();
		rx2GPO0Control->readAsync();
		gaintableAllIndex->readAsync();
	});
}

ElnaWidget::~ElnaWidget() {}
