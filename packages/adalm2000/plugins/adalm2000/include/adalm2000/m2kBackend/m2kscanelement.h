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
#pragma once

#include "component/scanelement.h"

namespace scopy::adalm2000 {

// Channel selection for an M2k stream. The enable state is local: the M2k streams apply
// StreamConfig::enabledChannels when they configure (analog out enables the selected DACs),
// so the element only tracks the selection.
class M2kScanElement : public component::ScanElement
{
	Q_OBJECT
public:
	M2kScanElement(long index, bool isOutput, QObject *parent);

	bool isEnabled() const override { return m_enabled; }
	QCoro::Task<CommandResponse<void>> enableAsync(bool en) override;

private:
	bool m_enabled = true;
};

} // namespace scopy::adalm2000
