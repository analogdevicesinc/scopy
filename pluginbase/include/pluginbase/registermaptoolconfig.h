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

#ifndef REGISTERMAPTOOLCONFIG_H
#define REGISTERMAPTOOLCONFIG_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include <cstdint>
#include <optional>

namespace scopy {

/**
 * @brief One register map configuration entry, as plain data.
 * Each entry configures one device of the register map tool.
 */
struct RegisterMapToolConfig
{
	enum Access
	{
		IIO,  ///< IIO read/write, as today
		File, ///< CSV read/write from filePath
		None  ///< template only, no read/write
	};

	QString target;	      ///< the device name in the register map
	QString templatePath; ///< register XML path, empty for no template
	Access access = IIO;
	QString filePath;		      ///< CSV path, used when access is File
	std::optional<int> bitsPerRow;	      ///< empty: the value from regmap-config.json
	std::optional<uint32_t> addressSpace; ///< empty: the value from regmap-config.json
	QString provider;		      ///< name of the plugin that answered, filled in by the device

	// Access as text, for JSON data ("iio", "file", "none")
	static QString accessToString(Access access)
	{
		switch(access) {
		case File:
			return "file";
		case None:
			return "none";
		default:
			return "iio";
		}
	}

	// ok is false for an unknown text, the result is then IIO
	static Access accessFromString(const QString &text, bool *ok = nullptr)
	{
		const QString value = text.toLower();
		if(ok) {
			*ok = true;
		}
		if(value == "file") {
			return File;
		}
		if(value == "none") {
			return None;
		}
		if(value != "iio" && ok) {
			*ok = false;
		}
		return IIO;
	}
};

/**
 * @brief Implemented by device plugins that configure the register map tool of the same device.
 * The answer must not depend on the provider's own onConnect().
 * Other tools get their own config struct and provider interface when they need one.
 */
class IRegisterMapConfigProvider
{
public:
	virtual ~IRegisterMapConfigProvider() = default;
	virtual QList<RegisterMapToolConfig> registerMapConfiguration(const QVariantMap &query = {}) = 0;
};

// MessageBroker topic: a provider asks the register map to pull its configuration again.
// The message is the device param.
inline const QString REGISTER_MAP_CONFIGURE_TOPIC = "regmap/configure";

} // namespace scopy

Q_DECLARE_INTERFACE(scopy::IRegisterMapConfigProvider, "org.adi.Scopy.IRegisterMapConfigProvider/1.0")

#endif // REGISTERMAPTOOLCONFIG_H
