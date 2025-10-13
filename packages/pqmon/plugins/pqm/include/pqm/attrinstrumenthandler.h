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

#ifndef ATTRINSTRUMENTHANDLER_H
#define ATTRINSTRUMENTHANDLER_H

#include <QObject>
#include <harmonicsinstrument.h>
#include <rmsinstrument.h>
#include <pluginbase/resourcemanager.h>

namespace scopy::pqm {
class AttrInstrumentHandler : public QObject, public ResourceUser
{
	Q_OBJECT
public:
	AttrInstrumentHandler(QString m_uri, QObject *parent = nullptr);
	~AttrInstrumentHandler();

	void stop() override;

	void setHarmonicsInstrument(HarmonicsInstrument *newHarmonicsInstrument);

	void setRmsInstrument(RmsInstrument *newRmsInstrument);

public Q_SLOTS:
	void onToggle(bool en, const QString &tool);
	void concurrentEnable(QString pref, QVariant value);

private:
	bool isAnyToolRunning();
	void resourceManagerCheck(bool en);
	void resetValues();

	QMap<QString, bool> m_runningMap = {{RMS_TOOL, false}, {HARMONICS_TOOL, false}};
	bool m_concurrentAcq = false;
	bool m_resourceLock = false;
	QString m_uri;
	HarmonicsInstrument *m_harmonicsInstrument = nullptr;
	RmsInstrument *m_rmsInstrument = nullptr;
};
} // namespace scopy::pqm

#endif // ATTRINSTRUMENTHANDLER_H
