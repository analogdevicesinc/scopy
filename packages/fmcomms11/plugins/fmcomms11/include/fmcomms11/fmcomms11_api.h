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

#ifndef FMCOMMS11_API_H
#define FMCOMMS11_API_H

#include "scopy-fmcomms11_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::fmcomms11 {

class Fmcomms11Plugin;

class SCOPY_FMCOMMS11_EXPORT Fmcomms11_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Fmcomms11_API(Fmcomms11Plugin *plugin);
	~Fmcomms11_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// ADC attributes
	Q_INVOKABLE QString getAdcSamplingFrequency();
	Q_INVOKABLE QString getAdcScale();
	Q_INVOKABLE void setAdcScale(const QString &value);
	Q_INVOKABLE QString getAdcTestMode();
	Q_INVOKABLE void setAdcTestMode(const QString &value);

	// Input attenuator (HMC1119)
	Q_INVOKABLE QString getInputAttenuation();
	Q_INVOKABLE void setInputAttenuation(const QString &value);

	// DAC attributes
	Q_INVOKABLE QString getDacSamplingFrequency();
	Q_INVOKABLE QString getNcoFrequency();
	Q_INVOKABLE void setNcoFrequency(const QString &value);
	Q_INVOKABLE QString isFir85Enabled();
	Q_INVOKABLE void setFir85Enabled(const QString &value);

	// Output amplifier (ADL5240)
	Q_INVOKABLE QString getOutputGain();
	Q_INVOKABLE void setOutputGain(const QString &value);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);
	QString ncoKey();

	Fmcomms11Plugin *m_plugin;
};

} // namespace scopy::fmcomms11

#endif // FMCOMMS11_API_H
