/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QFileDialog>

///* pulseview and sigrok */
#include <boost/math/common_factor.hpp>
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

//#include "pattern_generator.hpp"

#include "digitalchannel_manager.hpp"

using namespace std;
using namespace adiscope;

namespace pv {
class MainWindow;
class DeviceManager;
class Session;

namespace view {
class View;
class TraceTreeItem;
}
namespace toolbars {
class MainBar;
}
namespace widgets {
class DeviceToolButton;
}
}

namespace sigrok {
class Context;
}

namespace adiscope {

ChannelManager::ChannelManager()
{
}
ChannelManager::~ChannelManager()
{

}

std::vector<int> ChannelManager::get_selected_indexes()
{
	unsigned int i=0;
	std::vector<int> selection;

	for (auto ch : channel_group) {
		if (ch->is_selected()) {
			selection.push_back(i);
		}

		i++;
	}

	return selection;
}

uint16_t ChannelManager::get_enabled_mask()
{
	unsigned int i=0;
	uint16_t ret=0;

	for (auto ch : channel_group) {
		if (ch->is_enabled()) {
			ret = ret | ch->get_mask();
		}

		i++;
	}

	return ret;
}

uint16_t ChannelManager::get_selected_mask()
{
	unsigned int i=0;
	uint16_t ret=0;

	for (auto ch : channel_group) {
		if (ch->is_selected()) {
			ret = ret | ch->get_mask();
		}

		i++;
	}

	return ret;
}

std::vector<int> ChannelManager::get_enabled_indexes()
{
	unsigned int i=0;
	std::vector<int> selection;

	for (auto ch : channel_group) {
		if (ch->is_enabled()) {
			selection.push_back(i);
		}

		i++;
	}

	return selection;
}

size_t ChannelManager::get_channel_group_count()
{
	return channel_group.size();
}

size_t ChannelManager::get_channel_count()
{
	return channel.size();
}

std::vector<ChannelGroup *> *ChannelManager::get_channel_groups()
{
	return &channel_group;
}

ChannelGroup *ChannelManager::get_channel_group(int index)
{
	return channel_group[index];
}

void ChannelManager::deselect_all()
{
	for (auto&& ch : channel_group) {
		ch->select(false);
	}
}

QColor Channel::getBgcolor() const
{
	return bgcolor;
}

void Channel::setBgcolor(const QColor& value)
{
	bgcolor = value;
}

QColor Channel::getEdgecolor() const
{
	return edgecolor;
}

void Channel::setEdgecolor(const QColor& value)
{
	edgecolor = value;
}

QColor Channel::getHighcolor() const
{
	return highcolor;
}

void Channel::setHighcolor(const QColor& value)
{
	highcolor = value;
}

QColor Channel::getLowcolor() const
{
	return lowcolor;
}

void Channel::setLowcolor(const QColor& value)
{
	lowcolor = value;
}

Channel::Channel(uint16_t id_,const std::string &label_)
{
	label = label_;
	id =id_;
	mask = 1<<id_;
	lowcolor = QColor(0xC0, 0x00, 0x00);
	highcolor = QColor(0x00, 0xC0, 0x00);
	edgecolor = QColor(0x80, 0x80, 0x80);
	bgcolor = QColor();
}
Channel::~Channel()
{

}

ChannelUI::ChannelUI(Channel *ch, QWidget *parent) : QWidget(parent)
{
	this->ch = ch;
}
ChannelUI::~ChannelUI()
{}

Channel *ChannelUI::get_channel()
{
	return ch;
}

uint16_t Channel::get_mask()
{
	return mask;
}

uint16_t Channel::get_id()
{
	return id;
}

void Channel::set_id(int val)
{
	id=val;
}

std::string Channel::get_label()
{
	return label;
}

void Channel::set_label(const std::string &label)
{
	this->label = label;
}

std::vector<Channel *> *ChannelGroup::get_channels()
{
	return &channels;
}

Channel *ChannelGroup::get_channel(unsigned int index)
{
	if (index < channels.size()) {
		return channels[index];
	}

	return nullptr;
}

QColor ChannelGroup::getBgcolor() const
{
	return bgcolor;
}

void ChannelGroup::setBgcolor(const QColor& value)
{
	bgcolor = value;
}

bool ChannelGroup::is_selected() const
{
	return selected;
}

void ChannelGroup::select(bool value)
{
	selected = value;
}

void ChannelGroup::group(bool value)
{
	this->grouped = value;
}
void ChannelGroup::enable(bool value)
{
	this->enabled  = value;
}

bool ChannelGroup::is_grouped() const
{
	return grouped;
}

bool ChannelGroup::is_enabled() const
{
	return enabled;
}

QColor ChannelGroup::getHighcolor() const
{
	return highcolor;
}

void ChannelGroup::setHighcolor(const QColor& value)
{
	highcolor = value;
}

QColor ChannelGroup::getLowcolor() const
{
	return lowcolor;
}

void ChannelGroup::setLowcolor(const QColor& value)
{
	lowcolor = value;
}

QColor ChannelGroup::getEdgecolor() const
{
	return edgecolor;
}

void ChannelGroup::setEdgecolor(const QColor& value)
{
	edgecolor = value;
}

ChannelGroup::ChannelGroup(Channel *ch)
{
	if (ch!=nullptr) {
		channels.push_back(ch);
		label = ch->get_label();
	}

	group(false);
	select(false);
	enable(true);
	lowcolor = QColor(0xC0, 0x00, 0x00);
	highcolor = QColor(0x00, 0xC0, 0x00);
	edgecolor = QColor(0x80, 0x80, 0x80);
	bgcolor = QColor();
}

ChannelGroup::ChannelGroup()
{
	group(false);
	select(false);
	enable(true);
	lowcolor = QColor(0xC0, 0x00, 0x00);
	highcolor = QColor(0x00, 0xC0, 0x00);
	edgecolor = QColor(0x80, 0x80, 0x80);
	bgcolor = QColor();
}

ChannelGroup::~ChannelGroup()
{
	//qDebug()<<"ChannelGroup destroyed";
}

void ChannelGroup::set_label(const std::string &label)
{
	this->label = label;
}

std::string ChannelGroup::get_label()
{
	return label;
}

uint16_t ChannelGroup::get_mask()
{
	uint16_t mask = 0;

	for (unsigned int i=0; i<channels.size(); i++) {
		mask = mask | channels[i]->get_mask();
	}

	return mask;
}

void ChannelGroup::add_channel(Channel *channel)
{
	channels.push_back(channel);
}

void ChannelGroup::remove_channel(int chIndex)
{
	channels.erase(channels.begin() + chIndex);
}

size_t ChannelGroup::get_channel_count()
{
	return channels.size();
}

std::vector<uint16_t> ChannelGroup::get_ids()
{
	std::vector<uint16_t> ret;

	for (auto ch: (*get_channels())) {
		ret.push_back(ch->get_id());
	}

	return ret;
}

ChannelGroupUI::ChannelGroupUI(ChannelGroup *chg,
                               QWidget *parent) : QWidget(parent)
{
	this->chg = chg;
}
ChannelGroupUI::~ChannelGroupUI()
{

}

ChannelGroup *ChannelGroupUI::get_group()
{
	return chg;
}

void ChannelGroupUI::select(bool selected)
{
	chg->select(selected);
}
void ChannelGroupUI::enable(bool enabled)
{
	chg->enable(enabled);
}

void DIOManager::init()
{

}

const char *DIOManager::channelNames[] = {
	"voltage0", "voltage1", "voltage2", "voltage3",
	"voltage4", "voltage5", "voltage6", "voltage7",
	"voltage8", "voltage9", "voltage10", "voltage11",
	"voltage12", "voltage13", "voltage14", "voltage15"
};

DIOManager::DIOManager(iio_context *ctx, Filter *filt) : ctx(ctx)
{
	dev = filt->find_device(ctx,TOOL_DIGITALIO);
	nrOfChannels = iio_device_get_channels_count(dev);
	outputEnabled = false;

	for (auto i=0; i<nrOfChannels; i++) {
		auto ch = getChannel(i);
		iio_channel_attr_write(ch, "direction", "in");
		iio_channel_attr_write(ch, "raw", "0");
	}

	direction = gpi = gpo = lockMask = outputEnabled = 0x00;
}

DIOManager::~DIOManager()
{

}

iio_channel *DIOManager::getChannel(int ch)
{
	return iio_device_find_channel(dev,channelNames[ch],0);
}


void DIOManager::setOutputMode(int chid, bool mode)
{
	auto ch = getChannel(chid);
	char strMode[20];

	if (mode) {
		strcpy(strMode,"open-drain");
	} else {
		strcpy(strMode,"push-pull");
	}

	iio_channel_attr_write(ch, "outputmode", strMode);
}

void DIOManager::setDeviceDirection(int chid, bool force)
{

	auto ch = getChannel(chid);

	if (force) {
		qDebug()<<"direction out channel - "<<chid<<"\n";
		iio_channel_attr_write(ch, "direction", "out");
		return;
	}


	if (!isLocked(chid)) {
		if (outputEnabled && getDirection(chid)) {
			qDebug()<<"direction out channel" <<chid;
			iio_channel_attr_write(ch, "direction", "out");
		} else {
			qDebug()<<"direction in channel" <<chid;
			iio_channel_attr_write(ch, "direction", "in");
		}
	}
}

int DIOManager::getGpo()
{
	return gpo;
}

void DIOManager::setOutRaw(int ch, bool val)
{
	if (val) {
		gpo|=1<<ch;
	} else {
		gpo&=(~(1<<ch));
	}

	setDeviceOutRaw(ch);
}

bool DIOManager::getOutRaw(int ch)
{
	return gpo & (1<<ch);
}

void DIOManager::setDeviceOutRaw(int ch)
{
	if (outputEnabled) {
		auto channel = getChannel(ch);

		if (getOutRaw(ch)) {
			iio_channel_attr_write(channel, "raw", "1");
		} else {
			iio_channel_attr_write(channel, "raw", "0");
		}
	}
}

int DIOManager::getGpi()
{
	gpi = 0;

	for (auto i=0; i<nrOfChannels; i++) {
		gpi|= (getInRaw(i) << i);
	}

	return gpi;
}

bool DIOManager::getInRaw(int ch)
{
	auto channel = getChannel(ch);
	char buf[10];
	iio_channel_attr_read(channel,"raw",buf,10);

	if (buf[0]=='1') {
		return 1;
	} else {
		return 0;
	}
}

void DIOManager::setDirection(int ch, bool output)
{
	if (output) {
		direction|=(1<<ch);
	} else {
		direction&=(~(1<<ch));
	}

	setDeviceDirection(ch,false);
}

bool DIOManager::getDirection(int ch)
{
	return direction&(1<<ch);
}

void DIOManager::setMode(int mask)
{
	int i=0;
	for(i=0;i<nrOfChannels;i++) {
		setOutputMode(i,mask&0x01);
		mask=mask>>1;
	}

}

void DIOManager::lock(int mask)
{
	lockMask = mask;
	int i=0;

	while (mask) {
		if (mask&0x01) {
			setDeviceDirection(i,true);
		}

		mask=mask>>1;
		i++;
	}

	Q_EMIT locked();
}
bool DIOManager::isLocked(int ch)
{
	return lockMask & (1<<ch);
}

int DIOManager::getLockMask()
{
	return lockMask;
}

void DIOManager::unlock()
{
	lockMask = 0;

	for (auto i=0; i<nrOfChannels; i++) {
		setDeviceDirection(i,false);
	}

	Q_EMIT unlocked();
}

bool DIOManager::getOutputEnabled()
{
	return outputEnabled;
}

void DIOManager::enableOutput(bool output)
{
	if (outputEnabled != output) {
		outputEnabled = output;

		for (auto i=0; i<nrOfChannels; i++) {
			setDeviceDirection(i,false);
			setDeviceOutRaw(i);
		}
	}
}

}
