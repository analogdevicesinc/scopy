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

#ifndef COMMANDFORMAT_H
#define COMMANDFORMAT_H

#include <QString>
#include <QVariantMap>

#include "inputconfig.h"
#include "outputconfig.h"

namespace scopy::extprocplugin {
class CommandFormat
{
public:
	virtual ~CommandFormat() {}

	virtual QString sendSetInputConfig(const InputConfig &config) = 0;
	virtual QString sendSetAnalysisConfig(const QString &type, const QVariantMap &config) = 0;
	virtual QString sendSetOutputConfig(const OutputConfig &config) = 0;
	virtual QString sendRun() = 0;
	virtual QString sendGetAnalysisTypes() = 0;
	virtual QString sendGetAnalysisInfo(const QString &type) = 0;
	virtual QVariantMap parseResponse(const QString &data) = 0;
	virtual QString getProtocolName() = 0;
};

} // namespace scopy::extprocplugin

#endif // COMMANDFORMAT_H
