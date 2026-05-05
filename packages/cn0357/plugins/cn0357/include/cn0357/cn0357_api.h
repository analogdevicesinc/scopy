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
 */

#ifndef CN0357_API_H
#define CN0357_API_H

#include "scopy-cn0357_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::cn0357 {

class Cn0357Plugin;

class SCOPY_CN0357_EXPORT Cn0357_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Cn0357_API(Cn0357Plugin *plugin);
	~Cn0357_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Generic IIOWidget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);
	Q_INVOKABLE void refresh();

	// ADC settings (IIOWidget-backed)
	Q_INVOKABLE QString getSamplingFrequency();
	Q_INVOKABLE void setSamplingFrequency(const QString &value);
	Q_INVOKABLE QString getAdcMillivolts();
	Q_INVOKABLE QString getSupplyVoltage();

	// Feedback settings (UI-only)
	Q_INVOKABLE QString getFeedbackType();
	Q_INVOKABLE void setFeedbackType(const QString &value);
	Q_INVOKABLE QString getRdacValue();
	Q_INVOKABLE void setRdacValue(const QString &value);
	Q_INVOKABLE void programRheostat();
	Q_INVOKABLE QString getFixedResistor();
	Q_INVOKABLE void setFixedResistor(const QString &value);

	// System settings (UI-only)
	Q_INVOKABLE QString getSensorSensitivity();
	Q_INVOKABLE void setSensorSensitivity(const QString &value);

	// Computed values (read-only)
	Q_INVOKABLE QString getConcentrationPpm();
	Q_INVOKABLE QString getFeedbackResistance();
	Q_INVOKABLE QString getPpmPerMv();
	Q_INVOKABLE QString getMvPerPpm();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);
	class Cn0357Tool *tool();

	Cn0357Plugin *m_plugin;
};

} // namespace scopy::cn0357

#endif // CN0357_API_H
