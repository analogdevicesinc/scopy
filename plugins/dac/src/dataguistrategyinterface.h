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

#ifndef DATAGUISTRATEGYINTERFACE_H
#define DATAGUISTRATEGYINTERFACE_H

#include "scopy-dac_export.h"
#include "databufferrecipe.h"
#include <QWidget>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DataGuiStrategyInterface
{
public:
	virtual ~DataGuiStrategyInterface() = default;
	virtual QWidget *ui() = 0;
	virtual void init() = 0;

Q_SIGNALS:
	virtual void recipeUpdated(DataBufferRecipe) = 0;

protected:
	DataBufferRecipe m_recipe;
};
} // namespace dac
} // namespace scopy
Q_DECLARE_INTERFACE(scopy::dac::DataGuiStrategyInterface, "scopy::DataGuiStrategyInterface")
#endif // DATAGUISTRATEGYINTERFACE_H
