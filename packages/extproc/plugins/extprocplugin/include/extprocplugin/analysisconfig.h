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

#ifndef ANALYSISCONFIG_H
#define ANALYSISCONFIG_H

#include <QVariantMap>
#include "outputinfo.h"

namespace scopy::extprocplugin {

class AnalysisConfig
{
public:
	AnalysisConfig();
	~AnalysisConfig();

	bool isValid() const;
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	QString analysisType() const;
	void setAnalysisType(const QString &type);

	OutputInfo outputInfo() const;
	void setOutputInfo(const OutputInfo &info);

	QVariantMap params() const;
	void setParams(const QVariantMap &params);

private:
	QString m_analysisType;
	OutputInfo m_outInfo;
	QVariantMap m_params;
};

} // namespace scopy::extprocplugin

#endif // ANALYSISCONFIG_H
