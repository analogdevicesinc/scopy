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

#ifndef JESDSTATUS_API_H
#define JESDSTATUS_API_H

#include "scopy-jesdstatus_export.h"
#include <pluginbase/apiobject.h>
#include <QString>

namespace scopy::jesdstatus {

class JesdStatusPlugin;
class JesdStatus;
class JesdStatusView;
class JesdStatusParser;

class SCOPY_JESDSTATUS_EXPORT JesdStatus_API : public ApiObject
{
	Q_OBJECT
public:
	explicit JesdStatus_API(JesdStatusPlugin *plugin);
	~JesdStatus_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Run control
	Q_INVOKABLE bool isRunning();
	Q_INVOKABLE void setRunning(bool running);

	// Device selection
	Q_INVOKABLE QStringList getDevices();
	Q_INVOKABLE QString getSelectedDevice();
	Q_INVOKABLE void setSelectedDevice(const QString &device);

	// Refresh
	Q_INVOKABLE void refresh();

	// Link status
	Q_INVOKABLE QString getLinkState();
	Q_INVOKABLE QString getLinkStatus();
	Q_INVOKABLE QString getMeasuredLinkClock();
	Q_INVOKABLE QString getReportedLinkClock();
	Q_INVOKABLE QString getMeasuredDeviceClock();
	Q_INVOKABLE QString getReportedDeviceClock();
	Q_INVOKABLE QString getDesiredDeviceClock();
	Q_INVOKABLE QString getLaneRate();
	Q_INVOKABLE QString getLaneRateDiv();
	Q_INVOKABLE QString getLmfcRate();
	Q_INVOKABLE QString getSysrefCaptured();
	Q_INVOKABLE QString getSysrefAlignmentError();
	Q_INVOKABLE QString getSyncState();

	// Lane status
	Q_INVOKABLE int getLaneCount();
	Q_INVOKABLE QString getEncoder();
	Q_INVOKABLE QString getLaneErrors(int lane);
	Q_INVOKABLE QString getLaneLatency(int lane);
	Q_INVOKABLE QString getLaneCgsState(int lane);
	Q_INVOKABLE QString getLaneInitFrameSync(int lane);
	Q_INVOKABLE QString getLaneInitLaneAlignSeq(int lane);
	Q_INVOKABLE QString getLaneExtMultiBlockAlignment(int lane);

private:
	JesdStatus *getJesdStatus();
	JesdStatusView *getCurrentView();
	JesdStatusParser *getCurrentParser();

	JesdStatusPlugin *m_plugin;
};
} // namespace scopy::jesdstatus
#endif // JESDSTATUS_API_H
