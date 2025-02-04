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

#ifndef IIOOPERATIONINTERFACE_H
#define IIOOPERATIONINTERFACE_H

#include "scopy-iioutil_export.h"
#include <QtXml/qdom.h>
#include <qobject.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOOperationInterface
{
public:
	enum IIOOperationType
	{
		SCAN,
		CONTEXT,
		DEVICE,
		CHANNEL,
		ATTRIBUTE,
	};

	virtual ~IIOOperationInterface() = default;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::IIOOperationInterface, "scopy::IIOOperationInterface")
#endif // IIOOPERATIONINTERFACE_H
