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
#include "m2kscanelement.h"

#include <QUuid>

using namespace scopy;
using namespace scopy::adalm2000;

M2kScanElement::M2kScanElement(long index, bool isOutput, QObject *parent)
	: ScanElement(parent)
{
	setIndex(index);
	// Matches the id of the Channel component of the same direction.
	setId(QStringLiteral("voltage%1").arg(index));
	setIsOutput(isOutput);
}

QCoro::Task<CommandResponse<void>> M2kScanElement::enableAsync(bool en)
{
	if(m_enabled != en) {
		m_enabled = en;
		Q_EMIT enabledChanged(en);
	}
	co_return CommandResponse<void>(QUuid());
}
