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

#ifndef AD9084_API_H
#define AD9084_API_H

#include "scopy-ad9084_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad9084 {

class AD9084Plugin;

class SCOPY_AD9084_EXPORT AD9084_API : public ApiObject
{
	Q_OBJECT
public:
	explicit AD9084_API(AD9084Plugin *plugin);
	~AD9084_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// RX channel attributes
	Q_INVOKABLE QString getRxAdcFrequency(int channel);
	Q_INVOKABLE void setRxAdcFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getRxChannelNcoFrequency(int channel);
	Q_INVOKABLE void setRxChannelNcoFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getRxChannelNcoPhase(int channel);
	Q_INVOKABLE void setRxChannelNcoPhase(int channel, const QString &value);
	Q_INVOKABLE QString getRxMainNcoFrequency(int channel);
	Q_INVOKABLE void setRxMainNcoFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getRxMainNcoPhase(int channel);
	Q_INVOKABLE void setRxMainNcoPhase(int channel, const QString &value);
	Q_INVOKABLE QString getRxTestMode(int channel);
	Q_INVOKABLE void setRxTestMode(int channel, const QString &value);
	Q_INVOKABLE QString getRxNyquistZone(int channel);
	Q_INVOKABLE void setRxNyquistZone(int channel, const QString &value);
	Q_INVOKABLE QString getRxLoopback(int channel);
	Q_INVOKABLE void setRxLoopback(int channel, const QString &value);
	Q_INVOKABLE QString isRxCfirEnabled(int channel);
	Q_INVOKABLE void setRxCfirEnabled(int channel, const QString &value);
	Q_INVOKABLE QString getRxCfirProfileSel(int channel);
	Q_INVOKABLE void setRxCfirProfileSel(int channel, const QString &value);
	Q_INVOKABLE QString isRxEnabled(int channel);
	Q_INVOKABLE void setRxEnabled(int channel, const QString &value);

	// TX channel attributes
	Q_INVOKABLE QString getTxDacFrequency(int channel);
	Q_INVOKABLE void setTxDacFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getTxChannelNcoFrequency(int channel);
	Q_INVOKABLE void setTxChannelNcoFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getTxChannelNcoPhase(int channel);
	Q_INVOKABLE void setTxChannelNcoPhase(int channel, const QString &value);
	Q_INVOKABLE QString getTxMainNcoFrequency(int channel);
	Q_INVOKABLE void setTxMainNcoFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getTxMainNcoPhase(int channel);
	Q_INVOKABLE void setTxMainNcoPhase(int channel, const QString &value);
	Q_INVOKABLE QString getTxChannelNcoGainScale(int channel);
	Q_INVOKABLE void setTxChannelNcoGainScale(int channel, const QString &value);
	Q_INVOKABLE QString getTxChannelNcoTestToneScale(int channel);
	Q_INVOKABLE void setTxChannelNcoTestToneScale(int channel, const QString &value);
	Q_INVOKABLE QString isTxChannelNcoTestToneEnabled(int channel);
	Q_INVOKABLE void setTxChannelNcoTestToneEnabled(int channel, const QString &value);
	Q_INVOKABLE QString getTxMainNcoTestToneScale(int channel);
	Q_INVOKABLE void setTxMainNcoTestToneScale(int channel, const QString &value);
	Q_INVOKABLE QString isTxMainNcoTestToneEnabled(int channel);
	Q_INVOKABLE void setTxMainNcoTestToneEnabled(int channel, const QString &value);
	Q_INVOKABLE QString isTxCfirEnabled(int channel);
	Q_INVOKABLE void setTxCfirEnabled(int channel, const QString &value);
	Q_INVOKABLE QString getTxCfirProfileSel(int channel);
	Q_INVOKABLE void setTxCfirProfileSel(int channel, const QString &value);
	Q_INVOKABLE QString isTxEnabled(int channel);
	Q_INVOKABLE void setTxEnabled(int channel, const QString &value);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);
	QString rxKey(int channel, const QString &attr);
	QString txKey(int channel, const QString &attr);

	AD9084Plugin *m_plugin;
};

} // namespace scopy::ad9084

#endif // AD9084_API_H
