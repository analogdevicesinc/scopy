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

#include "grdevicecomponent.h"
#include <gui/channelcomponent.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <widgets/menuheader.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>

#include <iioutil/iiocpp/iiodevice.h>
#include <iioutil/iiocpp/iiochannel.h>
#include <iioutil/iiocpp/iioattribute.h>
#include <iioutil/iiocpp/iioresult.h>

#include <style.h>

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::grutil;

GRDeviceComponent::GRDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
{
	m_node = node;
	name = node->name();
	m_pen = QPen(Style::getAttribute(json::theme::interactive_primary_idle));
	m_src = node->src();
	// connect(this, &GRDeviceAddon::updateBufferSize, this, &GRDeviceAddon::setBufferSize);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);
	createMenuControlButton();
}

QWidget *GRDeviceComponent::createChCommonAttrMenu(QWidget *parent)
{
	const struct iio_device *dev = m_src->iioDev();

	QList<IIOWidget *> attrWidgets;
	MenuSectionCollapseWidget *attr =
		new MenuSectionCollapseWidget("COMMON CHANNEL ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
					      MenuCollapseSection::MHW_BASEWIDGET, parent);
	const iio_context *ctx = IIODevice::get_context(dev);

	int chCount = IIODevice::get_channels_count(dev);
	if(chCount < 2) {
		return nullptr;
	}

	IIOResult<const iio_channel *> res1 = IIODevice::get_channel(dev, 0);
	if(!res1.ok()) {
		qDebug() << "Failed to get channel 0" << res1.error();
		return nullptr;
	}
	const iio_channel *ch = res1.data();

	int attrCount = IIOChannel::get_attrs_count(ch);

	for(int i = 0; i < attrCount; i++) {
		bool createAttr = true;
		IIOResult<const iio_attr *> res = IIOChannel::get_attr(ch, i);
		if(!res.ok()) {
			qDebug() << "Failed to get attribute" << i << res.error();
			continue;
		}
		const char *attrName = IIOAttribute::get_name(res.data());

		for(int j = 1; j < chCount; j++) {
			IIOResult<const iio_channel *> res1 = IIODevice::get_channel(dev, j);
			if(!res1.ok()) {
				qDebug() << "Failed to get channel" << j << res1.error();
				continue;
			}
			const iio_channel *ch1 = res1.data();
			IIOResult<const iio_attr *> res2 = IIOChannel::find_attr(ch1, attrName);
			if(!res2.ok()) {
				qDebug() << "Failed to find attribute" << attrName << "in channel" << j << res2.error();
				continue;
			}
			const char *attr1Name = IIOAttribute::get_name(res2.data());

			if(strcmp(attrName, attr1Name) != 0) {
				createAttr = false;
				break;
			}
		}
		if(createAttr) {
			qInfo() << "common " << attrName;
			IIOWidget *w = IIOWidgetBuilder(attr)
					       .context(const_cast<iio_context *>(ctx))
					       .device(const_cast<iio_device *>(dev))
					       .channel(const_cast<iio_channel *>(ch))
					       .attribute(attrName)
					       .buildSingle();

			//			iiowidgetbuilder.convertToMulti(w)
			/*createMultiDataStrategy
			Add rest of data strategies

			*/
			attrWidgets.append(w);
		}
	}

	if(attrWidgets.count() == 0) {
		return nullptr;
	}

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attr->setCollapsed(true);
	return attr;
}

QWidget *GRDeviceComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *attr = new MenuSectionCollapseWidget("ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
									MenuCollapseSection::MHW_BASEWIDGET, parent);

	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(attr).device(m_src->iioDev()).buildAll();
	const iio_context *ctx = IIODevice::get_context(m_src->iioDev());
	attrWidgets.append(IIOWidgetBuilder(attr)
				   .context(const_cast<iio_context *>(ctx))
				   .device(m_src->iioDev())
				   .attribute("Triggers")
				   .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				   .dataStrategy(IIOWidgetBuilder::DS::TriggerData)
				   .buildSingle());

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attr->setCollapsed(true);
	return attr;
}

QWidget *GRDeviceComponent::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(name, m_pen, w);
	QWidget *attrMenu = createAttrMenu(w);
	QWidget *chcommonattrMenu = nullptr; // createChCommonAttrMenu(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(attrMenu);
	if(chcommonattrMenu) {
		layScroll->addWidget(chcommonattrMenu);
	}

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

void GRDeviceComponent::createMenuControlButton(QWidget *parent)
{
	m_ctrl = new CollapsableMenuControlButton(parent);
	setupDeviceMenuControlButtonHelper(m_ctrl->getControlBtn(), name);
}

void GRDeviceComponent::setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name)
{
	devBtn->setName(name);
	devBtn->setCheckable(true);
	devBtn->button()->setVisible(false);
	devBtn->setOpenMenuChecksThis(true);
	devBtn->setDoubleClickToOpenMenu(true);
}

CollapsableMenuControlButton *GRDeviceComponent::ctrl() { return m_ctrl; }

bool GRDeviceComponent::sampleRateAvailable()
{
	for(auto c : qAsConst(m_channels)) {
		if(c->enabled())
			return m_src->sampleRateAvailable();
	}
	return false;
}

double GRDeviceComponent::sampleRate() { return m_src->readSampleRate(); }

void GRDeviceComponent::setBufferSize(uint32_t bufferSize) { m_src->setBuffersize(bufferSize); }

GRDeviceComponent::~GRDeviceComponent() {}

void GRDeviceComponent::onStart() {}

void GRDeviceComponent::onStop() {}

void GRDeviceComponent::onInit() {}

void GRDeviceComponent::onDeinit() {}

void GRDeviceComponent::addChannel(ChannelComponent *c) { m_channels.append(c); }

void GRDeviceComponent::removeChannel(ChannelComponent *c) { m_channels.removeAll(c); }
