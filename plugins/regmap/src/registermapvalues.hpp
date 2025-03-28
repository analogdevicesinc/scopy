/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef REGISTERMAPVALUES_HPP
#define REGISTERMAPVALUES_HPP

#include "scopy-regmap_export.h"

#include <QMap>
#include <QObject>

namespace scopy::regmap {
class IRegisterWriteStrategy;
class IRegisterReadStrategy;
class RegReadWrite;

class SCOPY_REGMAP_EXPORT RegisterMapValues : public QObject
{
	Q_OBJECT
public:
	explicit RegisterMapValues(QObject *parent = nullptr);
	~RegisterMapValues();

	QMap<uint32_t, uint32_t> *registerReadValues;

	void readDone(uint32_t address, uint32_t value);
	uint32_t getValueOfRegister(uint32_t address);
	void setReadStrategy(IRegisterReadStrategy *readStrategy);
	void setWriteStrategy(IRegisterWriteStrategy *writeStrategy);
	void registerDump(QString path);

	IRegisterReadStrategy *getReadStrategy() const;

	IRegisterWriteStrategy *getWriteStrategy() const;

	QMap<uint32_t, uint32_t> *getRegisterReadValues() const;
	bool hasValue(uint32_t address);

Q_SIGNALS:
	void registerValueChanged(uint32_t address, uint32_t value);
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);

private:
	IRegisterReadStrategy *readStrategy = nullptr;
	IRegisterWriteStrategy *writeStrategy = nullptr;
	QMetaObject::Connection m_readConnection;
	QMetaObject::Connection writeConnection;
};
} // namespace scopy::regmap
#endif // REGISTERMAPVALUES_HPP
