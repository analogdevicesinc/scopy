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

#ifndef IIOSCAN_H
#define IIOSCAN_H

#include "scopy-iioutil_export.h"
#include "iiocpp/iioresult.h"
#include <QObject>
#include <iio/iio.h>

#include "iiocpp/iioresult.h"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOScan : public QObject
{
	Q_OBJECT
	// FIXME: Do we actually need it to be a singleton, come back when signals are implemented?
protected:
	IIOScan(QObject *parent = nullptr);
	~IIOScan();

public:
	IIOScan(const IIOScan &) = delete;
	IIOScan &operator=(const IIOScan &) = delete;

	static IIOScan *GetInstance();

	// iio_scan needs to be set as a struct because there is a function called iio_scan, should there be one?
	static IIOResult<struct iio_scan *> scan(const iio_context_params *params, const char *backends);
	static void destroy(struct iio_scan *ctx);
	static size_t get_results_count(const struct iio_scan *ctx);
	static const char *get_description(const struct iio_scan *ctx, size_t idx);
	static const char *get_uri(const struct iio_scan *ctx, size_t idx);

	/* Top level functions, but are related to iio_scan */
	static unsigned int get_builtin_backends_count(void);
	static const char *get_builtin_backend(unsigned int idx);
	static bool has_backend(const iio_context_params *params, const char *backend);

private:
	static IIOScan *pinstance_;
};
} // namespace scopy

#endif // IIOSCAN_H
