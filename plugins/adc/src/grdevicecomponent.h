/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef GRDEVICECOMPONENT_H
#define GRDEVICECOMPONENT_H

#include "scopy-adc_export.h"
#include <gui/channelcomponent.h>
#include <gui/toolcomponent.h>
#include <gui/interfaces.h>

#include <gr-util/griiodevicesource.h>
#include <gui/widgets/menucontrolbutton.h>

#include <QLabel>
#include <QPen>

#include "adcacquisitionmanager.h"

namespace scopy::adc {
// class GRTimeChannelAddon;

class SCOPY_ADC_EXPORT GRDeviceComponent : public QWidget, public ToolComponent, public SampleRateProvider
{
	Q_OBJECT
public:
	GRDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent = nullptr);
	~GRDeviceComponent();

	GRIIODeviceSource *src() const;
	CollapsableMenuControlButton *ctrl();

	virtual bool sampleRateAvailable() override;
	virtual double sampleRate() override;

public Q_SLOTS:
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void setBufferSize(uint32_t bufferSize);

	void removeChannel(ChannelComponent *c);
	void addChannel(ChannelComponent *c);

private:
	QString name;
	QWidget *widget;
	GRIIODeviceSourceNode *m_node;
	GRIIODeviceSource *m_src;
	CollapsableMenuControlButton *m_ctrl;
	QPen m_pen;
	// QList<GRTimeChannelAddon *> m_channels;
	QWidget *createChCommonAttrMenu(QWidget *parent);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createMenu(QWidget *parent = nullptr);
	void createMenuControlButton(QWidget * = nullptr);

	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name);

	QList<ChannelComponent *> m_channels;

	// SampleRateProvider interface
};
} // namespace scopy::adc

#endif // GRDEVICECOMPONENT_H
