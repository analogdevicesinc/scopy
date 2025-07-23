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

#ifndef OUTPUTINFO_H
#define OUTPUTINFO_H

#include <QVariantMap>

namespace scopy::extprocplugin {

class OutputInfo
{
public:
	OutputInfo();

	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	int channelCount() const;
	void setChannelCount(int count);

	QStringList channelNames() const;
	void setChannelNames(const QStringList &names);

	QStringList channelFormat() const;
	void setChannelFormat(const QStringList &format);

	bool isValid();

private:
	int m_channelCount;
	QStringList m_channelNames;
	QStringList m_channelFormat;
};

} // namespace scopy::extprocplugin

#endif // OUTPUTINFO_H
