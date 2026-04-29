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
 */

#ifndef AD6676_API_H
#define AD6676_API_H

#include "scopy-ad6676plugin_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad6676 {

class Ad6676Plugin;

class SCOPY_AD6676PLUGIN_EXPORT AD6676_API : public ApiObject
{
	Q_OBJECT
public:
	explicit AD6676_API(Ad6676Plugin *plugin);
	~AD6676_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// ADC Settings
	Q_INVOKABLE QString getAdcFrequency();
	Q_INVOKABLE void setAdcFrequency(const QString &val);

	// Bandwidth Settings
	Q_INVOKABLE QString getBandwidth();
	Q_INVOKABLE void setBandwidth(const QString &val);
	Q_INVOKABLE QString getBwMarginLow();
	Q_INVOKABLE void setBwMarginLow(const QString &val);
	Q_INVOKABLE QString getBwMarginHigh();
	Q_INVOKABLE void setBwMarginHigh(const QString &val);
	Q_INVOKABLE QString getBwMarginIf();
	Q_INVOKABLE void setBwMarginIf(const QString &val);

	// Receive Settings
	Q_INVOKABLE QString getIntermediateFrequency();
	Q_INVOKABLE void setIntermediateFrequency(const QString &val);
	Q_INVOKABLE QString getSamplingFrequency();
	Q_INVOKABLE QString getHardwareGain();
	Q_INVOKABLE void setHardwareGain(const QString &val);
	Q_INVOKABLE QString getScale();
	Q_INVOKABLE void setScale(const QString &val);

	// Shuffler Settings
	Q_INVOKABLE QString getShufflerControl();
	Q_INVOKABLE void setShufflerControl(const QString &val);
	Q_INVOKABLE QString getShufflerThresh();
	Q_INVOKABLE void setShufflerThresh(const QString &val);

	// Test Settings
	Q_INVOKABLE QString getTestMode();
	Q_INVOKABLE void setTestMode(const QString &val);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString chnKey(const QString &attr);
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Ad6676Plugin *m_plugin;
};

} // namespace scopy::ad6676

#endif // AD6676_API_H
