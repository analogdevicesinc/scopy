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

#ifndef DAC_API_H
#define DAC_API_H

#include "scopy-dac_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy {
namespace dac {

class DACPlugin;
class DacInstrument;
class DacDataManager;
class BufferDacAddon;
class DdsDacAddon;
class TxChannel;
class TxTone;

class SCOPY_DAC_EXPORT DAC_API : public ApiObject
{
	Q_OBJECT
public:
	explicit DAC_API(DACPlugin *plugin);
	~DAC_API();

	// General
	Q_INVOKABLE QStringList getTools();

	// Device Management
	Q_INVOKABLE QStringList getDevices();
	Q_INVOKABLE int getDeviceCount();

	// Run Control
	Q_INVOKABLE bool isDacRunning();
	Q_INVOKABLE void setDacRunning(bool running);

	// Device Mode Control
	Q_INVOKABLE QString getDeviceMode(int deviceIndex);
	Q_INVOKABLE void setDeviceMode(int deviceIndex, QString mode);

	// Device Capabilities
	Q_INVOKABLE bool isDeviceBufferCapable(int deviceIndex);
	Q_INVOKABLE bool isDeviceDds(int deviceIndex);

	// Buffer Mode Settings
	Q_INVOKABLE double getBufferSize(int deviceIndex);
	Q_INVOKABLE void setBufferSize(int deviceIndex, double value);
	Q_INVOKABLE double getFileSize(int deviceIndex);
	Q_INVOKABLE void setFileSize(int deviceIndex, double value);
	Q_INVOKABLE double getKernelBuffers(int deviceIndex);
	Q_INVOKABLE void setKernelBuffers(int deviceIndex, double value);
	Q_INVOKABLE bool isCyclic(int deviceIndex);
	Q_INVOKABLE void setCyclic(int deviceIndex, bool cyclic);
	Q_INVOKABLE bool isRepeatFileBuffer(int deviceIndex);
	Q_INVOKABLE void setRepeatFileBuffer(int deviceIndex, bool repeat);
	Q_INVOKABLE QString getFilePath(int deviceIndex);
	Q_INVOKABLE void loadFile(int deviceIndex, QString path);

	// Buffer Channel Control
	Q_INVOKABLE QStringList getBufferChannels(int deviceIndex);
	Q_INVOKABLE bool isBufferChannelEnabled(int deviceIndex, QString channelUuid);
	Q_INVOKABLE void setBufferChannelEnabled(int deviceIndex, QString channelUuid, bool enabled);

	// DDS Tone Control
	Q_INVOKABLE QStringList getDdsTxList(int deviceIndex);
	Q_INVOKABLE QStringList getDdsChannels(int deviceIndex);
	Q_INVOKABLE int getDdsToneCount(int deviceIndex, QString channelUuid);
	Q_INVOKABLE QString getDdsToneFrequency(int deviceIndex, QString channelUuid, int toneIndex);
	Q_INVOKABLE void setDdsToneFrequency(int deviceIndex, QString channelUuid, int toneIndex, QString frequency);
	Q_INVOKABLE QString getDdsToneScale(int deviceIndex, QString channelUuid, int toneIndex);
	Q_INVOKABLE void setDdsToneScale(int deviceIndex, QString channelUuid, int toneIndex, QString scale);
	Q_INVOKABLE QString getDdsTonePhase(int deviceIndex, QString channelUuid, int toneIndex);
	Q_INVOKABLE void setDdsTonePhase(int deviceIndex, QString channelUuid, int toneIndex, QString phase);

private:
	DacInstrument *getInstrument();
	DacDataManager *getManager(int deviceIndex);
	BufferDacAddon *getBufferAddon(int deviceIndex);
	DdsDacAddon *getDdsAddon(int deviceIndex);
	TxChannel *findTxChannel(int deviceIndex, const QString &channelUuid);
	TxTone *findTone(int deviceIndex, const QString &channelUuid, int toneIndex);

	DACPlugin *m_plugin;
};

} // namespace dac
} // namespace scopy

#endif // DAC_API_H
