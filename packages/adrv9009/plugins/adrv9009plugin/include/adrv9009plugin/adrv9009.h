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

#ifndef ADRV9009_H
#define ADRV9009_H

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <iio.h>

namespace scopy::adrv9009 {
class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009 : public QWidget
{
	Q_OBJECT
public:
	Adrv9009(iio_context *ctx, QWidget *parent = nullptr);
	~Adrv9009();

private:
	iio_context *m_ctx = nullptr;
	iio_device *m_iio_dev = nullptr;
};
} // namespace scopy::adrv9009
#endif // ADRV9009_H
