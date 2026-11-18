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

#ifndef ADALM2000LOGIC_API_H
#define ADALM2000LOGIC_API_H

#include "scopy-adalm2000_export.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {
class M2kLogicController;

// Trigger conditions are the combo indices: 0 none, 1 rising edge, 2 falling edge, 3 low,
// 4 high, 5 any edge. Each `decoders` entry is one root decoder as JSON:
// {"id": "uart", "channels": {"rx": 0}, "options": {"baudrate": "9600"}}.
class SCOPY_ADALM2000_EXPORT Adalm2000Logic_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE setRunning STORED false)
	Q_PROPERTY(double sample_rate READ sampleRate WRITE setSampleRate)
	Q_PROPERTY(int buffer_size READ bufferSize WRITE setBufferSize)
	Q_PROPERTY(int plot_window READ plotWindow WRITE setPlotWindow)
	Q_PROPERTY(int delay READ delay WRITE setDelay)
	Q_PROPERTY(QList<bool> enabled READ enabled WRITE setEnabled)
	Q_PROPERTY(QStringList names READ names WRITE setNames)
	Q_PROPERTY(bool trigger_enabled READ triggerEnabled WRITE setTriggerEnabled)
	Q_PROPERTY(QList<int> trigger_conditions READ triggerConditions WRITE setTriggerConditions)
	Q_PROPERTY(int trigger_logic READ triggerLogic WRITE setTriggerLogic)
	Q_PROPERTY(bool external_trigger READ externalTrigger WRITE setExternalTrigger)
	Q_PROPERTY(int external_source READ externalSource WRITE setExternalSource)
	Q_PROPERTY(int external_condition READ externalCondition WRITE setExternalCondition)
	Q_PROPERTY(QStringList decoders READ decoders WRITE setDecoders)

public:
	explicit Adalm2000Logic_API(M2kLogicController *tool);

	bool running() const;
	void setRunning(bool on);
	double sampleRate() const;
	void setSampleRate(double hz);
	int bufferSize() const;
	void setBufferSize(int n);
	int plotWindow() const;
	void setPlotWindow(int n);
	int delay() const;
	void setDelay(int samples);
	QList<bool> enabled() const;
	void setEnabled(const QList<bool> &on);
	QStringList names() const;
	void setNames(const QStringList &names);
	bool triggerEnabled() const;
	void setTriggerEnabled(bool on);
	QList<int> triggerConditions() const;
	void setTriggerConditions(const QList<int> &conditions);
	int triggerLogic() const;
	void setTriggerLogic(int idx);
	bool externalTrigger() const;
	void setExternalTrigger(bool on);
	int externalSource() const;
	void setExternalSource(int idx);
	int externalCondition() const;
	void setExternalCondition(int idx);
	QStringList decoders() const;
	void setDecoders(const QStringList &decoders);

	Q_INVOKABLE void single();
	Q_INVOKABLE QList<int> samples(int pin) const;
	Q_INVOKABLE bool exportData(const QString &path) const;
	Q_INVOKABLE QString addDecoder(const QString &decoderId);
	Q_INVOKABLE void removeDecoder(const QString &uid);
	Q_INVOKABLE bool setDecoderChannel(const QString &uid, const QString &role, int pin);
	Q_INVOKABLE bool setDecoderOption(const QString &uid, const QString &option, const QString &value);
	Q_INVOKABLE QVariantList annotations(const QString &uid, int stage = 0) const;

private:
	M2kLogicController *m_tool;
};
} // namespace scopy::adalm2000
#endif // ADALM2000LOGIC_API_H
