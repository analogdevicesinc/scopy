/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef DCDEBUGGER_API_H
#define DCDEBUGGER_API_H

#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QPointer>
#include <QStringList>
#include <QVariantMap>

#include <pluginbase/apiobject.h>

class QModelIndex;

namespace scopy::dcdebugger {
class AcquisitionView;
class CommandLogView;
class DCDebuggerPlugin;
class DCDebuggerTool;
class GeneratorView;
class MonitorView;
class NodePanel;
class PingView;
class RegisterView;

// JS automation of the DC Debugger, registered as "dcdebugger". One object for every connected
// device: the calls go to the current one (setDevice()), by default the last connected.
//
// Nodes are named by their origin path, the one the command log shows ("ad9361-phy/voltage0/raw"),
// optionally with the backend id ("iio", "capi", ...) when several views of the device have it.
// The instrument calls (acquisition, generator, monitor, registers, ping) act on the view of the
// selected node (selectNode()) and drive its UI, so the screen follows the script. Calls that talk
// to the device wait for the result, up to timeoutMs.
class SCOPY_DCDEBUGGER_EXPORT DCDebugger_API : public ApiObject
{
	Q_OBJECT

public:
	static constexpr int DEFAULT_TIMEOUT_MS = 5000;

	// One API for every plugin instance: created with the first, deleted with the last.
	static DCDebugger_API *attach(DCDebuggerPlugin *plugin);
	static void detach(DCDebuggerPlugin *plugin);

	// Devices
	Q_INVOKABLE QStringList getDevices() const;
	Q_INVOKABLE QString getDevice() const;
	Q_INVOKABLE bool setDevice(const QString &uri);
	Q_INVOKABLE QStringList getTools() const;

	// Tree
	Q_INVOKABLE QStringList getBackends() const;
	Q_INVOKABLE QStringList getNodes(const QString &backend = {}) const;
	Q_INVOKABLE bool selectNode(const QString &path, const QString &backend = {});
	Q_INVOKABLE QString getSelectedNode() const;
	// Titles of the selected node's views: "Acquisition", "Generator", "Monitor", "Registers", "Ping".
	Q_INVOKABLE QStringList getViews() const;
	Q_INVOKABLE void setFilter(const QString &text);
	Q_INVOKABLE QString getFilter() const;
	Q_INVOKABLE void refresh();
	Q_INVOKABLE void rebuild();

	// Attributes
	Q_INVOKABLE QStringList getAttributes(const QString &path, const QString &backend = {}) const;
	Q_INVOKABLE QString readAttribute(const QString &path, const QString &backend = {},
					  int timeoutMs = DEFAULT_TIMEOUT_MS);
	Q_INVOKABLE bool writeAttribute(const QString &path, const QString &value, const QString &backend = {},
					int timeoutMs = DEFAULT_TIMEOUT_MS);
	Q_INVOKABLE QString getCachedValue(const QString &path, const QString &backend = {}) const;
	Q_INVOKABLE QStringList getAttributeOptions(const QString &path, const QString &backend = {}) const;

	// Acquisition (selected input stream)
	Q_INVOKABLE bool startAcquisition();
	Q_INVOKABLE bool stopAcquisition();
	Q_INVOKABLE bool isAcquisitionRunning() const;
	// One buffer; true once it arrived.
	Q_INVOKABLE bool singleAcquisition(int timeoutMs = DEFAULT_TIMEOUT_MS);
	Q_INVOKABLE int getAcquisitionSamples() const;
	Q_INVOKABLE bool setAcquisitionSamples(int samples);
	// "Time", "Spectrum" or "XY".
	Q_INVOKABLE QString getAcquisitionMode() const;
	Q_INVOKABLE bool setAcquisitionMode(const QString &mode);
	Q_INVOKABLE QStringList getAcquisitionChannels() const;
	Q_INVOKABLE bool setAcquisitionChannelEnabled(const QString &channel, bool enabled);
	// The measurements table as JSON: [{"Channel": ..., "Min": ..., ...}, ...].
	Q_INVOKABLE QString getMeasurements() const;
	Q_INVOKABLE bool exportAcquisitionCsv(const QString &path) const;
	// The status line of the view (state, or the last error).
	Q_INVOKABLE QString getAcquisitionStatus() const;

	// Generator (selected output stream)
	Q_INVOKABLE bool startGenerator();
	Q_INVOKABLE bool stopGenerator();
	Q_INVOKABLE bool isGeneratorRunning() const;
	Q_INVOKABLE QStringList getGeneratorChannels() const;
	Q_INVOKABLE QStringList getWaveforms() const;
	Q_INVOKABLE bool setGeneratorChannelEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE bool setGeneratorTone(const QString &channel, const QString &waveform, double frequency,
					  double amplitude, double offset = 0, double phase = 0);
	Q_INVOKABLE bool setGeneratorSamples(int samples);
	Q_INVOKABLE bool setGeneratorCyclic(bool cyclic);
	Q_INVOKABLE QString getGeneratorStatus() const;

	// Monitor (selected context / device)
	Q_INVOKABLE bool startMonitor();
	Q_INVOKABLE bool stopMonitor();
	Q_INVOKABLE bool isMonitorRunning() const;
	Q_INVOKABLE bool clearMonitor();
	Q_INVOKABLE QStringList getMonitorChannels() const;
	Q_INVOKABLE bool setMonitorChannelEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE bool setMonitorPeriod(double seconds);
	// Per enabled channel: {"value", "min", "max", "mean", "count", "error"}, as JSON.
	Q_INVOKABLE QString getMonitorValues() const;

	// Registers (selected context / device); addresses and values in hex ("0x1f" or "1f")
	Q_INVOKABLE QStringList getRegisterTargets() const;
	Q_INVOKABLE bool setRegisterTarget(int index);
	Q_INVOKABLE QString readRegister(const QString &address, int timeoutMs = DEFAULT_TIMEOUT_MS);
	Q_INVOKABLE bool writeRegister(const QString &address, const QString &value,
				       int timeoutMs = DEFAULT_TIMEOUT_MS);

	// Ping (selected ping node)
	Q_INVOKABLE bool ping(int timeoutMs = DEFAULT_TIMEOUT_MS);
	Q_INVOKABLE bool setPingMonitoring(bool enabled);

	// Command log
	Q_INVOKABLE bool setLogEnabled(bool enabled);
	Q_INVOKABLE bool isLogEnabled() const;
	Q_INVOKABLE void clearLog();
	Q_INVOKABLE int getLogEntryCount() const;
	Q_INVOKABLE QString getLogCsv() const;
	Q_INVOKABLE bool exportLogCsv(const QString &path) const;
	Q_INVOKABLE void setLogFilter(const QString &component, const QString &library = {},
				      const QString &function = {}, bool errorsOnly = false);
	// Code from the selected log rows (every visible one when none is).
	Q_INVOKABLE QStringList getCodeLanguages();
	Q_INVOKABLE QString generateCode(const QString &language = {});
	Q_INVOKABLE bool saveCode(const QString &path, const QString &language = {});

private:
	DCDebugger_API();

	DCDebuggerTool *tool() const;
	NodePanel *panel() const;
	template <typename View>
	View *view() const;
	QModelIndex find(const QString &path, const QString &backend) const;
	QObject *node(const QString &path, const QString &backend) const;
	CommandLogView *log() const;

	static DCDebugger_API *s_api;
	QList<QPointer<DCDebuggerPlugin>> m_plugins;
	QPointer<DCDebuggerPlugin> m_current;
};

} // namespace scopy::dcdebugger

#endif // DCDEBUGGER_API_H
