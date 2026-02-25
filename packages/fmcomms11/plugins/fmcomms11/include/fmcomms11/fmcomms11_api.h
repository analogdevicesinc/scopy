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

class SCOPY_FMCOMMS11_EXPORT FMCOMMS11_API : public ApiObject
{
	Q_OBJECT
public:
	explicit FMCOMMS11_API(Fmcomms11Plugin *plugin);
	~FMCOMMS11_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// RX: ADC (axi-ad9625-hpc)
	Q_INVOKABLE QString getAdcSamplingFrequency();
	Q_INVOKABLE QString getAdcScale();
	Q_INVOKABLE QString getTestMode();
	Q_INVOKABLE void setTestMode(const QString &value);

	// RX: VGA (adl5240)
	Q_INVOKABLE QString getVgaGain();
	Q_INVOKABLE void setVgaGain(const QString &value);

	// TX: DAC (axi-ad9162-hpc)
	Q_INVOKABLE QString getDacSamplingFrequency();
	Q_INVOKABLE QString getFir85Enable(const QString &channel);
	Q_INVOKABLE void setFir85Enable(const QString &channel, const QString &value);

	// TX: Attenuator (hmc1119)
	Q_INVOKABLE QString getAttenuation();
	Q_INVOKABLE void setAttenuation(const QString &value);

	// TX: DDS tones (altvoltage0-3 on axi-ad9162-hpc)
	Q_INVOKABLE QString getDdsFrequency(int tone);
	Q_INVOKABLE void setDdsFrequency(int tone, const QString &value);
	Q_INVOKABLE QString getDdsPhase(int tone);
	Q_INVOKABLE void setDdsPhase(int tone, const QString &value);
	Q_INVOKABLE QString getDdsScale(int tone);
	Q_INVOKABLE void setDdsScale(int tone, const QString &value);
	Q_INVOKABLE QString getDdsEnable(int tone);
	Q_INVOKABLE void setDdsEnable(int tone, const QString &value);

	// TX: NCO (altvoltage4 on axi-ad9162-hpc)
	Q_INVOKABLE QString getNcoFrequency();
	Q_INVOKABLE void setNcoFrequency(const QString &value);

	// LO/PLL (adf4355)
	Q_INVOKABLE QString getLoFrequency(int channel);
	Q_INVOKABLE void setLoFrequency(int channel, const QString &value);
	Q_INVOKABLE QString getLoPowerdown(int channel);
	Q_INVOKABLE void setLoPowerdown(int channel, const QString &value);
	Q_INVOKABLE QString getLoRefinFrequency(int channel);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Fmcomms11Plugin *m_plugin;
};

} // namespace scopy::fmcomms11

#endif // FMCOMMS11_API_H
