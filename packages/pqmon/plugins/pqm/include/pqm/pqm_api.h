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

#ifndef PQM_API_H
#define PQM_API_H

#include "scopy-pqm_export.h"
#include <pluginbase/apiobject.h>
#include <QMap>
#include <QString>

namespace scopy::pqm {

class PQMPlugin;
class RmsInstrument;
class HarmonicsInstrument;
class WaveformInstrument;
class SettingsInstrument;

class SCOPY_PQM_EXPORT PQM_API : public ApiObject
{
	Q_OBJECT
public:
	explicit PQM_API(PQMPlugin *pqmPlugin);
	~PQM_API();

	// Tool selection
	Q_INVOKABLE QStringList getTools();

	// RMS instrument
	Q_INVOKABLE bool isRmsRunning();
	Q_INVOKABLE void setRmsRunning(bool running);
	Q_INVOKABLE void rmsSingleShot();
	Q_INVOKABLE bool isRmsLoggingEnabled();
	Q_INVOKABLE void setRmsLoggingEnabled(bool enabled);
	Q_INVOKABLE QString getRmsLogPath();
	Q_INVOKABLE void setRmsLogPath(const QString &path);
	Q_INVOKABLE void resetRmsPqEvents();
	Q_INVOKABLE bool isRmsPqEvents();

	// Harmonics instrument
	Q_INVOKABLE bool isHarmonicsRunning();
	Q_INVOKABLE void setHarmonicsRunning(bool running);
	Q_INVOKABLE void harmonicsSingleShot();
	Q_INVOKABLE QString getHarmonicsActiveChannel();
	Q_INVOKABLE void setHarmonicsActiveChannel(const QString &channel);
	Q_INVOKABLE QString getHarmonicsType();
	Q_INVOKABLE void setHarmonicsType(const QString &type);
	Q_INVOKABLE bool isHarmonicsLoggingEnabled();
	Q_INVOKABLE void setHarmonicsLoggingEnabled(bool enabled);
	Q_INVOKABLE QString getHarmonicsLogPath();
	Q_INVOKABLE void setHarmonicsLogPath(const QString &path);
	Q_INVOKABLE void resetHarmonicsPqEvents();
	Q_INVOKABLE bool isHarmonicsPqEvents();

	// Waveform instrument
	Q_INVOKABLE bool isWaveformRunning();
	Q_INVOKABLE void setWaveformRunning(bool running);
	Q_INVOKABLE void waveformSingleShot();
	Q_INVOKABLE double getWaveformTimespan();
	Q_INVOKABLE void setWaveformTimespan(double value);
	Q_INVOKABLE bool isWaveformRollingMode();
	Q_INVOKABLE void setWaveformRollingMode(bool enabled);
	Q_INVOKABLE QString getWaveformTriggeredBy();
	Q_INVOKABLE void setWaveformTriggeredBy(const QString &channel);
	Q_INVOKABLE bool isWaveformLoggingEnabled();
	Q_INVOKABLE void setWaveformLoggingEnabled(bool enabled);
	Q_INVOKABLE QString getWaveformLogPath();
	Q_INVOKABLE void setWaveformLogPath(const QString &path);

	// Settings instrument
	Q_INVOKABLE void settingsRead();
	Q_INVOKABLE void settingsWrite();
	Q_INVOKABLE QString getSettingsAttributeValue(const QString &attrName);
	Q_INVOKABLE void setSettingsAttributeValue(const QString &attrName, const QString &value);

	// PQ Events trigger (for testing)
	Q_INVOKABLE bool triggerPqEvent(bool enable);

	// Get the actual log file path (used for test verification)
	Q_INVOKABLE QString getLogFilePath();

private:
	RmsInstrument *getRmsInstrument();
	HarmonicsInstrument *getHarmonicsInstrument();
	WaveformInstrument *getWaveformInstrument();
	SettingsInstrument *getSettingsInstrument();

	PQMPlugin *m_pqmPlugin;
};
} // namespace scopy::pqm
#endif // PQM_API_H
