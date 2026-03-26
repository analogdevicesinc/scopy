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

#ifndef CN0511_API_H
#define CN0511_API_H

#include "scopy-cn0511_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::cn0511 {

class Cn0511Plugin;
class CN0511;

class SCOPY_CN0511_EXPORT Cn0511_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Cn0511_API(Cn0511Plugin *plugin);
	~Cn0511_API();

	Q_INVOKABLE QStringList getTools();

	Q_INVOKABLE QString getFrequency();
	Q_INVOKABLE void setFrequency(const QString &val);

	Q_INVOKABLE QString getAmplitude();
	Q_INVOKABLE void setAmplitude(const QString &val);

	Q_INVOKABLE void applyCalibration();

	Q_INVOKABLE QString isDacAmpEnabled();
	Q_INVOKABLE void setDacAmpEnabled(const QString &val);

	Q_INVOKABLE void refresh();

private:
	CN0511 *getTool();
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Cn0511Plugin *m_plugin;
};

} // namespace scopy::cn0511
#endif // CN0511_API_H
