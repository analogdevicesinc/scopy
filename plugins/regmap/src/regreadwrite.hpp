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

#ifndef REGREADWRITE_HPP
#define REGREADWRITE_HPP

#include "scopy-regmap_export.h"

#include <iio.h>

#include <QObject>

// TODO set the right buf size
#define BUF_SIZE 16384

namespace scopy::regmap {
class SCOPY_REGMAP_EXPORT RegReadWrite : public QObject
{
	Q_OBJECT

public:
	explicit RegReadWrite(struct iio_device *dev, QObject *parent = nullptr);

	~RegReadWrite();

	void read(uint32_t address);
	void write(uint32_t address, uint32_t val);

Q_SIGNALS:
	void readDone(uint32_t address, uint32_t value);
	void readError(const char *err);
	void writeError(const char *err);
	void writeSuccess(uint32_t address);

private:
	struct iio_device *dev;
};
} // namespace scopy::regmap
#endif // REGREADWRITE_HPP
