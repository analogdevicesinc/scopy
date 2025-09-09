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

#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QVariantMap>

namespace scopy::extprocplugin {

class OutputConfig
{
public:
	OutputConfig();
	~OutputConfig();

	bool isValid() const;
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	QString outputFile() const;
	void setOutputFile(const QString &file);

	QString outputFileFormat() const;
	void setOutputFileFormat(const QString &format);

	QStringList enabledAnalysis() const;
	void setEnabledAnalysis(const QStringList &list);

private:
	QString m_outputFile;
	QString m_outputFileFormat;
	QStringList m_enabledAnalysis;
};
} // namespace scopy::extprocplugin

#endif // OUTPUTCONFIG_H
