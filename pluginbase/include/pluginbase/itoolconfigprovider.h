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

#ifndef ITOOLCONFIGPROVIDER_H
#define ITOOLCONFIGPROVIDER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace scopy {

/**
 * @brief One configuration entry for a tool, as plain data.
 * The tool decides what the properties mean (e.g. regmap: template, access, file).
 */
struct ToolConfiguration
{
	QString toolId;		///< tool the entry is for, e.g. "regmap"
	QString target;		///< what the entry configures inside the tool, e.g. a device name
	QVariantMap properties; ///< tool specific settings
	QString provider;	///< name of the plugin that answered, filled in by the device
};

/**
 * @brief Implemented by device plugins that configure other tools of the same device.
 * The answer must not depend on the provider's own onConnect().
 */
class IToolConfigProvider
{
public:
	virtual ~IToolConfigProvider() = default;
	virtual QList<ToolConfiguration> toolConfiguration(const QString &toolId, const QVariantMap &query = {}) = 0;
};

} // namespace scopy

Q_DECLARE_INTERFACE(scopy::IToolConfigProvider, "org.adi.Scopy.IToolConfigProvider/1.0")

#endif // ITOOLCONFIGPROVIDER_H
