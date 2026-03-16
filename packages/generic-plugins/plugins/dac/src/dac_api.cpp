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

#include "dac_api.h"
#include "dacplugin.h"
#include "dacinstrument.h"
#include "dacdatamanager.h"
#include "dacdatamodel.h"
#include "bufferdacaddon.h"
#include "ddsdacaddon.h"
#include "txchannel.h"
#include "txtone.h"
#include "filebrowser.h"

#include <pluginbase/toolmenuentry.h>
#include <gui/mapstackedwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <menuonoffswitch.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menucombo.h>

#include <QLoggingCategory>
#include <QComboBox>
#include <QCheckBox>

Q_LOGGING_CATEGORY(CAT_DAC_API, "DAC_API")

using namespace scopy::dac;

DAC_API::DAC_API(DACPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

DAC_API::~DAC_API() {}

// --- Private helpers ---

DacInstrument *DAC_API::getInstrument() { return dynamic_cast<DacInstrument *>(m_plugin->dac); }

DacDataManager *DAC_API::getManager(int deviceIndex)
{
	DacInstrument *inst = getInstrument();
	if(!inst || deviceIndex < 0 || deviceIndex >= inst->m_dacDataManagers.size()) {
		return nullptr;
	}
	return inst->m_dacDataManagers.at(deviceIndex);
}

BufferDacAddon *DAC_API::getBufferAddon(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	if(!mgr) {
		return nullptr;
	}
	return dynamic_cast<BufferDacAddon *>(mgr->dacAddonStack->get(QString::number(DacDataManager::DAC_BUFFER)));
}

DdsDacAddon *DAC_API::getDdsAddon(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	if(!mgr) {
		return nullptr;
	}
	return dynamic_cast<DdsDacAddon *>(mgr->dacAddonStack->get(QString::number(DacDataManager::DAC_DDS)));
}

TxChannel *DAC_API::findTxChannel(int deviceIndex, const QString &channelUuid)
{
	DdsDacAddon *addon = getDdsAddon(deviceIndex);
	if(!addon) {
		return nullptr;
	}

	for(auto it = addon->m_txWidgets.begin(); it != addon->m_txWidgets.end(); ++it) {
		MapStackedWidget *stack = it.value();
		TxMode *tm = dynamic_cast<TxMode *>(stack->currentWidget());
		if(!tm) {
			continue;
		}
		for(TxChannel *ch : tm->m_txChannels) {
			if(ch->channelUuid() == channelUuid) {
				return ch;
			}
		}
	}
	return nullptr;
}

TxTone *DAC_API::findTone(int deviceIndex, const QString &channelUuid, int toneIndex)
{
	TxChannel *ch = findTxChannel(deviceIndex, channelUuid);
	if(!ch) {
		return nullptr;
	}
	return ch->tone(toneIndex);
}

// --- General ---

QStringList DAC_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Device Management ---

QStringList DAC_API::getDevices()
{
	DacInstrument *inst = getInstrument();
	if(!inst) {
		return QStringList();
	}

	QStringList devices;
	for(DacDataManager *mgr : inst->m_dacDataManagers) {
		devices.append(mgr->getName());
	}
	return devices;
}

int DAC_API::getDeviceCount()
{
	DacInstrument *inst = getInstrument();
	return inst ? inst->m_dacDataManagers.size() : 0;
}

// --- Run Control ---

bool DAC_API::isDacRunning()
{
	DacInstrument *inst = getInstrument();
	if(!inst) {
		return false;
	}
	for(DacDataManager *mgr : inst->m_dacDataManagers) {
		if(mgr->isRunning()) {
			return true;
		}
	}
	return false;
}

void DAC_API::setDacRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "dac");
	if(tool) {
		Q_EMIT tool->runToggled(running);
	}
}

// --- Device Mode Control ---

QString DAC_API::getDeviceMode(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	if(!mgr || !mgr->m_mode) {
		return QString();
	}
	return mgr->m_mode->combo()->currentText();
}

void DAC_API::setDeviceMode(int deviceIndex, QString mode)
{
	DacDataManager *mgr = getManager(deviceIndex);
	if(!mgr || !mgr->m_mode) {
		return;
	}
	int index = mgr->m_mode->combo()->findText(mode);
	if(index >= 0) {
		mgr->m_mode->combo()->setCurrentIndex(index);
	}
}

// --- Device Capabilities ---

bool DAC_API::isDeviceBufferCapable(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	return mgr ? mgr->isBufferCapable() : false;
}

bool DAC_API::isDeviceDds(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	return mgr ? mgr->isDds() : false;
}

// --- Buffer Mode Settings ---

double DAC_API::getBufferSize(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->m_bufferSizeSpin ? addon->m_bufferSizeSpin->value() : 0.0;
}

void DAC_API::setBufferSize(int deviceIndex, double value)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(addon && addon->m_bufferSizeSpin) {
		addon->m_bufferSizeSpin->setValue(value);
	}
}

double DAC_API::getFileSize(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->m_fileSizeSpin ? addon->m_fileSizeSpin->value() : 0.0;
}

void DAC_API::setFileSize(int deviceIndex, double value)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(addon && addon->m_fileSizeSpin) {
		addon->m_fileSizeSpin->setValue(value);
	}
}

double DAC_API::getKernelBuffers(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->m_kernelCountSpin ? addon->m_kernelCountSpin->value() : 0.0;
}

void DAC_API::setKernelBuffers(int deviceIndex, double value)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(addon && addon->m_kernelCountSpin) {
		addon->m_kernelCountSpin->setValue(value);
	}
}

bool DAC_API::isCyclic(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->m_cyclicBtn ? addon->m_cyclicBtn->onOffswitch()->isChecked() : false;
}

void DAC_API::setCyclic(int deviceIndex, bool cyclic)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(addon && addon->m_cyclicBtn) {
		addon->m_cyclicBtn->onOffswitch()->setChecked(cyclic);
	}
}

bool DAC_API::isRepeatFileBuffer(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->m_repeatFileBufferBtn ? addon->m_repeatFileBufferBtn->onOffswitch()->isChecked() : false;
}

void DAC_API::setRepeatFileBuffer(int deviceIndex, bool repeat)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(addon && addon->m_repeatFileBufferBtn) {
		addon->m_repeatFileBufferBtn->onOffswitch()->setChecked(repeat);
	}
}

QString DAC_API::getFilePath(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	return addon && addon->fm ? addon->fm->getFilePath() : QString();
}

void DAC_API::loadFile(int deviceIndex, QString path)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(!addon) {
		return;
	}
	if(addon->fm) {
		addon->fm->m_filename = path;
	}
	addon->load(path);
}

// --- Buffer Channel Control ---

QStringList DAC_API::getBufferChannels(int deviceIndex)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(!addon) {
		return QStringList();
	}
	return addon->m_channelBtns.keys();
}

bool DAC_API::isBufferChannelEnabled(int deviceIndex, QString channelUuid)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(!addon) {
		return false;
	}
	MenuControlButton *btn = addon->m_channelBtns.value(channelUuid, nullptr);
	return btn ? btn->checkBox()->isChecked() : false;
}

void DAC_API::setBufferChannelEnabled(int deviceIndex, QString channelUuid, bool enabled)
{
	BufferDacAddon *addon = getBufferAddon(deviceIndex);
	if(!addon) {
		return;
	}
	MenuControlButton *btn = addon->m_channelBtns.value(channelUuid, nullptr);
	if(btn) {
		btn->checkBox()->setChecked(enabled);
	}
}

// --- DDS Tone Control ---

QStringList DAC_API::getDdsTxList(int deviceIndex)
{
	DacDataManager *mgr = getManager(deviceIndex);
	if(!mgr) {
		return QStringList();
	}
	return mgr->m_model->getDdsTxs().keys();
}

QStringList DAC_API::getDdsChannels(int deviceIndex)
{
	DdsDacAddon *addon = getDdsAddon(deviceIndex);
	if(!addon) {
		return QStringList();
	}

	QStringList channels;
	for(auto it = addon->m_txWidgets.begin(); it != addon->m_txWidgets.end(); ++it) {
		MapStackedWidget *stack = it.value();
		TxMode *tm = dynamic_cast<TxMode *>(stack->currentWidget());
		if(!tm) {
			continue;
		}
		for(TxChannel *ch : tm->m_txChannels) {
			channels.append(ch->channelUuid());
		}
	}
	return channels;
}

int DAC_API::getDdsToneCount(int deviceIndex, QString channelUuid)
{
	TxChannel *ch = findTxChannel(deviceIndex, channelUuid);
	return ch ? ch->toneCount() : 0;
}

QString DAC_API::getDdsToneFrequency(int deviceIndex, QString channelUuid, int toneIndex)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	return tone ? tone->frequency() : QString();
}

void DAC_API::setDdsToneFrequency(int deviceIndex, QString channelUuid, int toneIndex, QString frequency)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	if(tone) {
		tone->updateFrequency(frequency);
	}
}

QString DAC_API::getDdsToneScale(int deviceIndex, QString channelUuid, int toneIndex)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	return tone ? tone->scale() : QString();
}

void DAC_API::setDdsToneScale(int deviceIndex, QString channelUuid, int toneIndex, QString scale)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	if(tone) {
		tone->updateScale(scale);
	}
}

QString DAC_API::getDdsTonePhase(int deviceIndex, QString channelUuid, int toneIndex)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	return tone ? tone->phase() : QString();
}

void DAC_API::setDdsTonePhase(int deviceIndex, QString channelUuid, int toneIndex, QString phase)
{
	TxTone *tone = findTone(deviceIndex, channelUuid, toneIndex);
	if(tone) {
		tone->updatePhase(phase);
	}
}

#include "moc_dac_api.cpp"
