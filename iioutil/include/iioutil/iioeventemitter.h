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

#ifndef IIOEVENTEMITTER_H
#define IIOEVENTEMITTER_H

#include <qobject.h>
namespace scopy {
const int IIO_SUCCESS = 0;
const int IIO_ERROR = -1;
typedef enum
{
	SINGLE,
	STREAM
} IIOCallType;
class IIOEventEmitter
{
public:
	virtual void iioEvent(int retCode, scopy::IIOCallType type) = 0;
};
} // namespace scopy
Q_DECLARE_METATYPE(scopy::IIOCallType);
#endif // IIOEVENTEMITTER_H
